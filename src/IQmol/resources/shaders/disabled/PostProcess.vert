// BEGIN UNIFORM
uniform float TotStrength;  // 0.70
uniform float Strength;     // 0.30
uniform float Offset;       // 0.18
uniform float Falloff;
uniform float Radius;
// END UNIFORM



varying vec2  texcoord;
 
void main(void)
{
gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
gl_Position = sign( gl_Position );
 
// Texture coordinate for screen aligned (in correct range):
//uv = (vec2( gl_Position.x, - gl_Position.y ) + vec2( 1.0 ) ) * 0.5;
texcoord = gl_MultiTexCoord0.xy;
}
