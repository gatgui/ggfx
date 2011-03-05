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

#ifndef __ggfx_system_h_
#define __ggfx_system_h_

#include <ggfx/config.h>

namespace ggfx {
  
  BEGIN_MODULE_INTERFACE  ( Plugin       )
    DEFINE_MODULE_SYMBOL0 ( initialize   )
    DEFINE_MODULE_SYMBOL0 ( deInitialize )
  END_MODULE_INTERFACE
  
  class GGFX_API System {
    
    public:
      
      System();
      ~System();
      
      static System& Instance();
      static System* InstancePtr();
      
      void initialize(const std::string &pluginRepositoryPath);
      void loadPlugin(const std::string &pluginPath);
      void unloadPlugin(const std::string &pluginPath);
      
    private:
      
      bool enumPlugin(const gcore::Path &path);
      
      static System *msInstance;
      
      struct PluginEntry {
        Plugin *module;
        std::string path;
      };
      
      std::vector<PluginEntry> mPlugins;
  };


}


#endif

