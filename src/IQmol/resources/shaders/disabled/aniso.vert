// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Anisotropic Specular Reflection Shader
// This shader is useful for depicting surfaces
// such as velvet or brushed metal, as it allows
// you to stretch the highlight along the 
// SpecDirection vector (in object space)

uniform vec3 LightPos;
varying vec3 N;
varying vec3 P;
varying vec3 V;
varying vec3 L;

void main()
{    
    N = normalize(gl_NormalMatrix*gl_Normal);
	P = gl_Vertex.xyz;
    V = -vec3(gl_ModelViewMatrix*gl_Vertex);
	L = vec3(gl_ModelViewMatrix*(vec4(LightPos,1)-gl_Vertex));
    gl_Position = ftransform();
}
