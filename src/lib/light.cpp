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

#include <ggfx/light.h>

using namespace gmath;

namespace ggfx {
  
  Light::Light()
    : mType(LT_POINT), mEnable(true), mVirtual(false), mDirection(Vector3(0,0,-1)),
      mDiffuse(Vector4(1,1,1,1)), mAmbient(Vector4(0,0,0,1)),
      mSpecular(Vector4(1,1,1,1)), mSpotCutOff(180), mSpotExponent(0),
      mCastShadows(false) {
    mAttenuation[0] = 1.0f;
    mAttenuation[1] = 0.0f;
    mAttenuation[2] = 0.0f;
    mAllowChild = false;
  }
  
  Light::Light(const std::string &name)
    : Node(name), mType(LT_POINT), mEnable(true), mVirtual(false),
      mDirection(Vector3(0,0,-1)), mDiffuse(Vector4(1,1,1,1)), mAmbient(Vector4(0,0,0,1)),
      mSpecular(Vector4(1,1,1,1)), mSpotCutOff(180), mSpotExponent(0),
      mCastShadows(false) {
    mAttenuation[0] = 1.0f;
    mAttenuation[1] = 0.0f;
    mAttenuation[2] = 0.0f;
    mAllowChild = false;
  }
  
  Light::~Light() {
  }
  
  gmath::Vector3 Light::getWorldDirection() const {
    Vector3 dir = getWorldOrientation() * mDirection;
    return dir.normalize();
  }
  
}

