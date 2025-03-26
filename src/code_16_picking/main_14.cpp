#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>

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

int width, height;

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

/* load the shaders */
shader basic_shader;
shader picking_shader;
matrix_stack stack;

/* variables for storing the cone and cylinder  */
renderable r_sphere, r_torus, r_cone, r_cyl;

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

void draw_scene(bool);
/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (mods & GLFW_MOD_CONTROL) {

			/* this is just a piece of code to show how to find what point or object
			 is intersected by the view ray passing through the clicke pixel.
			 Does not do anything other the printing out the value found
			 */

			 // from viewport to world space
			float depthvalue;
			glReadPixels((int)xpos, height - (int)ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthvalue);
			glm::vec4 ndc = glm::vec4(-1.f + xpos / float(width) * 2, -1.f + (height - ypos) / float(height) * 2.f, -1.f + depthvalue * 2.f, 1.f);
			glm::vec4 hit1 = glm::inverse(proj * view) * ndc;
			hit1 /= hit1.w;
			std::cout << " hit point " << glm::to_string(hit1) << std::endl;

			// from viewport to world space with unProject
			glm::vec3 hit = glm::unProject(glm::vec3(xpos, height - ypos, depthvalue), view, proj, glm::vec4(0, 0, width, height));
			std::cout << " hit point " << glm::to_string(hit) << std::endl;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			draw_scene(true);
			// read back the color from the color buffer and compute the index
			GLubyte colu[4];
			glReadPixels(xpos, height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &colu[0]);
			std::cout << " rgba  " << (int)colu[0] << " " << (int)colu[1] << " " << (int)colu[2] << " " << (int)colu[3] << std::endl;

			int id = colu[0] + (colu[1] << 8) + (colu[2] << 16);
			std::cout << "selected ID: " << id << std::endl;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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
	if (action == GLFW_PRESS && ((mods & GLFW_MOD_CONTROL) == 0))
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




void draw_scene(bool pick = false) {
	shader sh = (pick) ? picking_shader : basic_shader;
	glUseProgram(sh.program);
	/* Render here */
	int r, g, b;

	stack.push();
	stack.mult(tb[0].matrix());

	if (pick) {
		r = 40000 & 0x000000FF;
		g = (40000 & 0x0000FF00) >> 8;
		b = (40000 & 0x00FF0000) >> 16;
	}else
	{
		r =  150;
		g = b = 0;
	}
	glUniform3f(sh["uColor"], r / 255.0, g / 255.0, b / 255.0);

	r_sphere.bind();
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-2.0, 0.2, 0.2)));
	glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
	stack.pop();

	if (pick) {
		r = 30000 & 0x000000FF;
		g = (30000 & 0x0000FF00) >> 8;
		b = (30000 & 0x00FF0000) >> 16;
	}
	else
	{
		b = 150;
		g = r = 0;

	}

	glUniform3f(sh["uColor"], r / 255.0, g / 255.0, b / 255.0);

	r_torus.bind();
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(2.0, 0.2, 0.2)));
	glUniformMatrix4fv(sh["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_torus().mode, r_torus().count, r_torus().itype, 0);
	stack.pop();

	stack.pop();


	stack.push();
	stack.mult(tb[1].matrix());

	stack.pop();
}


int main(int argc, char** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1000;
	height = 800;
	window = glfwCreateWindow(width, height, "code_14_picking", NULL, NULL);
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



	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	/* load the shaders */
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

	picking_shader.create_program("shaders/picking.vert", "shaders/picking.frag");

	/* create a  sphere   centered at the origin with radius 1*/
	r_sphere = shape_maker::sphere(2);

	/* create a cone (for the tip of the arrow) */
	r_torus = shape_maker::torus(0.5f, 1.f, 20, 20);

	/* create a cone (for the tip of the arrow) */
	r_cone = shape_maker::cone(1.f, 1.f, 10);

	/* create a cylinder (for the body of the arrow) */
	r_cyl = shape_maker::cylinder(10);


	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
	view = glm::lookAt(glm::vec3(0, 7, 6.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	

	/* set the viewport  */
	glViewport(0, 0, width, height);

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);

	glUseProgram(picking_shader.program);
	glUniformMatrix4fv(picking_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(picking_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(picking_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform3f(picking_shader["uColor"], 1.0, 0.0, 0.0);

	/* set the trackballs position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	curr_tb = 0;


	glActiveTexture(GL_TEXTURE0);


	glEnable(GL_DEPTH_TEST);
	glUseProgram(basic_shader.program);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_scene(false);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}
