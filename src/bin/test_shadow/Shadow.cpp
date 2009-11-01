#include "Shadow.h"

using namespace gmath;
using namespace ggfx;
using namespace std;

// ---

Shadow::Shadow() {
  mShadowFar = 0.0f;
  mShadowOffset = 0.6f;
  mShadowMapSize = 512.0f;
  mShadowDirLightExtrude = 100000.0f;
  mType = ST_UNIFORM;
}

Shadow::~Shadow() {
}

void Shadow::setType(Shadow::Type t) {
  mType = t;
}

void Shadow::setSceneParameters(const AABox &receiverBB, const AABox &casterBB) {
  mSceneReceiverBB = receiverBB;
  mSceneCasterBB = casterBB;
}

void Shadow::setShadowParameters(float sFar, float sOffset, float sSize, float sDLExtrude) {
  mShadowFar = sFar;
  mShadowOffset = sOffset;
  mShadowMapSize = sSize;
  mShadowDirLightExtrude = sDLExtrude;
}

void Shadow::setDirLightParameters(const Vector3 &dir) {
  mLightType = LT_DIR;
  mLightDir = dir;
}

void Shadow::setPointLightParameters(const Vector3 &pos) {
  mLightType = LT_POINT;
  mLightPos = pos;
}

void Shadow::setSpotLightParameters(
  const Vector3 &pos, const Vector3 &dir, float inner, float outer) {
  mLightType = LT_SPOT;
  mLightPos = pos;
  mLightDir = dir;
  mLightSpotInner = inner;
  mLightSpotOuter = outer;
}

void Shadow::setCameraParameters(
  float fovy, float aspect, float nplane, float fplane,
  const Matrix4 &view) {
  mCamFov = fovy;
  mCamAspect = aspect;
  mCamFar = fplane;
  mCamNear = nplane;
  mCamView = view;
  mCamViewInverse = mCamView.getFastInverse();
  mCamPos = Vector3(mCamViewInverse.getColumn(3));
  mCamDir = -Vector3(mCamViewInverse.getColumn(2));
  mCamFrustum = Frustum(PM_PERSPECTIVE, fovy, aspect, nplane, fplane);
  //mCamFrustum.update();
  //mCamHull.from(mCamViewInverse * mCamFrustum);
  mCamFrustum.transform(mCamViewInverse);
  mCamHull.from(mCamFrustum);
}

void Shadow::compute() {
  
  mFocusBody = mCamHull;
  //mFocusBody.clip(mSceneCasterBB);
  if (mLightType != LT_DIR) {
    mFocusBody.add(mLightPos);
  }
  mFocusBody.clip(mSceneCasterBB);
  // no light frustum yet
 
  if (mType == ST_CUSTOM) {
  
    switch (mLightType) {
      case LT_DIR:
        computeDirLightFrustum();
        break;
      case LT_SPOT:
        computeSpotLightFrustum();
        break;
      case LT_POINT:
      default:
        computePointLightFrustum();
        break;
    }
    updateBodies();
    
  } else if (mType == ST_UNIFORM) {
    
    
    if (mLightType == LT_POINT) {
      cout << "Point light not supported with Uniform shadows!" << endl;
      return;
    }
    
    size_t i;
    PointList pts;
    pts.build(mFocusBody);
    Vector3 up;
    for (i=0; i<pts.size(); ++i) {
      up += pts[i] - mCamPos;
    }
    up.normalize();
    Matrix4 Lv = Matrix4::MakeLookIn(mCamPos, mLightDir, up);
    // world -> light space
    AABox boxLS;
    for (i=0; i<pts.size(); ++i) {
      boxLS.merge( Lv * pts[i] );
    }
    // scale trans matrix
    Matrix4 Lp;
    Vector3 sum = boxLS.getMax() + boxLS.getMin();
    Vector3 invDelta = boxLS.getMax() - boxLS.getMin();
    invDelta.x = 1.0f / invDelta.x;
    invDelta.y = 1.0f / invDelta.y;
    invDelta.z = 1.0f / invDelta.z;
    
    Lp(0,0) = 2.0f * invDelta.x;
    Lp(0,3) = - sum.x * invDelta.x;
    Lp(1,1) = 2.0f * invDelta.y;
    Lp(1,3) = - sum.y * invDelta.y;
    Lp(2,2) = 2.0f * invDelta.z;
    Lp(2,3) = - sum.z * invDelta.z;
    
    mLightView = Lv;
    mLightProj = Matrix4::MakeScale(Vector3(1,1,-1)) * Lp;
    
    // update light hull
    
    Vector3 lf[8];
    Matrix4 iLv = Lv.getFastInverse();
    for (unsigned char j=0; j<8; ++j) {
      lf[j] = iLv * boxLS.getCorner(j);
    }
    mLightHull.from(lf);
    
  } else if (mType == ST_LiSPSM) {
    
    if (mLightType == LT_POINT) {
      cout << "Point light not supported with LiSPSM shadows!" << endl;
      return;
    }
    
    Vector3 left, up;
    Matrix4 LispM, Lv, Lp;
    
    float dotProd = mCamDir.dot(mLightDir);
    float sinGamma = float(sqrt(1.0f - dotProd*dotProd));
    
    mBodyB = mFocusBody;
    
    // calc up vec
    size_t i;
    PointList pts;
    pts.build(mFocusBody);
    for (i=0; i<pts.size(); ++i) {
      up += pts[i] - mCamPos;
    }
    up.normalize();
    left = mLightDir.cross(up).normalize();
    up = left.cross(mLightDir).normalize();
    
    Lv = Matrix4::MakeLookIn(mCamPos, mLightDir, up);
    
    // Transform B in light space and compute Cubic Hull as before
    AABox box;
    for (i=0; i<pts.size(); ++i) {
      box.merge( Lv * pts[i] );
    }
    
    // compute nOpt
    {
      float factor = 1.0f / sinGamma;
      float z_n = factor * mCamNear;
      float d = float(fabs(box.getMax().x - box.getMin().x));
      float z_f = z_n * d*sinGamma;
      float n = (z_n + sqrt(z_f * z_n)) / sinGamma;
      float f = n + d;
      Vector3 pos;
      pos = mCamPos - (n + mCamNear)*up;
      Lv = Matrix4::MakeLookIn(pos, mLightDir, up);
      
      LispM(1,1) = (f + n) / (f - n);
      LispM(1,3) = (-2 * f * n) / (f - n);
      LispM(3,1) = 1;
      LispM(3,3) = 0;
      
      Lp = LispM * Lv;
      
      box.reset();
      for (i=0; i<pts.size(); ++i) {
        box.merge( Lp * pts[i] );
      }
    }
    
    // scale fit again
    Vector3 sum = box.getMax() + box.getMin();
    Vector3 invDelta = box.getMax() - box.getMin();
    invDelta.x = 1.0f / invDelta.x;
    invDelta.y = 1.0f / invDelta.y;
    invDelta.z = 1.0f / invDelta.z;
    Lp = Matrix4::IDENTITY;
    Lp(0,0) = 2.0f * invDelta.x;
    Lp(0,3) = - sum.x * invDelta.x;
    Lp(1,1) = 2.0f * invDelta.y;
    Lp(1,3) = - sum.y * invDelta.y;
    Lp(2,2) = 2.0f * invDelta.z;
    Lp(2,3) = - sum.z * invDelta.z;
    
    Lp = Lp * LispM;
    
    mLightView = Lv;
    mLightProj = Matrix4::MakeScale(Vector3(1,1,-1)) * Lp;
    
    Vector3 lf[8];
    Matrix4 iLv = Lv.getFastInverse();
    for (unsigned char j=0; j<8; ++j) {
      lf[j] = iLv * box.getCorner(j);
    }
    mLightHull.from(lf);
  
  
  } else if (mType == ST_LiSPSM2 || mType == ST_UNIFORM2) {
    
    if (mLightType == LT_POINT) {
      cout << "Point light not supported with LiSPSM2 shadows!" << endl;
      return;
    }
    
    PointList B;
    Matrix4 Lp, Lv, L, PL;
    
    Matrix4 toCalcSpace(Matrix4::ZERO);
    toCalcSpace(0,0) = 1;
    toCalcSpace(1,2) = -1;
    toCalcSpace(2,1) = 1;
    toCalcSpace(3,3) = 1;
    
    CalcB(B);
    CalcLightSpace(Lv, Lp);
    
    Lp = toCalcSpace * Lp; // z = y, y = -z
    L = Lp * Lv;
    
    Vector3 projViewDir = GetProjViewDir(L);
    
    // LiSPSM specific
    {
      // the last pb should be here
      Lp = Matrix4::MakeLookIn(Vector3::ZERO, projViewDir, Vector3::UNIT_Y) * Lp;
      if (mType == ST_LiSPSM2 && fabs( mLightDir.dot(mCamDir) ) < 0.99f ) {
        Matrix4 LispM = CalcLispMatrix(Lp*Lv, B);
        Lp = LispM * Lp;
      }
    }
    
    PL = Lp * Lv;
    
    Lp = ScaleTranslateMatrix(PL,B) * Lp;
    
    Matrix4 toGLSpace(Matrix4::ZERO);
    toGLSpace(0,0) = 1;
    toGLSpace(1,2) = 1;
    toGLSpace(2,1) = -1;
    toGLSpace(3,3) = 1;
    
    Lp = toGLSpace * Lp;
    
    // done !
    
    mLightView = Lv;
    mLightProj = Matrix4::MakeScale(Vector3(1,1,-1)) * Lp;
  }

}

// ---

#include <limits>
#undef min
#undef max

Vector3 Shadow::CalcZ0(const Matrix4 &LS, const Vector3 &e, float zM, const Vector3 &eDir) {

  /*
  Vector3 eLS = LS * e;
  Vector3 aLS = LS * (e + eDir);
  Vector3 n = (aLS - eLS).normalize();
  float d = -eLS.dot(n);
  
  cout << "LS normal: " << n << endl;
  cout << "LS d: " << d << endl;
  
  // inverse trans
  
  // original solution
  Matrix4 itLS = LS.getInverse().getTranspose();
  Plane A(eDir, e);
  Vector4 Eq0(A.normal(), A.dist());
  A = Plane(itLS * Eq0);
  float d2 = -A.dist();
  Vector3 n2 = A.normal();
  
  cout << "LS normal 2: " << n2 << endl;
  cout << "LS distance 2: " << d2 << endl;
  
  return Vector3( eLS.x, (d - (n.z * zM) - (n.x * eLS.x)) / n.y, zM );
  */
  
  Vector3 eLS = LS * e;
  Plane A = LS * Plane(eDir, e);
  Ray ray(Vector3(eLS.x, 0.0f, zM), Vector3::UNIT_Y);
  float t;
  if (ray.intersect(A, &t)) {
    return ray.getPoint(t);
  } else {
    ray = Ray(Vector3(eLS.x, 0.0f, zM), -Vector3::UNIT_Y);
    if (ray.intersect(A, &t)) {
      return ray.getPoint(t);
    } else {
      return Vector3::ZERO;
    }
  }

}

float Shadow::CalcN(const Matrix4 &LS, const AABox &box) {
  
  Matrix4 iLS = LS.getInverse();
  
  Vector3 z0_ls = CalcZ0(LS, E, box.getMax().z, mCamDir);
  Vector3 z1_ls = Vector3(z0_ls.x, z0_ls.y, box.getMin().z);
  
  Vector3 z0_ws = iLS * z0_ls;
  Vector3 z1_ws = iLS * z1_ls;
  
  Vector3 z0_es = mCamView * z0_ws;
  Vector3 z1_es = mCamView * z1_ws;
  
  float z0 = z0_es.z;
  float z1 = z1_es.z;
  
  // check if we have to do uniform shadow mapping
	if ((z0<0 && z1>0) || (z1<0 && z0>0)) {
		// apply uniform shadow mapping
		return 0.0;
	}
	return mCamNear + sqrt( z0 * z1 ) * 5.0f;
  
  /*
  float d = fabs(box.max().z - box.min().z);
  return d / (sqrt(z1/z0) - 1);
  */
}

Matrix4 Shadow::CalcLispMatrix(const Matrix4 &LS, const PointList &B) {
  // compute B box in light soace
  size_t i;
  AABox box;
  for (i=0; i<B.size(); ++i) {
    box.merge( LS * B[i] );
  }
  // compute nOpt
  float n = CalcN(LS, box);
  cout << "Nopt = " << n << endl;
  //if (n >= std::numeric_limits<float>::infinity()) {
  //  return Matrix4::IDENTITY;
  //}
  if (n <= 0.0f) {
    return Matrix4::IDENTITY;
  }
  // compute projection center
  Vector3 e_ls = LS * E; //GetNearCameraPointE();
  Vector3 Cstart_ls(e_ls.x, e_ls.y, box.getMax().z);
  Vector3 C(Cstart_ls + n*Vector3::UNIT_Z);
  Matrix4 projCenter = Matrix4::MakeTranslate(-C);
  float d = fabs(box.getMax().z - box.getMin().z);
  //Matrix4 P = Matrix4::MakeFrustum(-1, 1, -1, 1, n, n+d);
  Matrix4 P = Matrix4::ZERO;
  P(0,0) = n;
  P(1,1) = n;
  P(2,2) = (-2*n - d) / d;
  P(2,3) = (-2 * n * (n + d)) / d;
  P(3,2) = -1;
  P = Matrix4::MakeScale(Vector3(1,1,-1)) * P;
  return P * projCenter;
}

Vector3 Shadow::GetNearCameraPointE() {
  PointList LVS;
  CalcLVS(LVS); // don't need to recalc each frame
  if (LVS.size() == 0) {
    return Vector3::ZERO;
  }
  Vector3 pW = LVS[0];
  Vector3 pE = mCamView * pW;
  float maxz = pE.z;
  int count = 1;
  for (size_t i=1; i<LVS.size(); ++i) {
    pE = mCamView * LVS[i];
    if (fabs(pE.z - maxz) < 0.001) {
      pW += LVS[i];
      ++count;
    } else if (pE.z > maxz) {
      pW = LVS[i];
      count = 1;
      maxz = pE.z;
    }
  }
  pW /= float(count);
  E = pW;
  return pW;
}

Vector3 Shadow::GetProjViewDir(const Matrix4 &LS) {
  Vector3 e = GetNearCameraPointE();
  Vector3 b = e + mCamDir;
  Vector3 e_ls = LS * e;
  Vector3 b_ls = LS * b;
  Vector3 projDir = b_ls - e_ls;
  projDir.y = 0;
  return projDir.normalize();
}

Matrix4 Shadow::CalcLightProj() {
  if (mLightType == LT_DIR) {
    return Matrix4::IDENTITY;
  } else {
    return Matrix4::MakePerspective(mCamFov, 1.0f, mCamNear, mCamFar);
  }
}

void Shadow::CalcSpotLightSpace(Matrix4 &Lv, Matrix4 &Lp, const Vector3 &up) {
  Lv = Matrix4::MakeLookIn(mLightPos, mLightDir, up);
  Lp = Matrix4::MakeScale(Vector3(1,1,-1)) * CalcLightProj();
}

void Shadow::CalcLightSpace(Matrix4 &Lv, Matrix4 &Lp) {
  if (mLightType == LT_DIR) {
    Vector3 up = mCamDir;
    Lv = Matrix4::MakeLookIn(mCamPos, mLightDir, up);
    Lp = Matrix4::IDENTITY;
  } else {
    CalcSpotLightSpace(Lv, Lp, mCamDir);
  }
}

Matrix4 Shadow::ScaleTranslateMatrix(const Matrix4 &LS, const PointList &B) {
  //AABox box;
  //for (int i=0; i<B.numPoints(); ++i) {
  //  box.merge( LS * B.point(i) );
  //}
  AABox box = (LS * B).getAAB();
  Vector3 invDelta = box.getMax() - box.getMin();
  Vector3 sum = box.getMax() + box.getMin();
  invDelta.x = 1.0f / invDelta.x;
  invDelta.y = 1.0f / invDelta.y;
  invDelta.z = 1.0f / invDelta.z;
  Matrix4 ST;
  ST(0,0) = 2.0f * invDelta.x;
  ST(0,3) = - sum.x * invDelta.x;
  ST(1,0) = 2.0f * invDelta.y;
  ST(1,3) = - sum.y * invDelta.y;
  ST(2,0) = 2.0f * invDelta.z;
  ST(2,3) = - sum.z * invDelta.z;
  return ST;
}


void Shadow::CalcFrustumPlanes(const Vector3 pts[8], Plane pl[6]) {
  
  pl[Frustum::PLANE_TOP].setup(
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_TOP|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_TOP|Frustum::CORNER_RIGHT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_TOP|Frustum::CORNER_RIGHT]);
  
  pl[Frustum::PLANE_BOTTOM].setup(
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_RIGHT]);
  
  pl[Frustum::PLANE_LEFT].setup(
    pts[Frustum::CORNER_FAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_TOP|Frustum::CORNER_LEFT]);
  
  pl[Frustum::PLANE_RIGHT].setup(
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_RIGHT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_RIGHT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_TOP|Frustum::CORNER_RIGHT]);
  
  pl[Frustum::PLANE_NEAR].setup(
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_RIGHT],
    pts[Frustum::CORNER_NEAR|Frustum::CORNER_TOP|Frustum::CORNER_RIGHT]);
  
  pl[Frustum::PLANE_FAR].setup(
    pts[Frustum::CORNER_FAR|Frustum::CORNER_BOTTOM|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_TOP|Frustum::CORNER_LEFT],
    pts[Frustum::CORNER_FAR|Frustum::CORNER_TOP|Frustum::CORNER_RIGHT]);
}

void Shadow::CalcFrustumPoints(const Matrix4 &iMpv, Vector3 pts[8], bool invZ) {
  
  AABox box;

  if (!invZ) {
    box = AABox(Vector3(-1,-1,-1), Vector3(1,1,1));
  } else {
    box = AABox(Vector3(-1,-1,1), Vector3(1,1,-1));
  }
  
  for (unsigned char i=0; i<8; ++i) {
    pts[i] = iMpv * box.getCorner(i);
  }
}

void Shadow::CalcB(PointList &B) {
  
  Matrix4 Cp = Matrix4::MakePerspective(mCamFov, mCamAspect, mCamNear, mCamFar);
  Matrix4 iCpv = (Cp * mCamView).getInverse();
  
  Vector3 pts[8];
  Plane pls[6];
  
  CalcFrustumPoints(iCpv, pts, true);
  
  mBodyB.from(pts);
  mCamHull = mBodyB;
  
  if (mLightType == LT_DIR) {
    
    mBodyB.clip(mSceneCasterBB);
    B.buildAndIncludeDir(mBodyB, mSceneCasterBB, mLightDir);
  
  } else {
    
    mBodyB.add(mLightPos);
    mBodyB.clip(mSceneCasterBB);
    
    Matrix4 Lp, Lv, iLS;
    CalcSpotLightSpace(Lv, Lp, mCamDir);
    iLS = (Lp * Lv).getInverse();
    CalcFrustumPoints(iLS, pts, false);
    mLightHull.from(pts);
    CalcFrustumPlanes(pts, pls);
    for (int j=0; j<6; ++j) {
      mBodyB.clip(pls[j]);
    }
    
    B.build(mBodyB);
  }
  
  mBodyBPts = B;
}

void Shadow::CalcLVS(PointList &LVS) {
  Matrix4 Cp = Matrix4::MakePerspective(mCamFov, mCamAspect, mCamNear, mCamFar);
  Matrix4 iCpv = (Cp * mCamView).getInverse();
  
  Vector3 pts[8];
  Plane pls[6];
  
  CalcFrustumPoints(iCpv, pts, true);
  
  mBodyLVS.from(pts);
  mBodyLVS.clip(mSceneCasterBB);
  
  if (mLightType != LT_DIR) {
  
    Matrix4 Lp, Lv, iLS;
    CalcSpotLightSpace(Lv, Lp, mCamDir);
    iLS = (Lp * Lv).getInverse();
    CalcFrustumPoints(iLS, pts, false);
    CalcFrustumPlanes(pts, pls);
    for (int i=0; i<6; ++i) {
      mBodyLVS.clip(pls[i]);
    }
  }
  
  LVS.build(mBodyLVS);
  
  mBodyLVSPts = LVS;
}

// ---

const Matrix4 Shadow::getLightProj() const {
  return mLightProj;
}

const Matrix4 Shadow::getLightView() const {
  return mLightView;
}

const ConvexHull3D Shadow::getBodyB() const {
  return mBodyB;
}

const ConvexHull3D Shadow::getBodyLVS() const {
  return mBodyLVS;
}

const ConvexHull3D Shadow::getBodyFocus() const {
  return mFocusBody;
}

const Frustum Shadow::getLightFrustum() const {
  return mLightFrustum;
}

const Frustum Shadow::getCameraFrustum() const {
  return mCamFrustum;
}

const ConvexHull3D Shadow::getBodyLight() const {
  return mLightHull;
}

const ConvexHull3D Shadow::getBodyCam() const {
  return mCamHull;
}

float Shadow::getShadowDist() {
  float shadowDist = mShadowFar;
  if (shadowDist <= 0) {
    shadowDist = 300.f * mCamNear;
  }
  return shadowDist;
}

float Shadow::getShadowOffset() {
  return getShadowDist() * mShadowOffset;
}

Vector3 Shadow::getUpForDir(const Vector3 &dir) {
  Vector3 up = Vector3::UNIT_Y;
  if (fabs(up.dot(dir)) >= 1.0f) {
    up = Vector3::UNIT_Z;
  }
  return up;
}

void Shadow::computeDirLightFrustum() {
  Vector3 pos = (mCamPos + (getShadowOffset() * mCamDir)) - (mShadowDirLightExtrude * mLightDir);
  mLightMode = PM_ORTHOGRAPHIC;
  mLightFov = 90.0f;
  mLightNear = getShadowDist();
  mLightFar = 100000.0f; // ?
  mLightView = Matrix4::MakeLookIn(pos, mLightDir, getUpForDir(mLightDir));
  updateLightFrustum();
}

void Shadow::computeSpotLightFrustum() {
  mLightMode = PM_PERSPECTIVE;
  mLightFov = 1.2f * mLightSpotOuter;
  mLightNear = mCamNear;
  mLightFar = getShadowDist();//100000.0f; // ?
  mLightView = Matrix4::MakeLookIn(mLightPos, mLightDir, getUpForDir(mLightDir));
  updateLightFrustum();
}

void Shadow::computePointLightFrustum() {
  //Vector3 aim = mCamPos + (getShadowOffset() * mCamDir);
  //Vector3 dir = aim - mLightPos;
  //dir.normalize();
  Vector3 dir;
  PointList focusPoints;
  focusPoints.build(mFocusBody);
  for (size_t i=0; i<focusPoints.size(); ++i) {
    Vector3 v = focusPoints[i] - mLightPos;
    dir += v;
  }
  dir.normalize();
  mLightMode = PM_PERSPECTIVE;
  mLightFov = 120.0f;
  mLightNear = mCamNear;
  mLightFar = getShadowDist();//100000.0f; // ?
  //mLightView = Matrix4::MakeLookAt(mLightPos, aim, getUpForDir(dir)); //mCamDir);
  mLightView = Matrix4::MakeLookIn(mLightPos, dir, getUpForDir(dir));
  updateLightFrustum();
}

void Shadow::updateLightFrustum() {
  mLightFrustum = Frustum(mLightMode, mLightFov, 1.0f, mLightNear, mLightFar);
  mLightFrustum.update();
  mLightProj = mLightFrustum.getProjectionMatrix();
  //Matrix4 LS = mLightProj * mLightView;
  //Matrix4 ST = ScaleTranslateMatrix(LS, mBodyLVSPts); // mBodyBPts
  //mLightProj = ST * mLightProj;
  Frustum tmp(mLightFrustum);
  tmp.transform(mLightView.getFastInverse());
  mLightHull.from(tmp);
  //mLightHull.from(mLightView.getFastInverse() * mLightFrustum);
}

void Shadow::updateBodies() {
  
  /*
  mBodyB = mCamHull;
  mBodyB.add(mLightPos);
  mBodyB.clip(mSceneCasterBB);
  mBodyB.clip(mLightHull);
  mBodyBPts.build(mBodyB);
  
  mBodyLVS = mLightHull;
  mBodyLVS.clip(mSceneCasterBB);
  mBodyLVS.clip(mCamHull);
  mBodyLVSPts.build(mBodyLVS);
  */
  
  mBodyB = mCamHull;
  if (mLightType != LT_DIR) {
    mBodyB.add(mLightPos);
    mBodyB.clip(mSceneCasterBB);
    mBodyB.clip(mLightHull);
    mBodyBPts.build(mBodyB);
  } else {
    mBodyB.clip(mSceneCasterBB);
    mBodyBPts.buildAndIncludeDir(mBodyB, mSceneCasterBB, mLightDir);
  }
  
  mBodyLVS = mCamHull;
  mBodyLVS.clip(mSceneCasterBB);
  if (mLightType != LT_DIR) {
    mBodyLVS.clip(mLightHull);
  }
  mBodyLVSPts.build(mBodyLVS);
}

