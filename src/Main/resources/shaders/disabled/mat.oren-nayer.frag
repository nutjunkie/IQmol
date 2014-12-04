#define NUM_LIGHTS	1

#define MAT_VARS	vec3 L;			\
	float VdotN;					\
	float LdotN;					\
	float irradiance;				\
	float angleDifference;			\
	float lut_val;

#define LIGHT(num)	L = normalize(gl_LightSource[num].position.xyz - V);		\
	VdotN = dot(Pn, Nn);														\
	LdotN = dot(L, Nn);															\
	irradiance = max(0., LdotN);												\
	angleDifference = max(0., dot(normalize(Pn-Nn*VdotN), normalize(L-Nn*LdotN)));	\
	lut_val = texture2D( lookup, vec2(VdotN, LdotN)*0.5+0.5).a;						\
	color += gl_LightSource[num].ambient*Ka+gl_LightSource[num].diffuse*Kd*(A+B*angleDifference*lut_val)*irradiance;


uniform vec4 Ka;
uniform vec4 Kd;
uniform sampler2D lookup;

varying vec3 N;
varying vec3 V;

void main(void)
{
	vec3 Pn = -normalize(V);
	vec3 Nn = normalize(N);

	float roughnessSquared = 0.5;
	float A = 1. - (0.5*roughnessSquared)/(roughnessSquared+0.33);
	float B = 1. - (0.45*roughnessSquared)/(roughnessSquared+0.09);

	vec4 color = vec4(0., 0., 0., 0.);

	MAT_VARS
	LIGHT(0)

	gl_FragColor = color;
}
