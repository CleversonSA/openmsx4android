uniform mat4 u_mvpMatrix;
uniform vec3 texSize; // not used

//Attribute is deprecated at GLSL ES 3.00
in vec4 a_position;
in vec3 a_texCoord;

//varying is deprecated at GLSL ES 3.00
out vec2 texCoord;
out vec2 videoCoord;

void main()
{
	gl_Position = u_mvpMatrix * a_position;
	texCoord   = a_texCoord.xy;
#if SUPERIMPOSE
	videoCoord = a_texCoord.xz;
#endif
}
