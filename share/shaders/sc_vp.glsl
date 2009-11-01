varying vec4 inPos;

void main() {
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  inPos  = gl_Position;
}
