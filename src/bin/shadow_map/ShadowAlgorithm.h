#ifndef __ShadowAlgorithm_h_
#define __ShadowAlgorithm_h_

#include <ggfx/light.h>
#include <ggfx/shader.h>

class ShadowAlgorithm {

  public:
    
    // ---
    
    struct ISceneData {
      virtual ~ISceneData() {}
      // Shadow Casters
      virtual const gmath::AABox& getSceneBox() const = 0;
      // Camera [position and direction should be in world space coordinates]
      virtual float getCameraFovy() const = 0;
      virtual float getViewAspect() const = 0;
      virtual float getCameraNear() const = 0;
      virtual float getCameraFar() const = 0;
      virtual const gmath::Matrix4& getCameraViewMatrix() const = 0;
      virtual const gmath::Matrix4& getCameraInverseViewMatrix() const = 0;
      virtual const gmath::Vector3& getCameraPosition() const = 0;
      virtual const gmath::Vector3& getCameraViewDirection() const = 0;
      // Light [position and direction should be in world space coordinates
      //        and only called when applicable]
      // (parameters for the light we want to compute shadow matrix for)
      virtual const gmath::Vector3& getLightPosition() const = 0;
      virtual const gmath::Vector3& getLightPositionEye() const = 0;
      virtual const gmath::Vector3& getLightDirection() const = 0;
      virtual const float getSpotLightOuterAngle() const = 0;
      virtual const float getSpotLightInnerAngle() const = 0;
      virtual ggfx::LightType getLightType() const = 0;
      // Parameters
      // put the light far enough from the scene
      virtual float getDirectionalLightDistance() const = 0;
      // shadow map
      virtual int getShadowMapSize() const = 0;
      virtual int getShadowMapTexUnit() const = 0;
    };
    
    // ---
    
    enum Flags {
      NO_CAMERA_UPDATE = 0x01,
      USE_CUSTOM_UP = 0x02
    };
    
    // ---
    
    ShadowAlgorithm(const ISceneData *scn=NULL);
    virtual ~ShadowAlgorithm();
    
    inline const gmath::Matrix4& getProjectionMatrix() const {
      return mLightProj;
    }
    
    inline const gmath::Matrix4& getViewMatrix() const {
      return mLightView;
    }
    
    inline void setSceneData(const ISceneData *scn) {
      mScn = scn;
    }
    
    inline void setDepthBias(float db) {
      mDepthBias = db;
    }
    
    inline void enableCasterProgram() {
      if (mCasterProg) {
        mCasterProg->bind();
        setupCasterProgram();
      }
    }
    
    inline void disableCasterProgram() {
      if (mCasterProg) {
        mCasterProg->unbind();
      }
    }
    
    inline void enableReceiverProgram(bool noLighting=false) {
      mBoundReceiver = NULL;
      if (noLighting == true) {
        if (mReceiverProgNL) {
          mBoundReceiver = mReceiverProgNL;
        }
      } else {
        if (mReceiverProg) {
          mBoundReceiver = mReceiverProg;
        }
      }
      if (mBoundReceiver) {
        mBoundReceiver->bind();
        setupReceiverProgram(noLighting);
      }
      //if (mReceiverProg) {
      //  mReceiverProg->bind();
      //  setupReceiverProgram(noLighting);
      //}
    }
    
    inline void disableReceiverProgram() {
      if (mBoundReceiver) {
        mBoundReceiver->unbind();
        mBoundReceiver = NULL;
      }
      //if (mReceiverProg) {
      //  mReceiverProg->unbind();
      //}
    }
    
    inline void setLightViewUp(const gmath::Vector3 &up) {
      mLightViewUp = up;
    }
    
    virtual bool update(unsigned int flags = 0);
    virtual void initializePrograms();
    
  protected:
    
    void updateCameraHull();
    void updateLightParameters();
    
    virtual gmath::Vector3 calcDirLightPosition();
    virtual gmath::Vector3 calcPointLightDirection();
    virtual void calcLightViewProj(const gmath::Vector3 &viewUp);
    
    virtual void setupCasterProgram();
    virtual void setupReceiverProgram(bool noLighting=false);
    
  protected:
    
    gmath::Matrix4 mLightProj;
    gmath::Matrix4 mLightView;
    const ISceneData *mScn;
    
    float mDepthBias;
    gmath::Vector3 mLightViewUp;
    
    // Internal data
    gmath::ConvexHull3D mCameraHull;
    ggfx::LightType  mLightType;
    gmath::Vector3    mLightPos;
    gmath::Vector3    mLightDir;
    gmath::Frustum    mLightFrustum;
    
    ggfx::Program *mCasterProg;
    ggfx::Program *mReceiverProg;
    ggfx::Program *mReceiverProgNL;
    ggfx::Program *mBoundReceiver;
};

// ---




// ---



#endif


