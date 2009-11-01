/*

Copyright (C) 2009  Gaetan Guidet

This file is part of ggfx.

ggfx is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

ggfx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#ifndef __ggfx_Light_h_
#define __ggfx_Light_h_

#include <ggfx/node.h>

namespace ggfx {

  enum LightType {
    LT_DIR = 0,
    LT_POINT,
    LT_SPOT,
    LT_MAX
  };
  
  class GGFX_API Light : public Node {
    protected:
    
      friend class Scene;
      
      Light();
      Light(const std::string &name);
      virtual ~Light();
    
    public:
      
      gmath::Vector3 getWorldDirection() const;
      // should not be able to add children here !
      
      inline void setEnabled(bool on) {
        mEnable = on;
      }
      inline void setType(LightType lt) {
        mType = lt;
      }
      inline void setDirection(const gmath::Vector3 &d) {
        mDirection = d;
      }
      inline void setDiffuse(const gmath::Vector4 &d) {
        mDiffuse = d;
      }
      inline void setAmbient(const gmath::Vector4 &a) {
        mAmbient = a;
      }
      inline void setSpecular(const gmath::Vector4 &s) {
        mSpecular = s;
      }
      inline void setSpotCutOff(float angle) {
        mSpotCutOff = angle;
      }
      inline void setSpotExponent(float exp) {
        mSpotExponent = exp;
      }
      inline void setConstantAttenuation(float v) {
        mAttenuation[0] = v;
      }
      inline void setLinearAttenuation(float v) {
        mAttenuation[1] = v;
      }
      inline void setQuadraticAttenuation(float v) {
        mAttenuation[2] = v;
      }
      inline void setCastShadow(bool on) {
        mCastShadows = on;
      }
      
      inline bool isEnabled() const {
        return mEnable;
      }
      inline bool isVirtual() const {
        return mVirtual;
      }
      inline LightType getType() const {
        return mType;
      }
      inline const gmath::Vector3& getDirection() const {
        return mDirection;
      }
      inline const gmath::Vector4& getDiffuse() const {
        return mDiffuse;
      }
      inline const gmath::Vector4& getAmbient() const {
        return mAmbient;
      }
      inline const gmath::Vector4& getSpecular() const {
        return mSpecular;
      }
      inline float getSpotCutOff() const {
        return mSpotCutOff;
      }
      inline float getSpotExponent() const {
        return mSpotExponent;
      }
      inline float getConstantAttenuation() const {
        return mAttenuation[0];
      }
      inline float getLinearAttenuation() const {
        return mAttenuation[1];
      }
      inline float getQuadraticAttenuation() const {
        return mAttenuation[2];
      }
      inline float getAttenuationFactor(float d) const {
        return 1.0f / (mAttenuation[0] + d * (mAttenuation[1] + d * mAttenuation[2]));
      }
      inline bool canCastShadows() const {
        return mCastShadows;
      }
      
      // Shadow Stuffs [Shadow Algorithm]
      
    protected:
      
      LightType mType;
      bool mEnable;
      bool mVirtual;
      gmath::Vector3 mDirection;
      gmath::Vector4 mDiffuse;
      gmath::Vector4 mAmbient;
      gmath::Vector4 mSpecular;
      float mSpotCutOff;
      float mSpotExponent;
      float mAttenuation[3];
      bool mCastShadows;
      // bool mVirtual; --> no HW light, I.E. for shaders only OK
  };

}

#endif

