// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Facing Ratio Shader
// Blends from inner to outer colour depending
// on the facing ratio of the fragment. Useful for
// getting the scanning electron microscope look.

uniform vec4 OuterColour;
uniform vec4 InnerColour;

varying vec3 N;
varying vec3 V;

void main()
{ 
    float ratio = dot(normalize(V),normalize(N));
    clamp(ratio,0.0,1.0);
    gl_FragColor = mix(OuterColour,InnerColour,ratio);
}
