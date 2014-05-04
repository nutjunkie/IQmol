#version 120

// BEGIN UNIFORM
uniform float user_Ambient;                // 0.50
uniform float user_Highlights;             // 0.40
uniform float user_Diffuse;                // 0.60
uniform float user_Shininess;              // 0.90
uniform bool  user_Enhance_Surface_Edges;  // 1
// END UNIFORM
 
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;

 
void main()
{
   vec3 LightPos = gl_LightSource[0].position.xyz;
   color = gl_Color;

   vec3 v = vec3(gl_ModelViewMatrix * gl_Vertex);
   vec3 E = normalize(-v);
   vec3 L = normalize(LightPos - v);
   vec3 N = normalize(gl_NormalMatrix * gl_Normal);
   vec3 R = normalize(2.0 * dot(N, L) * N - L);

   float ambient   = user_Ambient;
   float diffuse   = user_Diffuse*max(0.0, dot(N, L));
   float shininess = max(0.01, 50.0*(1.0-user_Shininess));
   float specular  = user_Highlights*pow(max(0.0, dot(R, E)), shininess);

   viewDirection = -normalize(v.xyz);
   normal = N;

   vec3 rgb = (ambient+diffuse) * gl_Color.rgb + specular * vec3(1.0, 1.0, 1.0);
   color = vec4(rgb, gl_Color.a);
   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}
