#include "daisy_seed.h"
#include "daisysp.h"
#include "funbox.h"

//
// Template for creating a pedal on the GuitarML Funbox v3 / Daisy Seed platform.
// Start from here to fill out your effect processing and controls.
//
// Uses the self-contained funbox::Hardware class (no libDaisy patching required).
// Provides Stereo In/Out, 6 knobs, 3 3-way switches, 4 dip switches,
// 2 SPST footswitches, 2 LEDs, and (on v3) an expression pedal input.
//

using namespace daisy;
using namespace daisysp;
using namespace funbox;

// Funbox hardware object (wraps the Daisy Seed)
Hardware hw;

Parameter param1, param2, param3, param4, param5, param6;

bool bypass;

// Cached switch states: [0] = left/up position, [1] = right/down position
bool pswitch1[2], pswitch2[2], pswitch3[2], pdip[4];

// Index lookups into hw.switches for each 3-way switch and the dip switches
const int switch1[2] = {SWITCH_1_LEFT, SWITCH_1_RIGHT};
const int switch2[2] = {SWITCH_2_LEFT, SWITCH_2_RIGHT};
const int switch3[2] = {SWITCH_3_LEFT, SWITCH_3_RIGHT};
const int dip[4]     = {SWITCH_DIP_1, SWITCH_DIP_2, SWITCH_DIP_3, SWITCH_DIP_4};

void updateSwitch1() // left / center / right
{
    if(pswitch1[0]) { // left
    }
    else if(pswitch1[1]) { // right
    }
    else { // center
    }
}

void updateSwitch2() // left / center / right
{
    if(pswitch2[0]) { // left
    }
    else if(pswitch2[1]) { // right
    }
    else { // center
    }
}

void updateSwitch3() // left / center / right
{
    if(pswitch3[0]) { // left
    }
    else if(pswitch3[1]) { // right
    }
    else { // center
    }
}

void UpdateButtons()
{
    // Toggle bypass and LED 1 when the left footswitch is released
    if(hw.GetButton(FOOTSWITCH_1).FallingEdge())
    {
        bypass = !bypass;
        hw.SetLed(LED_1, bypass ? 0.0f : 1.0f);
    }

    hw.UpdateLeds();
}

void UpdateSwitches()
{
    // Detect changes in the 3 On-Off-On switches and the dip switches

    // 3-way Switch 1
    bool changed1 = false;
    for(int i = 0; i < 2; i++)
    {
        if(hw.GetButton(switch1[i]).Pressed() != pswitch1[i])
        {
            pswitch1[i] = hw.GetButton(switch1[i]).Pressed();
            changed1    = true;
        }
    }
    if(changed1)
        updateSwitch1();

    // 3-way Switch 2
    bool changed2 = false;
    for(int i = 0; i < 2; i++)
    {
        if(hw.GetButton(switch2[i]).Pressed() != pswitch2[i])
        {
            pswitch2[i] = hw.GetButton(switch2[i]).Pressed();
            changed2    = true;
        }
    }
    if(changed2)
        updateSwitch2();

    // 3-way Switch 3
    bool changed3 = false;
    for(int i = 0; i < 2; i++)
    {
        if(hw.GetButton(switch3[i]).Pressed() != pswitch3[i])
        {
            pswitch3[i] = hw.GetButton(switch3[i]).Pressed();
            changed3    = true;
        }
    }
    if(changed3)
        updateSwitch3();

    // Dip switches
    for(int i = 0; i < 4; i++)
    {
        if(hw.GetButton(dip[i]).Pressed() != pdip[i])
        {
            pdip[i] = hw.GetButton(dip[i]).Pressed();
            // Action for dip switches handled in the audio callback
        }
    }
}

// Runs at a fixed rate to prepare audio samples
static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    hw.ProcessAllControls();

    UpdateButtons();
    UpdateSwitches();

    float vparam1 = param1.Process();
    float vparam2 = param2.Process();
    float vparam3 = param3.Process();
    float vparam4 = param4.Process();
    float vparam5 = param5.Process();
    float vparam6 = param6.Process();

    // Handle knob changes here
    (void)vparam1;
    (void)vparam2;
    (void)vparam3;
    (void)vparam4;
    (void)vparam5;
    (void)vparam6;

    for(size_t i = 0; i < size; i++)
    {
        if(bypass)
        {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
        else
        {
            float inL = in[0][i];
            float inR = in[1][i];

            // Process your signal here
            if(pdip[0] == false) // Mono
            {
                out[0][i] = inL;
                out[1][i] = inL;
            }
            else // Stereo
            {
                out[0][i] = inL;
                out[1][i] = inR;
            }
        }
    }
}

int main(void)
{
    hw.Init();

    float samplerate = hw.AudioSampleRate();

    param1.Init(hw.controls[KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
    param2.Init(hw.controls[KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
    param3.Init(hw.controls[KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
    param4.Init(hw.controls[KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
    param5.Init(hw.controls[KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
    param6.Init(hw.controls[KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

    bypass = true;
    hw.SetLed(LED_1, 0.0f);
    hw.SetLed(LED_2, 0.0f);
    hw.UpdateLeds();

    (void)samplerate;

    hw.StartAudio(AudioCallback);

    while(1) {}
}
