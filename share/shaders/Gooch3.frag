#version 120
varying vec3 L, N, R, V;
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

	/* Edges */
	NV = dot(N2, V2);
	if (NV > 0.0 && NV < 0.30) {
		finalColor += vec4(0.4, 0.4, 0.4, 0.0);
	}

	gl_FragColor = finalColor;
}
