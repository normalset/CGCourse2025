#define GLEW_STATIC
#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"


/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  


int main(void)
{
	int w = 1024, h = 1024; //window size

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(w, h, "code_05_my_first_car", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	printout_opengl_glsl_info();

	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");


	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube = shape_maker::cube(0.5f, 0.3f, 0.f);

	/* create a  cylinder with base on the XZ plane, and height=2*/
	renderable r_cyl = shape_maker::cylinder(30, 0.2f, 0.1f, 0.5f);

	/* define the three axis to show a frame */
	renderable r_frame = shape_maker::frame();

	/* create a  rectangle with base on the XY plane, centered at the origin and side 2*/
	renderable r_plane = shape_maker::quad();

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 100.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 3.f, 0.f));


	/*Initialize the matrix to implement the continuos rotation aroun the y axis*/
	glm::mat4 R = glm::mat4(1.f);


	glm::mat4 model_plane = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	model_plane = glm::scale(model_plane, glm::vec3(4.f, 4.f, 4.f));


	/* define a transformation that will be applyed the entire body of the car */
	//glm::mat4 rotcar = glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.0, -2.0));
	//rotcar = glm::rotate(rotcar, -0.1f, glm::vec3(1.0, 0.0, 0.0));
	//rotcar = glm::translate(rotcar, glm::vec3(0.0, 0.0, 2.0));

	/* define a transformation that tranform the cube into the main body of the car */
	glm::mat4 b1 = glm::scale(glm::mat4(1.f), glm::vec3(1.0, 0.75, 2.0));
	glm::mat4 b2 = glm::translate(glm::mat4(1.f), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 cube_to_car_body = b1 * b2;

	//define a transformatio to create upper part of the train
	glm::mat4 b3 = glm::scale(glm::mat4(1.f), glm::vec3(1.0, 0.5, 1.5));
	glm::mat4 b4 = glm::translate(glm::mat4(1.f), glm::vec3(0.0, 4.0 , 0.325));
	glm::mat4 cube_to_upper_train = b3 * b4;

	//define a transformation to create front part of the train
	glm::mat4 b5 = glm::scale(glm::mat4(1.f), glm::vec3(1.0, 0.5, 0.3));
	glm::mat4 b6 = glm::translate(glm::mat4(1.f), glm::vec3(0.0, 1.0, -7.725));
	glm::mat4 cube_to_front_train = b5 * b6;

	//define steam cylinder
	glm::mat4 s1 = glm::scale(glm::mat4(1.f) , glm::vec3(0.45 , 0.5, 0.45));
	glm::mat4 s2 = glm::translate(glm::mat4(1.f) , glm::vec3(0.0 , 5.0 , 0.5));
	glm::mat4 cyl_to_steam = s1 * s2; 

	/* define a transformation that tranform the cylinder into a wheel */
	glm::mat4 cyl_to_wheel;
	glm::mat4 w1 = glm::rotate(glm::mat4(1.0), glm::radians(90.f), glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 w2 = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.1, 0.5));
	glm::mat4 w3 = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.0, 0.0));
	cyl_to_wheel = w1 * w2 * w3;

	//define connector block trasformation
	glm::mat4 con1 = glm::scale(glm::mat4(1.0), glm::vec3(0.25, 0.25, 0.5));
	glm::mat4 con2 = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.0, 4.0));
	glm::mat4 cube_to_connector = con1 * con2;
	
	//define trasformation to get the second part of the train
	glm::mat4 trans_train = glm::translate(glm::mat4(1.f), glm::vec3(0 , 0 , 4.5)); 

	//define transformation for train's rails
	glm::mat4 rail1 = glm::scale(glm::mat4(1.0), glm::vec3(0.1, 0.1, 8.0));
	glm::mat4 rail2 = glm::translate(glm::mat4(1.0), glm::vec3(11.0, -5.0, 0.0));
	glm::mat4 rail3 = glm::translate(glm::mat4(1.0), glm::vec3(-11.0, -5.0, 0.0));


	std::vector<glm::mat4> wheels;
	wheels.resize(4);
	wheels[0] = glm::translate(glm::mat4(1.0), glm::vec3(-1.15, 0.0, -1.0));;
	wheels[0] = wheels[0] * cyl_to_wheel;

	wheels[1] = glm::translate(glm::mat4(1.0), glm::vec3(1.15, 0.0, -1.0));;
	wheels[1] = wheels[1] * cyl_to_wheel;

	wheels[2] = glm::translate(glm::mat4(1.0), glm::vec3(-1.15, 0.0, 1.0));;
	wheels[2] = wheels[2] * cyl_to_wheel;

	wheels[3] = glm::translate(glm::mat4(1.0), glm::vec3(1.15, 0.0, 1.0));;
	wheels[3] = wheels[3] * cyl_to_wheel;

	//glm::mat4 model_plane = glm::rotate(glm::mat4(1.f),glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
	//model_plane = glm::scale(model_plane, glm::vec3(3.f, 3.f, 1.f));

	glEnable(GL_DEPTH_TEST);
	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

	matrix_stack stack;
	stack.load_identity();
	float angle = 0;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* one full rotation every 6 seconds*/
		angle = (60.f * clock() / CLOCKS_PER_SEC);

		R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f));

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &R[0][0]);

		glUseProgram(basic_shader.program);
		glUniform1i(basic_shader["uShade"], 1);

		/* render box and cylinders so that the look like a car */
		stack.load_identity();
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.5f, 0.0)));
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.5, 0.5, 0.5)));

		stack.push(); 
		for (int i = 0; i < 2; ++i) {

			if (i == 1) {
				stack.mult(trans_train);
			}

			/*draw the cube tranformed into the train's body*/
			stack.push();
			stack.mult(cube_to_car_body);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.7, 0.2, 0.2);
			r_cube.bind();
			glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			stack.pop();

			/*draw the wheels */
			r_cyl.bind();
			glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
			for (int iw = 0; iw < 4; ++iw) {
				stack.push();
				stack.mult(wheels[iw]);
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glDrawElements(r_cyl().mode, r_cyl().count, r_cyl().itype, 0);
				stack.pop();
			}
		}
		stack.pop();

		//draw the connector piece
		stack.push();
		stack.mult(cube_to_connector);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0, 1.0, 0.0);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		/*draw the cube tranformed into the train's upper body*/
		stack.push();
		stack.mult(cube_to_upper_train);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		/*draw the cube tranformed into the train's front part*/
		stack.push();
		stack.mult(cube_to_front_train);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		//draw steam cylinder
		stack.push();
		stack.mult(cyl_to_steam);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
		r_cyl.bind();
		glDrawElements(r_cyl().mode, r_cyl().count, r_cyl().itype, 0);
		stack.pop();

		/*draw the cube tranformed into the rails*/
		stack.push();
		stack.mult(rail1 * rail2);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.3, 0.3, 0.3);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		
		stack.push();
		stack.mult(rail1* rail3);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.3, 0.3, 0.3);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();


		glUniform3f(basic_shader["uColor"], 0.1f, 0.7f, 0.1f);
		glUniform1i(basic_shader["uShade"], 1);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &model_plane[0][0]);
		r_plane.bind();
		glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
		


		glm::mat4 scale_frame = glm::scale(glm::mat4(1.f), glm::vec3(3.f, 3.f, 3.f));
		glm::mat4 scaleframe_matrix = glm::scale(glm::mat4(1.0), glm::vec3(3, 3, 3));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &scale_frame[0][0]);
		glUniform1i(basic_shader["uShade"], 0);
		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}
	glUseProgram(0);
	glfwTerminate();
	return 0;
}