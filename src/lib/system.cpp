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

#include <ggfx/system.h>

using namespace std;

namespace ggfx {
  
  System* System::msInstance = NULL;

  System::System() {
    assert(msInstance == NULL);
    msInstance = this;
  }
  
  System::~System() {
    msInstance = NULL;
    for (size_t i=0; i<mPlugins.size(); ++i) {
      delete mPlugins[i].module;
    }
  }
  
  System& System::Instance() {
    assert(msInstance);
    return *msInstance;
  }
  
  System* System::InstancePtr() {
    return msInstance;
  }
  
  bool System::enumPlugin(const string &dir, const string &fname, gcore::FileType type) {
    if (type == gcore::FT_FILE) {

      if (gcore::FileExtension(fname) == "gpl") {

        std::string path = gcore::JoinPath(dir, fname);
        cout << "Loading plugin: " << path << endl;
        loadPlugin(path);
      }
    }
    return true;
  }
  
  void System::initialize(const std::string &pluginRepositoryPath) {
    gcore::EnumFilesCallback callback;
    gcore::MakeCallback(this, METHOD(System,enumPlugin), callback);
    if (pluginRepositoryPath.length() == 0) {
      gcore::ForEachInDir(".", callback);
    } else {
      gcore::ForEachInDir(pluginRepositoryPath, callback);
    }
  }
  
  void System::loadPlugin(const std::string &pluginPath) {
    Plugin *plugin = new Plugin(pluginPath);
    if (plugin->_opened()) {
      plugin->initialize();
      PluginEntry pe;
      pe.module = plugin;
      pe.path = pluginPath;
      mPlugins.push_back(pe);
    } else {
      delete plugin;
    }
  }
  
  void System::unloadPlugin(const std::string &pluginPath) {
    for (size_t i=0; i<mPlugins.size(); ++i) {
      if (mPlugins[i].path == pluginPath) { // this is a bit light, should convert to abs path
        mPlugins[i].module->deInitialize();
        delete mPlugins[i].module;
        mPlugins.erase(mPlugins.begin()+i);
        return;
      }
    }
  }
}

