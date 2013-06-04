#version 120

varying float xray;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;


void main()
{

   if (xray < 0.99) {
      if( length( dFdx( normal ) + dFdy( normal ) ) < 0.0 ) discard;
      float alpha = 1.0 - abs( dot( viewDirection, normal ) );
      alpha = clamp(alpha, xray, 1.0);
      gl_FragColor = vec4(alpha*color.rgb, alpha);
   }else {
      gl_FragColor = color;
   }
}
