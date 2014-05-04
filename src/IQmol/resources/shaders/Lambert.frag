#version 120

uniform bool  user_Enhance_Surface_Edges;
uniform float user_Ambient;

varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;


void main()
{
   // Regular Lambert
   const vec3 lightDir = vec3(0.0, 0.0, 1.0);  // must be normalized
   float ambient = 0.5*user_Ambient;

   vec3 n = normalize(normal);
   float kd = abs(dot(lightDir, n));   
   vec4 fragColor = vec4(kd*color.rgb + vec3(ambient), color.a);

   if (user_Enhance_Surface_Edges) {
      if( length(dFdx(n)+dFdy(n)) < 0.0) discard;
      float alpha = 1.0 - abs(dot(viewDirection, n));
      alpha = clamp(alpha, color.a, 1.0);
      gl_FragColor = vec4(alpha*fragColor.rgb, alpha);
   }else {
      gl_FragColor = fragColor;
   }
}
