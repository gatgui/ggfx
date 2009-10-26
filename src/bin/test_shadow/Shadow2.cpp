#include "Shadow2.h"
using namespace std;
using namespace gmath;
using namespace ggfx;

const Matrix4 Shadow2::NormalToLightSpace(1,  0,  0,  0,
																			    0,  0, -1,  0,
																			    0,  1,  0,  0,
																			    0,  0,  0,  1 );

const Matrix4 Shadow2::LightToNormalSpace(1,  0,  0,  0,
																			    0,  0,  1,  0,
																			    0, -1,  0,  0,
																			    0,  0,  0,  1 );

const Matrix4 Shadow2::InvZ = Matrix4::MakeScale(Vector3(1, 1, -1));

Shadow2::Shadow2() {
  mShadowFar = 0.0f;
  mShadowOffset = 0.6f;
  mShadowMapSize = 512.0f;
  mShadowDirLightExtrude = 10000.0f;
  mLiSPSM = false;
  mTSM = false;
  mFocusZone = 0.4f;
  mTSMD.step = 8;
  mTSMD.Nt_step[0] = Matrix4::IDENTITY;
}

Shadow2::~Shadow2() {
}

void Shadow2::setSceneParameters(const AABox &receiverBB, const AABox &casterBB) {
  mSceneReceiverBB = receiverBB;
  mSceneCasterBB = casterBB;
}

void Shadow2::setShadowParameters(float sFar, float sOffset, float sSize, float sDLExtrude) {
  mShadowFar = sFar;
  mShadowOffset = sOffset;
  mShadowMapSize = sSize;
  mShadowDirLightExtrude = sDLExtrude;
}

void Shadow2::setDirLightParameters(const Vector3 &dir) {
  mLightType = LT_DIR;
  mLightDir = dir;
}

void Shadow2::setPointLightParameters(const Vector3 &pos) {
  mLightType = LT_POINT;
  mLightPos = pos;
}

void Shadow2::setSpotLightParameters(const Vector3 &pos, const Vector3 &dir, float inner, float outer) {
  mLightType = LT_SPOT;
  mLightPos = pos;
  mLightDir = dir;
  mLightSpotInner = inner;
  mLightSpotOuter = outer;
}

void Shadow2::setCameraParameters(float fovy, float aspect, float nplane, float fplane, const Matrix4 &view) {
  mCamFov = fovy;
  mCamAspect = aspect;
  mCamFar = fplane;
  mCamNear = nplane;
  mCamView = view;
  mCamViewInverse = mCamView.getFastInverse();
  mCamPos = Vector3(mCamViewInverse.getColumn(3));
  mCamDir = -Vector3(mCamViewInverse.getColumn(2));
  // compute frustum [if camera WERE a frustum would be easier ^^]
  mCamFrustum.setMode(PM_PERSPECTIVE);
  mCamFrustum.setAspect(aspect);
  mCamFrustum.setFovy(fovy);
  mCamFrustum.setNearClip(mCamNear);
  mCamFrustum.setFarClip(mCamFar);
  //mCamFrustum.update();
  //mCamFrustum = mCamViewInverse * mCamFrustum;
  mCamFrustum.transform(mCamViewInverse);
}

float Shadow2::getShadowDist() const {
  float shadowDist = mShadowFar;
  if (shadowDist <= 0) {
    shadowDist = 3000.f * mCamNear;
  }
  return shadowDist;
}

float Shadow2::getShadowOffset() const {
  return mShadowOffset * getShadowDist();
}

void Shadow2::calcShadowMatrix(Matrix4 *P, Matrix4 *V, Frustum *frustum) {
  Frustum tmpFrustum;
  Matrix4 Lp, Lv;
  float shadowOffset = getShadowOffset();
  
  // should find a better way to compute light camera near and far plane
  
  if (mLightType == LT_DIR) {
    Vector3 pos = (mCamPos + (shadowOffset * mCamDir)) - (mShadowDirLightExtrude * mLightDir);
    //Lv = Matrix4::MakeLookIn(mCamPos, mLightDir, mCamDir);
    Lv = Matrix4::MakeLookIn(pos, mLightDir, mCamDir);
    tmpFrustum.setMode(PM_ORTHOGRAPHIC);
    tmpFrustum.setNearClip(getShadowDist());
    tmpFrustum.setFovy(90.0f);
    //tmpFrustum.setAspect(mCamAspect);
    tmpFrustum.setAspect(1);
    tmpFrustum.update();
    //Lp = Matrix4::IDENTITY;
    Lp = tmpFrustum.getProjectionMatrix();
    
  } else if (mLightType == LT_POINT) {
    //Vector3 dir = ((mCamPos + shadowOffset*mCamDir) - mLightPos).normalize();
    Vector3 dir;
    for (size_t i=0; i<mFocusPts.size(); ++i) {
      dir += (mFocusPts[i] - mLightPos);
    }
    dir.normalize();
    Lv = Matrix4::MakeLookIn(mLightPos, dir, mCamDir);
    tmpFrustum.setMode(PM_PERSPECTIVE);
    tmpFrustum.setNearClip(mCamNear);
    tmpFrustum.setFovy(120.0f);
    //tmpFrustum.setAspect(mCamAspect);
    tmpFrustum.setAspect(1);
    tmpFrustum.update();
    Lp = tmpFrustum.getProjectionMatrix();
    
  } else {
    Lv = Matrix4::MakeLookIn(mLightPos, mLightDir, mCamDir);
    tmpFrustum.setMode(PM_PERSPECTIVE);
    tmpFrustum.setNearClip(mCamNear);
    tmpFrustum.setFovy(1.2f * mLightSpotOuter);
    //tmpFrustum.setAspect(mCamAspect);
    tmpFrustum.setAspect(1);
    tmpFrustum.update();
    Lp = tmpFrustum.getProjectionMatrix();
    
  }
  if (P) {
    *P = Lp;
  }
  if (V) {
    *V = Lv;
  }
  if (frustum) {
    //*frustum = Lv.getFastInverse() * tmpFrustum; // light space -> world space
    *frustum = tmpFrustum;
    frustum->transform(Lv.getFastInverse());
  }
}

void Shadow2::calcB(PointList *pts, ConvexHull3D *hull) {
  
  PointList B;
  ConvexHull3D BodyB;
  
  BodyB.from(mCamFrustum);
  
  if (mLightType != LT_DIR) {
    
    BodyB.add(mLightPos);
    BodyB.clip(mSceneCasterBB);
    
    calcShadowMatrix(NULL, NULL, &mLightFrustum);
    BodyB.clip(mLightFrustum);
    
    B.build(BodyB);
    
  } else {
    
    BodyB.clip(mSceneCasterBB);
    B.buildAndIncludeDir(BodyB, mSceneCasterBB, -mLightDir);
  }
  
  if (pts) {
    *pts = B;
  }
  if (hull) {
    *hull = BodyB;
  }
}

void Shadow2::calcLVS(PointList *pts, ConvexHull3D *hull) {
  
  PointList LVS;
  ConvexHull3D BodyLVS;
  
  BodyLVS.from(mCamFrustum);
  
  if (mLightType != LT_DIR) {
    
    calcShadowMatrix(NULL, NULL, &mLightFrustum);    
    BodyLVS.clip(mLightFrustum);
  }
  
  BodyLVS.clip(mSceneCasterBB);
  
  LVS.build(BodyLVS);
  
  if (pts) {
    *pts = LVS;
  }
  if (hull) {
    *hull = BodyLVS;
  }
}

Vector3 Shadow2::calcNearCameraPoint(const PointList &pts) {
  if (pts.size() == 0) {
	  return Vector3(0,0,0);
	}
	int count = 1;
	Vector3 nearW = pts[0];
	Vector3 nearE = mCamView * nearW;
	float nearZ = nearE.z;
	for (size_t i=1; i<pts.size(); ++i) {
	  const Vector3 &ptW = pts[i];
	  Vector3 ptE = mCamView * ptW;
	  if (fabs(ptE.z - nearZ) < 0.001) {
	    nearW += ptW;
	    count += 1;
	  } else if (ptE.z > nearZ) {
	    nearW = ptW;
	    nearZ = ptE.z;
	    count = 1;
	  }
	}
	mE = nearW / float(count);
	return mE;
}


Vector3 Shadow2::calcProjViewDir(const Matrix4 &LS, const PointList &pts) {
  Vector3 eW = calcNearCameraPoint(pts);
  Vector3 bW = eW + mCamDir;
  Vector3 eLS = LS * eW;
  Vector3 bLS = LS * bW;
  Vector3 dir = bLS = eLS;
  dir.y = 0.0f;
  return dir.normalize();
}

Matrix4 Shadow2::calcUnitCubeTransform(const Matrix4 &LS, const PointList &pts) {
  
  // that's basicaly an orthograhic projection !
  
  PointList ptsLS = LS * pts;
  
  Vector3 s = ptsLS.getAAB().getMax() + ptsLS.getAAB().getMin();
  Vector3 id = ptsLS.getAAB().getMax() - ptsLS.getAAB().getMin();
  id.x = 1.0f / id.x;
  id.y = 1.0f / id.y;
  id.z = 1.0f / id.z;
  
  Vector3 trans = - s * id;
  Vector3 scale = 2 * id;
  
  return Matrix4(
    scale.x, 0, 0, trans.x,
    0, scale.y, 0, trans.y,
    0, 0, scale.z, trans.z, // should be -scale.z then ?
    0, 0, 0, 1);
}

Vector3 Shadow2::calcZ0(const Matrix4 &LS, float z) {
  
  Vector3 eLS = LS * getE();
  
  Plane plane(mCamDir, getE());
  
  plane.transform(LS);
  
  Ray ray(Vector3(eLS.x, 0.0f, z), Vector3::UNIT_Y);
  float t;
  
  if (ray.intersect(plane, &t)) {
    return ray.getPoint(t);
  } else {
    ray = Ray(Vector3(eLS.x, 0.0f, z), -Vector3::UNIT_Y);
    if (ray.intersect(plane, &t)) {
      return ray.getPoint(t);
    } else {
      return Vector3::ZERO;
    }
  }
}

float Shadow2::calcN(const Matrix4 &LS, const AABox &bBoxLS) {
  
  Vector3 z0LS = calcZ0(LS, bBoxLS.getMax().z);
  Vector3 z1LS = Vector3(z0LS.x, z0LS.y, bBoxLS.getMin().z);
  
  Matrix4 toEye = mCamView * LS.getInverse();
  
  float z0 = (toEye * z0LS).z;
  float z1 = (toEye * z1LS).z;
  
  if ((z0<0 && z1>0) || (z0>0 && z1<0)) {
    return 0.0f;
  }
  
#if 0
  float d = float(fabs(bBoxLS.getMax().z - bBoxLS.getMin().z));
	return d / ( float(sqrt(z1/z0)) - 1.0f );
#else
# if 0
	float d = float(fabs(mCamFar - mCamNear));
  float dp = mCamDir.dot(mLightDir); // cosGamma
  float sinGamma = float(sqrt(1.0f - dp*dp));
	return (mCamNear + float(sqrt(mCamNear * (mCamNear + d*sinGamma)))) / sinGamma;
# else
  return (mCamNear + float(sqrt(z0 * z1) * 5.0f));
# endif
#endif
}

Matrix4 Shadow2::calcLiSPSM(const Matrix4 &LS, const PointList &B) {
  
  PointList bLS = LS * B;
  
  Vector3 eLS = LS * getE();
  
  float nOpt = calcN(LS, bLS.getAAB());
  
  if (nOpt <= 0.0f) {
    return Matrix4::IDENTITY;
  }
  
  Vector3 C = Vector3(eLS.x, eLS.y, bLS.getAAB().getMax().z) + nOpt * Vector3::UNIT_Z;
  
  Matrix4 T = Matrix4::MakeTranslate(-C);
  
  float d = float(fabs(bLS.getAAB().getMax().z - bLS.getAAB().getMin().z));
  
  Matrix4 P = Matrix4::MakeFrustum(-1, 1, -1, 1, nOpt, nOpt+d);
  
  return InvZ * P * T;
}

void Shadow2::compute() {
  
  if (mLightType == LT_POINT) {
    ConvexHull3D focusBody(mCamFrustum);
    focusBody.add(mLightPos);
    focusBody.clip(mSceneCasterBB);
    mFocusPts.build(focusBody);
  }
  
  calcShadowMatrix(&mLightProj, &mLightView, NULL);
  
  if (mSceneCasterBB.isNull()) {
    return;
  }
  
  PointList B, LVS;
  
  calcB(&B, NULL);
  if (B.size() == 0) {
    return;
  }
  
  if (mTSM) {
    
    size_t i, j;
    
    // use B
    Matrix4 LS = mLightProj * mLightView;
    
    Vector3 C, E, F, tsmDir;
    
    E = mCamPos;
    for (i=0; i<B.size(); ++i) {
      tsmDir += (B[i] - mCamPos).normalize();
    }
    tsmDir.normalize();
    float newNear, newFar;
    newNear = newFar = (B[0] - E).dot(tsmDir);
    for (i=1; i<B.size(); ++i) {
      float cur = (B[i] - E).dot(tsmDir);
      if (cur < newNear) {
        newNear = cur;
      } else if (cur > newFar) {
        newFar = cur;
      }
    }
    float newFocusFar = newNear + (mFocusZone * (newFar - newNear));
    C = E + newFar*tsmDir;
    F = E + newFocusFar*tsmDir;
    E = E + newNear*tsmDir;
    // that is ... in eye space of course !
    
    // Transform to light space
    E = LS * E;
    F = LS * F;
    C = LS * C;
    B = LS * B;
    
    // Translate to 2D
    ConvexHull2D::PointArray pts;
    for (i=0; i<B.size(); ++i) {
      pts.push_back(Vector2(B[i].x, B[i].y));
    }
    Vector2 nMid(E.x, E.y);
    Vector2 fMid(C.x, C.y);
    Vector2 fFoc(F.x, F.y);
    ConvexHull2D hull(pts);
    //cout << "Hull2D: " << hull << endl;
    // Find top and bottom
    Vector2 l = (fMid - nMid).normalize();
    float top, bottom;
    top = bottom = 0;
    if (fMid == nMid) {
      cout << "Doh !!!" << endl;
      mTSMD.Nt = Matrix4::IDENTITY;
      return;
      
    } else {
      float cur;
      top = (hull[0] - nMid).dot(l);
      bottom = top;
      for (j=1; j<hull.size(); ++j) {
        cur = (hull[j] - nMid).dot(l);
        if (cur < top) {
          top = cur;
        } else if (cur > bottom) {
          bottom = cur;
        }
      }
    }
    // Find center of projection
    float lambda = fabs(top - bottom);
    float delta = (fFoc - nMid).dot(l) - top; // delta'
    float epsilon = -1.0f + (2.0f * mFocusZone); // -0.6f;
    float n = fabs(top + ((lambda * delta * (1 + epsilon)) / (lambda * (1 - epsilon) - 2*delta)));
    Vector2 q = nMid + (top - n) * l;
    // Find left most and right most convex hull point that maximize the angle
    Vector2 rightMost = fMid;
    Vector2 leftMost = fMid;
    Vector2 to = q + l;
    for (i=0; i<hull.size(); ++i) {
      float tmp = ConvexHull2D::IsLeft(q, to, hull[i]);
      if (fabs(tmp) < 0.000001) {
        // same line
        continue;
      } else if (tmp > 0) {
        // on left side
        if (ConvexHull2D::IsLeft(q, leftMost, hull[i]) >= 0) {
          leftMost = hull[i];
        }
      } else if (tmp < 0) {
        // on right side
        if (ConvexHull2D::IsLeft(q, rightMost, hull[i]) <= 0) {
          rightMost = hull[i];
        }
      }
    }
    
    // Try auto-adjusting
    // now we have left and right, in fact, the center is pretty bad in some cases
    // arrange so that center -> left vector forms an angle of 90 degree with center -> right
    if (mTSMAdjust) {
      //float a = RadToDeg * (rightMost-q).normalize().dot((leftMost-q).normalize());
      float a = ToDegree( (rightMost-q).normalize().dot((leftMost-q).normalize()) );
      cout << "Angle: (n = " << n << ") " << a << endl;
      int cnt = 0;
      while (a < 50) {
        n *= 2;
        q = nMid + (top-n)*l;
        // should recompute left / right
      
        rightMost = fMid;
        leftMost = fMid;
        to = q + l;
        for (i=0; i<hull.size(); ++i) {
          float tmp = ConvexHull2D::IsLeft(q, to, hull[i]);
          if (fabs(tmp) < 0.000001) {
            continue;
          } else if (tmp > 0) {
            if (ConvexHull2D::IsLeft(q, leftMost, hull[i]) >= 0) {
              leftMost = hull[i];
            }
          } else {
            if (ConvexHull2D::IsLeft(q, rightMost, hull[i]) <= 0) {
              rightMost = hull[i];
            }
          }
        }
      
        //a = RadToDeg * (rightMost-q).normalize().dot((leftMost-q).normalize());
        a = ToDegree( (rightMost-q).normalize().dot((leftMost-q).normalize()) );
      
        ++cnt;
      }
      cout << "==> Angle (n = " << n << ") [in " << cnt << " steps]: " << a << endl;
    }
    
    // Compute the 4 vertices of the trapezoid
    Vector2 t0, t1, t2, t3; // t2, t3 (near), t0, t1 (far)
    Vector2 e;
    float dp;
    e = (rightMost - q).normalize();
    dp = e.dot(l);
    if (fabs(dp) < 0.000001) {
      t3 = q + n*l;
      t0 = q + (n+lambda)*l;
    } else {
      dp = 1.0f / dp;
      t3 = q + (n * dp)*e;
      t0 = q + ((lambda + n) * dp)*e;
    }
    e = (leftMost - q).normalize();
    dp = e.dot(l);
    if (fabs(dp) < 0.000001) {
      t2 = q + n*l;
      t1 = q + (n+lambda)*l;
    } else {
      dp = 1.0f / dp;
      t2 = q + (n * dp)*e;
      t1 = q + ((lambda + n) * dp)*e;
    }
    
    // Update TSM debug data
    mTSMD.B = pts;
    mTSMD.hull = hull;
    mTSMD.nMid = nMid;
    mTSMD.fMid = fMid;
    mTSMD.fFoc = fFoc;
    mTSMD.l = l;
    mTSMD.top = top;
    mTSMD.bottom = bottom;
    mTSMD.n = n;
    mTSMD.q = q;
    mTSMD.right = rightMost;
    mTSMD.left = leftMost;
    mTSMD.t0 = t0;
    mTSMD.t1 = t1;
    mTSMD.t2 = t2;
    mTSMD.t3 = t3;
    
    // Compute transform to map trapezoid to unit square
    Vector4 t[4] = {
      Vector4(t0.x, t0.y, 1, 1),
      Vector4(t1.x, t1.y, 1, 1),
      Vector4(t2.x, t2.y, 1, 1),
      Vector4(t3.x, t3.y, 1, 1)
    };
    Vector4 o(q.x, q.y, 1, 1);
    Vector4 u, v;
    Matrix4 T1, R, T2, H, S1, N, T3, S2;
    u = 0.5f * (t[2] + t[3]);
    T1 = Matrix4::MakeTranslate(Vector3(-u.x, -u.y, 0));
    mTSMD.Nt_step[1] = T1;
    u = (t[2]-t[3]);
    u /= float(sqrt(u.x*u.x + u.y*u.y));
    R(0,0) = -u.x; // u.x;
    R(0,1) = -u.y; // u.y;
    R(1,0) =  u.y; //-u.y;
    R(1,1) = -u.x; // u.x;
    mTSMD.Nt = R * T1;
    mTSMD.Nt_step[2] = mTSMD.Nt;
    u = mTSMD.Nt * o;
    T2 = Matrix4::MakeTranslate(Vector3(-u.x, -u.y, 0));
    mTSMD.Nt = T2 * mTSMD.Nt;
    mTSMD.Nt_step[3] = mTSMD.Nt;
    u = 0.5f * (mTSMD.Nt * (t[2] + t[3]));
    H = Matrix4::IDENTITY;
    H(0,1) = - u.x / u.y;
    mTSMD.Nt = H * mTSMD.Nt;
    mTSMD.Nt_step[4] = mTSMD.Nt;
    u = mTSMD.Nt * t[2];
    S1 = Matrix4::MakeScale(Vector3(1.0f/u.x, 1.0f/u.y, 1));
    mTSMD.Nt_step[5] = S1 * mTSMD.Nt;
    N = Matrix4(1,0,0,0, 0,1,0,1, 0,0,1,0, 0,1,0,0);
    mTSMD.Nt = N * S1 * mTSMD.Nt;
    mTSMD.Nt_step[6] = mTSMD.Nt;
    u = mTSMD.Nt * t[0];
    v = mTSMD.Nt * t[2];
    T3(1,3) = -0.5f * (u.y/u.w + v.y/v.w);
    mTSMD.Nt = T3 * mTSMD.Nt;
    mTSMD.Nt_step[7] = mTSMD.Nt;
    u = mTSMD.Nt * t[0];
    S2 = Matrix4::MakeScale(Vector3(1, -u.w/u.y, 1));
    mTSMD.Nt = S2 * mTSMD.Nt;
    mTSMD.Nt_step[8] = mTSMD.Nt;
    
    mLightView = LS;
    mLightProj = mTSMD.Nt_step[mTSMD.step];
    
  } else { 
  
    calcLVS(&LVS, NULL);
  
    mLightProj = InvZ * mLightProj;
    mLightProj = NormalToLightSpace * mLightProj; // viewing along y+ instead of z-
  
    Matrix4 LS = mLightProj * mLightView;
    Vector3 dir = calcProjViewDir(LS, LVS);
    Matrix4 R = Matrix4::MakeLookIn(Vector3::ZERO, dir, Vector3::UNIT_Y);
    mLightProj = R * mLightProj;
    
    if (mLiSPSM == true) {
      float cosGamma = mLightDir.dot(mCamDir);
      if (fabs(cosGamma) < 0.99) {
        // have nearly no impact... sucks !
        LS = mLightProj * mLightView;
        Matrix4 LiSPM = calcLiSPSM(LS, B);
        cout << "LiSPM: " << LiSPM << endl;
        mLightProj = LiSPM * mLightProj;
      }
    } // else: default focused shadow maps
    
    LS = mLightProj * mLightView;
    Matrix4 UCT = calcUnitCubeTransform(LS, B);
    mLightProj = UCT * mLightProj;
    
    mLightProj = LightToNormalSpace * mLightProj; // set back matrix to view on z-
    mLightProj = InvZ * mLightProj;
  }
}
