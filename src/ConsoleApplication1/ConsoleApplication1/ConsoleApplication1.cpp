#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <random>



std::vector<float> create_box2d(int xsize, int ysize) {
    std::vector<float> points;

    float xstep = 0.2f / float(xsize);
    float ystep = 0.2f / float(ysize);

    std::cout << "xstep : " << xstep << " ystep: " << ystep << "\n"; 

    float basex_start = -0.1f;
    float basey_start = 0.1f;

    for (int i = 0; i < ysize; ++i) {
        float basey = basey_start - i * ystep;
        for (int k = 0; k < xsize; ++k) {
            float basex = basex_start + k * xstep;

            // Vertex 0
            points.push_back(basex);
            points.push_back(basey);

            // Vertex 1
            points.push_back(basex + xstep);
            points.push_back(basey);

            // Vertex 2
            points.push_back(basex);
            points.push_back(basey - ystep);

            // Vertex 0
            points.push_back(basex + xstep);
            points.push_back(basey - ystep);

            // Vertex 1
            points.push_back(basex + xstep);
            points.push_back(basey);

            // Vertex 2
            points.push_back(basex);
            points.push_back(basey - ystep);
        }
    }

    return points;
}


std::vector<float> colorTriangles(const std::vector<float>& points) {
    std::vector<float> colors;

    for (size_t i = 0; i < points.size(); i += 2) {
        colors.push_back(rand() % 2); // Randomly assign 0 or 1
        colors.push_back(rand() % 2); 
        colors.push_back(rand() % 2); 
    }

    return colors;
}

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

    ///* create render data in RAM */
    GLuint positionAttribIndex = 0;
    //float positions[] = { 0.0, 0.0,  // 1st vertex
    //                      0.5, 0.0,  // 2nd vertex
    //                      0.5, 0.5
    //};
    std::vector<float> positions = create_box2d(3, 3); 
    std::cout << "Positions: " << positions.size() << "\n";
    for (float x : positions) {
        std::cout << x << ' '; 
    }
    std::cout << "\n";


    ///* create  a vertex array object */
    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    ///* create a buffer for the render data in video RAM */
    GLuint positionsBuffer;
    glCreateBuffers(1, &positionsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionsBuffer);

    ///* declare what data in RAM are filling the bufferin video RAM */
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, positions, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttribIndex);
    ///* specify the data format */
    glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, false, 0, 0);

    ///* create render data in RAM */
    GLuint colorAttribIndex = 1;
    //float colors[] = { 1.0, 0.0, 0.0,    // 1st vertex's color
    //                    0.0, 1.0, 0.0,   // 2nd vertex's color
    //                    0.0, 0.0, 1.0
    //};
    std::vector<float> colors = colorTriangles(positions);
    std::cout << "Colors:\n";
    for (float x : colors) {
        std::cout << x << ' ';
    }
   

    ///* create a buffer for the render data in video RAM */
    GLuint colorBuffer;
    glCreateBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);

    ///* declare what data in RAM are filling the bufferin video RAM */
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9, colors, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float) , colors.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(colorAttribIndex);
    ///* specify the data format */
    glVertexAttribPointer(colorAttribIndex, 3, GL_FLOAT, false, 0, 0);


    ///* create a vertex shader */
    std::string  vertex_shader_src = "#version 460\n \
        in vec2 aPosition;\
        in vec3 aColor;\
        out vec3 vColor;\
        void main(void)\
        {\
         gl_Position = vec4(aPosition, 0.0, 1.0);\
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
        void main(void)\
        {\
            color = vec4(vColor, 1.0);\
        }";
    const GLchar* fs_source = (const GLchar*)fragment_shader_src.c_str();

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
    glGetProgramiv(program_shader, GL_LINK_STATUS, &linked);
    if (linked) {
        glUseProgram(program_shader);
    }


    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, positions.size() / 2  );

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}