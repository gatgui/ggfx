
varying vec4 hpos;

void main() {
  hpos = gl_ModelViewProjectionMatrix * gl_Vertex;  
  gl_Position = hpos;
}

