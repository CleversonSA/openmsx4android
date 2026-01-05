#include "CliServer.hh"
#include "CommandLineParser.hh"
#include "Display.hh"
#include "EnumSetting.hh"
#include "EventDistributor.hh"
#include "MSXException.hh"
#include "Reactor.hh"
#include "RenderSettings.hh"
#include "Thread.hh"

#include "one_of.hh"
#include "random.hh"
#include "setenv.hh" // openmsx wrapper

#include <SDL.h>

#include <android/log.h>

#include <cstdlib>
#include <exception>
#include <span>
#include <string>
#include <vector>

namespace openmsx {

    static void logi(const char* msg) {
        __android_log_print(ANDROID_LOG_INFO, "openmsx4android", "%s", msg);
    }
    static void loge(const char* msg) {
        __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "%s", msg);
    }



    static void  initializeSDL_minimal()
    {
        // IMPORTANT:
        // If your SDLActivity already initialized SDL, SDL_InitSubSystem is safe.
        // If not, this will initialize required subsystems.
        int flags = 0;
        flags |= SDL_INIT_JOYSTICK;
#ifndef NDEBUG
        flags |= SDL_INIT_NOPARACHUTE;
#endif

        // Prefer InitSubSystem to avoid conflicts if SDL was already initialized.
        if (SDL_WasInit(0) == 0) {
            if (SDL_Init(flags) < 0) {
                std::string s = "Couldn't init SDL: ";
                s += SDL_GetError();
                throw FatalError(s);
            }
        } else {
            if (SDL_InitSubSystem(flags) < 0) {
                std::string s = "Couldn't init SDL subsystems: ";
                s += SDL_GetError();
                throw FatalError(s);
            }
        }

        // Keep same behavior as upstream
        setenv("FREETYPE_PROPERTIES", "truetype:interpreter-version=35", 0);
    }

// Expose a function we can call from your SDL glue main.cpp
    int android_entry(int argc, char** argv)
    {
        int exitCode = 0;

        SDL_SetHint(SDL_HINT_VIDEO_EGL_ALLOW_TRANSPARENCY, "0");
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
        SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_PREFER_LIBDECOR, "0");
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
        SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
        SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "0");

        SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

        Reactor reactor;

        try {
            randomize();
            initializeSDL_minimal();

            Thread::setMainThread();

            std::span<char*> args{argv, size_t(argc)};

            CommandLineParser parser(reactor);
            parser.parse(args);
            auto parseStatus = parser.getParseStatus();

            if (parseStatus != one_of(CommandLineParser::Status::EXIT,
                                      CommandLineParser::Status::TEST)) {
                // If you need to set Android-specific data dirs, do it BEFORE this.
                reactor.runStartupScripts(parser);

                auto& display = reactor.getDisplay();
                auto& render = display.getRenderSettings().getRendererSetting();

                if ((render.getEnum() == RenderSettings::RendererID::UNINITIALIZED) &&
                    (parseStatus != CommandLineParser::Status::CONTROL)) {

                    render.setValue(render.getDefaultValue());
                    reactor.getEventDistributor().deliverEvents();
                }

                CliServer cliServer(reactor.getCommandController(),
                                    reactor.getEventDistributor(),
                                    reactor.getGlobalCliComm());

                if (parser.getParseStatus() == CommandLineParser::Status::RUN) {
                    reactor.powerOn();
                }

                display.repaint();
                reactor.run();
            }
        } catch (FatalError& e) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "FatalError: %s", e.getMessage().c_str());
            std::string s = "Fatal error: ";
            s += e.getMessage();
            loge(s.c_str());
            exitCode = 1;
            std::_Exit(1); //Forca
        } catch (MSXException& e) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "MSXException: %s", e.getMessage().c_str());
            std::string s = "Uncaught exception: ";
            s += e.getMessage();
            loge(s.c_str());
            exitCode = 1;
            std::_Exit(1); //Forca
        } catch (std::exception& e) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "std::exception: %s", e.what());

            std::string s = "Uncaught std::exception: ";
            s += e.what();
            loge(s.c_str());
            exitCode = 1;
            std::_Exit(1); //Forca
        } catch (...) {
            __android_log_print(ANDROID_LOG_ERROR, "openmsx4android", "Multi dimension of time error! Idk");
            loge("Uncaught exception of unexpected type.");
            exitCode = 1;
            std::_Exit(1); //Forca
        }

        // Let SDLActivity handle final SDL_Quit when app exits.
        // If you call SDL_Quit() here, you may break activity lifecycle.
        return exitCode;
    }

} // namespace openmsx

extern "C" int openmsx_android_main(int argc, char** argv)
{
    for (int i = 0; i < argc; ++i) {
        __android_log_print(ANDROID_LOG_INFO, "openmsx4android", "argv[%d]=%s", i, argv[i]);
    }

    return openmsx::android_entry(argc, argv);
}
