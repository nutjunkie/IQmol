uniform sampler2D Image;
uniform ivec2 Image_size;

varying vec2 texcoord;


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


void main()
{
   float dx = 1.0 / float(Image_size.x);
   float dy = 1.0 / float(Image_size.y);

   vec2 offset[9];
   offset[0] = vec2(-dx, -dy);  offset[1] = vec2(0.0, -dy);  offset[2] = vec2(+dx, -dy);
   offset[3] = vec2(-dx, 0.0);  offset[4] = vec2(0.0, 0.0);  offset[5] = vec2(+dx, 0.0);
   offset[6] = vec2(-dx, +dy);  offset[7] = vec2(0.0, +dy);  offset[8] = vec2(+dx, +dy);

   //float kernel[9];
   //kernel[0] = -1.0;  kernel[1] = -1.0;  kernel[2] = -1.0; 
   //kernel[3] = -1.0;  kernel[4] = +8.0;  kernel[5] = -1.0; 
   //kernel[6] = -1.0;  kernel[7] = -1.0;  kernel[8] = -1.0; 

   // Read neighboring pixel intensities
   float pix[9];
   for (int i = 0; i < 9; ++i) {
       pix[i] = luminance(vec3(texture2D(Image, texcoord + offset[i])));
   }

   // Intensity differences around neighboring pixels
   float d1 = abs(pix[1]-pix[7]);
   float d2 = abs(pix[5]-pix[3]);
   float d3 = abs(pix[0]-pix[8]);
   float d4 = abs(pix[2]-pix[6]);

   float delta = (d1+d2+d3+d4)/4.0;
   float maxD  = max(d1, max(d2, max(d3, d4)));
   float scale = ramp(0.25, 0.40, maxD);

   const vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
   vec4 color = texture2D(Image, texcoord);
   color = black;
   color.g = scale;

   gl_FragColor = vec4(color.rgb, 1.0);
}
