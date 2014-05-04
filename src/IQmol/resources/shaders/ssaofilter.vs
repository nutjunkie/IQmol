#version 120

void main()
{
	gl_TexCoord[0] = gl_Vertex;
	gl_Position = gl_Vertex * 2.0 - 1.0;
}
