#version 120

varying float xray;
varying vec4  color;
varying vec3  normal;
varying vec3  ambientColor;
varying vec3  viewDirection;


void main()
{
   // Regular Lambert
   const vec3 lightDir = vec3(0.0, 0.0, 1.0);  // must be normalized

   vec3 n = normalize(normal);
   float kd = abs(dot(lightDir, n));   
   vec4 fragColor = vec4(color.rgb*kd + ambientColor, color.a);

   // Apply edge enhancement
   if (xray < 0.99) {
      if( length(dFdx(n)+dFdy(n)) < 0.0) discard;
      float alpha = 1.0 - abs(dot(viewDirection, n));
      alpha = clamp(alpha, xray, 1.0);
      gl_FragColor = vec4(alpha*fragColor.rgb, alpha);
   }else {
      gl_FragColor = fragColor;
   }
}
