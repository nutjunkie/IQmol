// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Toon (Cel) NPR Shader
// Standard toon model with configurable
// colours for the different shading areas

uniform vec4 HighlightColour;
uniform vec4 MidColour;
uniform vec4 ShadowColour;
uniform float HighlightSize;
uniform float ShadowSize;
uniform float OutlineWidth;

varying vec3 N;
varying vec3 L;
varying vec3 V;

void main()
{ 
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	vec3 v = normalize(V);
	
    float lambert = dot(l,n);
    vec4 colour = MidColour;
    if (lambert>1-HighlightSize) colour = HighlightColour;
    if (lambert<ShadowSize) colour = ShadowColour;
    if (dot(n,v)<OutlineWidth) colour = vec4(0,0,0,1);

    gl_FragColor = colour;
}
