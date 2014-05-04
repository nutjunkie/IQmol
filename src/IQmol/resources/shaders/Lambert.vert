#version 120

// BEGIN UNIFORM
uniform float user_Saturation;            // 0.7
uniform float user_Ambient;               // 0.2
uniform bool  user_Enhance_Surface_Edges; // 1
// END UNIFORM

varying vec4 color;
varying vec3 normal;
varying vec3 viewDirection;


void main()
{
   const vec3 one = vec3(1.0, 1.0, 1.0);

   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
   
   vec4 vertexPosition = gl_ModelViewMatrix*gl_Vertex;
   viewDirection = -normalize(vertexPosition.xyz);

   color  = vec4(user_Saturation*gl_Color.rgb + (1.0-user_Saturation)*one, gl_Color.a);
   normal = normalize(gl_NormalMatrix*gl_Normal);
}
