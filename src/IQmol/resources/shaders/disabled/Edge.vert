#version 120

varying vec2 texcoord;
varying vec4 color;


void main()
{
   color = gl_Color;
   texcoord = gl_MultiTexCoord0.xy;
   gl_Position = ftransform();
}
