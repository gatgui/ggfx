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

#ifndef __ggfx_object_h_
#define __ggfx_object_h_

#include <ggfx/node.h>
#include <ggfx/material.h>

namespace ggfx {
  
  
  class GGFX_API AttachableObject {
    public:
      
      friend class Node;
      
      AttachableObject();
      virtual ~AttachableObject();
      
      inline Node* getParentNode() const {
        return mNode;
      }
      
      // listen to node events [detached etc]
      
    protected:
      
      Node *mNode;
  };
  
  class GGFX_API RenderableObject {
    public:
      
      RenderableObject();
      virtual ~RenderableObject();
      
      void setMaterial(Material *m);
      
      inline Material* getMaterial() const {
        return mMaterial;
      }
      
      virtual void render(class Renderer *r) const = 0; // this is just the darw operation
      
      void onMaterialDestroyed(const Material *m);
      void onMaterialChanged(const Material *m, Material::ChangedParam param);
      
    protected:
      
      Material *mMaterial;
  };
  
}


#endif

