//
// Created by cleve on 16/12/2025.
//
#include <android/log.h>
#include <SDL.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "MVP0", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MVP0", __VA_ARGS__)

extern "C" int SDL_main(int argc, char** argv) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Habilita teclado virtual / IME (ANDROID)
    SDL_StartTextInput();

    // (Opcional) capturar botão "Voltar" como tecla em vez de fechar Activity
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

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

    LOGI("Window created. Type using soft keyboard or BT keyboard.");

    bool running = true;
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
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
