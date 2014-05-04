//uniform float xmin;
//uniform float xmax;
//uniform float ymin;
//uniform float ymax;
//uniform float zmin;
//uniform float zmax;

varying vec4 color;
varying vec3 normal;
varying vec4 pos;
varying vec3 wpos;

vec3 lightDir = vec3( 0., 0., 1. );

void main()
{
  float xmin = 0.0;
  float xmax = 0.0;
  float ymin = 0.0;
  float ymax = 0.0;
  float zmin = 0.0;
  float zmax = 0.0;

  vec3 viewDir = normalize( wpos );     
  if( pos.x <= xmin || pos.x >= xmax ||
          pos.y <= ymin || pos.y >= ymax ||
          pos.z <= zmin || pos.z >= zmax ) discard;
  
  float kd = abs( dot( normalize( lightDir ), normal ) );
  //float ks = pow( abs( dot( normalize( lightDir ), viewDir ) ), 30.0 );
  gl_FragColor = vec4( color.rgb * kd /*+ ks*/, color.a );

}
