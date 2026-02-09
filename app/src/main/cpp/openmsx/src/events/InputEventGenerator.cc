#include "InputEventGenerator.hh"

#include "Event.hh"
#include "EventDistributor.hh"
#include "FileOperations.hh"
#include "GlobalSettings.hh"
#include "IntegerSetting.hh"
#include "SDLKey.hh"

#include "one_of.hh"
#include "outer.hh"
#include "unreachable.hh"
#include "utf8_unchecked.hh"

#include <utility>

#if defined(__ANDROID__)
#include <jni.h>
#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_DEBUG, "openMSX-SDL", __VA_ARGS__)
#else
#define ALOG(...)
#endif

namespace openmsx {

#if defined(__ANDROID__)
    bool imeEnabled = false;

    static const char* sdlEventName(uint32_t type)
    {
        switch (type) {
            case SDL_QUIT: return "SDL_QUIT";

            case SDL_APP_TERMINATING: return "SDL_APP_TERMINATING";
            case SDL_APP_LOWMEMORY: return "SDL_APP_LOWMEMORY";
            case SDL_APP_WILLENTERBACKGROUND: return "SDL_APP_WILLENTERBACKGROUND";
            case SDL_APP_DIDENTERBACKGROUND: return "SDL_APP_DIDENTERBACKGROUND";
            case SDL_APP_WILLENTERFOREGROUND: return "SDL_APP_WILLENTERFOREGROUND";
            case SDL_APP_DIDENTERFOREGROUND: return "SDL_APP_DIDENTERFOREGROUND";

            case SDL_WINDOWEVENT: return "SDL_WINDOWEVENT";

            case SDL_KEYDOWN: return "SDL_KEYDOWN";
            case SDL_KEYUP: return "SDL_KEYUP";

            case SDL_TEXTEDITING: return "SDL_TEXTEDITING";
            case SDL_TEXTINPUT: return "SDL_TEXTINPUT";

            case SDL_MOUSEMOTION: return "SDL_MOUSEMOTION";
            case SDL_MOUSEBUTTONDOWN: return "SDL_MOUSEBUTTONDOWN";
            case SDL_MOUSEBUTTONUP: return "SDL_MOUSEBUTTONUP";

            case SDL_FINGERDOWN: return "SDL_FINGERDOWN";
            case SDL_FINGERUP: return "SDL_FINGERUP";
            case SDL_FINGERMOTION: return "SDL_FINGERMOTION";

            case SDL_JOYBUTTONDOWN: return "SDL_JOYBUTTONDOWN";
            case SDL_JOYBUTTONUP: return "SDL_JOYBUTTONUP";

            case SDL_CONTROLLERBUTTONDOWN: return "SDL_CONTROLLERBUTTONDOWN";
            case SDL_CONTROLLERBUTTONUP: return "SDL_CONTROLLERBUTTONUP";

            case SDL_USEREVENT: return "SDL_USEREVENT";
            default: return "SDL_UNKNOWN_EVENT";
        }
    }

    static void logSdlEvent(const SDL_Event& e)
    {
        ALOG("EVENT type=%u (%s)", e.type, sdlEventName(e.type));

        switch (e.type) {

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                ALOG("  KEY %s sym=%d scancode=%d mod=0x%x repeat=%d",
                     e.type == SDL_KEYDOWN ? "DOWN" : "UP",
                     (int)e.key.keysym.sym,
                     (int)e.key.keysym.scancode,
                     (int)e.key.keysym.mod,
                     e.key.repeat);
                break;

            case SDL_TEXTINPUT:
                ALOG("  TEXTINPUT '%s'", e.text.text);
                break;

            case SDL_TEXTEDITING:
                ALOG("  TEXTEDITING '%s' start=%d len=%d",
                     e.edit.text, e.edit.start, e.edit.length);
                break;

            case SDL_WINDOWEVENT:
                ALOG("  WINDOWEVENT code=%d", e.window.event);
                break;

            case SDL_USEREVENT:
                ALOG("  USEREVENT code=%d data1=%p data2=%p",
                     e.user.code, e.user.data1, e.user.data2);
                break;

            default:
                break;
        }
    }

    static void fillKeyEvent(SDL_Event& e, Uint32 type, Uint32 ts,
                             SDL_Keycode sym, SDL_Keymod mod, uint32_t unicode)
    {
        e = {};
        e.type = type;
        e.key.type = type;
        e.key.timestamp = ts;
        e.key.state = (type == SDL_KEYDOWN) ? SDL_PRESSED : SDL_RELEASED;
        e.key.repeat = 0;
        e.key.keysym.sym = sym;
        e.key.keysym.mod = mod;
        e.key.keysym.scancode = SDL_GetScancodeFromKey(sym);
        e.key.keysym.unused = unicode; // openMSX usa isso como unicode
    }

    static bool unicodeToSdlKey(uint32_t u, SDL_Keycode& sym, SDL_Keymod& mod)
    {
        mod = KMOD_NONE;

        // Controle básico
        if (u == '\n' || u == '\r') { sym = SDLK_RETURN; return true; }
        if (u == '\t')             { sym = SDLK_TAB;    return true; }
        if (u == ' ')              { sym = SDLK_SPACE;  return true; }

        // Letras
        if (u >= 'a' && u <= 'z') {
            sym = SDL_Keycode(SDLK_a + (u - 'a'));
            return true;
        }
        if (u >= 'A' && u <= 'Z') {
            sym = SDL_Keycode(SDLK_a + (u - 'A'));
            mod = KMOD_SHIFT;
            return true;
        }

        // Dígitos
        if (u >= '0' && u <= '9') {
            sym = SDL_Keycode(SDLK_0 + (u - '0'));
            return true;
        }

        // Pontuação simples (sem shift)
        switch (u) {
            case '\'': sym = SDLK_QUOTE;       return true; // aspas simples
            case '`':  sym = SDLK_BACKQUOTE;   return true; // crase
            case '-':  sym = SDLK_MINUS;       return true;
            case '=':  sym = SDLK_EQUALS;      return true;
            case '[':  sym = SDLK_LEFTBRACKET; return true;
            case ']':  sym = SDLK_RIGHTBRACKET;return true;
            case '\\': sym = SDLK_BACKSLASH;   return true;
            case ';':  sym = SDLK_SEMICOLON;   return true;
            case ',':  sym = SDLK_COMMA;       return true;
            case '.':  sym = SDLK_PERIOD;      return true;
            case '/':  sym = SDLK_SLASH;       return true;
        }

        // Pontuação com SHIFT
        switch (u) {
            case '"':  sym = SDLK_QUOTE;        mod = KMOD_SHIFT; return true; // aspas duplas
            case '^':  sym = SDLK_6;            mod = KMOD_SHIFT; return true; // circunflexo
            case '~':  sym = SDLK_BACKQUOTE;    mod = KMOD_SHIFT; return true; // til
            case '!':  sym = SDLK_1;            mod = KMOD_SHIFT; return true;
            case '@':  sym = SDLK_2;            mod = KMOD_SHIFT; return true;
            case '#':  sym = SDLK_3;            mod = KMOD_SHIFT; return true;
            case '$':  sym = SDLK_4;            mod = KMOD_SHIFT; return true;
            case '%':  sym = SDLK_5;            mod = KMOD_SHIFT; return true;
            case '&':  sym = SDLK_7;            mod = KMOD_SHIFT; return true;
            case '*':  sym = SDLK_8;            mod = KMOD_SHIFT; return true;
            case '(':  sym = SDLK_9;            mod = KMOD_SHIFT; return true;
            case ')':  sym = SDLK_0;            mod = KMOD_SHIFT; return true;
            case '_':  sym = SDLK_MINUS;        mod = KMOD_SHIFT; return true;
            case '+':  sym = SDLK_EQUALS;       mod = KMOD_SHIFT; return true;
            case '{':  sym = SDLK_LEFTBRACKET; mod = KMOD_SHIFT; return true;
            case '}':  sym = SDLK_RIGHTBRACKET;mod = KMOD_SHIFT; return true;
            case '|':  sym = SDLK_BACKSLASH;   mod = KMOD_SHIFT; return true;
            case ':':  sym = SDLK_SEMICOLON;   mod = KMOD_SHIFT; return true;
            case '<':  sym = SDLK_COMMA;       mod = KMOD_SHIFT; return true;
            case '>':  sym = SDLK_PERIOD;      mod = KMOD_SHIFT; return true;
            case '?':  sym = SDLK_SLASH;       mod = KMOD_SHIFT; return true;
        }

        return false; // ainda não mapeado
    }


    void InputEventGenerator::androidCommitText(uint32_t timestamp, const char* utf8)
    {
        // Use relógio atual para evitar bursts com timestamp igual
        uint32_t ts = SDL_GetTicks();

        while (true) {
            uint32_t unicode = utf8::unchecked::next(utf8);
            if (unicode == 0) break;

            SDL_Keycode sym;
            SDL_Keymod mod;

            if (!unicodeToSdlKey(unicode, sym, mod)) {
                // fallback: não sabemos mapear -> ignore por enquanto
                // (quando você quiser, dá pra expandir pra pontuação)
                continue;
            }

            auto pushDownUp = [&](SDL_Keycode k, SDL_Keymod m, uint32_t u) {
                SDL_Event down, up;
                fillKeyEvent(down, SDL_KEYDOWN, ts,     k, m, u);
                fillKeyEvent(up,   SDL_KEYUP,   ts + 2, k, m, u);
                eventDistributor.distributeEvent(KeyDownEvent(down));
                eventDistributor.distributeEvent(KeyUpEvent(up));
                ts += 4;
            };

            if (mod & KMOD_SHIFT) {
                // 1) SHIFT DOWN (mantém pressionado)
                SDL_Event shDown;
                fillKeyEvent(shDown, SDL_KEYDOWN, ts, SDLK_LSHIFT, KMOD_NONE, 0);
                eventDistributor.distributeEvent(KeyDownEvent(shDown));
                ts += 1;

                // 2) LETRA (sem mod; SHIFT já está "físico" down)
                pushDownUp(sym, KMOD_NONE, unicode);

                // 3) SHIFT UP (depois da letra)
                SDL_Event shUp;
                fillKeyEvent(shUp, SDL_KEYUP, ts + 2, SDLK_LSHIFT, KMOD_NONE, 0);
                eventDistributor.distributeEvent(KeyUpEvent(shUp));
                ts += 1;
            } else {
                // sem shift
                pushDownUp(sym, mod, unicode);
            }
        }
    }


    extern "C"
    JNIEXPORT void JNICALL
    Java_org_libsdl_app_OpenMSX4AndroidSDLActivity_nativeSendDpad(
            JNIEnv*, jclass, jint dir, jboolean isDown)
    {
        SDL_Scancode sc;
        SDL_Keycode kc;

        switch (dir) {
            case 0: sc = SDL_SCANCODE_UP;    kc = SDLK_UP;    break;
            case 1: sc = SDL_SCANCODE_DOWN;  kc = SDLK_DOWN;  break;
            case 2: sc = SDL_SCANCODE_LEFT;  kc = SDLK_LEFT;  break;
            case 3: sc = SDL_SCANCODE_RIGHT; kc = SDLK_RIGHT; break;
            case 4: sc = SDL_SCANCODE_RETURN; kc = SDLK_RETURN; break;
            case 5: sc = SDL_SCANCODE_SPACE; kc = SDLK_SPACE; break;
            case 6: sc = SDL_SCANCODE_TAB; kc = SDLK_TAB; break;
            case 7: sc = SDL_SCANCODE_ESCAPE; kc = SDLK_ESCAPE; break;
            default:;
                return;
        }

        imeEnabled = false;
        SDL_StopTextInput();

        SDL_Event ev;
        SDL_zero(ev);
        ev.type = isDown ? SDL_KEYDOWN : SDL_KEYUP;
        ev.key.type = ev.type;
        ev.key.state = isDown ? SDL_PRESSED : SDL_RELEASED;
        ev.key.repeat = 0;
        ev.key.keysym.scancode = sc;
        ev.key.keysym.sym = kc;
        ev.key.keysym.mod = KMOD_NONE;

        SDL_PushEvent(&ev);
    }

    extern "C"
    JNIEXPORT void JNICALL
    Java_org_libsdl_app_OpenMSX4AndroidSDLActivity_nativeRequestTextInput(JNIEnv* env, jclass cls, jboolean enable)
{
    SDL_Event ev;
    SDL_zero(ev);
    ev.type = SDL_USEREVENT;
    ev.user.code = enable ? 1 : 0;
    SDL_PushEvent(&ev);
}


#endif

InputEventGenerator::InputEventGenerator(CommandController& commandController,
                                         EventDistributor& eventDistributor_)
	: eventDistributor(eventDistributor_)
	, joystickManager(commandController)
	, grabInput(
		commandController, "grabinput",
		"This setting controls if openMSX takes over mouse and keyboard input",
		false, Setting::Save::NO)
	, escapeGrabCmd(commandController)
{
	setGrabInput(grabInput.getBoolean());
	eventDistributor.registerEventListener(EventType::WINDOW, *this);
}

InputEventGenerator::~InputEventGenerator()
{
	eventDistributor.unregisterEventListener(EventType::WINDOW, *this);
}

void InputEventGenerator::wait()
{
	// SDL bug workaround
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		SDL_Delay(100);
	}

	if (SDL_WaitEvent(nullptr)) {
		poll();
	}
}

void InputEventGenerator::poll()
{
	// Heuristic to emulate the old SDL1 behavior:
	//
	// SDL1 had a unicode field on each KEYDOWN event. In SDL2 that
	// information is moved to the (new) SDL_TEXTINPUT events.
	//
	// Though our MSX keyboard emulation code needs to relate KEYDOWN
	// events with the associated unicode. We try to mimic this by the
	// following heuristic:
	//   When two successive events in a single batch (so, the same
	//   invocation of poll()) are KEYDOWN followed by TEXTINPUT, then copy
	//   the unicode (of the first character) of the TEXT event to the
	//   KEYDOWN event.
	// Implementing this requires a lookahead of 1 event. So the code below
	// deals with a 'current' and a 'previous' event, and keeps track of
	// whether the previous event is still pending (not yet processed).
	//
	// In a previous version we also added the constraint that these two
	// consecutive events must have the same timestamp, but that had mixed
	// results:
	// - on Linux it worked fine
	// - on Windows the timestamps sometimes did not match
	// - on Mac the timestamps mostly did not match
	// So we removed this constraint.
	//
	// We also split SDL_TEXTINPUT events into (possibly) multiple KEYDOWN
	// events because a single event type makes it easier to handle higher
	// priority listeners that can block the event for lower priority
	// listener (console > hotkey > msx).

	SDL_Event event1, event2;
	auto* prev = &event1;
	auto* curr = &event2;
	bool pending = false;

	while (SDL_PollEvent(curr)) {

#if defined(__ANDROID__)
        if (curr->type == SDL_USEREVENT) {
            if (curr->user.code == 1) {
                imeEnabled = true;
                SDL_StartTextInput();
            } else {
                imeEnabled = false;
                SDL_StopTextInput();
            }
            continue;
        }

        logSdlEvent(*curr);

        // --- IME: texto confirmado ---
        if (curr->type == SDL_TEXTINPUT) {

            // descarrega qualquer estado pendente de KEYDOWN físico
            if (pending) {
                pending = false;
                handle(*prev);
            }

            if (curr->text.text[0] != '\0') {
                androidCommitText(curr->text.timestamp,curr->text.text);
                eventDistributor.distributeEvent(TextEvent(*curr));
            }
            continue;
        }

        // --- IME: composição (ignorar) ---
        if (curr->type == SDL_TEXTEDITING) {
            continue;
        }

        // --- ANDROID: perda de foco / mudança de estado do app ---
        if (curr->type == SDL_WINDOWEVENT) {
            if (curr->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                __android_log_print(
                        ANDROID_LOG_INFO,
                        "MVP-IME",
                        "FOCUS LOST -> resetting input state"
                );

                // IMPORTANTE: limpa estado interno de teclado
                pending = false;

                // NÃO continue; deixe o evento seguir para handle()
                // para que outras partes do sistema saibam da perda de foco
            }
        }

        if (curr->type >= SDL_APP_TERMINATING &&
            curr->type <= SDL_APP_DIDENTERFOREGROUND) {

            __android_log_print(
                    ANDROID_LOG_INFO,
                    "MVP-IME",
                    "APP STATE EVENT %u -> resetting input state",
                    curr->type
            );

            // Mesma ideia: não deixe estado interno travado
            pending = false;
        }

        if (imeEnabled && (curr->type == SDL_KEYDOWN || curr->type == SDL_KEYUP)) {
            SDL_Keycode sym = curr->key.keysym.sym;

            // deixa passar apenas teclas especiais
            bool special =
                    (sym == SDLK_RETURN) || (sym == SDLK_KP_ENTER) ||
                    (sym == SDLK_BACKSPACE) || (sym == SDLK_TAB) ||
                    (sym == SDLK_ESCAPE) ||
                    (sym == SDLK_LEFT) || (sym == SDLK_RIGHT) || (sym == SDLK_UP) || (sym == SDLK_DOWN) ||
                    (sym == SDLK_DELETE);

            if (!special) {
                // Ignora letras/números/pontuação vindos como KEYDOWN/UP durante IME
                // porque o texto “de verdade” já vem em SDL_TEXTINPUT.
                continue;
            }

            handle(*curr);
            continue;
        }
#endif

        if (pending && !imeEnabled) {
            pending = false;
            if (prev->type == SDL_KEYDOWN && (curr->type == SDL_TEXTINPUT)) {
                const char *utf8 = curr->text.text;
                auto unicode = utf8::unchecked::next(utf8);
                handleKeyDown(prev->key, unicode);
                if (unicode) { // possibly there are more characters
                    splitText(curr->text.timestamp, utf8);
                }
                eventDistributor.distributeEvent(TextEvent(*curr));
                continue;
            } else {
                handle(*prev);
            }
		}
		if (curr->type == SDL_KEYDOWN) {
			pending = true;
			std::swap(curr, prev);
		} else {
			handle(*curr);
		}
	}

    if (!SDL_PollEvent(curr)) {
    //    ALOG("SDL_PollEvent: no more events");
    }

	if (pending) {
		handle(*prev);
	}
}

void InputEventGenerator::setNewOsdControlButtonState(unsigned newState)
{
	unsigned deltaState = osdControlButtonsState ^ newState;
	using enum OsdControlEvent::Button;
	for (auto b : {LEFT, RIGHT, UP, DOWN, A, B}) {
		if (deltaState & (1 << std::to_underlying(b))) {
			if (newState & (1 << std::to_underlying(b))) {
				eventDistributor.distributeEvent(OsdControlReleaseEvent(b));
			} else {
				eventDistributor.distributeEvent(OsdControlPressEvent(b));
			}
		}
	}
	osdControlButtonsState = newState;
}

void InputEventGenerator::triggerOsdControlEventsFromJoystickAxisMotion(
	unsigned axis, int value)
{
	auto [neg_button, pos_button] = [&] {
		switch (axis) {
		using enum OsdControlEvent::Button;
		case 0:
			return std::pair{1u << std::to_underlying(LEFT),
			                 1u << std::to_underlying(RIGHT)};
		case 1:
			return std::pair{1u << std::to_underlying(UP),
			                 1u << std::to_underlying(DOWN)};
		default:
			// Ignore all other axis (3D joysticks and flight joysticks may
			// have more than 2 axis)
			return std::pair{0u, 0u};
		}
	}();

	if (value > 0) {
		// release negative button, press positive button
		setNewOsdControlButtonState(
			(osdControlButtonsState | neg_button) & ~pos_button);
	} else if (value < 0) {
		// press negative button, release positive button
		setNewOsdControlButtonState(
			(osdControlButtonsState | pos_button) & ~neg_button);
	} else {
		// release both buttons
		setNewOsdControlButtonState(
			osdControlButtonsState | neg_button | pos_button);
	}
}

void InputEventGenerator::triggerOsdControlEventsFromJoystickHat(int value)
{
	using enum OsdControlEvent::Button;
	unsigned dir = 0;
	if (!(value & SDL_HAT_UP   )) dir |= 1 << std::to_underlying(UP);
	if (!(value & SDL_HAT_DOWN )) dir |= 1 << std::to_underlying(DOWN);
	if (!(value & SDL_HAT_LEFT )) dir |= 1 << std::to_underlying(LEFT);
	if (!(value & SDL_HAT_RIGHT)) dir |= 1 << std::to_underlying(RIGHT);
	unsigned ab = osdControlButtonsState & ((1 << std::to_underlying(A)) |
	                                        (1 << std::to_underlying(B)));
	setNewOsdControlButtonState(ab | dir);
}

void InputEventGenerator::osdControlChangeButton(bool down, unsigned changedButtonMask)
{
	auto newButtonState = down
		? osdControlButtonsState & ~changedButtonMask
		: osdControlButtonsState | changedButtonMask;
	setNewOsdControlButtonState(newButtonState);
}

void InputEventGenerator::triggerOsdControlEventsFromJoystickButtonEvent(unsigned button, bool down)
{
	using enum OsdControlEvent::Button;
	osdControlChangeButton(
		down,
		((button & 1) ? (1 << std::to_underlying(B))
		              : (1 << std::to_underlying(A))));
}

void InputEventGenerator::triggerOsdControlEventsFromKeyEvent(SDLKey key, bool repeat)
{
	unsigned buttonMask = [&] {
		switch (key.sym.sym) {
		using enum OsdControlEvent::Button;
		case SDLK_LEFT:   return 1 << std::to_underlying(LEFT);
		case SDLK_RIGHT:  return 1 << std::to_underlying(RIGHT);
		case SDLK_UP:     return 1 << std::to_underlying(UP);
		case SDLK_DOWN:   return 1 << std::to_underlying(DOWN);
		case SDLK_SPACE:  return 1 << std::to_underlying(A);
		case SDLK_RETURN: return 1 << std::to_underlying(A);
		case SDLK_ESCAPE: return 1 << std::to_underlying(B);
		default: return 0;
		}
	}();
	if (buttonMask) {
		if (repeat) {
			osdControlChangeButton(!key.down, buttonMask);
		}
		osdControlChangeButton(key.down, buttonMask);
	}
}

static constexpr Uint16 normalizeModifier(SDL_Keycode sym, Uint16 mod)
{
	// Apparently modifier-keys also have the corresponding
	// modifier attribute set. See here for a discussion:
	//     https://github.com/openMSX/openMSX/issues/1202
	// As a solution, on pure modifier keys, we now clear the
	// modifier attributes.
	return (sym == one_of(SDLK_LCTRL, SDLK_LSHIFT, SDLK_LALT, SDLK_LGUI,
	                      SDLK_RCTRL, SDLK_RSHIFT, SDLK_RALT, SDLK_RGUI,
	                      SDLK_MODE))
		? 0
		: mod;
}

void InputEventGenerator::handleKeyDown(const SDL_KeyboardEvent& key, uint32_t unicode)
{
	/*if (PLATFORM_ANDROID && evt.key.keysym.sym == SDLK_WORLD_93) {
		event = JoystickButtonDownEvent(0, 0);
		triggerOsdControlEventsFromJoystickButtonEvent(
			0, false);
		androidButtonA = true;
	} else if (PLATFORM_ANDROID && evt.key.keysym.sym == SDLK_WORLD_94) {
		event = JoystickButtonDownEvent(0, 1);
		triggerOsdControlEventsFromJoystickButtonEvent(
			1, false);
		androidButtonB = true;
	} else*/ {
		SDL_Event evt;
		evt.key = SDL_KeyboardEvent{};
		memcpy(&evt.key, &key, sizeof(key));
		evt.key.keysym.mod = normalizeModifier(key.keysym.sym, key.keysym.mod);
		evt.key.keysym.unused = unicode;
		Event event = KeyDownEvent(evt);
		triggerOsdControlEventsFromKeyEvent({.sym = key.keysym, .down = true}, key.repeat);
		eventDistributor.distributeEvent(std::move(event));
	}
}

void InputEventGenerator::splitText(uint32_t timestamp, const char* utf8)
{
	while (true) {
		auto unicode = utf8::unchecked::next(utf8);
		if (unicode == 0) return;
		eventDistributor.distributeEvent(
			KeyDownEvent::create(timestamp, unicode));
	}
}

void InputEventGenerator::handle(SDL_Event& evt)
{
	// This is different between e.g. KDE and GNOME (and probably others)
	// Sometimes SDL sends both:
	// * SDL_QUIT and
	// * SDL_WINDOWEVENT_CLOSE-for-the-main-window.
	// Sometimes only one of these events. For both events openMSX should
	// exit, but we should only react to the first of these events, not
	// both.
	auto quitOnce = [&] -> std::optional<Event> {
		if (sendQuit) return {};
		sendQuit = true;
		return QuitEvent();
	};

	std::optional<Event> event;
	switch (evt.type) {
	case SDL_KEYUP:
		// Virtual joystick of SDL Android port does not have joystick
		// buttons. It has however up to 6 virtual buttons that can be
		// mapped to SDL keyboard events. Two of these virtual buttons
		// will be mapped to keys SDLK_WORLD_93 and 94 and are
		// interpreted here as joystick buttons (respectively button 0
		// and 1).
		// TODO Android code should be rewritten for SDL2
		// if (PLATFORM_ANDROID && evt.key.keysym.sym == SDLK_WORLD_93) {
		//	event = JoystickButtonUpEvent(0, 0);
		//	triggerOsdControlEventsFromJoystickButtonEvent(0, true);
		//	androidButtonA = false;
		// } else if (PLATFORM_ANDROID && evt.key.keysym.sym == SDLK_WORLD_94) {
		//	event = JoystickButtonUpEvent(0, 1);
		//	triggerOsdControlEventsFromJoystickButtonEvent(1, true);
		//	androidButtonB = false;
		//} else */ {
        {
			SDL_Event e;
			e.key = SDL_KeyboardEvent{};
			memcpy(&e.key, &evt.key, sizeof(evt.key));
			e.key.keysym.mod = normalizeModifier(evt.key.keysym.sym, evt.key.keysym.mod);
			event = KeyUpEvent(e);
			bool down = false;
			bool repeat = false;
			triggerOsdControlEventsFromKeyEvent({.sym = e.key.keysym, .down = down}, repeat);
		}
		break;
	case SDL_KEYDOWN:
		handleKeyDown(evt.key, 0);
		break;

	case SDL_MOUSEBUTTONUP:
		event = MouseButtonUpEvent(evt);
		break;
	case SDL_MOUSEBUTTONDOWN:
		event = MouseButtonDownEvent(evt);
		break;
	case SDL_MOUSEWHEEL:
		event = MouseWheelEvent(evt);
		break;
	case SDL_MOUSEMOTION:
		event = MouseMotionEvent(evt);
		if (auto* window = SDL_GL_GetCurrentWindow(); SDL_GetWindowGrab(window)) {
			int w, h;
			SDL_GetWindowSize(window, &w, &h);
			int x, y;
			SDL_GetMouseState(&x, &y);
			// There seems to be a bug in Windows in which the mouse can be locked on the right edge
			// only when moving fast to the right (not so fast actually) when grabbing is enabled
			// which stops generating mouse events
			// When moving to the left, events resume, and then moving even slower to the right fixes it
			// This only occurs when grabbing is explicitly enabled in windowed mode,
			// not in fullscreen mode (though not sure what happens with multiple monitors)
			// To reduce the impact of this bug, long range warping (e.g. to the middle of the window)
			// was attempted but that caused race conditions with fading in of gui elements
			// So, in the end it was decided that to go for the least kind of trouble
			// The value of 10 below is a heuristic value which seems to balance all factors
			// such as font size and the overall size of gui elements
			// and the speed of crossing virtual barriers
			static constexpr int MARGIN = 10;
			int xn = std::clamp(x, MARGIN, w - 1 - MARGIN);
			int yn = std::clamp(y, MARGIN, h - 1 - MARGIN);
			if (xn != x || yn != y) SDL_WarpMouseInWindow(window, xn, yn);
		}
		break;
	case SDL_JOYBUTTONUP:
		if (joystickManager.translateSdlInstanceId(evt)) {
			event = JoystickButtonUpEvent(evt);
			triggerOsdControlEventsFromJoystickButtonEvent(
				evt.jbutton.button, false);
		}
		break;
	case SDL_JOYBUTTONDOWN:
		if (joystickManager.translateSdlInstanceId(evt)) {
			event = JoystickButtonDownEvent(evt);
			triggerOsdControlEventsFromJoystickButtonEvent(
				evt.jbutton.button, true);
		}
		break;
	case SDL_JOYAXISMOTION: {
		if (auto joyId = joystickManager.translateSdlInstanceId(evt)) {
			const auto* setting = joystickManager.getJoyDeadZoneSetting(*joyId);
			assert(setting);
			int deadZone = setting->getInt();
			int threshold = (deadZone * 32768) / 100;
			auto value = (evt.jaxis.value < -threshold) ? evt.jaxis.value
				: (evt.jaxis.value >  threshold) ? evt.jaxis.value
								: 0;
			event = JoystickAxisMotionEvent(evt);
			triggerOsdControlEventsFromJoystickAxisMotion(
				evt.jaxis.axis, value);
		}
		break;
	}
	case SDL_JOYHATMOTION:
		if (auto joyId = joystickManager.translateSdlInstanceId(evt)) {
			event = JoystickHatEvent(evt);
			triggerOsdControlEventsFromJoystickHat(evt.jhat.value);
		}
		break;

	case SDL_JOYDEVICEADDED:
		joystickManager.add(evt.jdevice.which);
		break;

	case SDL_JOYDEVICEREMOVED:
		joystickManager.remove(evt.jdevice.which);
		break;

	case SDL_TEXTINPUT:
		splitText(evt.text.timestamp, evt.text.text);
		event = TextEvent(evt);
		break;

	case SDL_WINDOWEVENT:
		switch (evt.window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			if (WindowEvent::isMainWindow(evt.window.windowID)) {
				event = quitOnce();
				break;
			}
			[[fallthrough]];
		default:
			event = WindowEvent(evt);
			break;
		}
		break;

	case SDL_DROPFILE:
		event = FileDropEvent(
			FileOperations::getConventionalPath(evt.drop.file));
		SDL_free(evt.drop.file);
		break;

	case SDL_QUIT:
		event = quitOnce();
		break;

    case SDL_TEXTEDITING:
        // Android IME composition — ignore for MSX keyboard
        return;

    default:
		break;
	}

#if 0
	if (event) {
		std::cerr << "SDL event converted to: " << toString(event) << '\n';
	} else {
		std::cerr << "SDL event was of unknown type, not converted to an openMSX event\n";
	}
#endif

	if (event) eventDistributor.distributeEvent(std::move(*event));
}


void InputEventGenerator::updateGrab(bool grab)
{
	escapeGrabState = ESCAPE_GRAB_WAIT_CMD;
	setGrabInput(grab);
}

bool InputEventGenerator::signalEvent(const Event& event)
{
	std::visit(overloaded{
		[&](const WindowEvent& e) {
			const auto& evt = e.getSdlWindowEvent();
			if (e.isMainWindow() &&
			    evt.event == one_of(SDL_WINDOWEVENT_FOCUS_GAINED, SDL_WINDOWEVENT_FOCUS_LOST)) {
				switch (escapeGrabState) {
					case ESCAPE_GRAB_WAIT_CMD:
						// nothing
						break;
					case ESCAPE_GRAB_WAIT_LOST:
						if (evt.event == SDL_WINDOWEVENT_FOCUS_LOST) {
							escapeGrabState = ESCAPE_GRAB_WAIT_GAIN;
						}
						break;
					case ESCAPE_GRAB_WAIT_GAIN:
						if (evt.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
							escapeGrabState = ESCAPE_GRAB_WAIT_CMD;
						}
						setGrabInput(true);
						break;
					default:
						UNREACHABLE;
				}
			}
		},
		[](const EventBase&) {
			// correct but causes excessive clang compile-time
			// UNREACHABLE;
		}
	}, event);
	return false;
}

void InputEventGenerator::setGrabInput(bool grab) const
{
	SDL_SetWindowGrab(SDL_GL_GetCurrentWindow(), grab ? SDL_TRUE : SDL_FALSE);
}


// class EscapeGrabCmd

InputEventGenerator::EscapeGrabCmd::EscapeGrabCmd(
		CommandController& commandController_)
	: Command(commandController_, "escape_grab")
{
}

void InputEventGenerator::EscapeGrabCmd::execute(
	std::span<const TclObject> /*tokens*/, TclObject& /*result*/)
{
	auto& inputEventGenerator = OUTER(InputEventGenerator, escapeGrabCmd);
	if (inputEventGenerator.grabInput.getBoolean()) {
		inputEventGenerator.escapeGrabState =
			InputEventGenerator::ESCAPE_GRAB_WAIT_LOST;
		inputEventGenerator.setGrabInput(false);
	}
}

std::string InputEventGenerator::EscapeGrabCmd::help(
	std::span<const TclObject> /*tokens*/) const
{
	return "Temporarily release input grab.";
}

} // namespace openmsx
