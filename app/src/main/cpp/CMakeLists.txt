cmake_minimum_required(VERSION 3.22.1)
project(openmsx4android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------
# SDL2
# -------------------------
set(SDL_SHARED ON  CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL2)

# -------------------------
# TCL core (vendored)
# -------------------------
set(TCL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tcl)

# 1) Gerar um tclConfig.h mínimo (Tcl inclui isso via generic/tclPort.h)
set(TCL_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/tcl_generated)
file(MAKE_DIRECTORY ${TCL_GEN_DIR})

set(TCL_CONFIG_H ${TCL_GEN_DIR}/tclConfig.h)
file(WRITE  ${TCL_CONFIG_H} "#ifndef _TCLCONFIG\n#define _TCLCONFIG\n")
file(APPEND ${TCL_CONFIG_H} "/* Minimal config for Android NDK build */\n")
file(APPEND ${TCL_CONFIG_H} "#define TCL_PACKAGE_PATH \"{}\"\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_DIRENT_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_SYS_TIME_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_GETTIMEOFDAY 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_UNISTD_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_STDLIB_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_STRING_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_LIMITS_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_ERRNO_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_FCNTL_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_SYS_STAT_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_SYS_TYPES_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_DLFCN_H 1\n")
file(APPEND ${TCL_CONFIG_H} "#define HAVE_DLOPEN 1\n")
file(APPEND ${TCL_CONFIG_H} "#define TCL_THREADS 1\n")
file(APPEND ${TCL_CONFIG_H} "#endif\n")

# 2) Fontes: generic + unix (sem recursividade)
file(GLOB TCL_GENERIC_SRC ${TCL_DIR}/generic/*.c)

# regcomp.c inclui esses arquivos via #include; não compile separadamente
list(FILTER TCL_GENERIC_SRC EXCLUDE REGEX ".*/regc_(cvec|color|lex|locale|nfa)\\.c$")

# Regex engine internals: não compile unidades "rege_*.c" isoladas
list(FILTER TCL_GENERIC_SRC EXCLUDE REGEX ".*/rege_.*\\.c$")

# Regex compat (BSD re_comp/re_exec) não existe no Android; configure normalmente desliga isso
list(FILTER TCL_GENERIC_SRC EXCLUDE REGEX ".*/regfronts\\.c$")

file(GLOB TCL_UNIX_SRC    ${TCL_DIR}/unix/*.c)

file(GLOB TCL_TOMMATH_SRC
        ${TCL_DIR}/libtommath/*.c
)

# 3) Filtrar o que não serve
list(FILTER TCL_GENERIC_SRC EXCLUDE REGEX ".*/tcl.*Test.*\\.c$")
list(FILTER TCL_UNIX_SRC    EXCLUDE REGEX ".*/dltest/.*")
list(FILTER TCL_UNIX_SRC    EXCLUDE REGEX ".*/tclXt.*\\.c$")
list(FILTER TCL_UNIX_SRC    EXCLUDE REGEX ".*/tclUnixTest\\.c$")
list(FILTER TCL_UNIX_SRC    EXCLUDE REGEX ".*/tclAppInit\\.c$")

# manter só o loader correto no Android
list(FILTER TCL_UNIX_SRC EXCLUDE REGEX ".*/tclLoad(Aix|Dyld|Next|OSF|Shl)\\.c$")
# (deixe tclLoadDl.c)

add_library(tcl_core STATIC
        ${TCL_GENERIC_SRC}
        ${TCL_UNIX_SRC}
        ${TCL_TOMMATH_SRC}
)

# Força C puro e C99
set_target_properties(tcl_core PROPERTIES
        LINKER_LANGUAGE C
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
)

target_include_directories(tcl_core PUBLIC
        ${TCL_GEN_DIR}          # tclConfig.h gerado
        ${TCL_DIR}/generic
        ${TCL_DIR}/unix
        ${TCL_DIR}/libtommath
)

# libs do Android para Tcl: dl + m
target_link_libraries(tcl_core PUBLIC
        dl
        m
        z
)

target_compile_definitions(tcl_core PRIVATE
        CFG_RUNTIME_LIBDIR="\"\""
        CFG_RUNTIME_BINDIR="\"\""
        CFG_RUNTIME_SCRDIR="\"\""
        CFG_RUNTIME_INCDIR="\"\""
        CFG_RUNTIME_DOCDIR="\"\""
        CFG_RUNTIME_CONFIGDIR="\"\""
        CFG_INSTALL_LIBDIR="\"\""
        CFG_INSTALL_BINDIR="\"\""
        CFG_INSTALL_SCRDIR="\"\""
        CFG_INSTALL_INCDIR="\"\""
        CFG_INSTALL_DOCDIR="\"\""
        TCL_LIBRARY="\"\""
        TCL_PACKAGE_PATH="\"\""
        HAVE_ZLIB=1
        HAVE_UNISTD_H=1
        MP_FIXED_CUTOFFS=0
)

#target_compile_options(tcl_core PRIVATE
#        "-include" "unistd.h"
#)

#------------------------
# imgui core (vendored)
# -------------------------

#set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

#add_library(imgui_core STATIC
#        ${IMGUI_DIR}/imgui.cpp
#        ${IMGUI_DIR}/imgui_draw.cpp
#        ${IMGUI_DIR}/imgui_tables.cpp
#        ${IMGUI_DIR}/imgui_widgets.cpp
        # ${IMGUI_DIR}/imgui_demo.cpp  # opcional

#        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
#        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
#)

#target_include_directories(imgui_core PUBLIC
#        ${IMGUI_DIR}
#        ${IMGUI_DIR}/backends
#)

# Para Android normalmente é OpenGL ES
#target_compile_definitions(imgui_core PUBLIC
#        IMGUI_IMPL_OPENGL_ES2=1
#)

# ImGui usa SDL2 headers
#target_link_libraries(imgui_core PUBLIC
#        SDL2
#)


# -------------------------
# Freetype core (vendored)
# -------------------------
set(FREETYPE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/freetype)

# FreeType tem CMake oficial
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
add_subdirectory(${FREETYPE_DIR} freetype_build)

# ---- Tell FindFreetype where our vendored freetype is ----
set(FREETYPE_INCLUDE_DIRS
        ${FREETYPE_DIR}/include
        CACHE PATH "" FORCE
)

# libfreetype.a produzido pelo target "freetype"
get_target_property(_FT_LIB freetype IMPORTED_LOCATION)
#get_target_property(_FT_LIB freetype LOCATION)
set(FREETYPE_LIBRARY ${_FT_LIB} CACHE FILEPATH "" FORCE)

# Algumas versões do FindFreetype também olham essas:
set(FREETYPE_INCLUDE_DIR ${FREETYPE_DIR}/include CACHE PATH "" FORCE)
set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY} CACHE FILEPATH "" FORCE)


if(TARGET freetype AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()


# (opcional) reduzir dependências do freetype
# você pode setar opções antes do add_subdirectory dependendo da versão do freetype:
# set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
# set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
# set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)





# -------------------------
# SDL_ttf (vendored)
# -------------------------
set(SDL2_TTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL_ttf)

# Evitar que SDL_ttf tente baixar dependências
#set(SDL2TTF_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)   # simplifica
set(SDL2TTF_FREETYPE ON CACHE BOOL "" FORCE)    # garante que quer freetype
set(SDL2TTF_FREETYPE_VENDORED OFF CACHE BOOL "" FORCE)

# SDL_ttf deve seguir o mesmo tipo de SDL2 (estático) do projeto
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#set(SDL2_SHARED OFF CACHE BOOL "" FORCE)
#set(SDL2_STATIC ON CACHE BOOL "" FORCE)


get_property(_tgts DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SDL2 PROPERTY BUILDSYSTEM_TARGETS)
message(STATUS "SDL targets: ${_tgts}")

# Se o SDL_ttf tiver CMake oficial:
add_subdirectory(${SDL2_TTF_DIR} sdl2_ttf_build)

# Senão, criar um target manual (exemplo genérico):
# add_library(SDL2_ttf STATIC
#     ${SDL2_TTF_DIR}/SDL_ttf.c
# )
# target_include_directories(SDL2_ttf PUBLIC ${SDL2_TTF_DIR})

# SDL_ttf precisa SDL2 + freetype (+ zlib às vezes)
#target_link_libraries(SDL2_ttf PUBLIC
#        SDL2
#        freetype
#        z
#)
#target_include_directories(SDL2_ttf PUBLIC
#target_include_directories(SDL2_ttf PUBLIC
#        ${FREETYPE_DIR}/include
#)



# -------------------------
# openMSX core (vendored)
# -------------------------
set(OPENMSX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openmsx)
set(OPENMSX_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openmsx_generated)

file(GLOB_RECURSE OPENMSX_SRC
        ${OPENMSX_DIR}/src/*.cc
        ${OPENMSX_DIR}/src/*.c
)

file(GLOB_RECURSE OPENMSX_GENERATED_SRC
        ${OPENMSX_GEN_DIR}/*.hh
        ${OPENMSX_GEN_DIR}/*.cc
)

foreach(f ${OPENMSX_SRC})
    if(f MATCHES "tcl\\.hh$")
        message(STATUS "FOUND TCL HEADER IN SRC LIST: ${f}")
    endif()
endforeach()


# Remove openmsx's original entrypoint
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/main\\.cc$")

# MVP0: sem laserdisc
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/laserdisc/.*")
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/video/ld/.*")

# MVP0: no MIDI
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/serial/MidiSessionALSA.*")

# MVP0: no Testes
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/unittest/.*")

# MVP0: sem suporte a PNG
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/video/PNG\\.cc$")
list(FILTER OPENMSX_SRC EXCLUDE REGEX ".*/src/video/PNG\\.hh$")

add_library(openmsx_core STATIC
        ${OPENMSX_SRC}
        ${OPENMSX_GENERATED_SRC}
)

target_include_directories(openmsx_core PUBLIC
        ${OPENMSX_GEN_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/openmsx_android/stubs
        ${OPENMSX_DIR}/src
        # SDL2
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include
        # TCL
        ${TCL_DIR}/generic
        ${TCL_DIR}/unix
        ${TCL_GEN_DIR}
        # ImgUI
        #${CMAKE_CURRENT_SOURCE_DIR}/imgui
        # freetype
        ${FREETYPE_DIR}/include
        ${FREETYPE_DIR}/include/freetype
        # SDL ttf
        ${SDL2_TTF_DIR}
        # Se existir algo como ${OPENMSX_DIR}/3rdparty/include, inclua aqui:
        # ${OPENMSX_DIR}/3rdparty
        ${OPENMSX_DIR}/src/utils
        ${OPENMSX_DIR}/src/3rdparty
        ${OPENMSX_DIR}/src/commands
        ${OPENMSX_DIR}/src/file
        ${OPENMSX_DIR}/src/config
        ${OPENMSX_DIR}/src/input
        ${OPENMSX_DIR}/src/settings
        ${OPENMSX_DIR}/src/events
        ${OPENMSX_DIR}/src/video
        ${OPENMSX_DIR}/src/video/v9990
        ${OPENMSX_DIR}/src/video/scalers
        ${OPENMSX_DIR}/src/console
        ${OPENMSX_DIR}/src/imgui
        ${OPENMSX_DIR}/src/thread
        ${OPENMSX_DIR}/src/memory
        ${OPENMSX_DIR}/src/debugger
        ${OPENMSX_DIR}/src/cpu
        ${OPENMSX_DIR}/src/sound
        ${OPENMSX_DIR}/src/serial
        ${OPENMSX_DIR}/src/ide
        ${OPENMSX_DIR}/src/cassette
        ${OPENMSX_DIR}/src/resource
        ${OPENMSX_DIR}/src/fdc
        ${OPENMSX_DIR}/src/security
        #${OPENMSX_DIR}/src/laserdisc
        #${OPENMSX_DIR}/unittest
        ${OPENMSX_DIR}/src/3rdparty/imgui
        ${OPENMSX_DIR}/src/3rdparty/imgui/misc/freetype
        ${OPENMSX_DIR}/src/3rdparty/ImGuiFileDialog
)

target_compile_definitions(openmsx_core PRIVATE
        ANDROID=1
        PLATFORM_ANDROID=1
        _FILE_OFFSET_BITS=64
        #OPENGL_VERSION=OPENGL_ES_2_0
        #OPENGL_VERSION=OPENGL_ES_3_0
        OPENGL_VERSION=OPENGL_ES_3_0
        #IMGUI_IMPL_OPENGL_ES2=1
        IMGUI_IMPL_OPENGL_ES3=1
        SDL_VIDEO_DRIVER_ANDROID=1
)


# Alguns projetos assumem isso no Linux; no Android pode não ser necessário,
# mas costuma ajudar com código legado.
target_compile_options(openmsx_core PRIVATE
        -Wall
        -Wextra
)

target_compile_features(openmsx_core PUBLIC cxx_std_23)

target_link_libraries(openmsx_core PUBLIC
        SDL2
        tcl_core
        #imgui_core
        freetype
        SDL2_ttf
        EGL
        GLESv3
)


# -------------------------
# Main library loaded by SDLActivity: libmain.so
# -------------------------
add_library(main SHARED
        main.cpp
        openmsx_android/openmsx_entry.cpp
)

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include
)

target_link_libraries(main
        openmsx_core
        SDL2
        EGL
        GLESv3
        z
        android
        log
)
