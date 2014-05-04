#version 120

// BEGIN UNIFORM
uniform float user_Saturation;            // 1.0
uniform bool  user_Enhance_Surface_Edges; // 0
// END UNIFORM

varying vec4 color;
varying vec3 normal;
varying vec3 viewDirection;

void main()
{
   const vec3 one = vec3(1.0, 1.0, 1.0);
   const vec3 eye = vec3(0.0, 0.0, 0.0);

   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
   vec4 vertexPosition = gl_ModelViewMatrix*gl_Vertex;
   viewDirection = normalize(eye-vertexPosition.xyz);

   color  = vec4(user_Saturation*gl_Color.rgb + (1.0 - user_Saturation)*one, gl_Color.a);
   normal = normalize(gl_NormalMatrix*gl_Normal);
}
