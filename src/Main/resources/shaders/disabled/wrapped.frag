// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Wrapped Diffuse Shader
// A diffuse lighting model which allows you 
// to change the shading to approximate (very
// approximately) global illumination.

uniform vec3 Tint;
uniform float WrapAngle;

varying vec3 N;
varying vec3 L;

void main()
{ 
    float lambert = dot(normalize(L),normalize(N));
    gl_FragColor = vec4(Tint*1-acos(lambert)/WrapAngle,1.0);
}
