uniform mat4 u_mvpMatrix;
uniform mat3 u_normalMatrix;

//Attribute is deprecated at GLSL ES 3.00
in vec3 a_position;
in vec3 a_normal;
in vec2 a_texCoord;

//varying is deprecated at GLSL ES 3.00
out float v_color;
out vec2 v_texCoord;

void main()
{
	gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
	v_texCoord = a_texCoord;
	v_color = (u_normalMatrix * a_normal).z;
}
