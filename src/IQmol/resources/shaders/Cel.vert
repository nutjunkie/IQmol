#version 120

// BEGIN UNIFORM
uniform float user_Intensity;  // 0.80
uniform float user_Spread;     // 0.50
uniform bool  user_Antialias;  // 0
// END UNIFORM

varying vec3  normal;
varying vec4  color;


void main()
{
   color = gl_Color;
   normal = gl_NormalMatrix * gl_Normal;
   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}
