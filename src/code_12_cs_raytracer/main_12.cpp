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
    window = glfwCreateWindow(512, 512, "code_12_cs_raytracer", NULL, NULL);


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



    ///* create a  shader */
    shader texture_shader;
    texture_shader.create_program("./shaders/texture.vert","./shaders/texture.frag");

    ///* create a   shader */
    shader raytracer;
    raytracer.create_program("./shaders/cs_raytracer.comp");


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
   

    /* cal glGetError and print out the result in a more verbose style
    * __LINE__ and __FILE__ are precompiler directive that replace the value with the
    * line and file of this call, so you know where the error happened
    */
    check_gl_errors(__LINE__, __FILE__);

    glUseProgram(texture_shader.program);
    glUniform1i(texture_shader["uColorImage"], 0);

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

        glUseProgram(raytracer.program);
    
        // dispatch 16x16x1 workgorups
        glDispatchCompute(512/32,512/32 , 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


        /* draw  a FSQ with the texture stitched on it, to show
        * the result of the compute shader
        */
        glUseProgram(texture_shader.program);
        glClearColor(0.0, 0.0, 0.0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0,6);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

	return 0;
}