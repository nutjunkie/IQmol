// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Blinn/Phong Shader
// This is the standard per-fragment lighting
// shading model

varying float ambientScale;
varying float diffuseScale;
varying float specularScale;
varying float roughness;

varying vec4 color;
varying vec3 normal;
varying vec3 V;
varying vec3 L;

void main()
{ 
    vec3 l = normalize(L);
    vec3 n = normalize(normal);
    vec3 v = normalize(V);
    vec3 h = normalize(l+v);

    vec3 AmbientColor = color.rgb;
    vec3 DiffuseColor = color.rgb;
    vec3 SpecularColor = color.rgb;

    float diffuse = dot(l,n);
    float specular = pow(max(0.0,dot(n,h)),1.0/roughness);
    
    gl_FragColor = vec4(AmbientColor*ambientScale+ 
                        DiffuseColor*diffuse*diffuseScale+
                        SpecularColor*specular*specularScale,1);
}
