#version 120

uniform float user_Highlights;
uniform float user_Noise_Phase; 
uniform float user_Noise_Intensity;
uniform bool  user_Enhance_Transparency;
uniform bool  user_Enhance_Edges;

uniform bool  user_light_Front;
uniform bool  user_light_Highlight;
uniform bool  user_light_Left;
uniform bool  user_light_Lower;

uniform vec4  backgroundColor;

varying float shine;
varying vec4  color;
varying vec3  normal;
varying vec3  viewDirection;
varying vec3  v_texCoord3D;

varying float fogFactor;

//---------------------------- Snoise Functions ------------------------------------
vec3 mod289(vec3 x) 
{
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) 
{
   return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) 
{
   return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
   return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
   const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
   const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

   // First corner
   vec3 i  = floor(v + dot(v, C.yyy) );
   vec3 x0 = v - i + dot(i, C.xxx) ;

   // Other corners
   vec3 g = step(x0.yzx, x0.xyz);
   vec3 l = 1.0 - g;
   vec3 i1 = min( g.xyz, l.zxy );
   vec3 i2 = max( g.xyz, l.zxy );

   vec3 x1 = x0 - i1 + C.xxx;
   vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
   vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

   // Permutations
   i = mod289(i); 
   vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

   // Gradients: 7x7 points over a square, mapped onto an octahedron.
   // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
   float n_ = 0.142857142857; // 1.0/7.0
   vec3  ns = n_ * D.wyz - D.xzx;

   vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

   vec4 x_ = floor(j * ns.z);
   vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

   vec4 x = x_ *ns.x + ns.yyyy;
   vec4 y = y_ *ns.x + ns.yyyy;
   vec4 h = 1.0 - abs(x) - abs(y);
 
   vec4 b0 = vec4( x.xy, y.xy );
   vec4 b1 = vec4( x.zw, y.zw );

   vec4 s0 = floor(b0)*2.0 + 1.0;
   vec4 s1 = floor(b1)*2.0 + 1.0;
   vec4 sh = -step(h, vec4(0.0));

   vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
   vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

   vec3 p0 = vec3(a0.xy,h.x);
   vec3 p1 = vec3(a0.zw,h.y);
   vec3 p2 = vec3(a1.xy,h.z);
   vec3 p3 = vec3(a1.zw,h.w);

   //Normalise gradients
   vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
   p0 *= norm.x;
   p1 *= norm.y;
   p2 *= norm.z;
   p3 *= norm.w;

   // Mix final noise value
   vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
   m = m * m;
   return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}



float myNoise(vec3 texCoord, float time)
{
   // Perturb the texcoords with three components of noise
   vec3 uvw = texCoord + 0.1*vec3(snoise(texCoord + vec3(0.0, 0.0, time)),
                                  snoise(texCoord + vec3(43.0, 17.0, time)),
                                  snoise(texCoord + vec3(-17.0, -43.0, time)));

   // Six components of noise in a fractal sum
   float x = snoise(uvw - vec3(0.0, 0.0, time));
   x += 0.5 * snoise(uvw * 2.0 - vec3(0.0, 0.0, time*1.4)); 
   x += 0.25 * snoise(uvw * 4.0 - vec3(0.0, 0.0, time*2.0)); 
   x += 0.125 * snoise(uvw * 8.0 - vec3(0.0, 0.0, time*2.8)); 
   x += 0.0625 * snoise(uvw * 16.0 - vec3(0.0, 0.0, time*4.0)); 
   x += 0.03125 * snoise(uvw * 32.0 - vec3(0.0, 0.0, time*5.6)); 
   x *= 0.7;
   return x;
}


//-----------------------------------------------------------------------------------

void main()
{   
//{0.371391, 0., 0.928477}
//{0.303046, 0.808122, -0.505076}
//{-1., 0., 0.}
//{0., -0.980581, 0.196116}
   const vec3 white = vec3(1.0);
   const vec4 lightDirection0 = vec4( 0.4,  0.0,  1.0, 1.0);  // similar to gl_LightSource[0]
   const vec4 lightDirection1 = vec4( 0.3,  0.8, -0.5, 1.0);  // good
   const vec4 lightDirection2 = vec4(-0.5,  0.0,  0.0, 1.0);
   const vec4 lightDirection3 = vec4( 0.0, -1.0,  0.2, 1.0);

   vec3  N     = normalize(normal);
   vec3  rgb   = color.rgb;
   float alpha = color.a;

   if (user_Enhance_Transparency) {
      if (length(dFdx(N)+dFdy(N)) < 0.0) discard;
      float mask = 1.0 - abs(dot(-viewDirection, N));
      alpha = clamp(mask, alpha, 1.0);
      if (user_Enhance_Edges) {rgb *= alpha; }
   }

   float specular = 0.0;
   if (user_light_Front) {
      vec3 lightDirection  = normalize(lightDirection0.xyz);
      vec3 reflectance = reflect(lightDirection, N);
      specular += lightDirection0.w * pow(max(0.0, dot(reflectance, viewDirection)), shine);
   }
   if (user_light_Highlight) {
      vec3 lightDirection  = normalize(lightDirection1.xyz);
      vec3 reflectance = reflect(lightDirection, N);
      specular += lightDirection1.w * pow(max(0.0, dot(reflectance, viewDirection)), shine);
   }
   if (user_light_Left) {
      vec3 lightDirection  = normalize(lightDirection2.xyz);
      vec3 reflectance = reflect(lightDirection, N);
      specular += lightDirection2.w * pow(max(0.0, dot(reflectance, viewDirection)), shine);
   }
   if (user_light_Lower) {
      vec3 lightDirection  = normalize(lightDirection3.xyz);
      vec3 reflectance = reflect(lightDirection, N);
      specular += lightDirection3.w * pow(max(0.0, dot(reflectance, viewDirection)), shine);
   }
   specular *= user_Highlights;

   rgb += specular * white;

   if (user_Noise_Intensity > 0.01) {
      float time = 10.0*user_Noise_Intensity;
      float x = myNoise(v_texCoord3D, time);
      rgb += 0.5*user_Noise_Intensity*(rgb + vec3(x, x, x));
   }

   float fog = clamp(fogFactor, 0.0, 1.0);
   rgb = mix(backgroundColor.xyz, rgb, fog);
 
   gl_FragColor = vec4(rgb, alpha);
}
