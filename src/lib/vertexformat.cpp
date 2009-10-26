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

#include <ggfx/vertexformat.h>
#include <ggfx/mesh.h>

#define MAPPING_SIZE ggfx::VAS_MAX * sizeof(unsigned char)
#define NO_MAPPING   0xFF

#if 1
# define ATTR_ORDER_DONT_CARE
#endif

namespace ggfx {
  
  static size_t GL_TypeSize[VAT_MAX] = {
    sizeof(GLubyte),
    sizeof(GLushort),
    sizeof(GLuint),
    sizeof(GLbyte),
    sizeof(GLshort),
    sizeof(GLint),
    sizeof(GLfloat),
    sizeof(GLdouble)
  };
  
  static std::string STR_Type[VAT_MAX] = {
    "ubyte",
    "ushort",
    "uint",
    "byte",
    "short",
    "int",
    "float",
    "double"
  };
  
  static std::string STR_Semantic[VAS_MAX] = {
    "position",
    "normal",
    "color0",
    "color1",
    "texcoord",
    "tangent",
    "binormal",
    "weights",
    "indices",
    "generic"
  };
  
  // ---
  
  VertexAttribute::VertexAttribute()
    : mSem(VAS_MAX), mType(VAT_MAX), mNumElements(0), mOffset(0) {
  }
  
  VertexAttribute::VertexAttribute(const std::string &str) {
    fromString(str);
  }
  
  VertexAttribute::VertexAttribute(
    VertexAttributeSemantic sem, VertexAttributeType type, unsigned char nelems
  ) : mSem(sem), mType(type), mNumElements(nelems), mOffset(0) {
  }
  
  VertexAttribute::VertexAttribute(const VertexAttribute &rhs)
    : mSem(rhs.mSem), mType(rhs.mType), mNumElements(rhs.mNumElements), mOffset(rhs.mOffset) {
  }
  
  VertexAttribute::~VertexAttribute() {
  }
  
  void VertexAttribute::invalidate() {
    mSem = VAS_MAX;
    mType = VAT_MAX;
    mNumElements = 0;
  }
  
  bool VertexAttribute::isValid() const {
    switch (mSem) {
      case VAS_POSITION: {
        return (mNumElements>=2 && mNumElements<=4 && mType>=VAT_SHORT);
      }
      case VAS_NORMAL: {
        return (mNumElements==3 && mType>=VAT_BYTE);
      }
      case VAS_COLOR0: {
        return (mNumElements==3 || mNumElements==4);
      }
      case VAS_COLOR1: {
        return (mNumElements==3);
      }
      case VAS_TEXCOORD: {
        return (mNumElements>=1 && mNumElements<=4 && mType>=VAT_SHORT);
      }
      case VAS_TANGENT:
      case VAS_BINORMAL:
      case VAS_BLEND_WEIGHTS:
      case VAS_BLEND_INDICES:
      case VAS_GENERIC: {
        return (mNumElements>=1 && mNumElements<=4);
      }
      default: return false;
    }
  }
  
  VertexAttribute& VertexAttribute::operator=(const VertexAttribute &rhs) {
    if (this != &rhs) {
      mSem = rhs.mSem;
      mType = rhs.mType;
      mNumElements = rhs.mNumElements;
      mOffset = rhs.mOffset;
    }
    return *this;
  }
  
  bool VertexAttribute::operator==(const VertexAttribute &rhs) const {
    return ((mSem==rhs.mSem) && (mType==rhs.mType) && (mNumElements==rhs.mNumElements));
  }
  
  bool VertexAttribute::operator!=(const VertexAttribute &rhs) const {
    return !operator==(rhs);
  }
  
  size_t VertexAttribute::getBytesSize() const {
    return mNumElements * GL_TypeSize[mType];
  }
  
  std::string VertexAttribute::toString() const {
    std::ostringstream oss;
    oss << STR_Semantic[mSem] << "-" << STR_Type[mType] << "-" << (int)mNumElements;
    return oss.str();
  }
  
  bool VertexAttribute::fromString(const std::string &str) {
    size_t p0, p1;
    p0 = str.find('-');
    if (p0 == std::string::npos) {
      return false;
    }
    p1 = str.find('-', p0+1);
    if (p1 == std::string::npos) {
      return false;
    }
    if (str.find('-', p1+1) != std::string::npos) {
      return false;
    }
    std::string semStr = str.substr(0, p0);
    std::string typStr = str.substr(p0+1, p1-p0-1);
    std::string numStr = str.substr(p1+1);
    for (int i=0; i<VAS_MAX; ++i) {
      if (semStr == STR_Semantic[i]) {
        for (int j=0; j<VAT_MAX; ++j) {
          if (typStr == STR_Type[j]) {
            std::istringstream iss(numStr);
            int n;
            iss >> n;
            mNumElements = (unsigned char)n;
            mSem = (VertexAttributeSemantic)i;
            mType = (VertexAttributeType)j;
            if (!isValid()) {
              invalidate();
              return false;
            } else {
              return true;
            }
          }
        }
        break;
      }
    }
    return false;
  }
  
  // ---
  
  std::map<std::string, VertexFormat> VertexFormat::msFormats;
  
  VertexFormat* VertexFormat::Register(const VertexFormat &rhs) {
    std::string str = rhs.toString();
    std::map<std::string, VertexFormat>::iterator it = msFormats.find(str);
    if (it == msFormats.end()) {
      msFormats[str] = rhs;
    }
    return &(msFormats[str]);
  }
  
  VertexFormat* VertexFormat::Get(const std::string &str) {
    std::map<std::string, VertexFormat>::iterator it = msFormats.find(str);
    if (it != msFormats.end()) {
      return &(it->second);
    } else {
      VertexFormat fmt;
      if (fmt.fromString(str)) {
        msFormats[str] = fmt;
        return &(msFormats[str]);
      }
      return NULL;
    }
  }
  
  // ---

  VertexFormat::VertexFormat()
    :mNumGenerics(0), mNumTexcoords(0), mBytesSize(0) {
    memset(mMapping, NO_MAPPING, MAPPING_SIZE);
  }
  
  VertexFormat::VertexFormat(const VertexFormat &rhs)
    :mAttribs(rhs.mAttribs), mNumGenerics(rhs.mNumGenerics),
     mNumTexcoords(rhs.mNumTexcoords), mBytesSize(rhs.mBytesSize) {
    memcpy(mMapping, rhs.mMapping, MAPPING_SIZE);
  }
  
  VertexFormat::~VertexFormat() {
    mAttribs.clear();
  }

  VertexFormat& VertexFormat::operator=(const VertexFormat &rhs) {
    if (this != &rhs) {
      mAttribs = rhs.mAttribs;
      mNumGenerics = rhs.mNumGenerics;
      mNumTexcoords = rhs.mNumTexcoords;
      mBytesSize = rhs.mBytesSize;
      memcpy(mMapping, rhs.mMapping, MAPPING_SIZE);
    }
    return *this;
  }
  
  bool VertexFormat::operator==(const VertexFormat &rhs) const {
    if (mAttribs.size() != rhs.mAttribs.size()) {
      return false;
    }
    if (mNumGenerics != rhs.mNumGenerics) {
      return false;
    }
    if (mNumTexcoords != rhs.mNumTexcoords) {
      return false;
    }
#ifdef ATTR_ORDER_DONT_CARE
    std::vector<bool> validated(mAttribs.size(), false);
    for (size_t i=0; i<mAttribs.size(); ++i) {
      for (size_t j=0; j<rhs.mAttribs.size(); ++j) {
        if (!validated[j] && (mAttribs[i] == rhs.mAttribs[j])) {
          validated[j] = true;
          break;
        }
      }
    }
    for (size_t k=0; k<mAttribs.size(); ++k) {
      if (!validated[k]) {
        return false;
      }
    }
#else // ATTR_ORDER_DONT_CARE
    for (size_t i=0; i<mAttribs.size(); ++i) {
      if (mAttribs[i] != rhs.mAttribs[i]) {
        return false;
      }
    }
#endif
    return true;
  }
  
  bool VertexFormat::operator!=(const VertexFormat &rhs) const {
    return !operator==(rhs);
  }
  
  bool VertexFormat::addAttribute(const VertexAttribute &attrib) {
    if (attrib.isValid()) {
      VertexAttributeSemantic sem = attrib.getSemantic();
      if (sem == VAS_TEXCOORD) {
        if (mMapping[sem] == NO_MAPPING) {
          mAttribs.push_back(attrib);
          mAttribs.back().mOffset = mBytesSize;
          mMapping[sem] = (unsigned char)(mAttribs.size()-1);
        } else {
          AttributeIt it = mAttribs.begin() + (mMapping[sem]+mNumTexcoords-1);
          size_t off = it->mOffset + attrib.getBytesSize();
          it = mAttribs.insert(++it, attrib);
          it->mOffset = off;
          while (++it != mAttribs.end()) {
            mMapping[it->getSemantic()]++;
            it->mOffset += attrib.getBytesSize();
          }
        }
        ++mNumTexcoords;
        mBytesSize += attrib.getBytesSize();
        return true;
      } else if (sem == VAS_GENERIC) {
        if (mMapping[sem] == NO_MAPPING) {
          mAttribs.push_back(attrib);
          mAttribs.back().mOffset = mBytesSize;
          mMapping[sem] = (unsigned char)(mAttribs.size()-1);
        } else {
          AttributeIt it = mAttribs.begin() + (mMapping[sem]+mNumGenerics-1);
          size_t off = it->mOffset + attrib.getBytesSize();
          it = mAttribs.insert(++it, attrib);
          it->mOffset = off;
          while (++it != mAttribs.end()) {
            mMapping[it->getSemantic()]++;
            it->mOffset += attrib.getBytesSize();
          }
        }
        ++mNumGenerics;
        mBytesSize += attrib.getBytesSize();
        return true;
      } else {
        if (mMapping[sem] == NO_MAPPING) {
          mAttribs.push_back(attrib);
          mAttribs.back().mOffset = mBytesSize;
          mBytesSize += mAttribs.back().getBytesSize();
          mMapping[sem] = (unsigned char)(mAttribs.size()-1);
          return true;
        }
      }
    }
    return false;
  }
  
  bool VertexFormat::hasAttribute(VertexAttributeSemantic sem) const {
    return (mMapping[sem] != NO_MAPPING);
  }

  const VertexAttribute& VertexFormat::getAttribute(VertexAttributeSemantic sem, size_t idx) const {
    if (sem == VAS_GENERIC || sem == VAS_TEXCOORD) {
      return mAttribs[mMapping[sem]+idx];
    } else {
      return mAttribs[mMapping[sem]];
    }
  }
  
  bool VertexFormat::isValid() const {
    if (mAttribs.size() < 1) {
      return false;
    }
    if (mMapping[VAS_POSITION] == NO_MAPPING) {
      return false;
    }
    return true;
  }
  
  void VertexFormat::reset() {
    mAttribs.clear();
    memset(mMapping, NO_MAPPING, MAPPING_SIZE);
    mBytesSize = 0;
    mNumTexcoords = 0;
    mNumGenerics = 0;
  }
  
  std::string VertexFormat::toString() const {
    std::string str = "";
    if (mAttribs.size() >= 1) {
      for (size_t i=0; i<mAttribs.size()-1; ++i) {
        str += mAttribs[i].toString() + "|";
      }
      str += mAttribs[mAttribs.size()-1].toString();
    }
    return str;
  }
  
  bool VertexFormat::fromString(const std::string &str) {
    VertexAttribute attr;
    std::string attrStr;
    size_t p0 = 0;
    size_t p1 = 0;
    reset();
    p1 = str.find('|', p0);
    while (p1 != std::string::npos) {
      if (attr.fromString(str.substr(p0, p1-p0))) {
        addAttribute(attr);
      }
      p0 = p1+1;
      p1 = str.find('|', p0);
    }
    if (attr.fromString(str.substr(p0))) {
      addAttribute(attr);
    }
    if (!isValid()) {
      reset();
      return false;
    } else {
      return true;
    }
  }
  
  // ---
  
  VertexIterator::VertexIterator(Mesh *mesh)
    : mNumVertices(0), mFormat(NULL), mElement(NULL), mElementIndex(0), mBO(NULL) {
    if (mesh) {
      mNumVertices = mesh->getNumSharedVertices();
      if (mNumVertices > 0) {
        mElement = mesh->getClientSharedVertexBuffer();
        if (mElement) {
          mFormat = mesh->getSharedFormat();
        } else {
          mNumVertices = 0;
        }
      }
    }
  }

  VertexIterator::VertexIterator(Mesh *mesh, BufferObject::Access access, bool discard)
    : mNumVertices(0), mFormat(NULL), mElement(NULL), mElementIndex(0), mBO(NULL) {
    if (mesh) {
      mNumVertices = mesh->getNumSharedVertices();
      if (mNumVertices > 0) {
        mBO = mesh->getSharedVertexBuffer();
        if (mBO->lock(access, &mElement, discard)) {
          mFormat = mesh->getSharedFormat();
        } else {
          mNumVertices = 0;
          mBO = NULL;
        }
      }
    }
  }

  VertexIterator::VertexIterator(SubMesh *submesh)
    : mNumVertices(0), mFormat(NULL), mElement(NULL), mElementIndex(0), mBO(NULL) {
    if (submesh) {
      mNumVertices = submesh->getNumVertices();
      if (mNumVertices > 0) {
        mElement = submesh->getClientVertexBuffer();
        if (mElement) {
          mFormat = submesh->getFormat();
        } else {
          mNumVertices = 0;
        }
      }
    }
  }

  VertexIterator::VertexIterator(SubMesh *submesh, BufferObject::Access access, bool discard)
    : mNumVertices(0), mFormat(NULL), mElement(NULL), mElementIndex(0), mBO(NULL) {
    if (submesh) {
      mNumVertices = submesh->getNumVertices();
      if (mNumVertices > 0) {
        mBO = submesh->getVertexBuffer();
        if (mBO->lock(access, &mElement, discard)) {
          mFormat = submesh->getFormat();
        } else {
          mNumVertices = 0;
          mBO = NULL;
        }
      }
    }
  }

  VertexIterator::~VertexIterator() {
    if (mBO) {
      mBO->unlock();
    }
  }
  
  void VertexIterator::next() {
    if (mElementIndex < mNumVertices) {
      ++mElementIndex;
      mFormat->incrementPtr(&mElement);
    }
  }

  void VertexIterator::prev() {
    if (mElementIndex > 0) {
      --mElementIndex;
      mFormat->decrementPtr(&mElement);
    }
  }

  void VertexIterator::jumpTo(size_t idx) {
    if (idx < mNumVertices) {
      if (idx > mElementIndex) {
        size_t d = idx - mElementIndex;
        mElement = (void*)((char*)mElement + d*mFormat->getBytesSize());
      } else {
        size_t d = mElementIndex - idx;
        mElement = (void*)((char*)mElement - d*mFormat->getBytesSize());
      }
    }
    mElementIndex = idx;
  }
  
}

