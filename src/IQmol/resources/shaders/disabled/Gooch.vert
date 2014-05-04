#version 120

// BEGIN UNIFORM
uniform float Outline; // 0.20
// END UNIFORM

varying vec3 N;
varying vec3 V;
varying vec3 L;
varying float outline;


void main()
{    

   vec3 LightPos = vec3(0.0, 0.0, 10.0);
   outline = Outline;

    N = normalize(gl_NormalMatrix*gl_Normal);
    V = -vec3(gl_ModelViewMatrix*gl_Vertex);
	L = vec3(gl_ModelViewMatrix*(vec4(LightPos,1)-gl_Vertex));
    gl_Position = ftransform();
}
