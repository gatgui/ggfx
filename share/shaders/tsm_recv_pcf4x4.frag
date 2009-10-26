uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pL;
varying vec4 pT;
varying vec3 l;
varying vec3 n;

void main() {
  
  vec3 nn = normalize(n);
  vec3 nl = normalize(l);
  
  float ts  = 1.0 / float(shadowWidth);
  float dc  = max(0.0, dot(nn, nl));
  
  vec2 tc = 0.5 * (pT.xy / pT.w) + 0.5;
  
  float depth = pL.z / pL.w;
  
  // 4x4 PCF (Gauss)
  //   Compute gauss coeff
  const int fw = 4; // --> 7x7 gauss
  float isigma = 4.0 / float(fw);
  float tmp0 = isigma * 0.3989422804; // cst: 1.0 / sqrt(2 * PI)
  float tmp1 = -0.5 * isigma * isigma;
  float coeff[fw];
  for (int k=0; k<fw; ++k) {
    coeff[k] = tmp0 * exp(float(k*k) * tmp1);
  }
  //   Compute shadow factor
  float lit = 1.0;
  for (int v=-fw+1; v<fw; v+=1) {
    float rowVal = 0.0;
    float ny = tc.y + float(v)*ts;
    if (ny<0.0 || ny>1.0) {
      for (int l=1; l<fw; ++l) {
        rowVal += coeff[l];
      }
      rowVal *= 2.0;
      rowVal += coeff[0];
    } else {
      for (int u=-fw+1; u<fw; u+=1) {
        float nx = tc.x + float(u)*ts;
        if (nx<0.0 || nx>1.0) {
          rowVal += coeff[(u < 0 ? -u : u)];
        } else {
          rowVal += coeff[(u < 0 ? -u : u)] * float(texture2D(shadowMap, vec2(nx,ny)).r > depth);
        }
      }
    }
    lit += coeff[(v < 0 ? -v : v)] * rowVal;
  }
  
  gl_FragColor.rgb = vec3(lit * dc);
  gl_FragColor.a = 1.0;
}

