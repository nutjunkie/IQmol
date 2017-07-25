#version 120

// BEGIN UNIFORM
uniform float user_Ambient;                // 0.50
uniform float user_Diffuse;                // 0.60
uniform float user_Highlights;             // 0.40
uniform float user_Shininess;              // 0.90

uniform float user_Saturation;             // 1.00
uniform float user_Noise_Intensity;        // 0.00
uniform float user_Fog_Strength;           // 0.00

uniform bool  user_Enhance_Transparency;   // 1
uniform bool  user_Enhance_Edges;          // 0
uniform bool  user_Hemisphere_Lighting;    // 1

uniform bool  user_light_Front;            // 1
uniform bool  user_light_Highlight;        // 1
uniform bool  user_light_Left;             // 0
uniform bool  user_light_Lower;            // 0
uniform vec4  backgroundColor; 
// END UNIFORM
 
varying float shine;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;
varying vec3  v_texCoord3D;
varying float fogFactor;


void main()
{
   const vec3 white = vec3(1.0);
   const vec4 lightDirection0 = vec4( 0.4,  0.0,  1.0, 1.0);  // similar to gl_LightSource[0]
   const vec4 lightDirection1 = vec4( 0.3,  0.8, -0.5, 1.0);  // good
   const vec4 lightDirection2 = vec4(-0.5,  0.0,  0.0, 1.0);
   const vec4 lightDirection3 = vec4( 0.0, -1.0,  0.2, 1.0);

   vec3 vertexPosition = vec3(gl_ModelViewMatrix * gl_Vertex);  // in eye coordinates
   vec3 vertexNormal   = normalize(gl_NormalMatrix * gl_Normal);

   float alpha   = gl_Color.a;
   float ambient = user_Ambient;
   float diffuse = 0.0;

   if (user_light_Front) {
      vec3 lightDirection  = normalize(lightDirection0.xyz);          // deprecate
      diffuse += lightDirection0.w * max(0.0, dot(lightDirection, vertexNormal));
   }
   if (user_light_Highlight) {
      vec3 lightDirection  = normalize(lightDirection1.xyz);          // deprecate
      diffuse += lightDirection1.w * max(0.0, dot(lightDirection, vertexNormal));
   }
   if (user_light_Left) {
      vec3 lightDirection  = normalize(lightDirection2.xyz);          // deprecate
      diffuse += lightDirection2.w * max(0.0, dot(lightDirection, vertexNormal));
   }
   if (user_light_Lower) {
      vec3 lightDirection  = normalize(lightDirection3.xyz);          // deprecate
      diffuse += lightDirection3.w * max(0.0, dot(lightDirection, vertexNormal));
   }
   diffuse *= user_Diffuse;

   if (user_Enhance_Edges) {
      ambient += (1.0-alpha) * (1.0-ambient);
   }
   if (user_Hemisphere_Lighting) {
      ambient *= (0.5 + 0.5*vertexNormal.y);
   }

   vec3 rgb = gl_Color.rgb;
   rgb *= (ambient+diffuse); 
   rgb  = user_Saturation*rgb +  (1.0-user_Saturation)*white;

   shine         = max(0.01, 50.0*(1.0-user_Shininess));
   color         = vec4(rgb, alpha);
   normal        = vertexNormal;
   viewDirection = normalize(vertexPosition);
   v_texCoord3D  = gl_Vertex.xyz;
   gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
   gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;

   float fogDistance = gl_ClipVertex.z;
   float fogDensity  = 0.1*user_Fog_Strength;
   fogFactor         = 1.0 /exp( (fogDistance * fogDensity)* (fogDistance * fogDensity));
}
