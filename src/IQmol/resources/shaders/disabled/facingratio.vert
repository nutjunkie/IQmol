
// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Facing Ratio Shader
// Blends from inner to outer colour depending
// on the facing ratio of the fragment. Useful for
// getting the scanning electron microscope look.

varying vec3 N;
varying vec3 V;

void main()
{    
    N = normalize(gl_NormalMatrix*gl_Normal); 
	V = -vec3(gl_ModelViewMatrix*gl_Vertex);
    gl_Position = ftransform();
}
