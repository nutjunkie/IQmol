#version 120

// BEGIN UNIFORM
uniform float user_Edge_Thresh; // 0.35
// END UNIFORM


uniform sampler2D FilterMap;
uniform vec2 FilterMap_delta;
uniform bool FiltersAreValid;

varying vec3 L, N, R, V;


float smoothEdge(vec2 coord, float dx, float dy) 
{
   vec2 offset[9];
   offset[0] = vec2(-dx, -dy);  offset[1] = vec2(0.0, -dy);  offset[2] = vec2(+dx, -dy);
   offset[3] = vec2(-dx, 0.0);  offset[4] = vec2(0.0, 0.0);  offset[5] = vec2(+dx, 0.0);
   offset[6] = vec2(-dx, +dy);  offset[7] = vec2(0.0, +dy);  offset[8] = vec2(+dx, +dy);

   // Gaussian blur filter 
   float G[9];
   G[0] = 1.0;  G[1] =  1.0;  G[2] = 1.0;
   G[3] = 1.0;  G[4] =  2.0;  G[5] = 1.0;
   G[6] = 1.0;  G[7] =  1.0;  G[8] = 1.0;

   float edge = 0.0;
   for (int i = 0; i < 9; ++i) {
       edge += G[i] * (texture2D(FilterMap, coord + offset[i])).r;
   } 
   return edge/10.0;
}



void main()
{
	vec4 finalColor = vec4(0.0);
	float NL, VR, NV;
	vec3 N2, L2, R2, V2;
	vec4 kCool, kWarm;

	/* N, L, R and V are interpolated, so we need to renormalize them */
	N2 = normalize(N);
	L2 = normalize(L);
	R2 = normalize(R);
	V2 = normalize(V);

	/* Gooch lighting */
	NL = dot(N2, L2);
	kCool = vec4(0.0, 0.0, 0.55, 0) + 0.25*gl_FrontLightProduct[0].diffuse;
	kWarm = vec4(0.3, 0.3, 0.0, 0) + 0.5*gl_FrontLightProduct[0].diffuse;
	finalColor += kCool*(1 - NL)/2 + kWarm * (1 + NL)/2;

	/* Specular lighting */
	VR = dot(V2, R2);
	if (VR > 0.0) {
		finalColor += gl_FrontLightProduct[0].specular * pow(VR, gl_FrontMaterial.shininess);
	}

    vec3 border = 0.1*vec3(1.0, 1.0, 1.0);

    NV = dot(N2, V2);
    if (NV > 0.0 && NV < user_Edge_Thresh) {
       if (FiltersAreValid) {
          vec2 coord = vec2(gl_FragCoord.x*FilterMap_delta.x, gl_FragCoord.y*FilterMap_delta.y);
          vec4 mask = texture2D(FilterMap, coord);
          
          if (mask.r > 0.5) finalColor.rgb = vec3(1.0, 0.0, 0.0);
          //if (mask.r > 0.5) finalColor.rgb = border;
          finalColor.rgb = smoothEdge(coord, FilterMap_delta.x, FilterMap_delta.y) * vec3(1.0, 0.0, 0.0);

       }else {
          finalColor.rgb = border;
       }
    }

	gl_FragColor = finalColor;
}
