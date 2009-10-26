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

#include <ggfx/node.h>
using namespace gmath;
using namespace std;

namespace ggfx {

  Node::Node()
    :mName(""), mLocalScale(Vector3(1,1,1)), mInheritScale(true),
     mInheritOrientation(true), mDirtyFlags(DIRTY_MATRICES|DIRTY_WORLD|DIRTY_BOUNDS),
     mParent(NULL), mAllowChild(true) {
  }
  
  Node::Node(const string &name)
    :mName(name), mLocalScale(Vector3(1,1,1)), mInheritScale(true),
     mInheritOrientation(true), mDirtyFlags(DIRTY_MATRICES|DIRTY_WORLD|DIRTY_BOUNDS),
     mParent(NULL), mAllowChild(true) {
  }
  
  Node::~Node() {
    for (size_t i=0; i<mChildren.size(); ++i) {
      mChildren[i]->mParent = NULL;
      mChildren[i]->dirtyWorldTransform();
    }
    mChildren.clear();
    cout << "Node::~Node: \"" << mName << "\"" << endl;
  }
  
  void Node::setTransformAsRest() {
    mRestPosition = mLocalPosition;
    mRestScale = mLocalScale;
    mRestOrientation = mLocalOrientation;
    mRestMatrix = getLocalMatrix();
  }
  
  void Node::setTransformToRest() {
    mLocalPosition = mRestPosition;
    mLocalScale = mRestScale;
    mLocalOrientation = mRestOrientation;
    dirtyLocalTransform();
  }
  
  const Matrix4& Node::getWorldMatrix() const {
    if (mDirtyFlags & DIRTY_WORLD_MATRIX) {
      updateWorldMatrix();
    }
    return mWorldMatrix;
  }
  
  const Matrix4& Node::getLocalMatrix() const {
    if (mDirtyFlags & DIRTY_LOCAL_MATRIX) {
      updateLocalMatrix();
    }
    return mLocalMatrix;
  }
  
  const Matrix4& Node::getRestMatrix() const {
    return mRestMatrix;
  }
  
  const AABox& Node::getBounds() const {
    if (mDirtyFlags & DIRTY_BOUNDS) {
      updateBounds();
    }
    return mBounds;
  }

  const Vector3& Node::getWorldPosition() const {
    if (mDirtyFlags & DIRTY_WORLD) {
      updateWorldTransform();
    }
    return mWorldPosition;
  }
  
  const Vector3& Node::getWorldScale() const {
    if (mDirtyFlags & DIRTY_WORLD) {
      updateWorldTransform();
    }
    return mWorldScale;
  }
  
  const Quat& Node::getWorldOrientation() const {
    if (mDirtyFlags & DIRTY_WORLD) {
      updateWorldTransform();
    }
    return mWorldOrientation;
  }
  
  void Node::setPosition(const Vector3 &v) {
    mLocalPosition = v;
    dirtyLocalTransform();
  }
  
  void Node::setOrientation(const Quat &q) {
    mLocalOrientation = q;
    dirtyLocalTransform();
  }
  
  void Node::setScale(const Vector3 &v) {
    mLocalScale = v;
    dirtyLocalTransform();
  }

  void Node::translate(const Vector3 &t, TransformReference ref) {
    if (ref == TR_LOCAL) {
      mLocalPosition += mLocalOrientation * t;
    } else if (ref == TR_WORLD) {
      if (mParent) {
        mLocalPosition +=
          (mParent->getWorldOrientation().getInverse() * t) / mParent->getWorldScale();
      } else {
        mLocalPosition += t;
      }
    } else { // TR_PARENT
      mLocalPosition += t;
    }
    dirtyLocalTransform();
  }
  
  void Node::rotate(const Quat &q, TransformReference ref) {
    if (ref == TR_LOCAL) {
      mLocalOrientation *= q;
    } else if (ref == TR_WORLD) {
      Quat wo = getWorldOrientation(); // will update if necessary
      mLocalOrientation = mLocalOrientation * wo.getInverse() * q * wo;
    } else { // TR_PARENT
      mLocalOrientation = q * mLocalOrientation;
    }
    dirtyLocalTransform();
  }
  
  void Node::scale(const Vector3 &s) {
    mLocalScale *= s;
    dirtyLocalTransform();
  }
  
  void Node::updateWorldTransform() const {
    if (mParent) {
      if (mInheritOrientation) {
        mWorldOrientation = mParent->getWorldOrientation() * mLocalOrientation;
      } else {
        mWorldOrientation = mLocalOrientation;
      }
      if (mInheritScale) {
        mWorldScale = mParent->getWorldScale() * mLocalScale;
      } else {
        mWorldScale = mLocalScale;
      }
      mWorldPosition = mParent->getWorldOrientation() * (mParent->getScale() * mLocalPosition);
      mWorldPosition += mParent->getWorldPosition();
    } else {
      mWorldOrientation = mLocalOrientation;
      mWorldScale = mLocalScale;
      mWorldPosition = mLocalPosition;
    }
    mDirtyFlags = mDirtyFlags & ~DIRTY_WORLD;
  }
  
  void Node::updateLocalMatrix() const {
    Convert::ToMatrix4(mLocalOrientation, mLocalMatrix);
    mLocalMatrix.setColumn(3, Vector4(mLocalPosition, 1.0));
    mLocalMatrix *= Matrix4::MakeScale(mLocalScale);
    mDirtyFlags = mDirtyFlags & ~DIRTY_LOCAL_MATRIX;
  }
  
  void Node::updateWorldMatrix() const {
    if (mDirtyFlags & DIRTY_WORLD) {
      updateWorldTransform();
    }
    Convert::ToMatrix4(mWorldOrientation, mWorldMatrix);
    mWorldMatrix.setColumn(3, Vector4(mWorldPosition, 1.0));
    mWorldMatrix *= Matrix4::MakeScale(mWorldScale);
    mDirtyFlags = mDirtyFlags & ~DIRTY_WORLD_MATRIX;
  }
  
  void Node::updateBounds() const {
    mBounds.reset();
    // if attach geometry --> create bounds
    for (size_t i=0; i<mChildren.size(); ++i) {
      if (mChildren[i]->getBounds().isNull()) {
        continue;
      }
      AABox cb = mChildren[i]->getLocalMatrix() * mChildren[i]->getBounds();
      for (unsigned char j=0; j<8; ++j) {
        mBounds.merge(cb.getCorner(j));
      }
    }
    mDirtyFlags = mDirtyFlags & ~DIRTY_BOUNDS;
  }
  
  void Node::cumulateTransform(
    const Vector3 &pos, const Quat &rot, const Vector3 &scl, float w, float s
  ) {
    // w in [0, 1]
    if (w <= 0.0f) {
      return;
    }
    
    translate(w*s*pos);
    
    rotate(Quat::IDENTITY.slerp(rot, w));
    
    scale(Vector3::UNIT_SCALE + (w * s * (scl - Vector3::UNIT_SCALE)));
  }
  
  void Node::inheritScale(bool on) {
    if (on != mInheritScale) {
      mInheritScale = on;
      dirtyLocalTransform();
    }
  }
  
  void Node::inheritOrientation(bool on) {
    if (on != mInheritOrientation) {
      mInheritOrientation = on;
      dirtyLocalTransform();
    }
  }
  
  void Node::dirtyLocalTransform() {
    mDirtyFlags = mDirtyFlags | DIRTY_MATRICES | DIRTY_WORLD | DIRTY_BOUNDS;
    for (size_t i=0; i<mChildren.size(); ++i) {
      mChildren[i]->dirtyWorldTransform();
    }
  }
  
  void Node::dirtyWorldTransform() {
    mDirtyFlags = mDirtyFlags | DIRTY_WORLD | DIRTY_WORLD_MATRIX;
    for (size_t i=0; i<mChildren.size(); ++i) {
      mChildren[i]->dirtyWorldTransform();
    }
  }
  
  void Node::dirtyBounds() {
    mDirtyFlags = mDirtyFlags | DIRTY_BOUNDS;
  }
  
  size_t Node::getNumChildren() const {
    return mChildren.size();
  }
  
  Node* Node::getNode(size_t i) const {
    if (i < getNumChildren()) {
      return mChildren[i];
    }
    return NULL;
  }
  
  Node* Node::getNode(const std::string &name) const {
    for (size_t i=0; i<getNumChildren(); ++i) {
      if (mChildren[i]->getName() == name) {
        return mChildren[i];
      }
    }
    return NULL;
  }
  
  void Node::attachChild(Node *child) {
    if (!mAllowChild) {
      return;
    }
    for (size_t i=0; i<getNumChildren(); ++i) {
      if (mChildren[i] == child) {
        return;
      }
    }
    mChildren.push_back(child);
    child->mParent = this;
    dirtyBounds();
  }
  
  void Node::detachChild(Node *child) {
    if (!mAllowChild) {
      return;
    }
    ChildIt it = mChildren.begin();
    while (it != mChildren.end()) {
      if (*it == child) {
        mChildren.erase(it);
        child->mParent = NULL;
        dirtyBounds();
        return;
      }
      ++it;
    }
  }
  
}

