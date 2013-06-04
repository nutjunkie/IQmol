#version 120

varying float xray;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;
 
void main()
{   

   if (xray < 0.99) {
      // Apply edge enhancement
      vec3 n = normalize(normal);
      if(length(dFdx(n)+dFdy(n)) < 0.0) discard;
      float alpha = 1.0 - abs(dot(viewDirection, n));
      alpha = clamp(alpha, xray, 1.0);
      gl_FragColor = vec4(alpha*color.rgb, alpha);
   }else {
      gl_FragColor = color;
   }
}
