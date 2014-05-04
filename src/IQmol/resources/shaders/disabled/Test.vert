#version 120

// BEGIN UNIFORM
uniform float Saturation;   // 0.7
uniform float Ambient;      // 0.2
uniform float Min_Opacity;  // 0.0
uniform float Max_Opacity;  // 1.0
// END UNIFORM

varying vec4  color;
varying vec3  normal;
varying vec3  ambientColor;

varying float alpha;
varying float minOpacity;
varying float maxOpacity;
varying vec3  viewDirection;


void main()
{
   const vec3 one = vec3(1.0, 1.0, 1.0);
   const vec3 eyePosition = vec3(0.0, 0.0, 0.0);

   gl_Position = ftransform();
   normal = normalize(gl_NormalMatrix*gl_Normal);

   ambientColor = 0.5*Ambient*one;

   vec4 vertexPosition = gl_ModelViewMatrix*gl_Vertex;
   viewDirection = normalize(eyePosition-vertexPosition.xyz);


   alpha = gl_Color.a;
   color = vec4(Saturation*gl_Color.rgb + (1.0-Saturation)*one, gl_Color.a);

   maxOpacity = Max_Opacity;
   minOpacity = Min_Opacity;
}
