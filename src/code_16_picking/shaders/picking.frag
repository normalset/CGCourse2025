#version 460 core  
out vec4 color; 
in vec3 vPos;
uniform vec3 uColor;

void main(void) 
{    
   color = vec4(uColor, 1.0); 
} 