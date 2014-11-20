/*
 * \file config.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * This is the compilation configuration file for LyX.
 * It was generated by cmake.
 * You might want to change some of the defaults if something goes wrong
 * during the compilation.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

// obligatory flags
//#define QT_NO_STL
//#define QT_NO_KEYWORDS
#define HAVE_ICONV

#include "configCompiler.h"

#ifdef LYX_ENABLE_PCH
#include "pcheaders.h"
#endif


#define PACKAGE "LyX2.0"
#define PACKAGE_VERSION "2.0.6"
#define PACKAGE_STRING "LyX 2.0.6"
#define PACKAGE_BUGREPORT "lyx-devel@lists.lyx.org"
#define LYX_DATE "2013-04-29"
#define VERSION_INFO "CMake Build"
#define LYX_DIR_VER "LYX_DIR_20x"
#define LYX_USERDIR_VER "LYX_USERDIR_20x"
#define LYX_MAJOR_VERSION 2
#define LYX_MINOR_VERSION 0

#ifdef _MSC_VER
#define PROGRAM_SUFFIX ""
#define LYX_ABS_INSTALLED_DATADIR "C:\Program Files\piket"
#define LYX_ABS_INSTALLED_LOCALEDIR "C:\Program Files\piket"
#define LYX_ABS_TOP_SRCDIR "E:/git/lyx-2.0.6"
#else
#define PROGRAM_SUFFIX "2.0"
#define LYX_ABS_INSTALLED_DATADIR "/usr/local/lyx2.0"
#define LYX_ABS_INSTALLED_LOCALEDIR "/usr/share/local/lyx2.0/locale"
#define LYX_ABS_TOP_SRCDIR LYXSRCDIR
#endif

#ifndef _WIN32
#define USE_POSIX_PACKAGING 1
#else
#define USE_WINDOWS_PACKAGING 1
#endif

/* #undef USE_MACOSX_PACKAGING */
/* #undef PATH_MAX */

/* #undef WORDS_BIGENDIAN */

/* #undef LYX_MERGE_FILES */

#ifdef _WIN32
#define LYX_USE_TR1 1
#endif
/* #undef LYX_USE_TR1_REGEX */

/* #undef ASPELL_FOUND */
#ifdef ASPELL_FOUND
#define USE_ASPELL 1
#endif

/* #undef AIKSAURUSLIB_FOUND */
#ifdef AIKSAURUSLIB_FOUND
#define HAVE_LIBAIKSAURUS 1
#define AIKSAURUS_H_LOCATION ""
#endif

/* #undef ENCHANT_FOUND */
#ifdef ENCHANT_FOUND
#define USE_ENCHANT 1
#endif

#define HUNSPELL_FOUND 1
#ifdef HUNSPELL_FOUND
#define USE_HUNSPELL 1
#endif

#ifndef _MSC_VER
	#define HAVE_GETTEXT
	#define LYX_NLS 1
		#ifdef LYX_NLS
			#ifndef ENABLE_NLS
			#define ENABLE_NLS
			#endif
		#endif
	#endif

#endif // config.h guard



// Unguarded cleanup of global namespace:

#ifdef ColorMode
#undef ColorMode
#endif

#ifdef FocusOut
#undef FocusOut
#endif

#ifdef FocusIn
#undef FocusIn
#endif

#ifdef KeyRelease
#undef KeyRelease
#endif

#ifdef CursorShape
#undef CursorShape
#endif

#ifdef IGNORE
#undef IGNORE
#endif

#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Status
#undef Status
#endif

#ifdef IN
#undef IN
#endif

#ifdef KeyPress
#undef KeyPress
#endif

