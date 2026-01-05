uniform mat4 u_mvpMatrix;

//Attribute is deprecated at GLSL ES 3.00
in vec4 a_position;
in vec4 a_color;

//Attribute is deprecated at GLSL ES 3.00
out vec4 v_color;

void main()
{
	gl_Position = u_mvpMatrix * a_position;
	v_color  = a_color;
}
