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

#ifndef __ggfx_material_h_
#define __ggfx_material_h_

#include <ggfx/config.h>

namespace ggfx {
  
  
  class GGFX_API Material {
    public:
      
      // --- Events ---
      
      enum ChangedParam {
        PARAM_DIFFUSE = 0,
        PARAM_SPECULAR,
        PARAM_AMBIENT,
        PARAM_SHININESS,
        PARAM_EMISSIVE
      };
      
      struct EventHandlers {
        gcore::Functor2<const Material*, ChangedParam> changed;
        gcore::Functor1<const Material*> destroyed;
      };
      
      // ---
  
      Material();
      Material(const std::string &name);
      virtual ~Material();
      
      inline void setDiffuse(const gmath::Vector4 &v) {
        mDiffuse = v;
        notifyChanged(PARAM_DIFFUSE);
      }
      inline void setAmbient(const gmath::Vector4 &v) {
        mAmbient = v;
        notifyChanged(PARAM_AMBIENT);
      }
      inline void setSpecular(const gmath::Vector4 &v) {
        mSpecular = v;
        notifyChanged(PARAM_SPECULAR);
      }
      inline void setEmissive(const gmath::Vector4 &v) {
        mEmissive = v;
        notifyChanged(PARAM_EMISSIVE);
      }
      inline void setShininess(float shininess) {
        mShininess = shininess;
        notifyChanged(PARAM_SHININESS);
      }
      
      inline float getShininess() const {
        return mShininess;
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
      inline const gmath::Vector4& getEmissive() const {
        return mEmissive;
      }
      inline const std::string& getName() const {
        return mName;
      }
      
      inline void connect(void *ptr, const EventHandlers &handlers) {
        mEventHandlersMap[ptr] = handlers;
      }
      
      inline void disconnect(void *ptr) {
        mEventHandlersMap.erase(mEventHandlersMap.find(ptr));
      }
      
    protected:
      
      void notifyChanged(ChangedParam p);
      void notifyDestroyed();
      
    protected:
      
      std::string mName;
      gmath::Vector4 mDiffuse;
      gmath::Vector4 mAmbient;
      gmath::Vector4 mSpecular;
      gmath::Vector4 mEmissive;
      float mShininess;
      
      typedef std::map<void*, EventHandlers> EventHandlersMap;
      typedef EventHandlersMap::iterator EventHandlersIt;
      typedef EventHandlersMap::const_iterator EventHandlersConstIt;
      
      EventHandlersMap mEventHandlersMap;
  
  };
}


#endif
