#define GLEW_STATIC
#define GLW_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL



#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image.h>
#include <stb_image_write.h>

#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\carousel\carousel.h"
#include "..\common\carousel\carousel_to_renderable.h"


#include "..\common\carousel\carousel_loader.h"

#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>

#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"
#include "..\common\texture.h"

#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"

#include "carousel_extra.h"

std::string shaders_path("shaders/");
std::string assets_path("assets/");
std::string textures_path = assets_path + "textures/";
std::string models_path = assets_path + "models/";

int window_width = 1440;
int window_height = 900;

#define COLOR_RED    glm::vec3(1.f,0.f,0.f)
#define COLOR_GREEN  glm::vec3(0.f,1.f,0.f)
#define COLOR_BLUE   glm::vec3(0.f,0.f,1.f)
#define COLOR_BLACK  glm::vec3(0.f,0.f,0.f)
#define COLOR_WHITE  glm::vec3(1.f,1.f,1.f)
#define COLOR_YELLOW glm::vec3(1.f,1.f,0.f)

//textures and shading
typedef enum shadingMode {
	SHADING_TEXTURED_FLAT,     // 0
	SHADING_MONOCHROME_FLAT,   // 1
	SHADING_TEXTURED_PHONG,    // 2
	SHADING_MONOCHROME_PHONG   // 3
} shadingMode_t;

typedef enum textureSlot {
	TEXTURE_DEFAULT,			//0
	TEXTURE_GRASS,				//1
	TEXTURE_TRACK,				//2 //todo changed 
	TEXTURE_DIFFUSE,			//3
	TEXTURE_SHADOWMAP_SUN,		//4	
	TEXTURE_SHADOWMAP_LAMPS,	//5
	TEXTURE_SHADOWMAP_CARS		//6
} textureSlot_t;

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

matrix_stack stack;
float scaling_factor = 1.0;

//load functions
texture texture_grass, texture_track;
//Using MipMap_linear for minification
void load_textures() { 
	texture_grass.load(textures_path + "grass.png", TEXTURE_GRASS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	texture_track.load(textures_path + "track.png", TEXTURE_TRACK);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//define bounding boxes and renderable for all loaded models
box3 bbox_car, bbox_camera, bbox_lamp, bbox_tree;
std::vector <renderable> model_car, model_camera, model_lamp, model_tree; 
void load_models() { //todo change file names
	gltf_loader gltfLoader;
	gltfLoader.load_to_renderable(models_path + "car1.glb", model_car, bbox_car);
	gltfLoader.load_to_renderable(models_path + "camera4.glb", model_camera, bbox_camera);
	gltfLoader.load_to_renderable(models_path + "lamp2.glb", model_lamp, bbox_lamp);
	gltfLoader.load_to_renderable(models_path + "styl-pine.glb", model_tree, bbox_tree);
}

//load shaders
shader shader_basic, shader_world, shader_depth, shader_fsq;
void load_shaders() {
	shader_basic.create_program((shaders_path + "basic.vert").c_str(), (shaders_path + "basic.frag").c_str());
	shader_world.create_program((shaders_path + "world.vert").c_str(), (shaders_path + "world.frag").c_str());
	shader_depth.create_program((shaders_path + "depth.vert").c_str(), (shaders_path + "depth.frag").c_str());
	shader_fsq.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "fsq.frag").c_str());
}

//fun to draw loaded models
void drawLoadedModel(matrix_stack stack, std::vector<renderable> obj, box3 bbox, shader s) {
	float scale = 1.f / bbox.diagonal(); 
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale)));
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center())));

	// render each renderable
	for (unsigned int i = 0; i < obj.size(); ++i) {
		obj[i].bind();
		stack.push();
		// each object had its own transformation that was read in the gltf file
		stack.mult(obj[i].transform);

		if (obj[i].mater.base_color_texture != -1) {
			glActiveTexture(GL_TEXTURE0 + TEXTURE_DIFFUSE);
			glBindTexture(GL_TEXTURE_2D, obj[i].mater.base_color_texture);
		}

		glUniform1i(s["uColorImage"], TEXTURE_DIFFUSE);
		glUniformMatrix4fv(s["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
		stack.pop();
	}
}

// Draw functions

race r; 
renderable fram;

void draw_frame(glm::mat4 F) {
	glUseProgram(shader_basic.program); 
	glUniformMatrix4fv(shader_basic["uModel"], 1, GL_FALSE, &F[0][0]);
	glUniform3f(shader_basic["uColor"], -1.f, 0.6f, 0.f);
	fram.bind();
	glDrawArrays(GL_LINES, 0, 6);
	glUseProgram(0); 
}

//Draw line to indicate sun direction
void draw_sunDirection(glm::vec3 sundir) {
	glUseProgram(shader_basic.program);
	glUniform3f(shader_basic.program["uColor"], 1.f, 1.f, 1.f);
	glUniformMatrix4fv(shader_basic["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glBegin(GL_LINES); 
	glVertex3f(0, 0, 0);
	glVertex3f(sundir.x, sundir.y, sundir.z);
	glEnd(); 
	glUseProgram(0);
}

//Draw terrain 
renderable r_terrain;
void draw_terrain(shader sh, matrix_stack stack) {
	glUseProgram(sh.program);
	r_terrain.bind();

	glActiveTexture(GL_TEXTURE0 + TEXTURE_GRASS);
	glBindTexture(GL_TEXTURE_2D, texture_grass.id);
	glUniform1i(sh["uMode"], SHADING_TEXTURED_PHONG);
	glUniform1i(sh["uColorImage"], TEXTURE_GRASS);
	glUniform1f(sh["uShininess"], 25.f);
	glUniform1f(sh["uDiffuse"], 0.9f);
	glUniform1f(sh["uSpecular"], 0.1f);
	glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
	glUseProgram(0);
}

//Draw Track
renderable r_track;
void draw_track(shader sh, matrix_stack stack) {
	glUseProgram(sh.program);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.1, 0.1);
	r_track.bind();

	// we bump the track upwards to prevent it from falling under the terrain
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.15f, 0.f)));

	glActiveTexture(GL_TEXTURE0 + TEXTURE_TRACK);
	glBindTexture(GL_TEXTURE_2D, texture_track.id);
	glUniform1i(sh["uMode"], SHADING_TEXTURED_PHONG);
	glUniform1f(sh["uShininess"], 50.f);
	glUniform1f(sh["uDiffuse"], 0.8f);
	glUniform1f(sh["uSpecular"], 0.5f);
	glUniform1i(sh["uColorImage"], TEXTURE_TRACK);
	glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_track().mode, r_track().count, r_track().itype, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);

	stack.pop();
	glUseProgram(0);
}

void draw_cars(shader sh, matrix_stack stack) {
	glUseProgram(sh.program);
	for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
		stack.push();
		stack.mult(r.cars()[ic].frame);
		stack.mult(glm::scale(glm::mat4(1.), glm::vec3(3.5f)));
		stack.mult(glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));  // make the car face the direction it's going
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.15f, 0.f))); // bump it upwards so the wheels don't clip into the terrain

		glUniform1i(sh["uMode"], SHADING_TEXTURED_PHONG);
		glUniform1f(sh["uShininess"], 75.f);
		glUniform1f(sh["uDiffuse"], 0.7f);
		glUniform1f(sh["uSpecular"], 0.8f);
		drawLoadedModel(stack, model_car, bbox_car, sh);
		stack.pop();
	}
	glUseProgram(0);
}

std::vector<bool> draw_cameraman; 
void draw_cameramen(shader sh, matrix_stack stack) {
	// draw each cameraman
	glUseProgram(sh.program);
	for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
		if (!draw_cameraman[ic])
			continue;
		stack.push();
		stack.mult(r.cameramen()[ic].frame);
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(2.5f)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.25f, 0.f)));
		stack.mult(glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)));

		glUniform1i(sh["uMode"], SHADING_MONOCHROME_PHONG);
		glUniform3f(sh["uColor"], 0.2f, 0.2f, 0.2f);
		glUniform1f(sh["uShininess"], 50.f);
		glUniform1f(sh["uDiffuse"], 0.9f);
		glUniform1f(sh["uSpecular"], 0.6f);

		drawLoadedModel(stack, model_camera, bbox_camera, sh);
		stack.pop();
	}
	glUseProgram(0);
}

//draw lamps
renderable r_sphere;
std::vector<glm::mat4> lampT;
void draw_lamps(shader sh, matrix_stack stack) {
	glUseProgram(sh.program);
	glUniform1i(sh["uMode"], SHADING_TEXTURED_PHONG);
	glUniform1f(sh["uShininess"], 75.f);
	glUniform1f(sh["uDiffuse"], 0.7f);
	glUniform1f(sh["uSpecular"], 0.7f);
	for (unsigned int i = 0; i < lampT.size(); ++i) {
		stack.push();
		stack.load(lampT[i]);

		drawLoadedModel(stack, model_lamp, bbox_lamp, sh);

		stack.pop();
	}
	glUseProgram(0);
}


//draw trees
std::vector<glm::mat4> treeT;
void draw_trees(shader sh, matrix_stack stack) {
	glUseProgram(sh.program);
	glDisable(GL_CULL_FACE);

	glUniform1i(sh["uMode"], SHADING_TEXTURED_PHONG);
	glUniform1f(sh["uShininess"], 25.f);
	glUniform1f(sh["uDiffuse"], 1.f);
	glUniform1f(sh["uSpecular"], 0.1f);
	for (unsigned int i = 0; i < treeT.size(); i++) {
		stack.push();
		stack.load(treeT[i]);

		drawLoadedModel(stack, model_tree, bbox_tree, sh);

		stack.pop();
	}
	glUseProgram(0);
}

//Draw Textures
renderable r_quad;
void draw_texture(GLint tex_id, unsigned int texture_slot) {
	GLint at;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
	glUseProgram(shader_fsq.program);

	glActiveTexture(GL_TEXTURE0 + texture_slot);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(shader_fsq["uTexture"], texture_slot);
	r_quad.bind();
	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glActiveTexture(at);
}

renderable r_cube;
// draws the frustum represented by the given projection matrix
void draw_frustum(glm::mat4 projMatrix, glm::vec3 color) {
	glUseProgram(shader_basic.program);

	r_cube.bind();
	glUniform3f(shader_basic["uColor"], color.r, color.g, color.b);
	glUniformMatrix4fv(shader_basic["uModel"], 1, GL_FALSE, &glm::inverse(projMatrix)[0][0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[1].ind);
	glDrawElements(r_cube.elements[1].mode, r_cube.elements[1].count, r_cube.elements[1].itype, 0);

	glUseProgram(0);
}

// draws a box3
void draw_bbox(box3 bbox, glm::vec3 color) {
	glUseProgram(shader_basic.program);

	r_cube.bind();
	glm::mat4 T = glm::translate(glm::mat4(1.f), bbox.center());
	T = glm::scale(T, glm::abs(bbox.max - bbox.min) / 2.f);
	glUniform3f(shader_basic["uColor"], color.r, color.g, color.b);
	glUniformMatrix4fv(shader_basic["uModel"], 1, GL_FALSE, &T[0][0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[1].ind);
	glDrawElements(r_cube.elements[1].mode, r_cube.elements[1].count, r_cube.elements[1].itype, 0);

	glUseProgram(0);
}

void draw_scene(matrix_stack stack, bool depthOnly) {
	shader sh;
	if (depthOnly) {
		sh = shader_depth;
	}
	else {
		//draw_frame(glm::mat4(1.f));
		sh = shader_world;
	}

	// draw terrain and track
	if (depthOnly)
		glDisable(GL_CULL_FACE);   // terrain and track are not watertight
	else {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glFrontFace(GL_CW);
	draw_terrain(sh, stack);
	check_gl_errors(__LINE__, __FILE__);
	draw_track(sh, stack);
	check_gl_errors(__LINE__, __FILE__);

	// the following models have opposite polygon handedness
	glFrontFace(GL_CCW);
	draw_cars(sh, stack);

	// in the depth pass, cull the front face from the following watertight models
	if (depthOnly) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}

	//check_gl_errors(__LINE__, __FILE__);
	draw_cameramen(sh, stack);
	//check_gl_errors(__LINE__, __FILE__);
	draw_trees(sh, stack);
	//check_gl_errors(__LINE__, __FILE__);
	draw_lamps(sh, stack);
	check_gl_errors(__LINE__, __FILE__);
}




/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		tb[curr_tb].mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb[curr_tb].mouse_release();
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS)
		curr_tb = 1 - curr_tb;
}



int main(int argc, char** argv)
{
	/*
	race r;


	*/

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(window_width, window_height, "CarOusel", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* declare the callback functions on mouse events */
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();
	glEnable(GL_MULTISAMPLE);
	printout_opengl_glsl_info();

	carousel_loader::load("small_test.svg", "terrain_256.png", r);
	//add 10 cars
	for (int i = 0; i < 10; ++i) {
		r.add_car();
	}

	//Load 3D Models
	load_models(); 

	renderable fram = shape_maker::frame();
	renderable r_sphere = shape_maker::sphere(2);

	//todo figure out cube
	renderable r_cube = shape_maker::cube();

	r_quad = shape_maker::quad(); 

	//todo look up cameraman handling
	draw_cameraman.resize(r.cameramen().size());
	for (int i = 0; i < draw_cameraman.size(); i++) {
		draw_cameraman[i] = true; 
	}
	
	prepareTrack(r, r_track);
	prepareTerrain(r, r_terrain); 

	glViewport(0, 0, window_width, window_height); 
	glm::mat4 proj = glm::perspective(glm::radians(45.f), (float)window_width / (float)window_height, 0.001f, 10.f);

	load_textures();
	load_shaders(); 

	//glUseProgram(shader_world.program);
	//glUniformMatrix4fv(shader_world["uProj"], 1, GL_FALSE, &proj[0][0]);
	//glUniformMatrix4fv(shader_world["uView"], 1, GL_FALSE, &camera.matrix()[0][0]); //todo camera 
	//glUniform1i(shader_world["uColorImage"], 0);

	//glUseProgram(shader_basic.program); 
	//glUniformMatrix4fv(shader_basic["uProj"], 1, GL_FALSE, &proj[0][0]);
	//glUniformMatrix4fv(shader_basic["uView"], 1, GL_FALSE, &camera.matrix()[0][0]);

	//create matric stack and move view to have it on the scene
	matrix_stack stack;
	float scale = 1.f / r.bbox().diagonal();
	glm::vec3 center = r.bbox().center();
	stack.load_identity();
	stack.push();
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale)));
	stack.mult(glm::translate(glm::mat4(1.f), -center));

	//Start the scene
	r.start(11, 0, 0, 600);
	r.update(); 

	//Todo transform the scene's bounding box in world coordinates to pass it to the Sun Projector



	renderable r_trees;
	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	renderable r_lamps;
	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

	/* use the program shader "program_shader" */
	glUseProgram(basic_shader.program);

	/* define the viewport  */
	glViewport(0, 0, 800, 800);

	tb[0].reset();
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
	curr_tb = 0;

	proj = glm::perspective(glm::radians(45.f), 1.f, 1.f, 10.f);

	view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

	r.start(11, 0, 0, 600);
	r.update();

	glEnable(GL_DEPTH_TEST);

	glUseProgram(basic_shader.program);
	glUniform1i(basic_shader["uTexture"], 0); //Link texture to unit 0



	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		check_gl_errors(__LINE__, __FILE__);

		r.update();
		stack.load_identity();
		stack.push();
		stack.mult(tb[0].matrix());
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
		fram.bind();
		glDrawArrays(GL_LINES, 0, 6);

		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
		glEnd();


		float s = 1.f / r.bbox().diagonal();
		glm::vec3 c = r.bbox().center();

		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
		stack.mult(glm::translate(glm::mat4(1.f), -c));


		glDepthRange(0.01, 1);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1, 1, 1.0);
		r_terrain.bind();
		glDrawArrays(GL_POINTS, 0, r_terrain().count);
		glDepthRange(0.0, 1);

		for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
			stack.push();
			stack.mult(r.cars()[ic].frame);
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 0.1, 0.0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			fram.bind();
			glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}

		fram.bind();
		for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
			stack.push();
			stack.mult(r.cameramen()[ic].frame);
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4, 4)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			glDrawArrays(GL_LINES, 0, 6);
			stack.pop();
		}
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

		r_track.bind();
		glPointSize(3.0);
		glUniform3f(basic_shader["uColor"], 0.2f, 0.3f, 0.2f);
		glDrawArrays(GL_LINE_STRIP, 0, r_track.vn);
		glPointSize(1.0);


		r_trees.bind();
		glUniform3f(basic_shader["uColor"], 0.f, 1.0f, 0.f);
		glDrawArrays(GL_LINES, 0, r_trees.vn);


		r_lamps.bind();
		glUniform3f(basic_shader["uColor"], 1.f, 1.0f, 0.f);
		glDrawArrays(GL_LINES, 0, r_lamps.vn);

		stack.pop();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}


