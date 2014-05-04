#version 120

attribute float ambient_occlusion; 

varying vec4 color;
varying vec3 normal;

void main(void)
{
	gl_Position = ftransform();
    normal = normalize( gl_NormalMatrix * gl_Normal);

    if (ambient_occlusion > 0) {
       color = vec4(ambient_occlusion*gl_Color.rgb + 
          (1.0 - ambient_occlusion)*vec3(1.0, 1.0, 1.0),  gl_Color.a );
    }else {
       float saturation = abs(0.1*gl_Position.z);
       color = vec4(saturation*gl_Color.rgb + 
          (1.0 - saturation)*vec3(1.0, 1.0, 1.0),  gl_Color.a );
    }
}
