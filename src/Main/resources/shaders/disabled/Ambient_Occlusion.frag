#version 120
varying vec4 color;
varying vec3 normal;


void main()
{
   vec3 lightDir = vec3(0.0, 0.0, 1.0);
   vec3 ambientColor = vec3(0.0, 0.0, 0.0);

   float kd = abs( dot( normalize( lightDir ), normal ) );   
   gl_FragColor = vec4( color.rgb * kd + ambientColor, color.a );
}
