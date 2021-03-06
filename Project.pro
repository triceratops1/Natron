# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron <http://www.natron.fr/>,
# Copyright (C) 2016 INRIA and Alexandre Gauthier
#
# Natron is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Natron is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
# ***** END LICENSE BLOCK *****

TEMPLATE = subdirs

enable-breakpad {
    include(breakpadpro.pri)
}

SUBDIRS += \
    HostSupport \
    gflags \
    glog \
    ceres \
    libmv \
    openMVG \
    qhttpserver \
    hoedown \
    libtess \
    Engine \
    Renderer \
    Gui \
    Tests \
    App

# where to find the sub projects - give the folders
gflags.subdir      = libs/gflags
glog.subdir        = libs/glog
ceres.subdir       = libs/ceres
libmv.subdir       = libs/libmv
openMVG.subdir     = libs/openMVG
qhttpserver.subdir = libs/qhttpserver
hoedown.subdir     = libs/hoedown
libtess.subdir     = libs/libtess

# what subproject depends on others
glog.depends = gflags
ceres.depends = glog gflags
libmv.depends = gflags ceres
openMVG.depends = ceres
Engine.depends = libmv openMVG HostSupport libtess ceres
Renderer.depends = Engine
Gui.depends = Engine qhttpserver
Tests.depends = Gui Engine
App.depends = Gui Engine

OTHER_FILES += \
    Global/Enums.h \
    Global/GLIncludes.h \
    Global/GlobalDefines.h \
    Global/KeySymbols.h \
    Global/Macros.h \
    Global/MemoryInfo.h \
    Global/ProcInfo.h \
    Global/QtCompat.h \
    global.pri \
    config.pri

include(global.pri)
include(config.pri)
include(libs.pri)

*-xcode {
  # Qt 4.8.5's XCode generator has a bug and places moc_*.cpp files next to the sources instead of inside the build dir
  # However, setting the MOC_DIR doesn't fix that (Xcode build fails)
  # Simple rtule: don't use Xcode
  #MOC_DIR = $$OUT_PWD
  warning("Xcode generator wrongly places the moc files in the source directory. You thus cannot compile with different Qt versions using Xcode.")
}

CONFIG(debug, debug|release){
    message("Compiling in DEBUG mode.")
} else {
    message("Compiling in RELEASE mode.")
}
