#version 120

varying vec3 normal;
varying vec4 color;
varying vec3 viewDirection;

varying float xray;
varying float shine;
varying float ambient;
varying float diffuse_scale;
varying float specular_scale;


void main()
{
   // lighting
   vec3 lightDirection0 = vec3(-1.0,  0.0, 0.2 );
   vec3 lightDirection1 = vec3( 0.0,  1.0, 0.2 );
   vec3 lightDirection2 = vec3( 0.3,  0.4, 0.9 );
   vec3 lightDirection3 = vec3( 0.0, -1.0, 0.2 );
   vec4 lightscale      = vec4( 0.5, 0.3, 0.3, 0.1);

   vec3 n = normalize(normal);

   // calculate diffuse lighting contribution
   float diffuse = 0.0;
   diffuse += max(0.0, dot(n, lightDirection0)) * lightscale[0];
   diffuse += max(0.0, dot(n, lightDirection1)) * lightscale[1];
   diffuse += max(0.0, dot(n, lightDirection2)) * lightscale[2];
   diffuse += max(0.0, dot(n, lightDirection3)) * lightscale[3];
   diffuse *= diffuse_scale; // diffuse scaling factor

   // calculate specular lighting contribution with Phong highlights using
   // reflection vectors
   vec3 R0 = reflect(lightDirection0, n);
   vec3 R1 = reflect(lightDirection1, n);
   vec3 R2 = reflect(lightDirection2, n);
   vec3 R3 = reflect(lightDirection3, n);

   float specular = 0.0;
   specular += pow(max(0.0, dot(R0, viewDirection)), shine) * lightscale[0];
   specular += pow(max(0.0, dot(R1, viewDirection)), shine) * lightscale[1];
   specular += pow(max(0.0, dot(R2, viewDirection)), shine) * lightscale[2];
   specular += pow(max(0.0, dot(R3, viewDirection)), shine) * lightscale[3];
   specular *= specular_scale; // specular scaling factor

   vec3 objcolor = color.rgb * vec3(diffuse);
   vec3 fragColor = objcolor + vec3(ambient + specular);

   // Apply edge enhancement
   if (xray < 0.99) {
      if( length( dFdx(n) + dFdy(n) ) < 0.0 ) discard;
      float alpha = 1.0 - abs(dot(viewDirection, n));
      alpha = clamp(alpha, xray, 1.0);
      gl_FragColor = vec4(alpha*fragColor.rgb, alpha);
   }else {
      gl_FragColor = vec4(fragColor, color.a);
   } 

}


