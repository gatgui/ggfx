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

#include <ggfx/scene.h>

using namespace gmath;
using namespace std;

namespace ggfx {

  Scene::Scene() : mRoot(0) {
    mRoot = new Node("Root");
    mNodeMap[mRoot->getName()] = mRoot;
  }
  
  Scene::~Scene() {
    NodeMapIt it = mNodeMap.begin();
    while (it != mNodeMap.end()) {
      delete it->second;
      ++it;
    }
    mNodeMap.clear();
  }

  Node* Scene::createNode(
    const string &name, const Vector3 &position, const Quat &orientation,
    const Vector3 &scale)
  {
    if (hasNode(name)) {
      return NULL;
    } else {
      Node *n = new Node(name);
      mRoot->attachChild(n);
      n->setPosition(position);
      n->setOrientation(orientation);
      n->setScale(scale);
      mNodeMap[name] = n;
      return n;
    }
  }

  Node* Scene::createNode(
    const string &name, Node *parentNode, const Vector3 &position,
    const Quat &orientation, const Vector3 &scale)
  {
    if (hasNode(name)) {
      return NULL;
    } else {
      Node *n = new Node(name);
      if (parentNode) {
        parentNode->attachChild(n);
      } else {
        mRoot->attachChild(n);
      }
      n->setPosition(position);
      n->setOrientation(orientation);
      n->setScale(scale);
      mNodeMap[name] = n;
      return n;
    }
  }

  void Scene::destroyNode(Node *n) {
    if (n) {
      NodeMapIt it = mNodeMap.find(n->getName());
      if (it != mNodeMap.end()) {
        // children are out of the scene !
        delete it->second;
      }
    }
  }
  
  Light* Scene::createLight(
    const string &name, LightType type,
    const Vector3 &position, const Vector3 &direction)
  {
    if (hasLight(name)) {
      return NULL;
    } else {
      Light *l = new Light(name);
      mRoot->attachChild(l);
      l->setType(type);
      l->setPosition(position);
      l->setDirection(direction);
      mLightMap[name] = l;
      return l;
    }
  }
  
  Light* Scene::createLight(
    const string &name, Node *parentNode, LightType type,
    const Vector3 &position, const Vector3 &direction)
  {
    if (hasLight(name)) {
      return NULL;
    } else {
      Light *l = new Light(name);
      if (parentNode) {
        parentNode->attachChild(l);
      } else {
        mRoot->attachChild(l);
      }
      l->setType(type);
      l->setPosition(position);
      l->setDirection(direction);
      mLightMap[name] = l;
      return l;
    }
  }
  
  void Scene::destroyLight(Light *l) {
    if (l) {
      LightMapIt it = mLightMap.find(l->getName());
      if (it != mLightMap.end()) {
        delete it->second;
      }
    }
  }
}
