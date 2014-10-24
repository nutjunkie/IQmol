#version 120

uniform sampler2D NormalMap;
uniform vec2 NormalMap_delta;

uniform sampler2D RotationTexture;
uniform vec2 RotationTexture_delta;
uniform vec2 SamplingVectors[16];
uniform mat4x4 ProjectionInverse;


uniform float AOTotal;
uniform float AOStrength;
uniform float AORadius;

uniform float user_Normal;
uniform float user_Depth;
uniform float user_Color;
uniform bool  user_Add_Normal_Test;
uniform bool  user_Add_Color_Test;
uniform bool  user_Add_Depth_Test;

varying vec2 texcoord;
varying vec2 texcoord1;
varying vec2 texcoord2;
varying vec4 vertex;



float luminance(vec3 col)
{
   //const vec3 luma = normalize(vec3(0.2126, 0.7152, 0.0722));
   const vec3 luma = normalize(vec3(1.0));
   return dot(col, luma);
}


float luminance(vec2 coord)
{
   return luminance(vec3(texture2D(NormalMap, coord)));
}


float depth(vec2 coord)
{
   return texture2D(NormalMap, coord).a;
}


vec3 normal(vec2 coord)
{
   vec3 n = vec3(texture2D(NormalMap, coord));
   return normalize(2.0*n-1.0);
}


float edgeMap(vec2 coord, float dx, float dy)
{
   vec2 offset[9];
   offset[0] = vec2(-dx, -dy);  offset[1] = vec2(0.0, -dy);  offset[2] = vec2(+dx, -dy);
   offset[3] = vec2(-dx, 0.0);  offset[4] = vec2(0.0, 0.0);  offset[5] = vec2(+dx, 0.0);
   offset[6] = vec2(-dx, +dy);  offset[7] = vec2(0.0, +dy);  offset[8] = vec2(+dx, +dy);

   // Sobel operators  (also {1,2,1})
   float Sx[9];   
   Sx[0] = -3.0;  Sx[1] =  0.0;  Sx[2] = +3.0;
   Sx[3] =-10.0;  Sx[4] =  0.0;  Sx[5] =+10.0;
   Sx[6] = -3.0;  Sx[7] =  0.0;  Sx[8] = +3.0;

   float Sy[9];
   Sy[0] = -3.0;  Sy[1] =-10.0;  Sy[2] = -3.0;
   Sy[3] =  0.0;  Sy[4] =  0.0;  Sy[5] =  0.0;
   Sy[6] = +3.0;  Sy[7] =+10.0;  Sy[8] = +3.0;

   vec2 dL  = vec2(0.0);
   vec2 dD  = vec2(0.0);
   vec3 dNx = vec3(0.0);
   vec3 dNy = vec3(0.0);

   vec3  N;
   vec2 xi;

   for (int i = 0; i < 9; ++i) {
       xi   = coord + offset[i];
       N    = normal(xi);
       dL  += luminance(xi) * vec2(Sx[i], Sy[i]);
       dD  += depth(xi)     * vec2(Sx[i], Sy[i]);
       dNx += N * Sx[i];
       dNy += N * Sy[i];
   }

   if (!user_Add_Depth_Test && !user_Add_Depth_Test && !user_Add_Depth_Test) {
      return (length(dD) > 0.1) ? 1.0 : 0.0;
   }

   if (user_Add_Depth_Test  && length(dD) > user_Depth)  return 1.0;  // 0.1

   if (user_Add_Normal_Test && length(dNx) > user_Normal) return 1.0;
   if (user_Add_Normal_Test && length(dNx) > user_Normal) return 1.0;
   if (user_Add_Color_Test  && length(dL) > 3.0*user_Color)  return 1.0;
   return 0.0;
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

vec3 mod289(vec3 x ) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x ) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }


float snoise(vec2 v) 
{
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


float ambientOcclusion2(vec2 coord)
{
   vec4  nmap = texture2D(NormalMap, coord);
   vec3  n = normal(coord);
   float d = depth(coord);

   vec4 Position = ProjectionInverse * vec4(gl_TexCoord[0].st, d, 1.0);
   Position.xyz /= Position.w;
		
   if (dot(n, Position.xyz) > 0.0) n = -n;
   float Distance = length(Position.xyz);

   vec2 sr = normalize(texture2D(RotationTexture, gl_TexCoord[1].st).rg * 2.0 - 1.0) / 
           min(Distance, 4.0);
		
   mat2x2 ScaleRotMat = mat2x2(sr.x, sr.y, -sr.y, sr.x);

   float SSAO = 0.0;

   for (int i = 0; i < 16; i++) {
       vec2 TexCoord = clamp(ScaleRotMat*SamplingVectors[i] + gl_TexCoord[0].st, 0.0, 0.9995);
       vec4 position = ProjectionInverse * vec4(TexCoord, texture2D(NormalMap, TexCoord).a, 1.0);
               
			position.xyz /= position.w;

			vec3 p2p = position.xyz - Position.xyz;
			float Distance2 = dot(p2p, p2p);
			SSAO += max(dot(n, p2p), 0.0) / (sqrt(Distance2) * (1.0 + Distance2));
   }

   float aoFactor  = 1.0 - SSAO / 16.0;
   return aoFactor;
}



float ambientOcclusion(vec2 coord)
{ 

   float totStrength = 10.0*AOTotal;
   float strength    = 2.0*AOStrength;
   float rad         = 0.02*AORadius;
   float falloff     = 0.00002;
   float offset      = 100.0*0.18;

   // these are the random vectors inside a unit sphere
   vec3 pSphere[16];
   pSphere[0]  = vec3( 0.53812504,  0.18565957, -0.43192);
   pSphere[1]  = vec3( 0.13790712,  0.24864247,  0.44301823),
   pSphere[2]  = vec3( 0.33715037,  0.56794053, -0.005789503),
   pSphere[3]  = vec3(-0.69998050, -0.04511441, -0.0019965635),
   pSphere[4]  = vec3( 0.06896307, -0.15983082, -0.85477847),
   pSphere[5]  = vec3( 0.05609940,  0.00695497, -0.1843352),
   pSphere[6]  = vec3(-0.01465363,  0.14027752,  0.0762037),
   pSphere[7]  = vec3( 0.01001993, -0.19242251, -0.034443386),
   pSphere[8]  = vec3(-0.35775623, -0.53019694, -0.43581226),
   pSphere[9]  = vec3(-0.31692214,  0.10636072,  0.015860917),
   pSphere[10] = vec3( 0.01035034, -0.58698344,  0.0046293875),
   pSphere[11] = vec3(-0.08972908, -0.49408212,  0.3287904),
   pSphere[12] = vec3( 0.71199864, -0.01546900, -0.09183723),
   pSphere[13] = vec3(-0.05338234,  0.05967581, -0.5411899),
   pSphere[14] = vec3( 0.03526766, -0.06318860,  0.54602677),
   pSphere[15] = vec3(-0.47761092,  0.28479115, -0.0271716);

   // grab a normal for reflecting the sample rays later on

   vec3 rnd = vec3(snoise(coord+0.6*offset) , 
                   snoise(coord-offset-34.0), 
                   snoise(coord+offset));  // These man need scaling
   rnd = normalize(rnd);

   //rnd = vec3(1.0, 1.0, 1.0);

   rnd = 2.0*(rnd - vec3(0.5, 0.5, 0.5));
   vec3 fres = normalize(rnd);
 
   vec4  currentPixelSample = texture2D(NormalMap,coord);
   float currentPixelDepth  = currentPixelSample.a;
 
   // current fragment coords in screen space
   vec3 ep = vec3(coord.xy,currentPixelDepth);

    // get the normal of current fragment
   vec3 norm = currentPixelSample.xyz;
   norm = normalize(2.0*norm - vec3(1.0, 1.0, 1.0));
 
   // adjust for the depth ( not shure if this is good..)
   float radD = rad/currentPixelDepth;
 
   vec3 ray, se, occNorm;
   float occluderDepth, depthDifference, normDiff;
 
   const int   samples = 16;
   const float invSamples = 1.0/16.0;

   float bl = 0.0;
   for (int i = 0; i < samples; ++i) {
       // get a vector (randomized inside of a sphere with radius 1.0) 
       // from a texture and reflect it
       ray = radD*reflect(pSphere[i], fres);
 
       // if the ray is outside the hemisphere then change direction
       se = ep + sign(dot(ray, norm))*ray;
 
       // get the depth of the occluder fragment
       vec4 occluderFragment = texture2D(NormalMap,se.xy);
 
       // get the normal of the occluder fragment
       occNorm = occluderFragment.xyz;
       occNorm = 2.0*occluderFragment.xyz - vec3(1.0);
 
       // if depthDifference is negative = occluder is behind current fragment
       depthDifference = currentPixelDepth-occluderFragment.a;
 
       // calculate the difference between the normals as a weight
       normDiff = 1.0-dot(occNorm, norm);

       // the falloff equation, starts at falloff and is kind of 1/x^2 falling
       bl += step(falloff, depthDifference) * normDiff *
              (1.0-smoothstep(falloff, strength, depthDifference));
   }
 
   return 1.0-totStrength*bl/(1.0*samples);
}


void main()
{
   vec4 v = vertex;
   gl_FragColor.r = edgeMap(texcoord, NormalMap_delta.x, NormalMap_delta.y);
   gl_FragColor.g = ambientOcclusion(texcoord);
   gl_FragColor.b = ambientOcclusion2(texcoord);
   gl_FragColor.a = depth(texcoord);

   //texcoord  [0-1]
   vec2 i = texcoord1;
   vec2 j = texcoord2;

   vec2 cd = vec2(gl_FragCoord)*NormalMap_delta;

   gl_FragColor = texture2D(RotationTexture, texcoord2);
}

