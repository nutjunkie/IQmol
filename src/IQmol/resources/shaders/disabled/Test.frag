#version 120

varying vec4 color;
varying vec3 normal;
varying vec3 ambientColor;

varying float alpha;
varying float minOpacity;
varying float maxOpacity;
varying vec3  viewDirection;


void main()
{
   const vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));

   float kd = abs(dot(lightDir, normal));   
   vec4 fragColor = vec4( color.rgb * kd + ambientColor, color.a);

   if (alpha < 0.99) {
      if( length( dFdx( normal ) + dFdy( normal ) ) < 0.0 ) discard;
      float alpha = 1.0 - abs( dot( viewDirection, normal ) );
      alpha = clamp( alpha, minOpacity, maxOpacity );
      gl_FragColor = vec4( alpha*fragColor.rgb, alpha );
   }else {
      gl_FragColor = fragColor;
   }
}
