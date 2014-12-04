#version 120

uniform float Far;
uniform float Near;

varying vec3 normal;
varying float depth;


void main()
{
   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
   normal = gl_NormalMatrix * gl_Normal;
   vec3 pos = vec3(gl_ModelViewMatrix*gl_Vertex);
   depth = (-pos.z-Near)/(Far-Near);
}
