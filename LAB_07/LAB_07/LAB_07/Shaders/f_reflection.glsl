out vec4 FragColor;
in vec3 Normal;
in vec3 Position;
uniform vec3 camera_position;
uniform samplerCube cubemap;

void main() 
{
	vec3 E = normalize(Position - camera_position);
	vec3 R = reflect(E, normalize(Normal));
	FragColor = texture(cubemap, R);
}
