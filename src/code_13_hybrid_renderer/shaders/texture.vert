#version 460 core 
layout (location = 0) in vec3 aPosition; 
 
out vec2 vTexCoord; 
void main(void) 
{
    vTexCoord =  aPosition.xy*0.5+0.5;
    gl_Position = vec4(aPosition, 1.0);
}
