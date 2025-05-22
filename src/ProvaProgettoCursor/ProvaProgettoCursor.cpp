#define GLEW_STATIC
#define GLW_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL

#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "./common/gltf_loader.h"
#include "./common/texture.h"

//#include <stb_image.h>
//#include <stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include ".\common\debugging.h"
#include ".\common\renderable.h"
#include ".\common\shaders.h"
#include ".\common\simple_shapes.h"
#include ".\common\carousel\carousel.h"
#include ".\common\carousel\carousel_to_renderable.h"

#include ".\common\carousel\carousel_loader.h"

#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include ".\common\matrix_stack.h"
#include ".\common\intersection.h"
#include ".\common\trackball.h"
#include "./common/texture.h"


#define NUM_CARS 5
#define WIN_WIDTH 800
#define WIN_HEIGHT 800
#define N_GROUND_TILES 20.0   // the terrain is covered with 20^2 tiles

void debug(std::string s) {
	std::cout << "[ DEGUB ]" << s << std::endl;
}


trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

matrix_stack stack;
float scaling_factor = 1.0;

/* program shaders used */
shader texture_shader, flat_shader, basic_shader;

// Textures
texture grass_texture; 

enum textures {
	TEXTURE_DEFAULT,
	TEXTURE_GRASS, 
	TEXTURE_TRACK,

};

void load_textures() {
	grass_texture.load("./assets/textures/grass.png", TEXTURE_GRASS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // enable mipmap for minification
	//texture_track_diffuse.load(textures_path + "road_diff.png", TEXTURE_ROAD);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//Models

char model_car[65536] = { "./assets/models/Taxi.glb" };
char model_tree[65536] = { "./assets/models/AppleTree.glb" };

//declare a gltf_loader
gltf_loader gltfL;

//declare an axis aligned box cointaining the loaded obj
box3 car_bbox, tree_bbox, light_bbox;
std::vector <renderable> car_obj, tree_obj, light_obj;


void load_model() {
	// load a gltf scene into a vector of objects of type renderable "obj"
	// also fill  box containing the whole scene

	//Load Car model
	gltfL.load_to_renderable(model_car, car_obj, car_bbox);

	// Debug information
	std::cout << "Car Model loaded. Number of objects: " << car_obj.size() << std::endl;
	if (car_obj.size() > 0) {
		std::cout << "First object details:" << std::endl;
		std::cout << "Vertex count: " << car_obj[0].vn << std::endl;
		std::cout << "Has base color texture: " << (car_obj[0].mater.base_color_texture != 0 ? "yes" : "no") << std::endl;
		std::cout << "Has normal texture: " << (car_obj[0].mater.normal_texture != 0 ? "yes" : "no") << std::endl;
		std::cout << "Bounding box diagonal: " << car_bbox.diagonal() << std::endl;
		std::cout << "Bounding box center: " << car_bbox.center().x << ", " << car_bbox.center().y << ", " << car_bbox.center().z << std::endl;
	}

	//Load Tree Model
	gltfL.load_to_renderable(model_tree, tree_obj, tree_bbox);

	// Debug information
	std::cout << "Tree Model loaded. Number of objects: " << tree_obj.size() << std::endl;
	if (tree_obj.size() > 0) {
		std::cout << "First object details:" << std::endl;
		std::cout << "Vertex count: " << tree_obj[0].vn << std::endl;
		std::cout << "Has base color texture: " << (tree_obj[0].mater.base_color_texture != 0 ? "yes" : "no") << std::endl;
		std::cout << "Has normal texture: " << (tree_obj[0].mater.normal_texture != 0 ? "yes" : "no") << std::endl;
		std::cout << "Bounding box diagonal: " << tree_bbox.diagonal() << std::endl;
		std::cout << "Bounding box center: " << tree_bbox.center().x << ", " << tree_bbox.center().y << ", " << tree_bbox.center().z << std::endl;
	}
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

void draw_cars_models(matrix_stack& stack, const race& r) {
	glUseProgram(texture_shader.program);
	for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
		stack.push();
		// Apply car's frame transformation
		stack.mult(r.cars()[ic].frame);

		// Scale to reasonable size (adjust these values as needed)
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.01f, 0.01f, -0.01f)));

		// Set shader uniforms
		glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform1i(texture_shader["uRenderMode"], 1); // Use texture mode

		// Draw each part of the car
		for (unsigned int i = 0; i < car_obj.size(); ++i) {
			car_obj[i].bind();

			// Bind textures if they exist
			if (car_obj[i].mater.base_color_texture) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, car_obj[i].mater.base_color_texture);
			}

			if (car_obj[i].mater.normal_texture) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, car_obj[i].mater.normal_texture);
			}

			// Draw the car part
			glDrawElements(car_obj[i]().mode, car_obj[i]().count, car_obj[i]().itype, 0);
		}
		stack.pop();
	}
	glUseProgram(basic_shader.program);
}

void draw_trees_models(matrix_stack& stack, const race& r) {
	glUseProgram(texture_shader.program);
	for (unsigned int it = 0; it < r.trees().size(); ++it) {
		stack.push();
		// Apply tree's position transformation
		stack.mult(glm::translate(glm::mat4(1.0f), r.trees()[it].pos));

		// Scale to reasonable size and adjust orientation if needed
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.1f, 0.1f, 0.1f)));

		// Set shader uniforms
		glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform1i(texture_shader["uRenderMode"], 1); // Use texture mode

		// Draw each part of the tree
		for (unsigned int i = 0; i < tree_obj.size(); ++i) {
			tree_obj[i].bind();

			// Bind textures if they exist
			if (tree_obj[i].mater.base_color_texture) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tree_obj[i].mater.base_color_texture);
			}

			if (tree_obj[i].mater.normal_texture) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, tree_obj[i].mater.normal_texture);
			}

			// Draw the tree part
			glDrawElements(tree_obj[i]().mode, tree_obj[i]().count, tree_obj[i]().itype, 0);
		}
		stack.pop();
	}
	glUseProgram(basic_shader.program);
}

inline std::vector<GLfloat> generateTerrainTextureCoords(terrain t) {
    std::vector<GLfloat> v;
    const unsigned int Z = (t.size_pix[1]);
    const unsigned int X = (t.size_pix[0]);
    v.resize(2 * Z * X);

    unsigned int slot = 0;
    for (unsigned int iz = 0; iz < Z; ++iz) {
        for (unsigned int ix = 0; ix < X; ++ix) {
            v[slot] = N_GROUND_TILES * ix / float(X);
            v[slot + 1] = N_GROUND_TILES * iz / float(Z);
            slot += 2;
        }
    }

    return v;
}

int main(int argc, char** argv)
{
	race r;

	carousel_loader::load("small_test.svg", "terrain_256.png", r);

	//add 10 cars
	for (int i = 0; i < 10; ++i)
		r.add_car();

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 800, "CarOusel", NULL, NULL);
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

	//

	//load shaders
	std::string shaders_path = "./shaders/";
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());
	flat_shader.create_program((shaders_path + "flat.vert").c_str(), (shaders_path + "flat.frag").c_str());

	/* Set the uT matrix to Identity */
	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	//Setup shader program
	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);

	glUniform3f(texture_shader["uDiffuseColor"], 0.8f, 0.8f, 0.8f);
	glUniform1i(texture_shader["uColorImage"], 0);
	glUniform1i(texture_shader["uBumpmapImage"], 1);
	glUniform1i(texture_shader["uNormalmapImage"], 2);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniform3f(flat_shader["uColor"], 1.0, 1.0, 1.0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	check_gl_errors(__LINE__, __FILE__, true);

	load_model();


	//

	renderable fram = shape_maker::frame();

	renderable r_cube = shape_maker::cube();

	renderable r_track;
	r_track.create();
	game_to_renderable::to_track(r, r_track);

	renderable r_terrain;
	r_terrain.create();
	game_to_renderable::to_heightfield(r, r_terrain);
	std::vector<GLfloat> terrainTextureCoords = generateTerrainTextureCoords(r.ter());
   	r_terrain.add_vertex_attribute<GLfloat>(&terrainTextureCoords[0], terrainTextureCoords.size(), 4, 2);

	renderable r_trees;
	r_trees.create();
	game_to_renderable::to_tree(r, r_trees);

	renderable r_lamps;
	r_lamps.create();
	game_to_renderable::to_lamps(r, r_lamps);

	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	glUseProgram(basic_shader.program);
	glUniform1i(basic_shader["uUseTexture"], 0);

	/* use the program shader "program_shader" */
	glUseProgram(basic_shader.program);

	/* define the viewport  */
	glViewport(0, 0, 800, 800);

	tb[0].reset();
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
	curr_tb = 0;

	proj = glm::perspective(glm::radians(45.f), 1.f, 0.01f, 10.f);

	view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

	r.start(11, 0, 0, 600);
	r.update();

	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);
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
		glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

		//glUseProgram(texture_shader.program);
		//glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		//glUniform4fv(texture_shader["uLdir"], 1, &r.sunlight_direction().x);
		//glUniform1i(texture_shader["uRenderMode"], 1);
		//glUniform1i(texture_shader["uColorImage"], TEXTURE_GRASS); 
		
		// Set up texture for terrain
		r_terrain.bind();
		//glActiveTexture(GL_TEXTURE0 + TEXTURE_GRASS);
		glBindTexture(GL_TEXTURE_2D, grass_texture.id);
		glUniform1i(basic_shader["uTexture"], TEXTURE_GRASS);
		glUniform1i(basic_shader["uUseTexture"], 1);
		//glDrawArrays(GL_TRIANGLES, 0, r_terrain().count);
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
		//glDepthRange(0.0, 1);
		
		// Reset texture state
		glUseProgram(basic_shader.program);
		glUniform1i(basic_shader["uUseTexture"], 0);

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

		// Draw cars 
		//draw_cars_models(stack, r);

		// Draw trees
		//draw_trees_models(stack, r);

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

