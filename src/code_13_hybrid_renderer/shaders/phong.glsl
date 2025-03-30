
struct PhongMaterial{
 vec3  ambient_color;
 vec3  diffuse_color;
 vec3  specular_color;
 vec3  emissive_color;
 vec3  light_color;
 float shininess;
};
/* phong lighting */
vec3 phong ( vec3 L, vec3 V, vec3 N, PhongMaterial m){

	N = normalize(N);
	V = normalize(V);

	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N;
	
	float spec = ((LN>0.f)?1.f:0.f) * pow(max(0.0,dot(V,R)) ,m.shininess);
	
	return (m.ambient_color+LN*m.diffuse_color + spec * m.specular_color)*m.light_color;
}
