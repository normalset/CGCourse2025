#version 330 core  
out vec4 color; 

in vec4 vProjTexCoord;
in vec4 vViewDirWS;
in vec3 vLdirVS;
in vec3 vNormalVS;
in vec3 vNormalWS;

uniform int uRenderMode;

uniform sampler2D  uColorImage;
uniform samplerCube uSkybox;
uniform samplerCube uReflectionMap;


// this produce the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}

void main(void) 
{ 
	if(uRenderMode==0) // just diffuse gray
		color = vec4(vec3(max(0.0,dot(normalize(vLdirVS),normalize(vNormalVS)))),1.0);
	else
	if(uRenderMode==1){ // projective texture
		vec2 tc = ( (vProjTexCoord/vProjTexCoord.w).xy *0.5+0.5) ;
		vec4 c = texture(uColorImage,tc.xy); 
		vec3 cc =  vec3(max(0.0,dot(normalize(vLdirVS),normalize(vNormalVS)))) + c.xyz*c.w; 
		color = vec4(cc,1.0);
		}else
	if(uRenderMode==2){ // show the skybox 
		color = texture(uSkybox,normalize(vViewDirWS.xyz)); 
		}else
	if(uRenderMode == 3){ // reflection
		//-vViewDirWS.xyz+ 2*dot(vViewDirWS.xyz,vNormalWS)*vNormalWS
		vec3 r = reflect(normalize(vViewDirWS.xyz),normalize(vNormalWS));
		color = texture(uSkybox,r); 
	}else
	if(uRenderMode == 4){// refraction
		vec3 r = refract(normalize(vViewDirWS.xyz),normalize(vNormalWS),1.02);
		color = texture(uSkybox,r); 
	}
	if(uRenderMode == 5){ // on-the-fly environment map
		vec3 r = reflect(normalize(vViewDirWS.xyz),normalize(vNormalWS));
		color = texture(uReflectionMap,r);
	}
} 