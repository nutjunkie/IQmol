#version 120

// BEGIN UNIFORM
uniform float Ambient;              // 0.05
uniform float Diffuse;              // 0.40
uniform float Shininess;            // 0.90
uniform float Highlights;           // 0.70
uniform bool Enhance_Surface_Edges; // 0
// END UNIFORM

// Outputs to fragment shader
varying vec3 normal;
varying vec4 color;
varying vec3 viewDirection;

varying float xray;
varying float shine;
varying float ambient;
varying float diffuse_scale;
varying float specular_scale;


void main() 
{
   // transform vertex to Eye space for user clipping plane calculations
   vec4 ecpos = gl_ModelViewMatrix * gl_Vertex;
   gl_ClipVertex = ecpos;
   viewDirection = normalize(vec3(ecpos) / ecpos.w);

   // transform vertex to Clip space
   gl_Position = ftransform();

   if (Enhance_Surface_Edges && gl_Color.a < 0.99) {
      xray = gl_Color.a;
   }else {
      xray = 1.0;
   }

   color          = gl_Color;
   normal         = normalize(gl_NormalMatrix * gl_Normal);
   ambient        = 0.5 * Ambient;
   shine          = 100.0 * Shininess;
   diffuse_scale  = 4.0 * Diffuse;
   specular_scale = Highlights;
}

