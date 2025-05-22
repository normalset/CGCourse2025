#version 460 core
in vec3 vColor;
in vec2 vTexCoord;

out vec4 fragColor;

uniform vec3 uColor;
uniform sampler2D uTexture;
uniform int uUseTexture;

void main(void)
{
    if (uUseTexture == 1) {
        fragColor = texture(uTexture, vTexCoord.xy);
    } else if (uColor.x < 0) {
        fragColor = vec4(vColor, 1.0);
    } else {
        fragColor = vec4(uColor, 1.0);
    }
} 