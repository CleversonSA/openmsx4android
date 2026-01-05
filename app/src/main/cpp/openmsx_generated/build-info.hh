//
// Created by cleve on 18/12/2025.
//

#ifndef BUILD_INFO_HH
#define BUILD_INFO_HH

#ifdef __MINGW64__
#define ASM_X86 0
#define ASM_X86 0
#define ASM_X86_32 0
#define ASM_X86_64 0
#else
#define ASM_X86 0
#define ASM_X86_32 0
#define ASM_X86_64 0
#endif
#define PLATFORM_DINGUX 0
#define PLATFORM_ANDROID 1
#define HAVE_16BPP 1
#define HAVE_32BPP 1
#define MIN_SCALE_FACTOR 1
#define MAX_SCALE_FACTOR 4

namespace openmsx {

    static const bool OPENMSX_SET_WINDOW_ICON = false;
    static const char* const DATADIR = "/data/data/com.openmsx.openmsx4android/files/openmsx/share";
    static const char* const BUILD_FLAVOUR = "opt";
    static const char* const TARGET_PLATFORM = "android";
    static const char* const TARGET_CPU = "armv8";


} // namespace openmsx

#endif //BUILD_INFO_HH
