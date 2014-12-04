// BEGIN UNIFORM
uniform float TotStrength;  // 0.70
uniform float Strength;     // 0.30
uniform float Offset;       // 0.18
uniform float Falloff;
uniform float Radius;
// END UNIFORM


varying vec3 viewDirection;
varying vec2 texcoord;
varying vec4 color;
 
void main()
{
   gl_Position   = sign(gl_ModelViewProjectionMatrix*gl_Vertex);
   texcoord      = gl_MultiTexCoord0.xy;
   color         = gl_Color;
   viewDirection = -normalize(vec3(gl_ModelViewMatrix*gl_Vertex));
}
