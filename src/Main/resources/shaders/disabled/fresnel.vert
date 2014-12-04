// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Refract/reflect combined with facing ratio, and blinn specular
// (written on a plane from Dresden to Munich)

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
