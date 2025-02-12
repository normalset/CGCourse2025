#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <time.h>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"



int main(void)
{
    int w = 1024, h = 1024; //window size

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(w, h, "indexed", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();

    /* define a cube */
    renderable r_cube = shape_maker::cube(0.4f, 0.3f, 0.2f);

    /* define a cylinder */
    renderable r_cyl = shape_maker::cylinder(50);

    /* define the three axis to show a frame */
    renderable r_frame = shape_maker::frame();

    shader basic_shader;
    basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
   
	/* use the program shader "program_shader" */
	glUseProgram(basic_shader.program);

     
   
	glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -2.f));
	
	// create the view reference frame setting each row 
	// manually
	glm::mat4 view_matrix;
	// view_matrix[0] = glm::vec4(1.f, 0.f, 0.f, 0.f);
	// view_matrix[1] = glm::vec4(0, 0.8, -0.6,0.f);
	// view_matrix[2] = glm::vec4(0, 0.6, 0.8, 0.0);
	// view_matrix[3] = glm::vec4(0, 3.f, 2.f, 1.0);
	// invert the reference frame to have the view matrix
	// view_matrix = glm::inverse(view_matrix);

	// do the same by calling the lookAt function
	view_matrix = glm::lookAt(glm::vec3(0,3,2),glm::vec3(0.f,0.f,-2.f), glm::vec3(0.f,1,0));

	// pass it to the shader
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view_matrix[0][0]);

	// create the projection matrix
	glm::mat4 proj_matrix = glm::frustum(-1.f, 1.f, -1.f,  1.f, 1.f, 10.f);

	// pass it to the shader
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj_matrix[0][0]);

    glClearColor(0, 0.4, 0.5,1);
	glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, w, h);

	float angle = 0;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		angle = ( 60.f*clock()/ CLOCKS_PER_SEC);
        glm::mat4 rot_matrix = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.0, 0.0));
        glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &rot_matrix[0][0]);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /*  PUT YOUR CODE HERE
        *   For each shape (cube, cylinder...)
        *   1. define the transformation matrices
        *   2. set them as the corresponding uniform of the shader
        *   3. make the rendering call glDraw..
        */


        r_cube.bind();
        model_matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -2.f));
        glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
        glUniform1i(basic_shader["uShade"], 1);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, NULL);


        r_cyl.bind();
        model_matrix = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -2.f));
        glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
        glUniform1i(basic_shader["uShade"], 1);
        glDrawElements(r_cyl().mode, r_cyl().count, r_cyl().itype, NULL);

        r_frame.bind();
        glm::mat4 scaleframe_matrix = glm::scale(glm::mat4(1.0), glm::vec3(3, 3, 3));
        model_matrix = model_matrix * scaleframe_matrix;
        glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
        glUniform1i(basic_shader["uShade"], 0);
        //glDrawArrays(r_frame().mode, 0,r_frame().count);
        glDrawArrays(GL_LINES,0,6);



        check_gl_errors(__LINE__,__FILE__);
 
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);

    glfwTerminate();
    return 0;
}
