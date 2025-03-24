#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
// #include <conio.h>
// #include <direct.h>
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

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "../common/gltf_loader.h"


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

/* trackballs for controlloing the scene (0) or the light direction (1) */
trackball tb[2];

/* which trackball is currently used */
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view ;


/* object that will be rendered in this scene*/
renderable r_frame, r_plane,r_line,r_torus;

/* program shaders used */
shader texture_shader,flat_shader;


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
	if(curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
 /* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
 if(action == GLFW_PRESS)
	 curr_tb = 1 - curr_tb;

}
/* callback function called when the windows is resized */
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

}


void print_info() {

	std::cout << "press left mouse button to control the trackball\n" ;
	std::cout << "press any key to switch between world and light control\n";
}


char image_name[65536] = { "../../src/code_10_texture/textures/brick_wall2-diff-512.png" };
GLuint texture_id;

GLuint  load_texture(std::string  imgname) {
	
	GLuint id;
	int x_size, y_size, n_components;
	unsigned char* data;
	data = stbi_load(imgname.c_str(), &x_size, &y_size, &n_components, 0);
	stbi__vertical_flip(data, x_size, y_size, n_components);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	int channels;
	switch (n_components) {
	case 1: channels = GL_RED; break;
	case 3: channels = GL_RGB; break;
	case 4: channels = GL_RGBA; break;
	default: assert(0);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	return id;
}

int selected = 0;
int selected_mesh = 0;

void gui_setup() {
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Model")) {
		if (ImGui::Selectable("plane", selected_mesh == 0)) selected_mesh = 0;
		if (ImGui::Selectable("torus", selected_mesh == 1)) selected_mesh = 1;
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Texture mode"))
	{
		if (ImGui::BeginMenu("Choose Images")) {
			ImGui::Text("Set "); ImGui::SameLine(); ImGui::InputText("as diffuse_map", image_name, 65536);

			if (ImGui::Button("Set these images as textures")) {
				texture_id =  load_texture(image_name);
			}
			ImGui::EndMenu();
		}

		if (ImGui::Selectable("Show texture coordinates", selected == 0)) selected = 0;
		if (ImGui::Selectable("Color only", selected == 1)) selected = 1;
		if (ImGui::Selectable("MipMap Levels", selected == 2)) selected = 2;
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Trackball")) {
		if (ImGui::Selectable("control scene", curr_tb == 0)) curr_tb = 0;
		if (ImGui::Selectable("control ligth", curr_tb == 1)) curr_tb = 1;

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	width = 1000;
	height = 800;
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "code_10_texture", NULL, NULL);

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
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	
	printout_opengl_glsl_info();


	/* load the shaders */
	texture_shader.create_program("./shaders/texture.vert","./shaders/texture.frag");
	flat_shader.create_program("./shaders/flat.vert","./shaders/flat.frag");

	/* Set the uT matrix to Identity */
	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/* create a  long line*/
	r_line = shape_maker::line(100.f);

	/* create 3 lines showing the reference frame*/
	r_frame = shape_maker::frame(4.0);
	
	/* create a rectangle*/
	shape s_plane;
	shape_maker::rectangle(s_plane, 1, 1);
	s_plane.to_renderable(r_plane);

	shape s_torus;
	shape_maker::torus(s_torus, 0.5, 1, 50, 50);
	s_torus.to_renderable(r_torus);

	/* initial light direction */
	Ldir = glm::vec4(0.0, 1.0, 0.0, 0.0);

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
	view = glm::lookAt(glm::vec3(0, 3, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);

	glUniform4f(texture_shader["uDiffuseColor"], 0.8f, 0.8f, 0.8f,1.f);
	glUniform1i(texture_shader["uColorImage"], 0);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);


	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniform3f(flat_shader["uColor"], 1.0, 1.0, 1.0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	check_gl_errors(__LINE__, __FILE__, true);

	print_info();

	matrix_stack stack;

	/* set the trackball position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	curr_tb = 0;

	/* define the viewport  */
	glViewport(0, 0, width, height);

	texture_id = load_texture(image_name);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		gui_setup();

		/* light direction transformed by the trackball tb[1]*/
		glm::vec4 curr_Ldir = tb[1].matrix()*Ldir;

		stack.push();
		stack.mult(tb[0].matrix());

		stack.push();

		glUseProgram(texture_shader.program);

		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform4fv(texture_shader["uLdir"], 1, &curr_Ldir[0]);
		glUniform1i(texture_shader["uRenderMode"], selected);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id );

		switch (selected_mesh) {
			case 0: r_plane.bind(); glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);break;
			case 1: r_torus.bind(); glDrawElements(r_torus().mode, r_torus().count, r_torus().itype, 0);break;
		}
		stack.pop();
		
		// render the reference frame
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], -1.0, 1.0, 1.0);

		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);
		glUseProgram(0);

		check_gl_errors( __LINE__,__FILE__,true);
		stack.pop();

		// render the light direction
		stack.push();
		stack.mult(tb[1].matrix());
		 
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], 1.0,1.0,1.0);
		r_line.bind();
		glDrawArrays(GL_LINES, 0, 2);
		glUseProgram(0);

		stack.pop();


		check_gl_errors(__LINE__, __FILE__);
	
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
