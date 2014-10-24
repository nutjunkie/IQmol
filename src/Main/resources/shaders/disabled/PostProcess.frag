#version 120


uniform sampler2D Normal;
uniform vec2 Normal_d;


// BEGIN UNIFORM
uniform float TotStrength;  // 0.70
uniform float Strength;     // 0.30
uniform float Radius;       // 0.50;
// END UNIFORM





//uniform float Multisample;

varying vec2 texcoord;


#define SAMPLES 16 // 10 is good
const float invSamples = 1.0/16.0;


float luminance(vec3 color)
{
   const vec3 luma = vec3(0.2126, 0.7152, 0.0722);
   return dot(color, luma);
}


float ramp(in float a, in float b, in float x) 
{
   float v = 0.0;;
   if (x > b) {
      v = 1.0;
   }else if (x > a) {
      v = (x-a) / (b-a);
   };
   return v;
}


vec4 edgeMap()
{
   float dx = Normal_d.x;
   float dy = Normal_d.y;

   vec2 offset[9];
   offset[0] = vec2(-dx, -dy);  offset[1] = vec2(0.0, -dy);  offset[2] = vec2(+dx, -dy);
   offset[3] = vec2(-dx, 0.0);  offset[4] = vec2(0.0, 0.0);  offset[5] = vec2(+dx, 0.0);
   offset[6] = vec2(-dx, +dy);  offset[7] = vec2(0.0, +dy);  offset[8] = vec2(+dx, +dy);

   // Sobel operators
   float Sx[9];
   Sx[0] = +3.0;  Sx[1] =  0.0;  Sx[2] = -3.0;
   Sx[3] =+10.0;  Sx[4] =  0.0;  Sx[5] =-10.0;
   Sx[6] = +3.0;  Sx[7] =  0.0;  Sx[8] = -3.0;

   float Sy[9];
   Sy[0] = +3.0;  Sy[1] =+10.0;  Sy[2] = +3.0;
   Sy[3] =  0.0;  Sy[4] =  0.0;  Sy[5] =  0.0;
   Sy[6] = -3.0;  Sy[7] =-10.0;  Sy[8] = -3.0;
    
   // Read neighboring pixel intensities
   float Gx = 0.0;
   float Gy = 0.0;
   float luma;
   for (int i = 0; i < 9; ++i) {
       luma = luminance(vec3(texture2D(Normal, texcoord + offset[i])));
       Gx  += luma*Sx[i];
       Gy  += luma*Sy[i];
   }

   vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
   if (sqrt(Gx*Gx+Gy*Gy) > 0.2) { color.g = 1.0; }

   return color;
}

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}


vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}


vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}


float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
  // First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

  // Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

  // Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

  // Gradients: 41 points uniformly over a line, mapped onto a diamond.
  // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

  // Normalise gradients implicitly by scaling m
  // Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

  // Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}






















void main(void)
{

   float totStrength = 5.0*TotStrength;
   float strength    = 2.0*Strength;
   float rad         = 0.02*Radius;
   float falloff     = 0.00002;
   float offset      = 100.0*0.18;

float  Multisample = 1.0;


vec2 coord = texcoord/Multisample;
gl_FragColor = texture2D(Normal, coord);
return;

vec3 norma  = vec3(texture2D(Normal, coord));

vec3 axis  = vec3(0.0, 0.0, 1.0);

gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

if (abs(dot(norma, axis)) < 0.9) {
   gl_FragColor.g = 1.0;
}
return;




// these are the random vectors inside a unit sphere
vec3 pSphere[16];
pSphere[0]  = vec3(0.53812504, 0.18565957, -0.43192);
pSphere[1]  = vec3(0.13790712, 0.24864247, 0.44301823),
pSphere[2]  = vec3(0.33715037, 0.56794053, -0.005789503),
pSphere[3]  = vec3(-0.6999805, -0.04511441, -0.0019965635),
pSphere[4]  = vec3(0.06896307, -0.15983082, -0.85477847),
pSphere[5]  = vec3(0.056099437, 0.006954967, -0.1843352),
pSphere[6]  = vec3(-0.014653638, 0.14027752, 0.0762037),
pSphere[7]  = vec3(0.010019933, -0.1924225, -0.034443386),
pSphere[8]  = vec3(-0.35775623, -0.5301969, -0.43581226),
pSphere[9]  = vec3(-0.3169221, 0.106360726, 0.015860917),
pSphere[10] = vec3(0.010350345, -0.58698344, 0.0046293875),
pSphere[11] = vec3(-0.08972908, -0.49408212, 0.3287904),
pSphere[12] = vec3(0.7119986, -0.0154690035, -0.09183723),
pSphere[13] = vec3(-0.053382345, 0.059675813, -0.5411899),
pSphere[14] = vec3(0.035267662, -0.063188605, 0.54602677),
pSphere[15] = vec3(-0.47761092, 0.2847911, -0.0271716);


   // grab a normal for reflecting the sample rays later on

   vec3 rnd = vec3(snoise(coord+0.6*offset) , snoise(coord-offset), snoise(coord+offset));  // These man need scaling
   rnd = 2.0*(rnd - vec3(0.5, 0.5, 0.5));
   vec3 fres = normalize(rnd);
 
   vec4 currentPixelSample = texture2D(Normal,coord);
 
   float currentPixelDepth = currentPixelSample.a;
 
   // current fragment coords in screen space
   vec3 ep = vec3(coord.xy,currentPixelDepth);
    // get the normal of current fragment
   vec3 norm = currentPixelSample.xyz;
   norm = normalize(2.0*norm - vec3(1.0, 1.0, 1.0));
 
   float bl = 0.0;
   // adjust for the depth ( not shure if this is good..)
   float radD = rad/currentPixelDepth;
 
   vec3 ray, se, occNorm;
   float occluderDepth, depthDifference, normDiff;
 
   for(int i = 0; i < SAMPLES; ++i) {
      // get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
      ray = radD*reflect(pSphere[i],fres);
 
      // if the ray is outside the hemisphere then change direction
      se = ep + sign(dot(ray,norm) )*ray;
 
      // get the depth of the occluder fragment
      vec4 occluderFragment = texture2D(Normal,se.xy);
 
      // get the normal of the occluder fragment
      occNorm = occluderFragment.xyz;
      occNorm = 2.0*occluderFragment.xyz - vec3(1.0);
 
      // if depthDifference is negative = occluder is behind current fragment
      depthDifference = currentPixelDepth-occluderFragment.a;
 
      // calculate the difference between the normals as a weight
 
      normDiff = (1.0-dot(occNorm,norm));
      // the falloff equation, starts at falloff and is kind of 1/x^2 falling
      bl += step(falloff,depthDifference)*normDiff*(1.0-smoothstep(falloff,strength,depthDifference));
   }
 
   // output the result
   float ao = 1.0-totStrength*bl*invSamples;
   

   gl_FragColor.r = ao;
   gl_FragColor.g = ao;
   gl_FragColor.b = ao;
return;

}
