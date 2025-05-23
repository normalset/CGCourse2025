#pragma once
#include <GL/glew.h>
#include <string>
#include <iostream>



struct texture {
	texture() { }
	~texture() {  }

	int x_size, y_size;
	int n_components;
	GLuint id;
	GLuint load(std::string name, GLuint tu) {
		unsigned char * data;
		data = stbi_load(name.c_str(), &x_size, &y_size, &n_components, 0);
		if (!data) {
			std::cerr << "Failed to load texture: " << name << std::endl;
			return 0;
		}
		stbi__vertical_flip(data, x_size, y_size, n_components);
		glActiveTexture(GL_TEXTURE0 + tu);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		int channels;
		switch (n_components) {
		case 1: channels = GL_RED; break;
		case 3: channels = GL_RGB; break;
		case 4: channels = GL_RGBA; break;
		default: 
			std::cerr << "Unsupported number of components: " << n_components << std::endl;
			stbi_image_free(data);
			return 0;
		}
		
		// Set proper internal format based on components
		GLint internalFormat;
		switch (n_components) {
		case 1: internalFormat = GL_RED; break;
		case 3: internalFormat = GL_RGB8; break;
		case 4: internalFormat = GL_RGBA8; break;
		default: internalFormat = GL_RGB8;
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		
		// Set proper texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
		
		// Enable anisotropic filtering if available
		if (GLEW_EXT_texture_filter_anisotropic) {
			float maxAniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
		}
		
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL Error in texture loading: " << error << std::endl;
		}
		
		return id;
	}

	GLuint create(int x_size, int y_size, GLuint channels) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		return id;
	}

	GLuint load_cubemap(std::string posx,std::string negx,
		std::string posy,std::string negy, 
		std::string posz,std::string negz, 
		GLuint tu) {
		unsigned char * data[6];
		data[0] = stbi_load(posx.c_str(), &x_size, &y_size, &n_components, 0);
		data[1] = stbi_load(negx.c_str(), &x_size, &y_size, &n_components, 0);
		data[2] = stbi_load(posy.c_str(), &x_size, &y_size, &n_components, 0);
		data[3] = stbi_load(negy.c_str(), &x_size, &y_size, &n_components, 0);
		data[4] = stbi_load(posz.c_str(), &x_size, &y_size, &n_components, 0);
		data[5] = stbi_load(negz.c_str(), &x_size, &y_size, &n_components, 0);

		glActiveTexture(GL_TEXTURE0 + tu);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		int channels;
		switch (n_components) {
		case 1: channels = GL_RED; break;
		case 3: channels = GL_RGB; break;
		case 4: channels = GL_RGBA; break;
		default: assert(0);
		}
		for(unsigned int i = 0;i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, data[i]);
	
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		for (unsigned int i = 0; i < 6; ++i)
			stbi_image_free(data[i]);
		return id;

	}

	GLuint create_cubemap(int x_size, int y_size,int n_components) {

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		int channels;
		switch (n_components) {
		case 1: channels = GL_RED; break;
		case 3: channels = GL_RGB; break;
		case 4: channels = GL_RGBA; break;
		default: assert(0);
		}
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return id;
	}
};
