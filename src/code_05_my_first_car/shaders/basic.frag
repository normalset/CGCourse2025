#version 460 core  
out vec4 color; 
in vec3 vColor;
in vec3 vPos;
uniform vec3 uColor;
uniform int uShade;

void main(void) 
{    
   float contrib = 1;	
   if(uShade == 1){
	// this part uses concept we haven't covered yet. Please ignore it
	vec3 N = normalize(cross(dFdx(vPos),dFdy(vPos)));
	vec3 L0 = normalize(vec3(2,2,0)-vPos);
	vec3 L1 = normalize(vec3(-2,1,0)-vPos);
	contrib = (max(0.f,dot(N,L0))+max(0.f,dot(N,L1)))*0.5;
    color = vec4(uColor*contrib, 1.0); 
   }
   else
    color = vec4(vColor, 1.0); 
} 