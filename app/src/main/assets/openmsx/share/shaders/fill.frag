in vec4 v_color;

//gl_FragColor is deprecated at GLSL ES 3.00
out vec4 fragColor;

void main()
{
	fragColor = v_color;

}
