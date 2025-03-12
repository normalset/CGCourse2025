#version 410 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 1) in vec3 aColor; 
layout (location = 4) in vec2 aTexCoord;
 
out vec2 vTexCoord;
out vec3 vLdirVS;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec4 uLdir;
uniform int uRenderMode;


void main(void) 
{ 
	vec3 ViewVS  =  (vec4(0.0,0.0,0.0,1.0) -uView*uModel*vec4(aPosition, 1.0)).xyz; 

	vLdirVS   = (uView*uLdir).xyz;
	vTexCoord = aTexCoord;
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0);
}
