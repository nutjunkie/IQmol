#version 120

varying vec3 Normal;

void main()
{
	Normal = gl_NormalMatrix * gl_Normal;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
