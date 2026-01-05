uniform sampler2D tex;
uniform sampler2D videoTex;

//varying is deprecated at GLSL ES 3.00
in vec2 texCoord;
in vec2 videoCoord;

//gl_FragColor is deprecated at GLSL ES 3.00
out vec4 fragColor;

void main()
{
#if SUPERIMPOSE
	vec4 col = texture(tex, texCoord);
	vec4 vid = texture(videoTex, videoCoord);
	fragColor = mix(vid, col, col.a);
#else
    //texture2D is deprecated at GLSL ES 3.00
    fragColor = texture(tex, texCoord);
#endif
}
