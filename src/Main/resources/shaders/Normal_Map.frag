#version 120

varying vec3  normal;
varying float depth;

void main()
{
   vec3 norm = 0.5*normalize(normal)+0.5;
   float z = clamp(depth, 0.0, 1.0);
   gl_FragColor = vec4(norm,z);
}

