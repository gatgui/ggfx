
varying vec4 epos;
varying vec4 hpos;

void main() {
  
  epos = gl_ModelViewMatrix * gl_Vertex;
    
  hpos = gl_ModelViewProjectionMatrix * gl_Vertex;
  
  gl_Position = hpos;
}

