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

#include <ggfx/object.h>

#define RO_METHOD(meth) METHOD(RenderableObject, meth)

namespace ggfx {
  
  AttachableObject::AttachableObject() {
  }
  
  AttachableObject::~AttachableObject() {
  }
  
  // ---

  RenderableObject::RenderableObject()
    : mMaterial(NULL) {
  }
  
  RenderableObject::~RenderableObject() {
    if (mMaterial) {
      mMaterial->disconnect(this);
    }
  }
  
  void RenderableObject::setMaterial(Material *m) {
    if (mMaterial) {
      mMaterial->disconnect(this);
    }
    mMaterial = m;
    if (mMaterial) {
      Material::EventHandlers handlers;
      gcore::MakeCallback(this, RO_METHOD(onMaterialDestroyed), handlers.destroyed);
      // gcore::MakeCallback(this, RO_METHOD(onMaterialChanged), handlers.changed);
      m->connect(this, handlers);
    }
  }
  
  void RenderableObject::onMaterialDestroyed(const Material *) {
    mMaterial = NULL;
  }
  
  void RenderableObject::onMaterialChanged(const Material*, Material::ChangedParam) {
    // Do nothing, for now ?
  }

}

#undef RO_METHOD

