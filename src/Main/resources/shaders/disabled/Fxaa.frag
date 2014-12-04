uniform sampler2D Image;
uniform vec2 Image_d;

varying vec4 color;
varying vec2 texcoord;

void main() 
{

   vec2 texcoordOffset = vec2(Image_d.x, Image_d.y);

  // The parameters are hardcoded for now, but could be
  // made into uniforms to control from the program.
  float FXAA_SPAN_MAX = 8.0;
  float FXAA_REDUCE_MUL = 1.0/8.0;
  float FXAA_REDUCE_MIN = (1.0/128.0);

  vec3 rgbNW = texture2D(Image, texcoord + (vec2(-1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbNE = texture2D(Image, texcoord + (vec2(+1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbSW = texture2D(Image, texcoord + (vec2(-1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbSE = texture2D(Image, texcoord + (vec2(+1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbM  = texture2D(Image, texcoord).xyz;
	
  vec3 luma = vec3(0.299, 0.587, 0.114);
  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM  = dot( rgbM, luma);
	
  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	  
  float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	
  dir = min(vec2(FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * texcoordOffset;
		
  vec3 rgbA = 0.5 * (
              texture2D(Image, texcoord + dir * (1.0/3.0 - 0.5)).xyz +
              texture2D(Image, texcoord + dir * (2.0/3.0 - 0.5)).xyz);

  vec3 rgbB = 0.5*rgbA  + 0.25 * (
              texture2D(Image, texcoord + dir * (0.0/3.0 - 0.5)).xyz +
              texture2D(Image, texcoord + dir * (3.0/3.0 - 0.5)).xyz);

  float lumaB = dot(rgbB, luma);

  if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
    gl_FragColor.rgb = rgbA;
  } else {
    gl_FragColor.rgb = rgbB;
  }
    gl_FragColor.rgb = rgbA;

  gl_FragColor.a = 1.0;
    
  gl_FragColor *= color;
  //gl_FragColor = vec4(rgbM, 1.0);

}
