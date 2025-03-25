 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 
 
out vec3 vLDirVS;
out vec3 vPosVS;
out vec3 vNormalVS;
out vec3 vColor;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uLDir;

uniform int	 uShadingMode;

void main(void) 
{ 
	PhongMaterial m = PhongMaterial( uAmbientColor,uDiffuseColor,uSpecularColor,uEmissiveColor,uLightColor,uShininess);

	vLDirVS   =  (uView*vec4(uLDir,0.f)).xyz; 

	mat3 normalMatrix = transpose(inverse(mat3(uModel)));
	vNormalVS =  (uView* vec4(normalMatrix*aNormal, 0.0)).xyz; 
 

	vPosVS = (uView*uModel*vec4(aPosition, 1.0)).xyz; 
	
	/* compute lighiting in the vertex shader (Gauraud shading) */
	vColor    = phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS),m);

    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 
}