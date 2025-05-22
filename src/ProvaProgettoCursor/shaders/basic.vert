#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 vColor;
out vec2 vTexCoord;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

void main(void) {
    vTexCoord = aTexCoord ; 
    vColor = aColor;
    gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}