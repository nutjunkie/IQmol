#version 120

// BEGIN UNIFORM
uniform float TotStrength;  // 0.70
uniform float Strength;     // 0.30
uniform float Offset;       // 0.18
uniform float Falloff;
uniform float Radius;
// END UNIFORM

uniform vec2 Scale_xy;
varying vec2 texcoord;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec4 vertex;
 
void main()
{
   gl_Position    =  gl_ModelViewProjectionMatrix*gl_Vertex;
   vertex         =  gl_ModelViewMatrix * gl_Vertex;
   texcoord       =  gl_MultiTexCoord0.xy;
   texcoord1      =  gl_MultiTexCoord1.xy;
   texcoord2      =  gl_MultiTexCoord2.xy;
   gl_TexCoord[0] =  gl_Vertex;
   gl_TexCoord[1] =  vec4(gl_Vertex.xy * Scale_xy, gl_Vertex.zw);

   // Transform the normal to modelview-space
   // vec3 normal = gl_NormalMatrix * gl_Normal;
   // Transform the vertex position To modelview-space
   // vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
   // Calculating the vector from the vertex position To The light position
   // vec3 lightDir = vec3(gl_LightSource[0].position - vertex);
   // http://nehe.gamedev.net/article/glsl_an_introduction/25007/
}

