#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../common/debugging.h"
#include "../common/shaders.h"

int main(int argc, char** argv) {

	GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "code_11_fsq_raytracer", NULL, NULL);


    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window); 

    /* Initialize the gle wrangler*/
    glewInit();

    /* query for the hardware and software specs and print the result on the console*/
    printout_opengl_glsl_info();

    ///* create render data in RAM */
    // create quad in NDC space
    GLuint positionAttribIndex = 0;
    float positions[] = { -1.0,-1.0,  // 1st vertex
                           1.0,-1.0,  // 2nd vertex
                           1.0, 1.0,  // 3nd vertex

                          -1.0,-1.0,  // 1st vertex
                           1.0, 1.0,  // 2nd vertex
                          -1.0, 1.0   // 3nd vertex 
    };


    ///* create  a vertex array object */
    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    ///* create a buffer for the render data in video RAM */
    GLuint positionsBuffer;
    glCreateBuffers(1, &positionsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);

    ///* declare what data in RAM are filling the bufferin video RAM */
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttribIndex);
    ///* specify the data format */
    glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, false, 0, 0);



    ///* create a vertex shader */
    shader raytracer;
    raytracer.create_program("./shaders/basic.vert", "./shaders/raytracer.frag");


    /* cal glGetError and print out the result in a more verbose style
    * __LINE__ and __FILE__ are precompiler directive that replace the value with the
    * line and file of this call, so you know where the error happened
    */
    check_gl_errors(__LINE__, __FILE__);

    glUseProgram(raytracer.program);

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

        /* Render here */
        glClearColor(0.0, 0.0, 0.0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        glDrawArrays(GL_TRIANGLES, 0,6);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

	return 0;
}