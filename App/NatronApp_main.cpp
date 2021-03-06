/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#if defined(Q_OS_UNIX)
#include <sys/time.h>     // for getrlimit on linux
#include <sys/resource.h> // for getrlimit
#if defined(__APPLE__)
#include <sys/syslimits.h> // OPEN_MAX
#endif
#endif

#include "Gui/GuiApplicationManager.h"

#include <cstdio>  // perror
#include <cstdlib> // exit
#include <fstream>
#include <sstream>
#include <iostream>

#include <QCoreApplication>

#include "Engine/CLArgs.h"

NATRON_NAMESPACE_USING

int
main(int argc,
     char *argv[])
{
#if defined(Q_OS_UNIX) && defined(RLIMIT_NOFILE)
    /*
     Avoid 'Too many open files' on Unix.

     Increase the number of file descriptors that the process can open to the maximum allowed.
     - By default, Mac OS X only allows 256 file descriptors, which can easily be reached.
     - On Linux, the default limit is usually 1024.

     Note that due to a bug in stdio on OS X, the limit on the number of files opened using fopen()
     cannot be changed after the first call to stdio (e.g. printf() or fopen()).
     Consequently, this has to be the first thing to do in main().
     */
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        if (rl.rlim_max > rl.rlim_cur) {
            rl.rlim_cur = rl.rlim_max;
            if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
#             if defined(__APPLE__) && defined(OPEN_MAX)
                // On Mac OS X, setrlimit(RLIMIT_NOFILE, &rl) fails to set
                // rlim_cur above OPEN_MAX even if rlim_max > OPEN_MAX.
                if (rl.rlim_cur > OPEN_MAX) {
                    rl.rlim_cur = OPEN_MAX;
                    setrlimit(RLIMIT_NOFILE, &rl);
                }
#             endif
            }
        }
    }
#endif
    
    CLArgs::printBackGroundWelcomeMessage();
    CLArgs args(argc, argv, false);

    if (args.getError() > 0) {
        return 1;
    }

    if ( args.isBackgroundMode() ) {
        AppManager manager;

        // coverity[tainted_data]
        if ( !manager.load(argc, argv, args) ) {
            return 1;
        } else {
            return 0;
        }
    } else {
        GuiApplicationManager manager;

        // coverity[tainted_data]
        return manager.load(argc, argv, args);

        //exec() is called within the GuiApplicationManager
    }
} // main

