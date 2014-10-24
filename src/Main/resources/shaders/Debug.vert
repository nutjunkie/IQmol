#version 120

varying vec4 color;
varying vec3 normal;
varying vec2 texcoord;
varying vec3 viewDirection;
 
void main()
{
    gl_Position   = gl_ModelViewProjectionMatrix*gl_Vertex;
    color         = gl_Color;
    normal        = normalize(gl_NormalMatrix*gl_Normal);
    texcoord      = gl_MultiTexCoord0.xy;
    viewDirection = -normalize(vec3(gl_ModelViewMatrix*gl_Vertex));
}
