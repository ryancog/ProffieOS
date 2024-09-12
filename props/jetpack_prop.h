// Revision 9 created by OlivierFlying747-8 (with a lot of help from ChatGPT)
// Revision 10, 11, 12 ,13, 14, 15, 16, 17 is only a variation on the LEDs code (copied at the bottom). None compile!
/*
Explanation:

    Button Handling:
        First Power Button Press:
            Plays the jetpack idle starting sound.
            Loops the idle sound until 1 minute passes, the jetpack is started, or
            the aux button is pressed (plays a false start sound and restarts the 1-minute idle loop).
            If no "Second Power Button Press" within 1 minute, the jetpack will shutdown.

            The maximum idle time can be customised in your config with:
            #define JETPACK_IDLE_TIME 1000 * 60 * 1 // Jetpack max idle time (default 1 min) before shutdown

        Second Power Button Press:
            Plays the jetpack starting sound.
            Loops the jetpack running sound until the power button is pressed again.

        Third Power Button Press:
            Plays the jetpack stopping sound.
            Loops the idle sound until 1 minute passes, the jetpack is restarted, or the Aux button is pressed
            (restarts the 1-minute idle loop).

        Aux Button Presses:
            When running: Plays a stuttering sound.
            When idle: Plays a false start sound and restarts the 1-minute idle loop.
            When off: Plays a self-destruct sound and stops the jetpack.

        The jetpack turns off automatically if idle for more than 1 minute.

    Loop Function:
        Monitors the 1-minute (customisable with #define JETPACK_IDLE_TIME) timer during idle mode to
        turn off the jetpack completely if the time expires.

*/

#ifndef PROPS_JETPACK_PROP_H
#define PROPS_JETPACK_PROP_H

#include "prop_base.h"
#include "../sound/sound_library.h"

#define PROP_TYPE Jetpack

#ifndef JETPACK_IDLE_TIME
#define JETPACK_IDLE_TIME 1000 * 60 * 1 //Jetpack max idle time in millisec (default 1 min) before shutdown
#endif

EFFECT(startidlemode);  // jetpack starting at idle
EFFECT(idle);           // jetpack running at idle
EFFECT(falsestart);     // jetpack false start sound
EFFECT(startjetpack);   // jetpack starting sound
EFFECT(running);        // jetpack running sound
EFFECT(stuttering);     // jetpack stuttering sound
EFFECT(shutdown);       // jetpack stopping sound
EFFECT(selfdestruct);   // jetpack exploding sound

class Jetpack : public PROP_INHERIT_PREFIX PropBase {
public:
    Jetpack() : PropBase(), jetpack_on_(false), idle_(false), timer_(0) {}
    const char* name() override { return "Jetpack"; }

    // Event handling for button presses
    bool Event(enum BUTTON button, EVENT event) override { // This is the correct syntax for Proffie OS, do not change.
        if (event != EVENT_PRESSED) return false;                      //note for ChatGPT who kept changing (enum BOTTON button, EVENT event)

        if (button == BUTTON_POWER) {
            if (jetpack_on_ && idle_) StartJetpack();
            else StartIdleMode();
            return true;
        } 
        if (button == BUTTON_AUX) {
            if (jetpack_on_) {
                if (!hybrid_font.PlayPolyphonic(&SFX_stuttering)) beep();
            } else if (idle_) {
                if (!hybrid_font.PlayPolyphonic(&SFX_falsestart)) beep();
                StartIdleLoop();
            } else {
                if (!hybrid_font.PlayPolyphonic(&SFX_selfdestruct)) beep();
            }
            return true;
        }
    }

    // Overriding Event2 to resolve ambiguity
    bool Event2(enum BUTTON button, EVENT event, uint32_t /* modifiers */) override { // This is the correct syntax for Proffie OS, do not change.
        return Event(button, event);  // Delegate to main Event handler
    }  // Does something need to go in here ???

    // Transition to Running Mode
    void StartJetpack() {
        jetpack_on_ = true;
        idle_ = false;
        if (!hybrid_font.PlayPolyphonic(&SFX_startjetpack)) beep();
        if (!hybrid_font.PlayPolyphonic(&SFX_running)) beep();
        Serial.println("Jetpack ON and Running");
    }

    // Transition to Idle Mode
    void StartIdleMode() {
        jetpack_on_ = false;
        idle_ = true;
        timer_ = millis();  // Start the idle timer
        if (!hybrid_font.PlayPolyphonic(&SFX_startidlemode)) beep();
        StartIdleLoop();
        Serial.println("Jetpack OFF, but Idling");
    }

    // Stop Idle Mode (Jetpack completely off)
    void StopIdleMode() {
        idle_ = false;
        Serial.println("Jetpack completely OFF");
    }

    // Loop for Idle Mode
    void StartIdleLoop() {
        if (!hybrid_font.PlayPolyphonic(&SFX_idle)) beep();
    }

    // Main loop to handle idle timer
    void Loop() override {
        if (idle_ && (millis() - timer_ > JETPACK_IDLE_TIME)) {  // 1 minute in milliseconds
            StopIdleMode();  // Stop jetpack if idle for more than defined time
        }
    }

  void DoMotion(const Vec3&, bool) override { }     //this is from Detonator.h

private:
    void beep() {
        beeper.Beep(0.05, 2000);
        beeper.Silence(0.05);
        beeper.Beep(0.05, 2000);
    }

    bool jetpack_on_;
    bool idle_;
    unsigned long timer_;
};

#endif // PROPS_JETPACK_PROP_H

