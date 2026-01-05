uniform sampler2D u_tex;

//varying is deprecated at GLSL ES 3.00
in float v_color;
in vec2 v_texCoord;

//gl_FragColor is deprecated at GLSL ES 3.00
out vec4 fragColor;

void main()
{
    //texture2D is deprecated at GLSL ES 3.00
	fragColor = v_color * texture(u_tex, v_texCoord);
}
