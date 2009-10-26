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

#ifndef __ggfx_node_h_
#define __ggfx_node_h_

#include <ggfx/config.h>

namespace ggfx {
  
  class GGFX_API Node {
    public:
  
      enum TransformReference {
        TR_PARENT = 0,
        TR_LOCAL,
        TR_WORLD
      };
      
      typedef std::vector<Node*> ChildArray;
      typedef ChildArray::iterator ChildIt;
      typedef ChildArray::const_iterator ChildConstIt;
      
    protected:
      
      friend class Scene;
      
      Node();
      Node(const std::string &name);
      virtual ~Node();
    
    public:
      
      inline const std::string& getName() const {
        return mName;
      }
      
      inline const gmath::Vector3& getPosition() const {
        return mLocalPosition;
      }
      inline const gmath::Vector3& getScale() const {
        return mLocalScale;
      }
      inline const gmath::Quat& getOrientation() const {
        return mLocalOrientation;
      }
      inline void getOrientation(gmath::Vector3 &axis, float &angle) {
        gmath::Convert::ToAxisAngle(mLocalOrientation, axis, angle);
      }
      
      inline const gmath::Vector3& getRestPosition() const {
        return mRestPosition;
      }
      inline const gmath::Vector3& getRestScale() const {
        return mRestScale;
      }
      inline const gmath::Quat& getRestOrientation() const {
        return mRestOrientation;
      }
      inline void getRestOrientation(gmath::Vector3 &axis, float &angle) {
        gmath::Convert::ToAxisAngle(mRestOrientation, axis, angle);
      }
      
      const gmath::Vector3& getWorldPosition() const;
      const gmath::Vector3& getWorldScale() const;
      const gmath::Quat& getWorldOrientation() const;
      inline void getWorldOrientation(gmath::Vector3 &axis, float &angle) {
        gmath::Convert::ToAxisAngle(getWorldOrientation(), axis, angle);
      }
      
      void setPosition(const gmath::Vector3 &v);
      void setOrientation(const gmath::Quat &q);
      void setScale(const gmath::Vector3 &v);
      inline void setOrientation(const gmath::Vector3 &a, float angle) {
        gmath::Quat q;
        gmath::Convert::ToQuat(a, angle, q);
        setOrientation(q);
      }
      
      void translate(const gmath::Vector3 &t, TransformReference ref = TR_PARENT);
      void rotate(const gmath::Quat &q, TransformReference ref = TR_LOCAL);
      void scale(const gmath::Vector3 &s);
      inline void rotate(const gmath::Vector3 &axis, float angle, TransformReference ref = TR_LOCAL) {
        gmath::Quat q;
        gmath::Convert::ToQuat(axis, angle, q);
        rotate(q, ref);
      }
      
      inline bool isScaleInherited() const {
        return mInheritScale;
      }
      inline bool isOrientationInherited() const {
        return mInheritOrientation;
      }
      void inheritScale(bool on);
      void inheritOrientation(bool on);
      
      const gmath::Matrix4& getWorldMatrix() const;
      const gmath::Matrix4& getLocalMatrix() const;
      const gmath::Matrix4& getRestMatrix() const;
      
      const gmath::AABox& getBounds() const;

      void updateWorldTransform() const;
      void updateLocalMatrix() const;
      void updateWorldMatrix() const;
      void updateBounds() const;
      
      size_t getNumChildren() const;
      Node* getNode(size_t i) const;
      Node* getNode(const std::string &name) const;
      void attachChild(Node *child);
      void detachChild(Node *child);
      
      void setTransformAsRest();
      void setTransformToRest();
      
      void cumulateTransform(const gmath::Vector3 &pos, const gmath::Quat &rot,
        const gmath::Vector3 &scl, float w=1.0f, float s=1.0f);
      
      // events ?
      
    protected:
      
      enum DirtyFlags {
        DIRTY_WORLD_MATRIX = 0x01,
        DIRTY_LOCAL_MATRIX = 0x02,
        DIRTY_MATRICES = DIRTY_WORLD_MATRIX|DIRTY_LOCAL_MATRIX,
        DIRTY_WORLD = 0x04,
        DIRTY_BOUNDS = 0x08
      };
      
      void dirtyLocalTransform();
      void dirtyWorldTransform();
      void dirtyBounds();
      
      std::string   mName;
      
      gmath::Vector3 mLocalPosition;
      gmath::Vector3 mLocalScale;
      gmath::Quat    mLocalOrientation;
      gmath::Vector3 mRestPosition;
      gmath::Vector3 mRestScale;
      gmath::Quat    mRestOrientation;
      
      bool mInheritScale;
      bool mInheritOrientation;
      
      mutable unsigned long mDirtyFlags;
      mutable gmath::AABox   mBounds;
      mutable gmath::Vector3 mWorldPosition;
      mutable gmath::Vector3 mWorldScale;
      mutable gmath::Quat    mWorldOrientation;
      mutable gmath::Matrix4 mWorldMatrix;
      mutable gmath::Matrix4 mLocalMatrix;
      mutable gmath::Matrix4 mRestMatrix;
      
      ChildArray mChildren;
      Node *mParent;
      
      bool mAllowChild;
  };
}

#endif
