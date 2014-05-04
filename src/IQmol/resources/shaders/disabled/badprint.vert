// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// BadPrint NPR Shader
// This shader tries to emulate the effect
// of a bad printing process. Can be controlled
// with different settings for RGB

uniform vec3 LightPos;
varying vec3 N;
varying vec3 P;
varying vec4 S;
varying vec3 L;

void main()
{    
    N = normalize(gl_NormalMatrix*gl_Normal);
    P = gl_Vertex.xyz;
    gl_Position = ftransform();
	L = vec3(vec4(LightPos,1)-gl_Position);
    S = gl_ProjectionMatrix*gl_Position;
}
