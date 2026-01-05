//
// Created by cleve on 16/12/2025.
//
#include <android/log.h>
#include <SDL.h>
#include "openmsx_android/openmsx_entry.h"

#include <sys/stat.h>


#include <string>
#include <vector>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "MVP0", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MVP0", __VA_ARGS__)



static void mkdirs(const std::string& fullPath) {
    // cria diretórios para um *arquivo* (até o último '/')
    size_t pos = 0;
    while ((pos = fullPath.find('/', pos + 1)) != std::string::npos) {
        std::string dir = fullPath.substr(0, pos);
        mkdir(dir.c_str(), 0700); // ok se já existir
    }
}

static bool copyRW(SDL_RWops* in, SDL_RWops* out) {
    char buf[64 * 1024];
    size_t r;
    while ((r = SDL_RWread(in, buf, 1, sizeof(buf))) > 0) {
        if (SDL_RWwrite(out, buf, 1, r) != r) return false;
    }
    return true;
}

static std::vector<std::string> readLinesFromAsset(const char* assetPath) {
    std::vector<std::string> lines;
    SDL_RWops* rw = SDL_RWFromFile(assetPath, "rb");
    if (!rw) {
        __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                            "Failed to open asset %s: %s", assetPath, SDL_GetError());
        return lines;
    }
    Sint64 sz = SDL_RWsize(rw);
    if (sz <= 0) { SDL_RWclose(rw); return lines; }

    std::string content;
    content.resize(size_t(sz));
    SDL_RWread(rw, content.data(), 1, size_t(sz));
    SDL_RWclose(rw);

    size_t start = 0;
    while (start < content.size()) {
        size_t end = content.find('\n', start);
        if (end == std::string::npos) end = content.size();
        std::string line = content.substr(start, end - start);
        // trim CR
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty() && line[0] != '#') lines.push_back(line);
        start = end + 1;
    }
    return lines;
}

static bool extractOpenmsxAssets(const std::string& systemDir) {
    // systemDir ex: /data/data/<pkg>/files/openmsx
    auto files = readLinesFromAsset("openmsx/index.txt");
    if (files.empty()) return false;

    for (const auto& rel : files) {
        std::string asset = "openmsx/" + rel;
        std::string out   = systemDir + "/" + rel;

        // já existe? pule
        SDL_RWops* test = SDL_RWFromFile(out.c_str(), "rb");
        if (test) { SDL_RWclose(test); continue; }

        SDL_RWops* in  = SDL_RWFromFile(asset.c_str(), "rb");
        if (!in) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                "Missing asset %s: %s", asset.c_str(), SDL_GetError());
            return false;
        }

        mkdirs(out);
        SDL_RWops* outRw = SDL_RWFromFile(out.c_str(), "wb");
        if (!outRw) {
            SDL_RWclose(in);
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                "Can't create %s: %s", out.c_str(), SDL_GetError());
            return false;
        }

        bool ok = copyRW(in, outRw);
        SDL_RWclose(in);
        SDL_RWclose(outRw);

        if (!ok) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android",
                                "Copy failed for %s", rel.c_str());
            return false;
        }
    }
    return true;
}

extern "C" int SDL_main(int argc, char** argv) {
    (void)argc; (void)argv;

    /*if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }*/

    /*SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);*/

    // Habilita teclado virtual / IME (ANDROID)
    //SDL_StartTextInput();

    // (Opcional) capturar botão "Voltar" como tecla em vez de fechar Activity
    /*SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

    SDL_Window* win = SDL_CreateWindow(
            "MVP0 SDL Window",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1280, 720,
            SDL_WINDOW_SHOWN
    );

    if (!win) {
        LOGE("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    LOGI("Window created. Type using soft keyboard or BT keyboard.");*/

    /*const char* ver = (const char*)glGetString(GL_VERSION);
    __android_log_print(ANDROID_LOG_INFO, "MVP0", "GL_VERSION=%s", ver ? ver : "(null)");*/

    /*bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_KEYDOWN:
                    LOGI("KEYDOWN sym=%d scancode=%d", ev.key.keysym.sym, ev.key.keysym.scancode);
                    if (ev.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
                        LOGI("Back pressed -> exiting");
                        running = false;
                    }
                    break;

                case SDL_TEXTINPUT:
                    LOGI("TEXTINPUT: %s", ev.text.text);
                    break;

                default:
                    break;
            }
        }

        SDL_Delay(10);
    }*/

    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"); // inofensivo fora do Linux

    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

// IMPORTANT: pedir um contexto OpenGL ES
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

// Para openMSX MVP: comece com ES 2.0 (mais compatível)
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

// Opcional, mas ajuda estabilidade
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    char* pref = SDL_GetPrefPath("com.openmsx", "openmsx4android");
    std::string prefPath = pref ? pref : "";
    SDL_free(pref);

    /*std::string systemDir = prefPath + "openmsx";   // <- contém "share/"
    __android_log_print(ANDROID_LOG_INFO, "openmsx4android",
                        "systemDir=%s", systemDir.c_str());
    */
    std::vector<std::string> args = {};



    std::string systemDir = std::string(prefPath) + "openmsx";
    extractOpenmsxAssets(systemDir);

    args = {"openmsx","-machine","Gradiente_Expert_GPC-1", "-command","set fullscreen on"};

    std::vector<char*> cargs;
    cargs.reserve(args.size());
    for (auto& s : args) cargs.push_back(s.data());



   std::string test = systemDir + "/share/machines/C-BIOS_MSX2+.xml";
   SDL_RWops* rw = SDL_RWFromFile(test.c_str(), "rb");
   __android_log_print(ANDROID_LOG_INFO, "openmsx4android", "test path=%s exists=%d",
                       test.c_str(), rw != nullptr);
   if (rw) SDL_RWclose(rw);

   /* Teste de sanidade
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* w = SDL_CreateWindow("Saramba",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!w) __android_log_print(ANDROID_LOG_ERROR,"openmsx4android", "CreateWindow foi pra puta que o pariu: %s", SDL_GetError());

    SDL_GLContext ctx = SDL_GL_CreateContext(w);
    if (!ctx) __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "CreateContext foi pra casa do chapéu: %s",SDL_GetError());

    __android_log_print(ANDROID_LOG_DEBUG,"openmsx4android", "Passou liso o Window! Entáo a parada é no emulador!");
*/
    return openmsx_android_main((int)cargs.size(), cargs.data());


    /*SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;*/
}
