import os
import platform
import scripts.app_helper as app

helper = app.Helper(ARGUMENTS)

APP_ROOT = helper.APP_ROOT
APP_CPPPATH=[
  os.path.join(APP_ROOT, "src"),
  os.path.join(APP_ROOT, "3rd/include"),
  os.path.join(APP_ROOT, "3rd/litehtml"),
  os.path.join(APP_ROOT, "3rd/litehtml/src"),
  os.path.join(APP_ROOT, "3rd/litehtml/include"),
  os.path.join(APP_ROOT, "3rd/litehtml/src/gumbo/include"),
  os.path.join(APP_ROOT, "3rd/litehtml/src/gumbo/include/gumbo"),
  os.path.join(APP_ROOT, "3rd/litehtml/include/litehtml"),
]
APP_CXXFLAGS=' -DLITEHTML_UTF8=1 '
if platform.system() == 'Windows' and not helper.awtk.TOOLS_NAME == 'mingw':
  APP_CXXFLAGS += ' /std:c++latest '
else:
  APP_CXXFLAGS += ' -std=gnu++17 '
helper.add_cxxflags(APP_CXXFLAGS).add_cpppath(APP_CPPPATH)
helper.set_dll_def('src/html_view.def').set_libs(['html_view']).call(DefaultEnvironment)

SConscriptFiles = ['3rd/SConscript', 'src/SConscript', 'demos/SConscript']
helper.SConscript(SConscriptFiles)
