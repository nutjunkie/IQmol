#version 120

// BEGIN UNIFORM
uniform float Saturation;            // 1.0
uniform bool  Enhance_Surface_Edges; // 0
// END UNIFORM

varying float xray;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;

void main()
{
   const vec3 one = vec3(1.0, 1.0, 1.0);
   const vec3 eye = vec3(0.0, 0.0, 0.0);

   gl_Position = ftransform();

   vec4 vertexPosition = gl_ModelViewMatrix*gl_Vertex;
   viewDirection = normalize(eye-vertexPosition.xyz);

   float alpha = gl_Color.a;

   if (Enhance_Surface_Edges && alpha < 0.99) {
      xray = alpha;
   }else {
      xray = 1.0;
   }


   color  = vec4(Saturation*gl_Color.rgb + (1.0-Saturation)*one, alpha);
   normal = normalize(gl_NormalMatrix*gl_Normal);
}
