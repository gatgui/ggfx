uniform float depthBias;

varying vec4 inPos;

void main() {
  
  float depthVal = (inPos.z / inPos.w) + depthBias;
  //depthVal = (0.5 * depthVal) + 0.5;
  
  gl_FragColor.r = depthVal;
  gl_FragColor.g = depthVal * depthVal; // VSM
  gl_FragColor.b = 0.0;
  gl_FragColor.a = 1.0;
}
