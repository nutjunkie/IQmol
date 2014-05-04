#version 120

varying vec3 Normal;

void main()
{
	gl_FragColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);
}
