#version 120

// BEGIN UNIFORM
uniform float Intensity;  // 0.80
uniform float Spread;     // 0.50
// END UNIFORM

varying vec3  normal;
varying vec4  color;
varying float scale;
varying float spread;

void main()
{
   scale = 0.5*Intensity;
   spread = Spread;
   color = gl_Color;
   normal = gl_NormalMatrix * gl_Normal;
   gl_Position = ftransform();
}
