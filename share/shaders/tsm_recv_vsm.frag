uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pL;
varying vec4 pT;
varying vec3 l;
varying vec3 h;
varying vec3 n;

void main() {

  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  vec3 nl = normalize(l);
  
  float ts  = 1.0 / float(shadowWidth);
  float dc  = max(0.0, dot(nn, nl));
  float sc  = pow(max(0.0, dot(nn, nh)), 36.0);
  
  //vec3 diffuse  = vec3(0.8, 0.2, 0.5);
  //vec3 specular = vec3(1.0, 1.0, 1.0);
  
  vec2 tc = 0.5 * ((pT.xy / pT.w) + vec2(1.0));
  
  // PCF 2x2 with bi-linear filtering
  //float depth = (0.5f * (pL.z / pL.w)) + 0.5f;
  float depth = pL.z / pL.w;
  // compute samples coordinates
  vec2 texcoord, t00, t10, t01, t11;
  texcoord.x = (tc.x * float(shadowWidth)) - 0.5;
  texcoord.y = (tc.y * float(shadowWidth)) - 0.5;
  float x0 = floor(texcoord.x);
  float x1 = ceil(texcoord.x);
  float fx = fract(texcoord.x);
  float y0 = floor(texcoord.y);
  float y1 = ceil(texcoord.y);
  float fy = fract(texcoord.y);
  t00 = vec2((x0+0.5)*ts, (y0+0.5)*ts);
  t10 = vec2((x1+0.5)*ts, (y0+0.5)*ts);
  t01 = vec2((x0+0.5)*ts, (y1+0.5)*ts);
  t11 = vec2((x1+0.5)*ts, (y1+0.5)*ts);
  // grab samples
  vec2 z00 = texture2D(shadowMap, t00).rg;
  vec2 z10 = texture2D(shadowMap, t10).rg;
  vec2 z01 = texture2D(shadowMap, t01).rg;
  vec2 z11 = texture2D(shadowMap, t11).rg;
  vec2 v0 = (1.0 - fx) * z00 + fx * z10;
  vec2 v1 = (1.0 - fx) * z01 + fx * z11;
  vec2 datum = (1.0 - fy) * v0 + fy * v1;
  // VSM
  float zVariance = datum.y - (datum.x * datum.x);
  float zDeviation = depth - datum.x;
  zDeviation = (zDeviation < 0.0) ? 0.0 : zDeviation;
  float visibility = zVariance / (zVariance + (zDeviation * zDeviation));
  float zTest = (depth < datum.x) ? 1.0 : 0.0;
  visibility = (zVariance > 0.0) ? visibility : zTest;
  // should lit outside texture boundaries
  visibility = (tc.x < 0.0 || tc.x > 1.0 || tc.y < 0.0 || tc.y > 1.0) ? 1.0 : visibility;
  
  //gl_FragColor.rgb = visibility * (dc*diffuse + sc*specular);
  gl_FragColor.rgb = vec3(visibility * (dc + sc));
  gl_FragColor.a = 1.0;
}

