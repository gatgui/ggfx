
varying vec2 tc0;

void main() {
  // pass-through
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  tc0 = gl_MultiTexCoord0.xy;
}



