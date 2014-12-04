#version 120

varying vec3 N;
varying vec3 V;
varying vec3 L;
varying float outline;


void main()
{     

vec3 WarmColour = vec3(0.5, 0.0, 0.0);
vec3 CoolColour = vec3(0.0, 0.0, 0.5);
vec3 SurfaceColour = gl_Color.rgb;

    vec3 l = normalize(L);
    vec3 n = normalize(N);
    vec3 v = normalize(V);
    vec3 h = normalize(l+v);
    
    float diffuse = dot(l,n);
    float specular = 0.1*pow(dot(n,h),50.0);
    
    vec3 cool = min(CoolColour+SurfaceColour,1.0);
    vec3 warm = min(WarmColour+SurfaceColour,1.0);
    
    vec3 colour = min(mix(cool,warm,diffuse)+specular,1.0);
    
    if (dot(n,v)<outline) colour=vec3(0,0,0);

    gl_FragColor = vec4(colour,1);
}
