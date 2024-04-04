// Vertex shader: Wave shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform float t; // in milliseconds

out vec3 Color; // resulting color from lighting calculations

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material material;

void main() {
	float a = 0.1;
	float w = 0.001;
	vec4 v = vec4(aPos,1.0);
	v.y = a*sin(w*t + 10*v.x)*sin(w*t + 10*v.z),
	gl_Position = P * V * M * v;
	
	// Transform vertex  position into eye (VCS) coordinates
    vec4 eyePosition = V * M * v;
	// Transform Light  position into eye (VCS) coordinates 
	vec4 eyeLightPos = V * vec4(light.position, 1.0);
    // Transform vertex normal into VCS
    vec3 N = normalize(transpose(inverse(mat3(V * M))) * aNormal);

	// Compute vectors E,L,R in VCS
	vec3 E = normalize(-eyePosition.xyz);
	vec3 L = normalize((eyeLightPos - eyePosition).xyz);
    vec3 R = reflect(-L, N);  

    // ambient
    vec3 ambient = light.power * material.ambient;
  	
    // diffuse 
    float diff = max(dot(L,N), 0.0);
    vec3 diffuse = light.power * light.color * diff * material.diffuse;

    // specular
    float spec = pow(max(dot(E, R), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular;  

    Color = ambient + diffuse + specular;
}