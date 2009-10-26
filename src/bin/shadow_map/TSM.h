#ifndef __TSM_h_
#define __TSM_h_

#include "ShadowAlgorithm.h"

class TSM : public ShadowAlgorithm {

  public:
    
    TSM(const ISceneData *scn=NULL);
    virtual ~TSM();
    
    virtual bool update(unsigned int flags = 0);
    
    virtual void initializePrograms();
    
    void enablePCF(bool on);
    void enableVSM(bool on);
    
    inline float getFocusFactor() const {
      return mFocusFactor;
    }
    
    inline void setFocusFactor(float f) {
      mFocusFactor = f;
    } 
    
  protected:
  
    virtual void setupCasterProgram();
    virtual void setupReceiverProgram(bool noLighting=false);
    
    void calcB(gmath::PointList &pts);
    
    void calcReferencePoints(
      const gmath::PointList &B, gmath::Vector3 &nearP,
      gmath::Vector3 &focusP, gmath::Vector3 &farP);
    
    bool calcFrustum2D(
      const gmath::PointList &B, const gmath::Vector3 &nearP,
      const gmath::Vector3 &focusP, const gmath::Vector3 &farP,
      gmath::Vector2 &center, gmath::Vector2 &t0, gmath::Vector2 &t1,
      gmath::Vector2 &t2, gmath::Vector2 &t3);
    
    gmath::Matrix4 calcNt(
      const gmath::Vector2 &inCenter,
      const gmath::Vector2 &inT0, const gmath::Vector2 &inT1,
      const gmath::Vector2 &inT2, const gmath::Vector2 &inT3);

  protected:
    
    float mFocusFactor;
    bool mPCF;
    bool mVSM;
};

#endif


