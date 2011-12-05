#ifndef NXNA_PLATFORM_PLATFORMDEFS_H
#define NXNA_PLATFORM_PLATFORMDEFS_H

// the Google Native Client
#ifdef __native_client__
#define NXNA_PLATFORM_NACL

// Windows
#elif defined _WIN32
#define NXNA_PLATFORM_WIN32

// some kind of Apple platform
#elif defined __APPLE__
#define NXNA_PLATFORM_APPLE
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE == 0
#define NXNA_PLATFORM_APPLE_OSX
#else
#define NXNA_PLATFORM_APPLE_IPHONE
#endif

// assume linux
#else
#define NXNA_PLATFORM_NIX

#endif

#endif // NXNA_PLATFORM_PLATFORMDEFS_H