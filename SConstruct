﻿import os
import platform
import scripts.app_helper as app

helper = app.Helper(ARGUMENTS)

APP_ROOT = helper.APP_ROOT
APP_CPPPATH=[
  os.path.join(APP_ROOT, "src"),
  os.path.join(APP_ROOT, "src/litehtml"),
  os.path.join(APP_ROOT, "src/litehtml/src"),
  os.path.join(APP_ROOT, "src/litehtml/include"),
  os.path.join(APP_ROOT, "src/litehtml/src/gumbo/include"),
  os.path.join(APP_ROOT, "src/litehtml/src/gumbo/include/gumbo"),
  os.path.join(APP_ROOT, "src/litehtml/include/litehtml")
]
APP_CXXFLAGS=' -DLITEHTML_UTF8=1 '
if platform.system() == 'Windows' and not helper.awtk.TOOLS_NAME == 'mingw':
  APP_CXXFLAGS += ' /std:c++latest '
else:
  APP_CXXFLAGS += ' -std=c++11 '
helper.add_cxxflags(APP_CXXFLAGS).add_cpppath(APP_CPPPATH)
helper.set_dll_def('src/html_view.def').set_libs(['html_view']).call(DefaultEnvironment)

SConscriptFiles = ['src/SConscript', 'demos/SConscript']
helper.SConscript(SConscriptFiles)
