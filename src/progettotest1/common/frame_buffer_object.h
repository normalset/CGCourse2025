#include <GL/glew.h>
#include <iostream>



struct frame_buffer_object {

	int w, h;
	GLuint id_fbo, id_tex,id_tex1, id_depth;
	bool use_texture_for_depth;

	void check(int fboStatus)
	{
		switch (fboStatus) {
		case GL_FRAMEBUFFER_COMPLETE:break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          std::cout<<"FBO Incomplete: Attachment\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  std::cout << "FBO Incomplete: Missing Attachment\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:      std::cout << "FBO Incomplete: Dimensions\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:         std::cout << "FBO Incomplete: Formats\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         std::cout << "FBO Incomplete: Draw Buffer\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         std::cout << "FBO Incomplete: Read Buffer\n"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:                    std::cout << "FBO Unsupported\n"; break;
		default:                                            std::cout << "Undefined FBO error\n"; break;
		}
	}


	void create(int w_, int h_,bool _use_texture_for_depth = false)
	{
		if ((w == w_) && (h == h_)&& _use_texture_for_depth== use_texture_for_depth)
			return;
		w = w_;
		h = h_;
		
		// Delete existing FBO and textures if they exist
		if (id_fbo) {
			glDeleteFramebuffers(1, &id_fbo);
			glDeleteTextures(1, &id_tex);
			glDeleteTextures(1, &id_tex1);
			if (use_texture_for_depth) {
				glDeleteTextures(1, &id_depth);
			} else {
				glDeleteRenderbuffers(1, &id_depth);
			}
		}
		
		glGenFramebuffers(1, &this->id_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, this->id_fbo);

		/* texture for color attachment*/
		glGenTextures(1, &this->id_tex);
		glBindTexture(GL_TEXTURE_2D, this->id_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->id_tex, 0);

		/* texture for color attachment 1*/
		glGenTextures(1, &this->id_tex1);
		glBindTexture(GL_TEXTURE_2D, this->id_tex1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->id_tex1, 0);

		check_gl_errors(__LINE__, __FILE__, true);
		use_texture_for_depth = _use_texture_for_depth;
		if (_use_texture_for_depth) {
			/* texture for depth attachment*/
			glGenTextures(1, &this->id_depth);
			glBindTexture(GL_TEXTURE_2D, this->id_depth);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->id_depth, 0);
		}
		else {
			/* render buffer for depth attachment*/
			glGenRenderbuffers(1, &this->id_depth);
			glBindRenderbuffer(GL_RENDERBUFFER, this->id_depth);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->id_depth);
		}

		// Specify draw buffers
		GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		check_gl_errors(__LINE__, __FILE__, true);
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		check(status);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Framebuffer is not complete!" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void remove()
	{
		glDeleteFramebuffers(1, &this->id_fbo);
		if(use_texture_for_depth)
			glDeleteTextures(1, &this->id_depth);
		else
			glDeleteRenderbuffers(1, &this->id_depth);
	}
};


