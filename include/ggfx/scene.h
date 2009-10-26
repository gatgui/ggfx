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

#ifndef __ggfx_scene_h_
#define __ggfx_scene_h_

#include <ggfx/node.h>
#include <ggfx/light.h>

template <typename IteratorType>
struct MapIterator {
  inline MapIterator(IteratorType beg, IteratorType end)
    :mBeg(beg), mEnd(end), mCur(mBeg) {
  }
  inline MapIterator(const MapIterator<IteratorType> &rhs)
    :mBeg(rhs.mBeg), mEnd(rhs.mEnd), mCur(rhs.mCur) {
  }
  inline ~MapIterator() {
  }
  inline MapIterator<IteratorType>& operator=(const MapIterator<IteratorType> &rhs) {
    mBeg = rhs.mBeg;
    mEnd = rhs.mEnd;
    mCur = rhs.mCur;
    return *this;
  }
  inline bool isValid() const {
    return (mCur != mEnd);
  }
  inline void next() {
    ++mCur;
  }
  inline typename IteratorType::value_type::first_type getKey() const {
    return mCur->first;
  }
  inline typename IteratorType::value_type::second_type getValue() const {
    return mCur->second;
  }
  private:
    IteratorType mBeg;
    IteratorType mEnd;
    IteratorType mCur;
};


namespace ggfx {
  
  class GGFX_API Scene {
    public:
    
      typedef std::map<std::string, Node*> NodeMap;
      typedef NodeMap::iterator NodeMapIt;
      typedef NodeMap::const_iterator NodeMapConstIt;
      typedef MapIterator<NodeMapIt> NodeIt;
      typedef MapIterator<NodeMapConstIt> NodeConstIt;
      
      typedef std::map<std::string, Light*> LightMap;
      typedef LightMap::iterator LightMapIt;
      typedef LightMap::const_iterator LightMapConstIt;
      typedef MapIterator<LightMapIt> LightIt;
      typedef MapIterator<LightMapConstIt> LightConstIt;
      
      Scene();
      virtual ~Scene();
      
      Node* createNode(
        const std::string &name,
        const gmath::Vector3 &position = gmath::Vector3::ZERO,
        const gmath::Quat &orientation = gmath::Quat::IDENTITY,
        const gmath::Vector3 &scale = gmath::Vector3::UNIT_SCALE);
      
      Node* createNode(
        const std::string &name, Node *parentNode,
        const gmath::Vector3 &position = gmath::Vector3::ZERO,
        const gmath::Quat &orientation = gmath::Quat::IDENTITY,
        const gmath::Vector3 &scale = gmath::Vector3::UNIT_SCALE);
      
      void destroyNode(Node *n);
      
      inline size_t getNumNodes() const {
        return mNodeMap.size();
      }
      
      inline bool hasNode(const std::string &name) const {
        return (mNodeMap.find(name) != mNodeMap.end());
      }
      
      inline Node* getNode(const std::string &name) const {
        NodeMapConstIt it = mNodeMap.find(name);
        if (it != mNodeMap.end()) {
          return it->second;
        } else {
          return NULL;
        }
      }
      
      inline NodeConstIt getNodeIterator() const {
        return NodeConstIt(mNodeMap.begin(), mNodeMap.end());
      }
      
      inline NodeIt getNodeIterator() {
        return NodeIt(mNodeMap.begin(), mNodeMap.end());
      }
      
      inline Node* getRootNode() {
        return mRoot;
      }
      
      
      Light* createLight(
        const std::string &name, LightType type, const gmath::Vector3 &position,
        const gmath::Vector3 &direction = gmath::Vector3::NEG_UNIT_Z);
      
      Light* createLight(
        const std::string &name, Node *parentNode, LightType type,
        const gmath::Vector3 &position,
        const gmath::Vector3 &direction = gmath::Vector3::NEG_UNIT_Z);
      
      void destroyLight(Light *l);
      
      inline size_t getNumLights() const {
        return mLightMap.size();
      }
      
      inline bool hasLight(const std::string &name) const {
        return (mLightMap.find(name) != mLightMap.end());
      }
      
      inline Light* getLight(const std::string &name) const {
        LightMapConstIt it = mLightMap.find(name);
        if (it != mLightMap.end()) {
          return it->second;
        } else {
          return NULL;
        }
      }
      
      inline LightConstIt getLightIterator() const {
        return LightConstIt(mLightMap.begin(), mLightMap.end());
      }
      
      inline LightIt getLightIterator() {
        return LightIt(mLightMap.begin(), mLightMap.end());
      }
      
      
    protected:
      
      NodeMap mNodeMap;
      NodeMap mOutNodeMap;
      Node *mRoot;
      LightMap mLightMap;
    
  };
  
  
}


#endif
