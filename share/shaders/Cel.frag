#version 120

uniform float user_Intensity;
uniform float user_Spread;
uniform bool  user_Antialias;

varying vec3  normal;
varying vec4  color;


// Redistributesthe intensity if the color blows out a channel
vec3 redistribute(const vec3 color)
{
   const float thresh = 0.999;
   vec3  c;
   float r = color.r;
   float g = color.g;
   float b = color.b;

   float m = max(r, g);
   m = max(m, b);

   if (m <= thresh) {
      c = vec3(r, g, b);
   }else {
      float total = r + g + b;
      if (total >= 3.0*thresh) {
         c = vec3(1.0, 1.0, 1.0);
      }else {
         float x = (3.0*thresh-total) / (3.0*m-total);
         float grey = thresh - x*m;
         c = vec3(grey+x*r, grey+x*g, grey+x*b);
      }
   }
 
   return c;
}


void main()
{
    const vec3 light = normalize(vec3(0.3, 0.4, 1.0));
	vec3 n = normalize(normal);
    
    float scale     = 0.5*user_Intensity;
    float spread    = user_Spread;
	float intensity = max(0.0, dot(light, n));

    // These are the intensity values of the step transitions:
    float s1 = pow(spread, 0.10);
    float s2 = spread;
    float s3 = spread*spread;

    // These are the mixing factors to generate the four colors:
    float f0 = 1.0 + scale;
    float f1 = 1.0;
    float f2 = 1.0 - scale;
    float f3 = 1.0 - 2.0*scale;

    // This is our epsilon value
    float e = 0.0;
    if (user_Antialias) e = max(0.0, fwidth(intensity));

    if (intensity > s1+e) {
       intensity = f0;
    }else if(intensity > s1-e) {
       float df  = 0.5*(intensity-s1+e)/e; 
       intensity = mix(f1, f0, df);

    }else if(intensity > s2+e) {
       intensity = f1;
    }else if(intensity > s2-e) {
       float df  = 0.5*(intensity-s2+e)/e; 
       intensity = mix(f2, f1, df);

    }else if(intensity > s3+e) {
       intensity = f2;
    }else if(intensity > s3-e) {
       float df  = 0.5*(intensity-s3+e)/e; 
       intensity = mix(f3, f2, df);

    }else {
       intensity = f3;
    }

    gl_FragColor = vec4(redistribute(intensity*color.rgb), color.a);
}
