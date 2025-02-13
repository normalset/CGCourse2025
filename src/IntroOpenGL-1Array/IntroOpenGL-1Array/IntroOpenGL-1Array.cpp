#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "E:\uni\ComputerGrafica\IntroOpenGL-1Array\IntroOpenGL-1Array\debugging.h"


int main(int argc, char** argv) {

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "code_02_my_first_triangle", NULL, NULL);


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

    GLuint triangleDataIndex = 0;
    float triangleData[] = {
        0.0, 0.0,   1.0, 0.0, 0.0,   //vertex 1
        0.5, 0.0,   0.0, 1.0, 0.0,   //vertex 2
        0.5, 0.5,   0.0, 0.0, 1.0,   //vertex 3
        0.0, 0.5,   1.0, 1.0, 1.0   //vertex 4
    };

    // create a buffer for the render data in video RAM
    GLuint triangleDataBuffer;
    glCreateBuffers(1, &triangleDataBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangleDataBuffer);

    ///* declare what data in RAM are filling the bufferin video RAM */
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleData), triangleData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(triangleDataIndex);
    
    /////* specify the data format */*/
    glVertexAttribPointer(triangleDataIndex, 2, GL_FLOAT, false, sizeof(float) * 5, 0);

    ///* specify the data format */
    glVertexAttribPointer(triangleDataIndex, 3, GL_FLOAT, false, sizeof(float) * 5, (void*)(sizeof(float) * 2));

   



    GLuint indices[] = { 0,1,2,0,1,3 };
    GLuint indexBuffer;
    glCreateBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);


    ///* create a vertex shader */
    std::string  vertex_shader_src = "#version 460\n \
        in vec2 aPosition;\
        in vec3 aColor;\
        out vec3 vColor;\
        uniform float uDelta;\
        void main(void)\
        {\
         gl_Position = vec4(aPosition+vec2(uDelta,0.0), 0.0, 1.0);\
         vColor = aColor;\
        }\
       ";
    const GLchar* vs_source = (const GLchar*)vertex_shader_src.c_str();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_source, NULL);
    glCompileShader(vertex_shader);


    ///* create a fragment shader */
    std::string   fragment_shader_src = "#version 460 \n \
        layout(location = 0) out vec4 color;\
        in vec3 vColor;\
        uniform float uDelta;\
        void main(void)\
        {\
            color = vec4(vColor+vec3(uDelta,0.0,0.0), 1.0);\
        }";
    const GLchar* fs_source = (const GLchar*)fragment_shader_src.c_str();

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_source, NULL);
    glCompileShader(fragment_shader);


    GLuint program_shader = glCreateProgram();
    glAttachShader(program_shader, vertex_shader);
    glAttachShader(program_shader, fragment_shader);


    // glBindAttribLocation(program_shader, positionAttribIndex, "aPosition");
    // glBindAttribLocation(program_shader, colorAttribIndex, "aColor");
    glBindAttribLocation(program_shader, triangleDataIndex, "aPosition");
    glBindAttribLocation(program_shader, triangleDataIndex, "aColor");
    glLinkProgram(program_shader);


    GLint linked;
    validate_shader_program(program_shader);
    glGetProgramiv(program_shader, GL_LINK_STATUS, &linked);
    if (linked) {
        glUseProgram(program_shader);
    }

    GLint loc = glGetUniformLocation(program_shader, "uDelta");

    /* cal glGetError and print out the result in a more verbose style
    * __LINE__ and __FILE__ are precompiler directive that replace the value with the
    * line and file of this call, so you know where the error happened
    */
    check_gl_errors(__LINE__, __FILE__);

    float d = 0.0001;
    float delta = 0;
    while (!glfwWindowShouldClose(window))
    {
        if (delta < 0 || delta > 0.5)
            d = -d;
        delta += d;

        glUniform1f(loc, delta);

        /* Render here */
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}