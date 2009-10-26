#include "ShadowAlgorithm.h"
using namespace gmath;
using namespace ggfx;
using namespace std;

ShadowAlgorithm::ShadowAlgorithm(const ISceneData *scn)
  :mScn(scn), mDepthBias(0.0f), mCasterProg(0), mReceiverProg(0),
   mReceiverProgNL(0), mBoundReceiver(0) {
}

ShadowAlgorithm::~ShadowAlgorithm() {
  if (mCasterProg) {
    ProgramManager::Instance().deleteProgram(mCasterProg);
  }
  if (mReceiverProg) {
    ProgramManager::Instance().deleteProgram(mReceiverProg);
  }
  if (mReceiverProgNL) {
    ProgramManager::Instance().deleteProgram(mReceiverProgNL);
  }
  mCasterProg = mReceiverProg = mReceiverProgNL = 0;
}

void ShadowAlgorithm::initializePrograms() {
  ProgramManager &mgr = ProgramManager::Instance();
  mCasterProg = mgr.createProgram("SM_Caster",
                                  "share/shaders/sc_vp.glsl",
                                  "share/shaders/sc_fp.glsl");
  mReceiverProg = mgr.createProgram("SM_Receiver",
                                    "share/shaders/sr_vp.glsl",
                                    "share/shaders/sr_fp.glsl");
  mReceiverProgNL = mgr.createProgram("SM_ReceiverNL",
                                      "share/shaders/sr_vp_noDiffuse.glsl",
                                      "share/shaders/sr_fp_noDiffuse.glsl");
}

void ShadowAlgorithm::setupCasterProgram() {
  mCasterProg->uniform("depthBias")->set(mDepthBias);
}

void ShadowAlgorithm::setupReceiverProgram(bool noLighting) {
  Matrix4 eyeToLightProj = mLightProj * mLightView * mScn->getCameraInverseViewMatrix();
  //mReceiverProg->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  //mReceiverProg->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  //mReceiverProg->uniform("eyeToLightProj")->set(eyeToLightProj);
  //mReceiverProg->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  mBoundReceiver->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  mBoundReceiver->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  mBoundReceiver->uniform("eyeToLightProj")->set(eyeToLightProj);
  if (noLighting == false) {
    mBoundReceiver->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  }
}

void ShadowAlgorithm::updateCameraHull() {
  Frustum cameraFrustum;
  cameraFrustum.setFovy(mScn->getCameraFovy());
  cameraFrustum.setAspect(mScn->getViewAspect());
  cameraFrustum.setNearClip(mScn->getCameraNear());
  cameraFrustum.setFarClip(mScn->getCameraFar());
  // transform from eye space to world space
  cameraFrustum.transform(mScn->getCameraInverseViewMatrix());
  mCameraHull.from(cameraFrustum);
}

void ShadowAlgorithm::updateLightParameters() {
  mLightType = mScn->getLightType();
  if (mLightType == LT_POINT) {
    // use given position, compute direction
    mLightPos = mScn->getLightPosition();
    mLightDir = calcPointLightDirection();
  } else {
    // use given direction
    mLightDir = mScn->getLightDirection();
    if (mLightType == LT_DIR) {
      // compute position
      mLightPos = calcDirLightPosition();
    } else {
      // use given position
      mLightPos = mScn->getLightPosition();
    }
  }
}

Vector3 ShadowAlgorithm::calcDirLightPosition() {
  ConvexHull3D body(mCameraHull);
  body.clip(mScn->getSceneBox());
  PointList pts(body);
  Vector3 center;
  for (size_t i=0; i<pts.size(); ++i) {
    center += pts[i];
  }
  center /= float(pts.size());
  return (center - mScn->getDirectionalLightDistance()*mLightDir);
}

Vector3 ShadowAlgorithm::calcPointLightDirection() {
  ConvexHull3D body(mCameraHull);
  body.add(mScn->getLightPosition());
  body.clip(mScn->getSceneBox());
  PointList pts(body);
  Vector3 dir;
  for (size_t i=0; i<pts.size(); ++i) {
    dir += (pts[i] - mLightPos);
  }
  dir.normalize();
  return dir;
}

void ShadowAlgorithm::calcLightViewProj(const Vector3 &up) {
  // compute better near
  // compute far
  // bad near and/or far will reduce depth buffer precision
  mLightView = Matrix4::MakeLookIn(mLightPos, mLightDir, up);
  if (mLightType == LT_DIR) {
    mLightFrustum.setMode(PM_ORTHOGRAPHIC);
    mLightFrustum.setFovy(90.0f);
    mLightFrustum.setNearClip(mScn->getCameraNear());
    mLightFrustum.setAspect(1.0f);
    // near = 3000.0f * CameraNear ?
  } else if (mLightType == LT_POINT) {mLightFrustum.setMode(PM_PERSPECTIVE);
    mLightFrustum.setFovy(120.0f);
    mLightFrustum.setNearClip(mScn->getCameraNear());
    mLightFrustum.setAspect(1.0f);
  } else {mLightFrustum.setMode(PM_PERSPECTIVE);
    mLightFrustum.setFovy(1.2f * mScn->getSpotLightOuterAngle());
    mLightFrustum.setNearClip(mScn->getCameraNear());
    mLightFrustum.setAspect(1.0f);
  }
  //mLightFrustum = mLightView.getFastInverse() * mLightFrustum;
  mLightFrustum.transform(mLightView.getFastInverse());
  mLightProj = mLightFrustum.getProjectionMatrix();
}

bool ShadowAlgorithm::update(unsigned int flags) {
  if (!mScn) {
    return false;
  }
  if ((flags & NO_CAMERA_UPDATE) == 0) {
    updateCameraHull();
  }
  updateLightParameters();
  Vector3 up;
  if ((flags & USE_CUSTOM_UP) == 0) {
    up = Vector3::UNIT_Y;
    if (fabs(up.dot(mLightDir)) >= 1.0f) {
      up = Vector3::UNIT_Z;
    }
  } else {
    up = mLightViewUp;
  }
  calcLightViewProj(up);
  return true;
}




