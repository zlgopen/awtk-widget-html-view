import os
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
APP_CXXFLAGS=''
if platform.system() == 'Windows':
  APP_CXXFLAGS += ' /std:c++17 '
else:
  APP_CXXFLAGS += ' -std=c++17 '
helper.add_cxxflags(APP_CXXFLAGS).add_cpppath(APP_CPPPATH)
helper.set_dll_def('src/html_view.def').set_libs(['html_view']).call(DefaultEnvironment)

SConscriptFiles = ['src/SConscript', 'demos/SConscript', 'tests/SConscript']
SConscript(SConscriptFiles)
