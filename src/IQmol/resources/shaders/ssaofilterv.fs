#version 120

uniform sampler2D SSAOTexture;
uniform float sy;

void main()
{
	float SSAO = 0.0;
	
	for(int y = -2; y <= 2; y++)
	{
		SSAO += texture2D(SSAOTexture, vec2(gl_TexCoord[0].s, y * sy + gl_TexCoord[0].t)).r * (3.0 - abs(float(y)));
	}
	
	gl_FragColor = vec4(vec3(SSAO / 9.0), 1.0);
}
