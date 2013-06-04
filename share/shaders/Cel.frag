#version 120

varying vec3  normal;
varying vec4  color;
varying float scale;
varying float spread;

void main()
{
	vec3 n = normalize(normal);
    vec3 lightPosition = vec3(gl_LightSource[0].position);
    lightPosition = normalize(vec3(0.2, 0.3, 1.0));
	float intensity = dot(lightPosition,n);

    vec3 c;

	if (intensity > pow(spread, 0.10)) { 
       c = (1.0+scale)*color.rgb;

       // Redistribute the intensity if we blow out a channel
       const float thresh = 0.999;
       float r = c.r;
       float g = c.g;
       float b = c.b;
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
    }else if (intensity > spread) {
        c = color.rgb;
	}else if (intensity > spread*spread) {
        c = (1.0-scale)*color.rgb;
	}else {
        c = (1.0-2.0*scale)*color.rgb;
    }

	gl_FragColor = vec4(c, color.a);
}
