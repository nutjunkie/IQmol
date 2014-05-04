// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Anisotropic Specular Reflection Shader
// This shader is useful for depicting surfaces
// such as velvet or brushed metal, as it allows
// you to stretch the highlight along the 
// SpecDirection vector (in object space)

uniform vec3 AmbientColour;
uniform vec3 DiffuseColour;
uniform vec3 SpecularColour;
uniform float AmbientIntensity;
uniform float DiffuseIntensity;
uniform float SpecularIntensity;
uniform float Roughness;
uniform float AnisoRoughness;
uniform vec3 SpecDirection;

varying vec3 N;
varying vec3 P;
varying vec3 V;
varying vec3 L;
    
void main()
{     
    vec3 l = normalize(L);
    vec3 n = normalize(N);
    vec3 v = normalize(V);
    vec3 t = cross(n,normalize(SpecDirection));
    vec3 h = normalize(l+v);
   
    float diffuse = dot(l,n);
    float specular = pow(dot(n,h),1/Roughness);
    
    // Heidrich-Seidel anisotropic distribution
    float ldott = dot(l,t);
    float vdott = dot(v,t);

    float aniso = pow(sin(ldott)*sin(vdott) +
                      cos(ldott)*cos(vdott),1/AnisoRoughness);

    gl_FragColor = vec4(AmbientColour*AmbientIntensity + 
                        DiffuseColour*diffuse*DiffuseIntensity +
                        SpecularColour*aniso*specular*
                        SpecularIntensity,1);
}
