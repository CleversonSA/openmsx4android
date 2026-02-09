#include "EventDelay.hh"

#include "Event.hh"
#include "EventDistributor.hh"
#include "MSXEventDistributor.hh"
#include "MSXException.hh"
#include "ReverseManager.hh"
#include "Timer.hh"

#include "narrow.hh"
#include "one_of.hh"
#include "stl.hh"

#include <SDL.h>

#include <algorithm>
#include <cassert>

namespace openmsx {

EventDelay::EventDelay(Scheduler& scheduler_,
                       CommandController& commandController,
                       EventDistributor& eventDistributor_,
                       MSXEventDistributor& msxEventDistributor_,
                       ReverseManager& reverseManager)
	: Schedulable(scheduler_)
	, eventDistributor(eventDistributor_)
	, msxEventDistributor(msxEventDistributor_)
	, prevReal(Timer::getTime())
	, delaySetting(
		commandController, "inputdelay",
		"delay input to avoid key-skips", 0.0, 0.0, 10.0)
{
	using enum EventType;
	for (auto type : {KEY_DOWN, KEY_UP,
	                  MOUSE_MOTION, MOUSE_BUTTON_DOWN, MOUSE_BUTTON_UP,
	                  JOY_AXIS_MOTION, JOY_HAT, JOY_BUTTON_DOWN, JOY_BUTTON_UP}) {
		eventDistributor.registerEventListener(type, *this, EventDistributor::Priority::MSX);
	}

	reverseManager.registerEventDelay(*this);
}

EventDelay::~EventDelay()
{
	using enum EventType;
	for (auto type : {JOY_BUTTON_UP, JOY_BUTTON_DOWN, JOY_HAT, JOY_AXIS_MOTION,
	                  MOUSE_BUTTON_UP, MOUSE_BUTTON_DOWN, MOUSE_MOTION,
	                  KEY_UP, KEY_DOWN}) {
		eventDistributor.unregisterEventListener(type, *this);
	}
}

bool EventDelay::signalEvent(const Event& event)
{
	toBeScheduledEvents.push_back(event);
	if (delaySetting.getDouble() == 0.0) {
		sync(getCurrentTime());
	}
	return false;
}

void EventDelay::sync(EmuTime curEmu)
{
	auto curRealTime = Timer::getTime();
	auto realDuration = curRealTime - prevReal;
	prevReal = curRealTime;
	auto emuDuration = curEmu - prevEmu;
	prevEmu = curEmu;

	double factor = emuDuration.toDouble() / narrow_cast<double>(realDuration);
	EmuDuration extraDelay = EmuDuration::sec(delaySetting.getDouble());

#if PLATFORM_ANDROID
	// The virtual keyboard on Android sends a key press and the
	// corresponding key release event directly after each other, without a
	// delay. It sends both events either when the user has finished a
	// short tap or alternatively after the user has hold the button
	// pressed for a few seconds and then has selected the appropriate
	// character from the multi-character-popup that the virtual keyboard
	// displays when the user holds a button pressed for a short while.
	// Either way, the key release event comes way too short after the
	// press event for the MSX to process it. The two events follow each
	// other within a few milliseconds at most. Therefore, on Android,
	// special logic must be foreseen to delay the release event for a
	// short while. This special logic correlates each key release event
	// with the corresponding press event for the same key. If they are
	// less then 2/50 second apart, the release event gets delayed until
	// the next sync call. The 2/50 second has been chosen because it can
	// take up to 2 vertical interrupts (2 screen refreshes) for the MSX to
	// see the key press in the keyboard matrix, thus, 2/50 seconds is the
	// minimum delay required for an MSX running in PAL mode.
	std::vector<Event> toBeRescheduledEvents;
#endif

	EmuTime time = curEmu + extraDelay;
	for (auto& e : toBeScheduledEvents) {
#if PLATFORM_ANDROID
        if (getType(e) == one_of(EventType::KEY_DOWN, EventType::KEY_UP)) {

            const auto& keyEvent = get_event<KeyEvent>(e);
            const auto keyCode = keyEvent.getKeyCode();
            const auto unicode = keyEvent.getUnicode();

            // --- chave para correlacionar DOWN/UP ---
            // Normal: usa keyCode sem o scancode-mask.
            // Unicode-only (keyCode == SDLK_UNKNOWN): usa unicode (pra não colidir tudo em UNKNOWN).
            int maskedKeyCode;
            if (keyCode == SDLK_UNKNOWN && unicode != 0) {
                maskedKeyCode = int(unicode) | 0x40000000; // namespace separado
            } else {
                maskedKeyCode = int(keyCode) & ~int(SDLK_SCANCODE_MASK);
            }

            auto it = std::ranges::find(nonMatchedKeyPresses, maskedKeyCode,
                                        [](const auto& p) { return p.first; });

            if (getType(e) == EventType::KEY_DOWN) {
                // guarda/atualiza o último KEY_DOWN visto para esta tecla (chave)
                if (it == end(nonMatchedKeyPresses)) {
                    nonMatchedKeyPresses.emplace_back(maskedKeyCode, e);
                } else {
                    it->second = e;
                }
            } else {
                // KEY_UP: se temos o DOWN correspondente, mede intervalo em ms usando timestamp do SDL
                if (it != end(nonMatchedKeyPresses)) {

                    const auto& pressSdl  = get_event<SdlEvent>(it->second).getCommonSdlEvent().timestamp;
                    const auto& releaseSdl = get_event<SdlEvent>(e).getCommonSdlEvent().timestamp;

                    // timestamps são uint32 ms; diferença funciona mesmo com wrap (unsigned)
                    uint32_t deltaMs = uint32_t(releaseSdl - pressSdl);

                    // 2/50s ≈ 40ms (como no comentário original)
                    if (deltaMs <= 40) {
                        // Recria KEYUP com timestamp atualizado (SDL_GetTicks())
                        SDL_Event newUp = get_event<SdlEvent>(e).getSdlEvent(); // copia o SDL_Event original

                        // atualiza timestamp para "agora"
                        newUp.key.timestamp = SDL_GetTicks();

                        // (Opcional/Importante p/ unicode-only)
                        // preserva unicode no 'unused' (hack do openMSX)
                        newUp.key.keysym.unused = keyEvent.getUnicode();

                        newUp.type = SDL_KEYUP;
                        newUp.key.type = SDL_KEYUP;
                        newUp.key.state = SDL_RELEASED;
                        newUp.key.repeat = 0;

                        Event newKeyupEvent = KeyUpEvent(newUp);
                        toBeRescheduledEvents.push_back(std::move(newKeyupEvent));
                        continue;
                    }

                    // se não reagendou, remove o registro de DOWN
                    move_pop_back(nonMatchedKeyPresses, it);
                }
            }
        }
#endif
		scheduledEvents.push_back(e);
		const auto& sdlEvent = get_event<SdlEvent>(e);
		uint32_t eventSdlTime = sdlEvent.getCommonSdlEvent().timestamp;
		uint32_t sdlNow = SDL_GetTicks();
		auto sdlOffset = int32_t(sdlNow - eventSdlTime);
#if defined(__ANDROID__)
        // If from virtual keyboard, It was necessary plus some timestamp to avoid
        // char typing miss and repetitions at MSX.
        if (sdlOffset < 0) {
            sdlOffset = int32_t(eventSdlTime - sdlNow);
        }
#endif
		assert(sdlOffset >= 0);
		auto offset = 1000 * int64_t(sdlOffset); // ms -> us
		EmuDuration emuOffset = EmuDuration::sec(factor * narrow_cast<double>(offset));
		auto schedTime = (emuOffset < extraDelay)
		               ? time - emuOffset
		               : curEmu;
		assert(curEmu <= schedTime);
		setSyncPoint(schedTime);
	}
	toBeScheduledEvents.clear();

#if PLATFORM_ANDROID
	append(toBeScheduledEvents, std::move(toBeRescheduledEvents));
#endif
}

void EventDelay::executeUntil(EmuTime time)
{
	try {
		auto event = std::move(scheduledEvents.front());
		scheduledEvents.pop_front();
		msxEventDistributor.distributeEvent(event, time);
	} catch (MSXException&) {
		// ignore
	}
}

void EventDelay::flush()
{
	EmuTime time = getCurrentTime();

	for (const auto& e : scheduledEvents) {
		msxEventDistributor.distributeEvent(e, time);
	}
	scheduledEvents.clear();

	for (const auto& e : toBeScheduledEvents) {
		msxEventDistributor.distributeEvent(e, time);
	}
	toBeScheduledEvents.clear();

	removeSyncPoints();
}

} // namespace openmsx
