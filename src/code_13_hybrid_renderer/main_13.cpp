#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include "../common/intersection.h"
#include "../common/trackball.h"


/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "../common/gltf_loader.h"
#include "../common/texture.h"
int width, height;

#define RASTERIZATION 0
#define RAYTRACING 1

int render_mode;

/* light direction in world space*/
glm::vec4 Ldir;

/* one trackball to manipulate the scene, one for the light direction */
trackball tb[2];

/* which trackball is currently used */
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

/* variables for storing the cone and cylinder  */
renderable r_sphere, r_quad, r_cone,r_cyl;

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (mods&GLFW_MOD_CONTROL) {

			/* this is just a piece of code to show how to find what point or object 
			 is intersected by the view ray passing through the clicke pixel.
			 Does not do anything other the printing out the value found
			 */

			// from viewport to world space
			float depthvalue;
			glReadPixels((int)xpos, height - (int)ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthvalue);
			glm::vec4 ndc = glm::vec4(-1.f + xpos / float( width) * 2, -1.f + (height - ypos) / float(height) * 2.f, -1.f + depthvalue*2.f, 1.f);
			glm::vec4 hit1 = glm::inverse(proj*view)*ndc;
			hit1 /= hit1.w;
			std::cout << " hit point " << glm::to_string(hit1) << std::endl;

			// from viewport to world space with unProject
			glm::vec3 hit = glm::unProject(glm::vec3(xpos, height - ypos, depthvalue), view, proj, glm::vec4(0, 0, width, height));
			std::cout << " hit point " << glm::to_string(hit) << std::endl;

			// read back the color from the color buffer and compute the index
			GLubyte colu[4];
			glReadPixels(xpos, height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &colu[0]);
			std::cout << " rgba  " << (int)colu[0] << " " << (int)colu[1] << " " << (int)colu[2] << " " << (int)colu[3] << std::endl;

			int id = colu[0] + (colu[1] << 8) + (colu[2] << 16);
			std::cout << "selected ID: " << id << std::endl;

			tb[0].set_center_radius(hit1, 2.f);
		}
		else
			tb[curr_tb].mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb[curr_tb].mouse_release();
		}
}

/* callback function called when the mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

/* callback function called when a key is pressed */
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* every time any key is presses it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS && ((mods & GLFW_MOD_CONTROL)==0))
		curr_tb = 1 - curr_tb;
}

/* callback function called when the windows is resized */
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
}


// variable for the lighting
float a_color[3] = { 0.15f,0.15f,0.15f };
float d_color[3] = { 0.5f,0.1f,0.2f };
float s_color[3] = { 0.5f,0.1f,0.2f };
float e_color[3] = { 0.5f,0.1f,0.2f };
float l_color[3] = { 0.9f,0.9f,0.9f };
float shininess = 1.0;
/*  shading_mode = 0 // no shading
	shading_mode = 1 // flat shading
	shading_mode = 2 // Gauraud shading
	shading_mode = 3 // Phong shading
	shading_mode = 4 // pbrBaseTexture

*/
int shading_mode = 3;

bool draw_i[2] = {true,false};
/* menu bar definition */
void gui_setup() {

	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Engine")) {
		if (ImGui::Selectable("rasterization", render_mode == 0)) render_mode = 0;
		if (ImGui::Selectable("ray tracing ", render_mode == 1)) render_mode = 1;
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Render Mode")) {
		if (ImGui::Selectable("none", shading_mode == 0)) shading_mode = 0;
		if (ImGui::Selectable("Flat-Per Face ", shading_mode == 1)) shading_mode = 1;
		if (ImGui::Selectable("Gaurad", shading_mode == 2)) shading_mode = 2;
		if (ImGui::Selectable("Phong", shading_mode == 3)) shading_mode = 3;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Light ")) {
		ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoOptions;
		ImGui::ColorEdit3("light color", (float*)&l_color, misc_flags);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Material ")) {
		ImGuiColorEditFlags misc_flags = ImGuiColorEditFlags_NoOptions;
		ImGui::ColorEdit3("amb color", (float*)&a_color, misc_flags);
		ImGui::ColorEdit3("diff color", (float*)&d_color, misc_flags);
		ImGui::ColorEdit3("spec color", (float*)&s_color, misc_flags);
		ImGui::SliderFloat("shininess", &shininess, 1.0, 500.f);
		ImGui::ColorEdit3("emiss color", (float*)&e_color, misc_flags);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Objects ")) {
		ImGui::Checkbox("Sphere", &draw_i[1]);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}


/* draw the arrow */
void draw_arrow( matrix_stack & stack, shader used_program) {
	r_cyl.bind();

	stack.push();
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.03, 0.75, 0.03)));
 	glUniformMatrix4fv(used_program["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glUniform1i(used_program["uShadingMode"],1);
	glUniform3f(used_program["uAmbientColor"], 0.15f, 0.15f, 0.15f);
	glUniform3f(used_program["uDiffuseColor"], 0.f,0.3f,0.8f);
	glUniform3f(used_program["uSpecularColor"], 0.f, 0.0f, 0.0f);
	glDrawElements(r_cyl().mode, r_cyl().count, r_cyl().itype, 0);
	stack.pop();

	stack.push();
	r_cone.bind();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 1.5, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.07, 0.2, 0.07)));
	glUniform3f(used_program["uDiffuseColor"], 0.1f, 0.8f, 0.2f);
	glUniformMatrix4fv(used_program["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_cone().mode, r_cone().count, r_cone().itype, 0);
	stack.pop();
}

int main(int argc , char ** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1024;
	height = 1024;
	window = glfwCreateWindow(width, height, "code_13_htbrid_renderer", NULL, NULL);
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
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	// Variabili per i limiti
	GLuint maxWorkGroupSize[3];
	GLuint maxWorkGroupCount[3];
	GLint maxWorkGroupInvocations;
	check_gl_errors(__LINE__, __FILE__);
	// Recupera il massimo numero di thread per workgroup in ciascuna dimensione
	int workGroupSizes[3] = { 0 };
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizes[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizes[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizes[2]);
	check_gl_errors(__LINE__, __FILE__);
	// Recupera il massimo numero di workgroup che puoi lanciare in ogni dimensione
//	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_COUNT, maxWorkGroupCount);
	std::cout << "Max Workgroup Count (X, Y, Z): "
		<< workGroupSizes[0] << ", "
		<< workGroupSizes[1] << ", "
		<< workGroupSizes[2] << std::endl;
	check_gl_errors(__LINE__, __FILE__);
	// Recupera il numero massimo di invocazioni di thread per workgroup
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxWorkGroupInvocations);
	std::cout << "Max Workgroup Invocations: " << maxWorkGroupInvocations << std::endl;

	check_gl_errors(__LINE__, __FILE__);

	/* initialize IMGUI */
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	/* end IMGUI initialization */

	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	/* load the shaders */
	shader basic_shader;
	basic_shader.create_program(
		join("shaders/directives.glsl", "shaders/phong_material.glsl", "shaders/phong.glsl","shaders/basic.vert"),
		join("shaders/directives.glsl", "shaders/phong_material.glsl", "shaders/phong.glsl","shaders/basic.frag")
	);


	/* create a  sphere   centered at the origin with radius 1*/
	r_sphere = shape_maker::sphere(2);
	
	/* create a quad */
	r_quad = shape_maker::quad();

	/* create a cone (for the tip of the arrow) */
	r_cone = shape_maker::cone(1.f, 1.f, 10);

	/* create a cylinder (for the body of the arrow) */
	r_cyl = shape_maker::cylinder(10);


	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
	view = glm::lookAt(glm::vec3(0, 7, 6.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glm::mat4 proj_inv = glm::inverse(proj);
	glm::mat4 view_inv = glm::inverse(view);

	/* Light direction is initialized as +Y */
	Ldir = glm::vec4(0, 1, 0,0);

	render_mode = RASTERIZATION;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	matrix_stack stack;

	// COMPUTE SHADERS STUFF

	///* create a   shader */

	shader texture_shader;
	texture_shader.create_program("./shaders/texture.vert", "./shaders/texture.frag");

	shader raytracer;
	raytracer.create_program(join("./shaders/directives.glsl", "./shaders/phong_material.glsl", "./shaders/phong.glsl","./shaders/cs_raytracer.comp"));
	
	texture mattoni;
	mattoni.load("./textures/brick_wall2-diff-512.png", 1);

	// create a texture    
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);

	/* this establishes that this texture is bound to the "image unit" 0.
	*  If a compute shader read/write to  the image unit 0, it is reading/writing
	* to this texture
	*/
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	check_gl_errors(__LINE__, __FILE__);


	/* set the viewport  */
	glViewport(0, 0, width, height);

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);

	/* set the trackballs position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	curr_tb = 0;

	
	glActiveTexture(GL_TEXTURE0);
	

	glEnable(GL_DEPTH_TEST);
	glUseProgram(basic_shader.program);
/* Loop until the user closes the window */
	int nf = 0;
	int cstart = clock();
	while (!glfwWindowShouldClose(window))
	{
		if (clock() - cstart > CLOCKS_PER_SEC) {
			std::cout << nf << std::endl;
			nf = 0;
			cstart = clock();
		}
		nf++;
		// light direction
		/* Update the light direction using the trackball tb[1]
		   It's just a rotation
		*/
		Ldir = tb[1].matrix() * glm::vec4(0, 1, 0, 0);

			
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		stack.load_identity();

		stack.push();
		stack.mult(tb[0].matrix());

		if ( render_mode == RASTERIZATION ) {
			glUseProgram(basic_shader.program);
			glUniform1i(basic_shader["uShadingMode"], shading_mode);
			glUniform3fv(basic_shader["uDiffuseColor"], 1, &d_color[0]);
			glUniform3fv(basic_shader["uAmbientColor"], 1, &a_color[0]);
			glUniform3fv(basic_shader["uSpecularColor"], 1, &s_color[0]);
			glUniform1f(basic_shader["uShininess"], shininess);
			glUniform3fv(basic_shader["uLightColor"], 1, &l_color[0]);
			glUniform3f(basic_shader["uLDir"], Ldir.x, Ldir.y, Ldir.z);

		//	glUniform1i(basic_shader["uMattoni"], 1);
		//	glActiveTexture(GL_TEXTURE1);
		//	glBindTexture(GL_TEXTURE_2D, mattoni.id);


			r_sphere.bind();
			stack.push();
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 3.0, 0.0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
			stack.pop();

			r_quad.bind();
			stack.push();
			stack.mult(glm::scale(glm::vec3(2.0, 1.0, 2.0)));
			stack.mult(glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1, 0.0, 0.0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);
			stack.pop();

			
			stack.push();
			stack.load_identity();
			stack.mult(tb[1].matrix());
			//glUseProgram(basic_shader.program);
			// glUniform3f(basic_shader["uLDir"], 0.f, 0.f, 1.f);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			draw_arrow(stack, basic_shader);
			stack.pop();
		}
		if (render_mode == RAYTRACING) {
			glUseProgram(raytracer.program);

			glUniformMatrix4fv(raytracer["uProjInv"], 1, GL_FALSE, &proj_inv[0][0]);
			glUniformMatrix4fv(raytracer["uViewInv"], 1, GL_FALSE, &view_inv[0][0]);
			glm::mat4 stackInv = glm::inverse(stack.m());
			glUniformMatrix4fv(raytracer["uModelInv"], 1, GL_FALSE, &stackInv[0][0]);
			glUniform3f(raytracer["uLDir"], Ldir.x, Ldir.y, Ldir.z);

			glUniform3fv(raytracer["uDiffuseColor"], 1, &d_color[0]);
			glUniform3fv(raytracer["uAmbientColor"], 1, &a_color[0]);
			glUniform3fv(raytracer["uSpecularColor"], 1, &s_color[0]);
			glUniform1f (raytracer["uShininess"], shininess);
			glUniform3fv(raytracer["uLightColor"], 1, &l_color[0]);
			

			glUniform1i(raytracer["uMattoni"], 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mattoni.id);


			// dispatch 16x16x1 workgorups
			glDispatchCompute(512   , 512  , 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


			/* draw  a FSQ with the texture stitched on it, to show
			* the result of the compute shader
			*/
			glUseProgram(texture_shader.program);
			glClearColor(0.0, 0.0, 0.0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, texture);
			r_quad.bind();
			glm::mat4 rot = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1, 0.0, 0.0));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &rot[0][0]);
			glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);
		}
	
		stack.pop();

		/* draw the Graphical User Interface */
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		gui_setup();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		/* end of graphical user interface */


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}
