#version 410 core  
out vec4 color; 

in vec2 vTexCoord;
in vec3 vLdirVS;

uniform int uRenderMode;
uniform vec4 uDiffuseColor;

uniform sampler2D uColorImage;

// this produces the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}

// Custom function to compute the LOD using dFdx and dFdy


void main(void) 
{ 
	if(uRenderMode==0)
		color = vec4(vTexCoord,0.0,1.0);
	else
	if(uRenderMode==1)
		color = texture2D(uColorImage,vTexCoord.xy);
		else
	if(uRenderMode==2) // mip mapping levels
	{
		color = vec4(hsv2rgb( floor(textureQueryLod(uColorImage,vTexCoord.xy)).y/9),1);
	}else
		color = vec4(1.0,.0,0.0,1.0);
} 