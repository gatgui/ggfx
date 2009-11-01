uniform sampler2D shadowMap;

varying vec4 pls;
varying vec3 l;
varying vec3 h;
varying vec3 n;

void main() {
  
  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  vec3 nl = normalize(l);
  
  //vec3 diffuse  = vec3(0.8, 0.2, 0.5);
  //vec3 specular = vec3(1.0, 1.0, 1.0);
  
  float dc  = max(0.0, dot(nn, nl));
  float sc  = pow(max(0.0, dot(nn, nh)), 36.0);
  
  vec2 tc = (0.5 * (pls.xy / pls.w)) + 0.5;
  float depth = pls.z / pls.w;
  float depthRef = texture2D(shadowMap, tc).x;
  float lit = float(depth < depthRef);
  
  //gl_FragColor.rgb = lit * (dc*diffuse + sc*specular);
  gl_FragColor.rgb = vec3(lit * (dc + sc));
  gl_FragColor.a = 1.0;
}

