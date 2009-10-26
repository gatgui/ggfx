import glob
import excons
import excons.tools
from excons.tools import gl
from excons.tools import glut

gcore_inc = None
gcore_lib = None
gmath_inc = None
gmath_lib = None

gcore_dir = ARGUMENTS.get("with-gcore", None)
if gcore_dir == None:
  gcore_inc = ARGUMENTS.get("with-gcore-inc", None)
  gcore_lib = ARGUMENTS.get("with-gcore-lib", None)
else:
  gcore_inc = gcore_dir + "/include"
  gcore_lib = gcore_dir + "/lib"
if gcore_inc == None or gcore_lib == None:
  print("### Warning ###")
  print("'gcore' library directory not specified")
  print("use 'with-gcore=' or 'with-gcore-inc' and 'with-gcore-lib' if needed")

gmath_dir = ARGUMENTS.get("with-gmath", None)
if gmath_dir == None:
  gmath_inc = ARGUMENTS.get("with-gmath-inc", None)
  gmath_lib = ARGUMENTS.get("with-gmath-lib", None)
else:
  gmath_inc = gmath_dir + "/include"
  gmath_lib = gmath_dir + "/lib"
if gmath_inc == None or gmath_lib == None:
  print("### Warning ###")
  print("'gmath' library directory not specified")
  print("use 'with-gmath=' or 'with-gmath-inc' and 'with-gmath-lib' if needed")

prjs = [
  { "name"  : "ggfx",
    "type"  : "sharedlib",
    "srcs"  : glob.glob("src/lib/*.cpp") + ["src/lib/glew.c"],
    "defs"  : ["GGFX_EXPORTS", "GLEW_BUILD"],
    "libs"  : ["gmath", "gcore"],
    "custom": [gl.Require],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  },
  { "name"  : "ggfx_obj",
    "type"  : "dynamicmodule",
    "ext"   : ".gpl",
    "prefix": "share/plugins/ggfx",
    "srcs"  : glob.glob("src/plugins/obj/*.cpp"),
    "libs"  : ["ggfx", "gmath", "gcore"],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  },
  { "name"  : "ggfx_ply",
    "type"  : "dynamicmodule",
    "ext"   : ".gpl",
    "prefix": "share/plugins/ggfx",
    "srcs"  : glob.glob("src/plugins/ply/*.cpp"),
    "libs"  : ["ggfx", "gmath", "gcore"],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  },
  { "name"  : "tests",
    "type"  : "testprograms",
    "srcs"  : glob.glob("src/tests/*.cpp"),
    "libs"  : ["ggfx", "gmath", "gcore"],
    "custom": [glut.Require],
    "deps"  : ["ggfx_obj", "ggfx_ply"],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  },
  { "name"  : "shadow_map",
    "type"  : "program",
    "srcs"  : glob.glob("src/bin/shadow_map/*.cpp"),
    "libs"  : ["ggfx", "gmath", "gcore"],
    "custom": [glut.Require],
    "deps"  : ["ggfx_obj", "ggfx_ply"],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  },
  { "name"  : "test_shadow",
    "type"  : "program",
    "srcs"  : glob.glob("src/bin/test_shadow/*.cpp"),
    "libs"  : ["ggfx", "gmath", "gcore"],
    "custom": [glut.Require],
    "deps"  : ["ggfx_obj", "ggfx_ply"],
    "incdirs": [gmath_inc, gcore_inc],
    "libdirs": [gmath_lib, gcore_lib],
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)




