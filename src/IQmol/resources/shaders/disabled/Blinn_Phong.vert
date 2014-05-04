// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Blinn/Phong Shader
// This is the standard per-fragment lighting
// shading model

// BEGIN UNIFORM
uniform float AmbientIntensity;   // 0.2
uniform float DiffuseIntensity;   // 0.2
uniform float SpecularIntensity;  // 0.2
uniform float Roughness;          // 1.0
// END UNIFORM



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
    vec3 LightPos = vec3(0.0, 0.0, 1.0);

    ambientScale  = AmbientIntensity;
    diffuseScale  = DiffuseIntensity;
    specularScale = SpecularIntensity;
    roughness     = Roughness;

    normal = normalize(gl_NormalMatrix*gl_Normal);

    V = -vec3(gl_ModelViewMatrix*gl_Vertex);
	L = vec3(gl_ModelViewMatrix*(vec4(LightPos,1)-gl_Vertex));

    color = gl_Color;
    gl_Position = ftransform();
}
