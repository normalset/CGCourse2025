#version 460 core  
out vec4 color; 
in vec2 vTexCoord; 
uniform sampler2D uColorImage;

void main(void) 
{ 
	color = texture2D(uColorImage,vTexCoord);
} 