#version 120
varying vec3 L, N, R, V;

void main()
{
	/* Compute some vectors that will be interpolated and passed to the
	 * fragment shader. All calculations use eye coordinates.
	 */

	/* Convert vertex position to eye coordinates */
	vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);

	/* Normalized vector from the surface to the light source, i.e. the
	 * reversed direction of the incoming ray. Same as L in the raytracer.
	 */
	L = normalize(vec3(gl_LightSource[0].position) - vertexPos);

	/* Convert normal from object coords to eye coords and normalize it.
	 * Needs special treatment because it's a normal vector.
	 */
	N = normalize(gl_NormalMatrix * gl_Normal);

	/* Compute R, the reflection of L, with R = -L + 2(L.N)N */
	R = -1*L + 2*dot(L, N)*N;

	/* Compute the view vector V. The viewer is in (0,0,0) in eye coords,
	 * so we can simply invert and normalize the vertex position here.
	 */
	V = normalize(-vertexPos);

    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
}
