out vec4 FragColor;
in vec3 Normal;
in vec3 Position;
uniform vec3 camera_position;
uniform samplerCube cubemap;

void main()
{ 
	vec3 E = normalize(Position - camera_position);
	float ratio = 1.00 / 1.52;
	vec3 R = refract(E, normalize(Normal), ratio);
	FragColor = texture(cubemap, R);
}
