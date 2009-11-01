uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pls;
varying vec3 l;
varying vec3 h;
varying vec3 n;


void main() {

  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  vec3 nl = normalize(l);
  
  float ts  = 1.0 / float(shadowWidth);
  float hts = 0.5 * ts;
  float tts = ts / 3.0;
  float dc  = max(0.0, dot(nn, nl));
  float sc  = pow(max(0.0, dot(nn, nh)), 36.0);
  
  //vec3 diffuse  = vec3(0.8, 0.2, 0.5);
  //vec3 specular = vec3(1.0, 1.0, 1.0);
  
  vec2 tc = 0.5 * ((pls.xy / pls.w) + vec2(1.0));
  
  float lit = 0.0;
  //float depth = (0.5 * (pls.z / pls.w)) + 0.5;
  float depth = (pls.z / pls.w);
  float xoff = -hts;
  
  for (int i=0; i<4; ++i, xoff+=tts) {
    float yoff = -hts;
    for (int j=0; j<4; ++j, yoff+=tts) {
      lit += float(depth < texture2D(shadowMap, tc + vec2(xoff, yoff)).r);
    }
  }
  lit /= 16.0;
  
  //gl_FragColor.rgb = lit * (dc*diffuse + sc*specular);
  gl_FragColor.rgb = vec3(lit * (dc + sc));
  gl_FragColor.a = 1.0;
}

