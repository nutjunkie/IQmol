#version 120

uniform sampler2D NormalMap;
uniform vec2 NormalMap_delta;

uniform sampler2D FilterMap;
uniform vec2 FilterMap_delta;
uniform bool FiltersAreValid;

uniform bool user_Normal_Map;
uniform bool user_Ambient_Occlusion;
uniform bool user_Edge_Detect;
uniform bool user_Depth_Buffer;

uniform float user_Ambient;

varying vec4 color;
varying vec3 normal;
varying vec2 texcoord;
varying vec3 viewDirection;


float smoothAmbientOcclusion(vec2 coord)
{
   float dx = FilterMap_delta.x;
   float dy = FilterMap_delta.y;
   float ao = 0.0;

   const int radius = 2;
   float weight = 0.0;
   vec2 xi;

   for (int r = 0; r <= radius; ++r) {
       for (int i = -r; i <= r; ++i) {
           for (int j = -r; j <= r; ++j) {
               xi  =  coord + vec2(i*dx, j*dy);
               ao += (texture2D(FilterMap, xi)).g;
               weight += 1.0;
           }
       }
   }
   return ao/weight;

   vec2 offset[9];
   offset[0] = vec2(-dx, -dy);  offset[1] = vec2(0.0, -dy);  offset[2] = vec2(+dx, -dy);
   offset[3] = vec2(-dx, 0.0);  offset[4] = vec2(0.0, 0.0);  offset[5] = vec2(+dx, 0.0);
   offset[6] = vec2(-dx, +dy);  offset[7] = vec2(0.0, +dy);  offset[8] = vec2(+dx, +dy);

   // Gaussian blur filter 
   float G[9];
   G[0] = 1.0;  G[1] =  1.0;  G[2] = 1.0;
   G[3] = 1.0;  G[4] =  2.0;  G[5] = 1.0;
   G[6] = 1.0;  G[7] =  1.0;  G[8] = 1.0;


   for (int i = 0; i < 9; ++i) {
       ao += G[i] * (texture2D(FilterMap, coord + offset[i])).g;
   }

   return ao/10.0;
}


void main()
{
   vec4 frag  = color;
   float aoFac = 1.0;
 
   vec3 nothing = viewDirection;

   if (FiltersAreValid) {

      vec2 coord = texcoord;
      coord = vec2(gl_FragCoord.x*FilterMap_delta.x, gl_FragCoord.y*FilterMap_delta.y);
      vec4 mask = texture2D(FilterMap, coord);

gl_FragColor = mask;
return;
    
      if (user_Normal_Map) {
         gl_FragColor = texture2D(NormalMap, coord);
         return;
       }

      if (user_Depth_Buffer) {
         frag.rgb = vec3(mask.a);
         frag.a   = 1.0;
      }

      if (user_Ambient_Occlusion) {
         //frag.rgb = vec3(smoothAmbientOcclusion(coord));
         //aoFac = smoothAmbientOcclusion(coord);
         gl_FragColor.rgb = vec3(mask.b);
         return;
      }

      if (user_Edge_Detect && mask.r > 0.5) {
         frag.r = 1.0;
         frag.g = 0.0;
         frag.b = 0.0;
         frag.a = 1.0;
   gl_FragColor = frag;
return;
      }
   }
   
   const vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
   vec3 n = normalize(normal);
   float ambient = 0.5*user_Ambient*aoFac;
   float kd = dot(lightDir, n) + ambient;
   frag = vec4(kd*frag.rgb, frag.a);

   gl_FragColor = frag;
   return;
}

