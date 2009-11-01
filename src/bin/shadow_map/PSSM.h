#ifndef __PSSM_h_
#define __PSSM_h_

#include "ShadowAlgorithm.h"

#if 1
# define PSSM_SINGLE_PASS_LIGHTING
#endif

class PSSM : public ShadowAlgorithm {
  
  protected:
    
    struct SplitLayer {
      float camNear;
      float camFar;
      gmath::ConvexHull3D B;
      gmath::ConvexHull3D camHull;
      gmath::Vector3 lightPos; // might change [for directional]
      gmath::Vector3 lightDir; // might change [for point]
      gmath::Vector3 lightUp;
      gmath::Frustum lightFrustum;
      gmath::Matrix4 proj;
      gmath::Matrix4 view;
    };
    
  public:
    
    PSSM(const ISceneData *scn=NULL);
    virtual ~PSSM();
    
    virtual bool update(unsigned int flags = 0);
    virtual void initializePrograms();
    
    void setLambda(float f) {
      mLambda = gmath::Clamp(f, 0.0f, 1.0f);
    }
    
    void setCurrentLayer(int i);
    
    inline float getCameraNear() const {
      return mSplits[mCurrentLayer].camNear;
    }
    
    inline float getCameraFar() const {
      return mSplits[mCurrentLayer].camFar;
    }
    
    inline const gmath::ConvexHull3D& getBodyB() const {
      return mSplits[mCurrentLayer].B;
    }
    
    inline const gmath::ConvexHull3D& getCameraHull() const {
      return mSplits[mCurrentLayer].camHull;
    }
    
  protected:
    
    inline void resizeSplits() {
      mSplits.resize(mNSplit);
    }
    
    inline SplitLayer& getLayer(int n) {
      return mSplits[n];
    }
    
    void computeNearFar(const gmath::Vector3 &from, const gmath::Vector3 &dir, float &camNear, float &camFar);
    void calcB(gmath::PointList &pts, gmath::ConvexHull3D *B = NULL);
    gmath::Matrix4 calcUnitCubeTransform(const gmath::Matrix4 &LS, const gmath::PointList &pts);
    void splitCameraFrustum(unsigned int flags);
    
    virtual void setupCasterProgram();
    virtual void setupReceiverProgram(bool noLighting=false);
    virtual void calcLightViewProj(const gmath::Vector3 &viewUp);
    
  protected:
    
    int mCurrentLayer;
    int mNSplit;
    float mLambda;
    std::vector<SplitLayer> mSplits;
    float mLightFar;
    gmath::ConvexHull3D mBodyB;
};

#endif


