#version 120

// BEGIN UNIFORM
uniform float user_Ambient;                // 0.05
uniform float user_Diffuse;                // 0.40
uniform float user_Shininess;              // 0.90
uniform float user_Highlights;             // 0.70
uniform bool  user_Enhance_Surface_Edges;  // 0
// END UNIFORM

varying vec3 normal;
varying vec4 color;
varying vec3 viewDirection;


void main() 
{
   vec4 ecpos = gl_ModelViewMatrix * gl_Vertex;
   gl_ClipVertex = ecpos;
   viewDirection = normalize(vec3(ecpos) / ecpos.w);

   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
   color       = gl_Color;
   normal      = gl_NormalMatrix * gl_Normal;
}

