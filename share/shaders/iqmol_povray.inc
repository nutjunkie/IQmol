// Textures availble in IQmol

#declare IQmol_Texture_Chrome =
texture {
   Chrome_Metal
   pigment { color rgbt col}
}


#declare IQmol_Texture_Crumpled =
texture {
   pigment { color rgbt col }
   normal  { agate 1.0 scale 0.5 }
}


#declare IQmol_Texture_Rippled =
texture {
   pigment { color rgbt col }
   normal  { spotted 1.0 scale 0.05 }
}


#declare IQmol_Texture_Rough =
texture {
   pigment { color rgbt col }
   normal  { dents 10.0 scale 0.01 }
}


#declare IQmol_Texture_Marble =
texture {
   pigment { 
      bozo turbulence 1.96
      color_map { [0.5 rgbt col]
                  [0.6 rgbt White] 
                  [1.0 rgbt <0.5,0.5,0.5,0.0>]
      }
      scale 0.05
   }
}

#declare IQmol_Texture_Marble_with_Black =
texture {
   pigment{ 
      bozo turbulence 1.96
      color_map { [0.5 rgbt col]
                  [0.6 rgbt Black] 
                  [1.0 rgbt <0.5,0.5,0.5,0.0>]
      }
      scale 0.05
   }
}


#declare IQmol_Texture_Skin =
texture {
   pigment{ color rgbt col };
   normal {
      pigment_pattern {
         crackle turbulence 0.2
         colour_map { [ 0.00, rgb 0 ]
                      [ 0.25, rgb 1 ]
                      [ 0.95, rgb 1 ]
                      [ 1.00, rgb 0 ]
         }
         scale 0.15
      }
   }
}


#declare IQmol_Texture_Swirl =
texture {
   pigment {
      gradient y
      turbulence 5
      octaves 2
      color_map {
         [0.0, 0.8  color Clear    color Clear]
         [0.8, 1.0  color rgbt col color  rgbt col ]
      }    
      scale 0.5
   }
   finish {
      phong       0.5
      specular    0.2
      diffuse     0.9
      ambient     0.8
      ambient     0.8
      reflection  0.1
   }
}


#declare IQmol_Texture_Bubble =
texture {
   pigment { rgbf <1,1,1,0.90>  }
   finish {
       Shiny
       diffuse    0.9
       ambient    0.7
       reflection 0.05
       irid { 0.8 thickness 1 turbulence 0.1}
    }
}


#declare IQmol_Texture_Shattered =
texture {
   pigment {
      crackle scale 0.5 
      turbulence 0.4
      color_map{ [ 0.04 color Black   ]
                 [ 0.09 color Black   ]
                 [ 0.12 color rgbt col]
                 [ 1.00 color rgbt col]
      }
      scale 0.15
   } 
   normal { bumps 0.75 scale 0.02 }
   translate<0.01, 0.04, 0.00>
}


#declare IQmol_Texture_Mesh =
texture {
   pigment {
      crackle scale 1.0 turbulence 0.5
      color_map{ [ 0.04 color rgbt col ]
                 [ 0.09 color rgbt col ]
                 [ 0.12 color Clear    ]
                 [ 1.00 color Clear    ]
      }
      scale 0.10
   }
   translate<0.01, 0.04, 0.00>
   finish {
      phong       0.5
      reflection  0.0
   }
}
