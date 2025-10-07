#ifndef _PHOTON_EXTENSION_DEFINES_H_
#define _PHOTON_EXTENSION_DEFINES_H_

#if defined(DM_PLATFORM_HTML5)
	#if defined(__EMSCRIPTEN_PTHREADS__)
		#define PHOTON_PLATFORM_SUPPORTED 0
	#else
		#define PHOTON_PLATFORM_SUPPORTED 1
	#endif
#elif defined(DM_PLATFORM_LINUX)
	#if defined(__aarch64__)
		#define PHOTON_PLATFORM_SUPPORTED 0
	#else
		#define PHOTON_PLATFORM_SUPPORTED 1
	#endif
#else
	#define PHOTON_PLATFORM_SUPPORTED 1
#endif

#endif