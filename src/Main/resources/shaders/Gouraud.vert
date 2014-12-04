#version 120

// BEGIN UNIFORM
uniform float user_Ambient;                // 0.75
uniform float user_Highlights;             // 0.75
uniform float user_Diffuse;                // 0.75
uniform float user_Shininess;              // 0.75
uniform float user_Saturation;             // 1.00
uniform bool  user_Enhance_Transparency;   // 1
uniform bool  user_Enhance_Edges;          // 0
uniform bool  user_Hemisphere_Lighting;    // 1
// END UNIFORM
 
varying vec4  color;

 
void main()
{
   const vec3 white = vec3(1.0, 1.0, 1.0);

   vec3 vertexPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
   vec3 vertexNormal   = normalize(gl_NormalMatrix * gl_Normal);
   vec3 lightPosition  = gl_LightSource[0].position.xyz;
   vec3 lightDirection = normalize(lightPosition - vertexPosition);
   vec3 viewDirection  = normalize(-vertexPosition);
   vec3 reflectance    = normalize(2.0 * dot(vertexNormal, lightDirection) * vertexNormal
                                    - lightDirection);

   float ambient   = user_Ambient;
   float diffuse   = user_Diffuse*max(0.0, dot(vertexNormal, lightDirection));
   float shininess = max(0.01, 50.0*(1.0-user_Shininess));
   float specular  = user_Highlights*pow(max(0.0, dot(reflectance, viewDirection)), shininess);

   float alpha = gl_Color.a;

   if (user_Enhance_Edges) {
      ambient += (1.0-alpha) * (1.0-ambient);
   }

   if (user_Hemisphere_Lighting) {
      ambient *= (0.5 + 0.5*vertexNormal.y);
   }

   vec3 rgb = (ambient+diffuse) * gl_Color.rgb + specular * white;

   if (user_Enhance_Transparency) {
      float mask = 1.0 - abs(dot(viewDirection, vertexNormal));
      alpha = clamp(mask, alpha, 1.0);
      if (user_Enhance_Edges) {rgb *= alpha; }
   }

   color = vec4(user_Saturation*rgb + (1.0-user_Saturation)*white, alpha);

   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}
