// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Lambertian Shading Model
// Simple diffuse only shader

uniform vec3 Tint;
varying vec3 N;
varying vec3 L;

void main()
{ 
    float lambert = dot(normalize(L),normalize(N));
    gl_FragColor = vec4(Tint*lambert,1.0);
}
