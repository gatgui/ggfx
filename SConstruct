import glob
import excons
import excons.tools
from excons.tools import gl
from excons.tools import glut

SConscript("gcore/SConstruct")
SConscript("gmath/SConstruct")

prjs = [
  { "name"    : "ggfx",
    "type"    : "sharedlib",
    "srcs"    : glob.glob("src/lib/*.cpp") + ["src/lib/glew.c"],
    "defs"    : ["GGFX_EXPORTS", "GLEW_BUILD"],
    "libs"    : ["gmath", "gcore"],
    "custom"  : [gl.Require],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  },
  { "name"    : "ggfx_obj",
    "type"    : "dynamicmodule",
    "ext"     : ".gpl",
    "prefix"  : "share/plugins/ggfx",
    "srcs"    : glob.glob("src/plugins/obj/*.cpp"),
    "libs"    : ["ggfx", "gmath", "gcore"],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  },
  { "name"    : "ggfx_ply",
    "type"    : "dynamicmodule",
    "ext"     : ".gpl",
    "prefix"  : "share/plugins/ggfx",
    "srcs"    : glob.glob("src/plugins/ply/*.cpp"),
    "libs"    : ["ggfx", "gmath", "gcore"],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  },
  { "name"    : "tests",
    "type"    : "testprograms",
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "libs"    : ["ggfx", "gmath", "gcore"],
    "custom"  : [glut.Require],
    "deps"    : ["ggfx_obj", "ggfx_ply"],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  },
  { "name"    : "shadow_map",
    "type"    : "program",
    "srcs"    : glob.glob("src/bin/shadow_map/*.cpp"),
    "libs"    : ["ggfx", "gmath", "gcore"],
    "custom"  : [glut.Require],
    "deps"    : ["ggfx_obj", "ggfx_ply"],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  },
  { "name"    : "test_shadow",
    "type"    : "program",
    "srcs"    : glob.glob("src/bin/test_shadow/*.cpp"),
    "libs"    : ["ggfx", "gmath", "gcore"],
    "custom"  : [glut.Require],
    "deps"    : ["ggfx_obj", "ggfx_ply"],
    "incdirs" : ["gcore/include", "gmath/include", "include"]
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)




