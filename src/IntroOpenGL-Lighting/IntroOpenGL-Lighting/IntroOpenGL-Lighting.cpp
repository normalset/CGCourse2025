#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "E:\uni\ComputerGrafica\3dparty\debugging.h"

int main(int argc, char** argv) {

    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(512, 512, "code_02_my_first_triangle", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();
    printout_opengl_glsl_info();

    GLuint positionAttribIndex = 0;
    float positions[] = { 0.0, 0.0,  0.5, 0.0,  0.5, 0.5,  0.0, 0.5 };

    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    GLuint positionsBuffer;
    glCreateBuffers(1, &positionsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttribIndex);
    glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, false, 0, 0);

    GLuint colorAttribIndex = 1;
    float colors[] = { 1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 1.0, 1.0 };

    GLuint colorBuffer;
    glCreateBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glEnableVertexAttribArray(colorAttribIndex);
    glVertexAttribPointer(colorAttribIndex, 3, GL_FLOAT, false, 0, 0);

    GLuint indices[] = { 0,1,2,0,2,3 };
    GLuint indexBuffer;
    glCreateBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    std::string vertex_shader_src = "#version 460\n"
        "in vec2 aPosition;\n"
        "in vec3 aColor;\n"
        "out vec3 vColor;\n"
        "uniform vec3 uLightPos;\n"                                 //definisco il punto di origine della luce
        "const vec3 normal = vec3(0.0, 0.0, 1.0);\n"                //definisco il vettore per la luce
        "void main(void) {\n"
        "    vec3 lightDir = normalize(uLightPos);\n"               //normalizzo il vettore posizione della luce
        "    float diffuse = max(dot(normal, lightDir), 0.01);\n"   //calcolo il dot product tra il vettore normalizzato e la direzione della luce
        "    vColor = aColor * diffuse;\n"                          //definisco il colore come il prodotto tra colore e diffusione della luce che lo illumina
        "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
        "}";

    const GLchar* vs_source = vertex_shader_src.c_str();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_source, NULL);
    glCompileShader(vertex_shader);

    std::string fragment_shader_src = "#version 460\n"
        "layout(location = 0) out vec4 color;\n"
        "in vec3 vColor;\n"
        "void main(void) {\n"
        "    color = vec4(vColor, 1.0);\n"
        "}";

    const GLchar* fs_source = fragment_shader_src.c_str();
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_source, NULL);
    glCompileShader(fragment_shader);

    GLuint program_shader = glCreateProgram();
    glAttachShader(program_shader, vertex_shader);
    glAttachShader(program_shader, fragment_shader);
    glBindAttribLocation(program_shader, positionAttribIndex, "aPosition");
    glBindAttribLocation(program_shader, colorAttribIndex, "aColor");
    glLinkProgram(program_shader);

    GLint linked;
    validate_shader_program(program_shader);
    glGetProgramiv(program_shader, GL_LINK_STATUS, &linked);
    if (linked) {
        glUseProgram(program_shader);
    }

    GLint lightPosLoc = glGetUniformLocation(program_shader, "uLightPos");

    check_gl_errors(__LINE__, __FILE__);

    float d = 0.001;
    float delta = 0;
    while (!glfwWindowShouldClose(window)) {
        if (delta < 0 || delta > 1)
            d = -d;
        delta += d;

        glUniform3f(lightPosLoc, delta, 1.0, 1.0); //nel ciclo di rendering passo la posizione della luce per aggiornare la variabile nello shader

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
