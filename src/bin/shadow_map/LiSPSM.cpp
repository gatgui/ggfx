#include "LiSPSM.h"
using namespace std;
using namespace gmath;
using namespace ggfx;

const Matrix4 LiSPSM::NormalToLightSpace(1,  0,  0,  0,
																			    0,  0, -1,  0,
																			    0,  1,  0,  0,
																			    0,  0,  0,  1 );

const Matrix4 LiSPSM::LightToNormalSpace(1,  0,  0,  0,
																			    0,  0,  1,  0,
																			    0, -1,  0,  0,
																			    0,  0,  0,  1 );

const Matrix4 LiSPSM::InvZ = Matrix4::MakeScale(Vector3(1, 1, -1));

LiSPSM::LiSPSM(const ISceneData *scn)
  :ShadowAlgorithm(scn), mPCF(false) {
}

LiSPSM::~LiSPSM() {
}

void LiSPSM::enablePCF(bool on) {
  /*
  if (mPCF == on) {
    return;
  }
  mPCF = on;
  ProgramManager &mgr = ProgramManager::Instance();
  mReceiverProg->bind();
  if (mPCF) {
    mReceiverProg->detach(mgr.getShader("LiSPSM_Receiver_fs"));
    mReceiverProg->attach(mgr.getShader("LiSPSM_Receiver_fs_pcf2x2"));
  } else {
    mReceiverProg->detach(mgr.getShader("LiSPSM_Receiver_fs_pcf2x2"));
    mReceiverProg->attach(mgr.getShader("LiSPSM_Receiver_fs"));
  }
  mReceiverProg->link();
  */
}

void LiSPSM::initializePrograms() {
  ProgramManager &mgr = ProgramManager::Instance();
  mCasterProg = mgr.createProgram("LiSPSM_Caster",
                                  "share/shaders/sc_vp.glsl",
                                  "share/shaders/sc_fp.glsl");
  //mReceiverProg = mgr.createProgram("LiSPSM_Receiver",
  //                                  "share/shaders/sr_vp.glsl",
  //                                  "share/shaders/sr_fp.glsl");
  //mgr.createShader("LiSPSM_Receiver_fs_pcf2x2", Shader::ST_FRAGMENT,
  //                 "share/shaders/sr_pcf2x2_fp.glsl");
  mReceiverProg = mgr.createProgram("LiSPSM_Receiver",
                                    "share/shaders/sr_vp.glsl",
                                    "share/shaders/sr_pcf2x2_fp.glsl");
  mReceiverProgNL = mgr.createProgram("LiSPSM_ReceiverNL",
                                      "share/shaders/sr_vp_noDiffuse.glsl",
                                      "share/shaders/sr_pcf2x2_fp_noDiffuse.glsl");
}

void LiSPSM::setupCasterProgram() {
  mReceiverProg->uniform("depthBias")->set(mDepthBias);
}

void LiSPSM::setupReceiverProgram(bool noLighting) {
  Matrix4 eyeToLightProj = mLightProj * mLightView * mScn->getCameraInverseViewMatrix();
  //mReceiverProg->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  //mReceiverProg->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  //mReceiverProg->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  //mReceiverProg->uniform("eyeToLightProj")->set(eyeToLightProj);
  mBoundReceiver->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  mBoundReceiver->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  mBoundReceiver->uniform("eyeToLightProj")->set(eyeToLightProj);
  if (noLighting == false) {
    mBoundReceiver->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  }
}

void LiSPSM::calcB(PointList &pts) {
  ConvexHull3D body(mCameraHull);
  if (mLightType != LT_DIR) {
    body.add(mLightPos);
    body.clip(mScn->getSceneBox());
    body.clip(mLightFrustum);
    pts.build(body);
  } else {
    body.clip(mScn->getSceneBox());
    pts.buildAndIncludeDir(body, mScn->getSceneBox(), -mLightDir);
  }
}

void LiSPSM::calcLVS(PointList &pts) {
  ConvexHull3D body(mCameraHull);
  if (mLightType != LT_DIR) {
    body.clip(mLightFrustum);
  }
  body.clip(mScn->getSceneBox());
  pts.build(body);
}

Vector3 LiSPSM::calcNearCameraPoint(const PointList &pts) {
  if (pts.size() == 0) {
	  return Vector3(0,0,0);
	}
	int count = 1;
	Vector3 nearW = pts[0];
	Vector3 nearE = mScn->getCameraViewMatrix() * nearW;
	float nearZ = nearE.z;
	for (size_t i=1; i<pts.size(); ++i) {
	  const Vector3 &ptW = pts[i];
	  Vector3 ptE = mScn->getCameraViewMatrix() * ptW;
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

Vector3 LiSPSM::calcProjViewDir(const Matrix4 &LS, const PointList &pts) {
  Vector3 eW = calcNearCameraPoint(pts);
  Vector3 bW = eW + mScn->getCameraViewDirection();
  Vector3 eLS = LS * eW;
  Vector3 bLS = LS * bW;
  Vector3 dir = bLS = eLS;
  dir.y = 0.0f;
  return dir.normalize();
}

Matrix4 LiSPSM::calcUnitCubeTransform(const Matrix4 &LS, const PointList &pts) {
  
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

Vector3 LiSPSM::calcZ0(const Matrix4 &LS, float z) {
  
  Vector3 eLS = LS * mE;
  
  Plane plane(mScn->getCameraViewDirection(), mE);
  
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

float LiSPSM::calcN(const Matrix4 &LS, const AABox &bBoxLS) {
  
  Vector3 z0LS = calcZ0(LS, bBoxLS.getMax().z);
  Vector3 z1LS = Vector3(z0LS.x, z0LS.y, bBoxLS.getMin().z);
  
  Matrix4 toEye = mScn->getCameraViewMatrix() * LS.getInverse();
  
  float z0 = (toEye * z0LS).z;
  float z1 = (toEye * z1LS).z;
  
  if ((z0<0 && z1>0) || (z0>0 && z1<0)) {
    return 0.0f;
  }
  
#if 0
  float d = float(fabs(bBoxLS.max().z - bBoxLS.min().z));
	return d / ( float(sqrt(z1/z0)) - 1.0f );
#else
# if 0
  float cn = mScn->getCameraNear();
	float d = float(fabs(mScn->getCameraFar() - cn));
  float dp = mScn->getCameraViewDirection().dot(mLightDir); // cosGamma
  float sinGamma = float(sqrt(1.0f - dp*dp));
	return (cn + float(sqrt(cn * (cn + d*sinGamma)))) / sinGamma;
# else
  return (mScn->getCameraNear() + float(sqrt(z0 * z1) * 5.0f));
# endif
#endif
}

Matrix4 LiSPSM::calcLiSPSM(const Matrix4 &LS, const PointList &B) {
  PointList bLS = LS * B;
  Vector3 eLS = LS * mE;
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

bool LiSPSM::update(unsigned int flags) {
  
  if (!mScn) {
    return false;
  }
  
  if ((flags & USE_CUSTOM_UP) == 0) {
    // if user didn't specify a custom up, set our own
    setLightViewUp(mScn->getCameraViewDirection());
    flags = flags | USE_CUSTOM_UP;
  }
  
  ShadowAlgorithm::update(flags);
  
  if (mScn->getSceneBox().isNull()) {
    return false;
  }
  
  PointList B, LVS;
  calcB(B);
  if (B.size() == 0) {
    return false;
  }
  
  calcLVS(LVS);
  
  mLightProj = InvZ * mLightProj;
  mLightProj = NormalToLightSpace * mLightProj; // viewing along y+ instead of z-

  Matrix4 LS = mLightProj * mLightView;
  Vector3 dir = calcProjViewDir(LS, LVS);
  Matrix4 R = Matrix4::MakeLookIn(Vector3::ZERO, dir, Vector3::UNIT_Y);
  mLightProj = R * mLightProj;
  
  float cosGamma = mLightDir.dot(mScn->getCameraViewDirection());
  if (fabs(cosGamma) < 0.99) {
    LS = mLightProj * mLightView;
    Matrix4 LiSPM = calcLiSPSM(LS, B);
    mLightProj = LiSPM * mLightProj;
  }
  
  LS = mLightProj * mLightView;
  Matrix4 UCT = calcUnitCubeTransform(LS, B);
  mLightProj = UCT * mLightProj;
  
  mLightProj = LightToNormalSpace * mLightProj; // set back matrix to view on z-
  mLightProj = InvZ * mLightProj;
  
  return true;
}

