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

#ifndef __ggfx_VertexFormat_h_
#define __ggfx_VertexFormat_h_

#include <ggfx/config.h>
#include <ggfx/bufferobject.h>

namespace ggfx {
	
	enum VertexAttributeSemantic {
    VAS_POSITION = 0,
    VAS_NORMAL,
    VAS_COLOR0,
    VAS_COLOR1,
    VAS_TEXCOORD,
    VAS_TANGENT,
    VAS_BINORMAL,
    VAS_BLEND_WEIGHTS,
    VAS_BLEND_INDICES,
    VAS_GENERIC,
    VAS_MAX
  };

  enum VertexAttributeType {
    VAT_UBYTE = 0,
    VAT_USHORT,
    VAT_UINT,
    VAT_BYTE,
    VAT_SHORT,
    VAT_INT,
    VAT_FLOAT,
    VAT_DOUBLE,
    VAT_MAX
  };
	
	class GGFX_API VertexAttribute {
    public:
      
      friend class VertexFormat;
      
      VertexAttribute();
      
      VertexAttribute(const std::string &str);
	
	    VertexAttribute(VertexAttributeSemantic sem, VertexAttributeType type, unsigned char nelems=1);
	    
	    VertexAttribute(const VertexAttribute &rhs);
	    
	    ~VertexAttribute();
	    
	    VertexAttribute& operator=(const VertexAttribute &rhs);
	    
	    bool operator==(const VertexAttribute &rhs) const;
	    
	    bool operator!=(const VertexAttribute &rhs) const;
	    
	    size_t getBytesSize() const;
	    
	    inline VertexAttributeSemantic getSemantic() const {
	      return mSem;
	    }
	    
	    inline VertexAttributeType getType() const {
	      return mType;
	    }
	    
	    inline unsigned char getNumElements() const {
	      return mNumElements;
	    }
	    
	    inline size_t getBytesOffset() const {
	      return mOffset;
	    }
	    
	    bool isValid() const;
	    
	    std::string toString() const;
	    
	    bool fromString(const std::string &str);
	
	  protected:
    
      void invalidate();
	    
	    VertexAttributeSemantic mSem;
	    VertexAttributeType mType;
	    unsigned char mNumElements;
	    size_t mOffset;
	
	};
	
	class GGFX_API VertexFormat {
    public:
	    
	    typedef std::vector<VertexAttribute> AttributeArray;
	    typedef AttributeArray::iterator AttributeIt;
	    typedef AttributeArray::const_iterator AttributeConstIt;
	    
	    VertexFormat();
	    
	    VertexFormat(const VertexFormat &rhs);
	    
	    ~VertexFormat();
	
	    VertexFormat& operator=(const VertexFormat &rhs);
	    
	    bool operator==(const VertexFormat &rhs) const;
	    
	    bool operator!=(const VertexFormat &rhs) const;
      
      void reset();
      
      bool isValid() const;
	    
	    bool addAttribute(const VertexAttribute &attrib);
	    
	    bool hasAttribute(VertexAttributeSemantic sem) const;
	    
	    inline size_t getNumAttributes() const {
	      return mAttribs.size();
	    }
      
      const VertexAttribute& getAttribute(VertexAttributeSemantic sem, size_t idx=0) const;
	    
	    inline const VertexAttribute& operator[](size_t i) const {
	      return mAttribs[i];
	    }
	    
	    inline size_t getBytesSize() const {
	      return mBytesSize;
	    }
      
      inline size_t getNumTexcoords() const {
        return (size_t)mNumTexcoords;
      }
      
      inline size_t getNumGenerics() const {
        return (size_t)mNumGenerics;
      }
      
      inline void* allocate(size_t count) {
        if (isValid()) {
          return malloc(count * mBytesSize);
        }
        return NULL;
      }
      
      inline void destroy(void *ptr) {
        if (ptr) {
          free(ptr);
        }
      }
      
      inline void* getElementPtr(void *buffer, size_t elt) const {
        return (void*)((char*)buffer + elt*mBytesSize);
      }
      
      inline void* getAttributePtr(void *element, VertexAttributeSemantic sem, size_t idx=0) const {
        return (void*)((char*)element + getAttribute(sem,idx).getBytesOffset());
      }
      
      inline void* getAttributePtr(void *buffer, size_t elt, VertexAttributeSemantic sem, size_t idx=0) const {
        return (void*)((char*)buffer + elt*mBytesSize + getAttribute(sem,idx).getBytesOffset());
      }
      
      inline void incrementPtr(void **ptr) const {
        *ptr = (void*)((char*)(*ptr) + mBytesSize);
      }
      
      inline void decrementPtr(void **ptr) const {
        *ptr = (void*)((char*)(*ptr) - mBytesSize);
      }
      
      std::string toString() const;
      
      bool fromString(const std::string &str);
      
    public:
      
      static VertexFormat* Register(const VertexFormat &rhs);
      
      static VertexFormat* Get(const std::string &str);
      
      static std::map<std::string, VertexFormat> msFormats;
	
	  protected:
	    
	    unsigned char mMapping[VAS_MAX];
	    AttributeArray mAttribs;
	    unsigned char mNumGenerics;
	    unsigned char mNumTexcoords;
	    size_t mBytesSize;
	};
	
	class GGFX_API VertexIterator {

    public:

      VertexIterator(class Mesh *mesh);
      VertexIterator(class Mesh *mesh, BufferObject::Access access, bool discard=false);
      VertexIterator(class SubMesh *submesh);
      VertexIterator(class SubMesh *submesh, BufferObject::Access access, bool discard=false);
      ~VertexIterator();

      inline bool isValid() const {
        return mElementIndex < mNumVertices;
      }

      void next();
      void prev();
      void jumpTo(size_t idx);

      inline bool hasAttribute(VertexAttributeSemantic sem) {
        return mFormat->hasAttribute(sem);
      }

      inline unsigned char getAttributeDim(VertexAttributeSemantic sem, size_t idx=0) {
        return mFormat->getAttribute(sem, idx).getNumElements();
      }

      inline size_t getNumTexcoords() const {
        return mFormat->getNumTexcoords();
      }

      inline size_t getNumGenerics() const {
        return mFormat->getNumGenerics();
      }

      inline float* asFloat(VertexAttributeSemantic sem, size_t idx=0) const {
        return (float*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline double* asDouble(VertexAttributeSemantic sem, size_t idx=0) const {
        return (double*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline char* asByte(VertexAttributeSemantic sem, size_t idx=0) const {
        return (char*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline short* asShort(VertexAttributeSemantic sem, size_t idx=0) const {
        return (short*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline int* asInt(VertexAttributeSemantic sem, size_t idx=0) const {
        return (int*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline unsigned char* asUnsignedByte(VertexAttributeSemantic sem, size_t idx=0) const {
        return (unsigned char*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline unsigned short* asUnsignedShort(VertexAttributeSemantic sem, size_t idx=0) const {
        return (unsigned short*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      inline unsigned int* asUnsignedInt(VertexAttributeSemantic sem, size_t idx=0) const {
        return (unsigned int*) mFormat->getAttributePtr(mElement, sem, idx);
      }

      // beware with int, should be 32 bits

    protected:

      size_t mNumVertices;
      VertexFormat *mFormat;
      mutable void *mElement;
      mutable size_t mElementIndex;
      class BufferObject *mBO;
  };
}

#endif


