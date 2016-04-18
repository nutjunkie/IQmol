#version 120


uniform float user_Ambient;
uniform float user_Diffuse;
uniform float user_Highlights;
uniform float user_Shininess;
uniform bool  user_Enhance_Surface_Edges;

varying vec3 normal;
varying vec4 color;
varying vec3 viewDirection;


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
   diffuse *= 4.0*user_Diffuse; // diffuse scaling factor

   // calculate specular lighting contribution with Phong highlights using
   // reflection vectors
   vec3 R0 = reflect(lightDirection0, n);
   vec3 R1 = reflect(lightDirection1, n);
   vec3 R2 = reflect(lightDirection2, n);
   vec3 R3 = reflect(lightDirection3, n);

   float specular = 0.0;
   float shine = 100.0 * user_Shininess;
   specular += pow(max(0.0, dot(R0, viewDirection)), shine) * lightscale[0];
   specular += pow(max(0.0, dot(R1, viewDirection)), shine) * lightscale[1];
   specular += pow(max(0.0, dot(R2, viewDirection)), shine) * lightscale[2];
   specular += pow(max(0.0, dot(R3, viewDirection)), shine) * lightscale[3];
   specular *= user_Highlights; // specular scaling factor

   vec3 objcolor = color.rgb * vec3(diffuse);
   vec3 fragColor = objcolor + vec3(0.5*user_Ambient + specular);

   if (user_Enhance_Surface_Edges) {
      if( length( dFdx(n) + dFdy(n) ) < 0.0 ) discard;
      float alpha = 1.0 - abs(dot(viewDirection, n));
      alpha = clamp(alpha, color.a, 1.0);
      gl_FragColor = vec4(alpha*fragColor.rgb, alpha);
   }else {
      gl_FragColor = vec4(fragColor, color.a);
   } 

}


