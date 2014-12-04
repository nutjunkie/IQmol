// Copyright (C) 2007 Dave Griffiths
// Fluxus Shader Library
// ---------------------
// Refract/reflect combined with facing ratio, and blinn specular
// (written on a plane from Dresden to Munich)

uniform samplerCube Texture;
uniform float RefractionIndex;
uniform vec3 SpecularColour;
uniform float Roughness;
uniform float SpecularIntensity;

varying vec3 V;
varying vec3 N;
varying vec3 L;

void main()
{
  vec3 v = normalize(V);
  vec3 i = -v;
  vec3 n = normalize(N);
  vec3 l = normalize(L);
  vec3 h = normalize(l+v);

  vec3 Refracted = refract(i,n,RefractionIndex);
  Refracted = vec3(gl_TextureMatrix[0] * vec4(Refracted,1.0));
  vec3 Reflected = reflect(i,n);
  Reflected = vec3(gl_TextureMatrix[0] * vec4(Reflected,1.0));

  float specular = pow(max(0.0,dot(n,h)),1/Roughness);
    
  vec3 refractColor = SpecularColour*specular*SpecularIntensity + 
                          mix(vec3(textureCube(Texture,Reflected)),
						  vec3(textureCube(Texture,Refracted)),
						  dot(n,v));

	
  gl_FragColor = vec4(refractColor,1.0);
}
