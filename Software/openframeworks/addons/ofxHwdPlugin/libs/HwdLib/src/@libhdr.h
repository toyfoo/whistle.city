/// \file
/// \brief File to be included to all exported headers that declare external linkage objects
/// \author Alexander Konin
/// \date 19.Jun.2014

#pragma once

// HWDLIB_SHARED - defined for both library build and user build.
// HWDLIB_BUILD - defined for library build only.

#ifdef WIN32
	#ifdef HWDLIB_SHARED
		#ifdef HWDLIB_BUILD
			#define HWDLIB_EXPORT __declspec(dllexport)
		#else
			#define HWDLIB_EXPORT __declspec(dllimport)
		#endif
	#else
		#define HWDLIB_EXPORT
	#endif
#else
	#define HWDLIB_EXPORT
#endif


/* ==TRASH==
#ifdef __linux__
#error __linux__
#endif

#ifdef WIN32
	#define LIBHDR_EXPORT_SHARED __declspec(dllexport)
	#define LIBHDR_IMPORT_SHARED __declspec(dllimport)
#else
	#define LIBHDR_EXPORT_SHARED
	#define LIBHDR_IMPORT_SHARED
#endif

// HWDLIB_SHARED - defined for both library build and user build.
// HWDLIB_BUILD - defined for library build only.

#ifdef HWDLIB_SHARED
#	ifdef HWDLIB_BUILD
#		define HWDLIB_EXPORT LIBHDR_EXPORT_SHARED
#	else
#		define HWDLIB_EXPORT LIBHDR_IMPORT_SHARED
#	endif
#else
#	define HWDLIB_EXPORT
#endif

#undef LIBHDR_EXPORT_SHARED
#undef LIBHDR_IMPORT_SHARED
*/
