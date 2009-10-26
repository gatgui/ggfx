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

#include <ggfx/material.h>

using namespace gmath;

namespace ggfx {

  Material::Material()
    : mName(""), mDiffuse(Vector4(1,1,1,1)), mAmbient(Vector4(0,0,0,1)), 
      mSpecular(Vector4(1,1,1,1)), mEmissive(Vector4(0,0,0,1)), mShininess(36.0f) {
  }
  
  Material::Material(const std::string &name)
    : mName(name), mDiffuse(Vector4(1,1,1,1)), mAmbient(Vector4(0,0,0,1)), 
      mSpecular(Vector4(1,1,1,1)), mEmissive(Vector4(0,0,0,1)), mShininess(36.0f) {
  }
  
  Material::~Material() {
    notifyDestroyed(); // in destroyed handler avoir calling material method
  }
  
  void Material::notifyChanged(Material::ChangedParam p) {
    EventHandlersIt it = mEventHandlersMap.begin();
    while (it != mEventHandlersMap.end()) {
      if (it->second.changed) {
        it->second.changed(this, p);
      }
      ++it;
    }
  }
  
  void Material::notifyDestroyed() {
    EventHandlersIt it = mEventHandlersMap.begin();
    while (it != mEventHandlersMap.end()) {
      if (it->second.destroyed) {
        it->second.destroyed(this);
      }
      ++it;
    }
  }
  
}
