#version 120

uniform vec2 sxy;

void main()
{
	gl_TexCoord[0] = gl_Vertex;
	gl_TexCoord[1] = vec4(gl_Vertex.xy * sxy, gl_Vertex.zw);
	gl_Position = gl_Vertex * 2.0 - 1.0;
}
