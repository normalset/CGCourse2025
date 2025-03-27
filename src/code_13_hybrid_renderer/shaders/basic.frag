
out vec4 color; 
in vec3 vColor;
in vec3 vPosVS;
in vec3 vNormalVS;
in vec3 vLDirVS;

uniform vec3 uLDir;
uniform vec3 uColor;

uniform int	 uShadingMode;

void main(void) 
{    
	PhongMaterial m = PhongMaterial( uAmbientColor,uDiffuseColor,uSpecularColor,uEmissiveColor,uLightColor,uShininess);

	if(uShadingMode == 1){
		vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
		color = vec4(phong(vLDirVS,normalize(-vPosVS),N,m),1.0);
	}
 	else
	if(uShadingMode == 2){
		color = vec4(vColor,1.0);
	}
 	else
	if(uShadingMode == 3){
		color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS),m),1.0);
	}
	else
	/* just output the interpolated vertex normal as color		*/
	/* Note: normal is a vector with values in [-1,-1,-1][1,1,1]*/
	/* and  must be remapped in  in [0,0,0][1,1,1]				*/ 
	color = vec4(normalize(vNormalVS)*0.5+0.5,1.0);
} 