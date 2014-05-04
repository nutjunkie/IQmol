varying vec4 pos;
varying vec3 wpos;
varying vec3 normal;
varying vec4 color;
void main()
{
  //uniform float factor;
  float factor = 0.24;

        pos = gl_Vertex;
        pos.xyz /= gl_Vertex.w;
        pos.xyz *= factor;
        normal = normalize( gl_NormalMatrix * gl_Normal );
        color = gl_Color;
        gl_Position = ftransform();
        wpos = gl_Position.xyz / gl_Position.w;
        /*pos = gl_Position;*/
        
}
