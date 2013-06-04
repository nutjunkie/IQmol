#version 120

// BEGIN UNIFORM
uniform float Ambient;                // 0.50
uniform float Highlights;             // 0.40
uniform float Diffuse;                // 0.60
uniform float Shininess;              // 0.90
uniform bool  Enhance_Surface_Edges;  // 1
// END UNIFORM
 
varying float xray;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;

 
void main( void )
{
   vec3 LightPos = gl_LightSource[0].position.xyz;
   color = gl_Color;

   vec3 v = vec3(gl_ModelViewMatrix * gl_Vertex);
   vec3 E = normalize(-v);
   vec3 L = normalize(LightPos - v);
   vec3 N = normalize(gl_NormalMatrix * gl_Normal);
   vec3 R = normalize(2.0 * dot(N, L) * N - L);

   float ambient   = Ambient;
   float diffuse   = Diffuse*max(0.0, dot(N, L));
   float shininess = max(0.01, 50.0*(1.0-Shininess));
   float specular  = Highlights*pow(max(0.0, dot(R, E)), shininess);

   viewDirection = -normalize(v.xyz);
   normal = N;

   float alpha = gl_Color.a;
   if (Enhance_Surface_Edges && alpha < 0.99) {
      xray = alpha;
   }else {
      xray = 1.0;
   }

   vec3 rgb = (ambient+diffuse) * gl_Color.rgb + specular * vec3(1.0, 1.0, 1.0);
   color = vec4(rgb, alpha);
   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}
