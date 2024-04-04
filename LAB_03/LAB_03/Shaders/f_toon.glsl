// Fragment shader: : Toon shading
// ================
#version 450 core

// Ouput data
out vec4 FragColor;

in vec3 L;
in vec3 N;
in vec3 E;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };

uniform Material material;
uniform PointLight light;

void main()
{
	vec4 a = vec4(0.0,0.5,0.8,1.0);
	vec4 b = vec4(0.0,0.3,0.6,1.0);
	vec4 c = vec4(0.0,0.2,0.5,1.0);
	vec4 d = vec4(0.0,0.3,0.6,1.0);
	vec4 e = vec4(0.0,0.0,0.1,1.0);
	vec4 prova1 = vec4(1.0,0.0,0.0,1.0);
	vec4 prova2 = vec4(0.0,1.0,0.0,1.0);
	vec4 color;
	float intensity = dot(normalize(L),normalize(N));
	if (intensity > 0.95)
		color = a;
	else if (intensity > 0.5)
		color = b;
	else if (intensity > 0.25)
		color = c;
	else if (intensity > 0.15)
		color = d;
	else
		color = e;
	float aa = dot(normalize(E), normalize(N));
	if(aa >= 0.0 && aa < 0.30)
		color = vec4(0.0, 0.0, 0.0, 1.0);

	FragColor = color;
}
