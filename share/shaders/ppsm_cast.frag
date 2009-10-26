
uniform float depthBias;

varying vec4 hpos;

void main() {
  float depth = (hpos.z / hpos.w) + depthBias;
  depth = 0.5*depth + 0.5;
  gl_FragColor.rgb = vec3(depth);
  gl_FragColor.a = 1.0;
}

