/** Funbox Hardware Support File
 *
 *  Self-contained hardware support for the GuitarML Funbox (v3) guitar pedal,
 *  built on the Daisy Seed. Modeled after the Aurora hardware support header
 *  (see aurora.h): a single `funbox::Hardware` object wraps the Daisy Seed and
 *  configures audio, knobs, expression, switches, LEDs and MIDI directly.
 *
 *  Unlike the previous approach, this header does NOT require patching libDaisy
 *  (the modified daisy_petal in mod/ is no longer needed). Create and Init a
 *  Hardware object at the start of main() in place of a DaisyPetal/DaisySeed.
 *
 *  Target hardware: Funbox v3 (expression on D15, MIDI in D30 / out D29,
 *  dip switches 1-4). v1/v2 boards simply leave the extra controls unused.
 */
#pragma once
#ifndef FUNBOX_HW_H
#define FUNBOX_HW_H

#include "daisy_seed.h"

namespace funbox
{
/** @brief indexed accessors for the momentary/toggle/dip switches
 *  Example usage:
 *  bool pressed = hw.GetButton(FOOTSWITCH_1).Pressed();
 */
enum Sw
{
    FOOTSWITCH_1,   /**< Left footswitch */
    FOOTSWITCH_2,   /**< Right footswitch */
    SWITCH_1_LEFT,  /**< 3-way toggle 1, up/left position */
    SWITCH_1_RIGHT, /**< 3-way toggle 1, down/right position */
    SWITCH_2_LEFT,  /**< 3-way toggle 2, up/left position */
    SWITCH_2_RIGHT, /**< 3-way toggle 2, down/right position */
    SWITCH_3_LEFT,  /**< 3-way toggle 3, up/left position */
    SWITCH_3_RIGHT, /**< 3-way toggle 3, down/right position */
    SWITCH_DIP_1,   /**< Dip switch 1 */
    SWITCH_DIP_2,   /**< Dip switch 2 */
    SWITCH_DIP_3,   /**< Dip switch 3 (v2/v3 only) */
    SWITCH_DIP_4,   /**< Dip switch 4 (v2/v3 only) */
    SW_LAST,
};

/** @brief indexed accessors for knob controls
 *  Example usage:
 *  float val = hw.GetKnobValue(KNOB_1);
 */
enum Knob
{
    KNOB_1,
    KNOB_2,
    KNOB_3,
    KNOB_4,
    KNOB_5,
    KNOB_6,
    KNOB_LAST,
};

/** @brief indexed accessors for the two footswitch LEDs
 *  Example usage:
 *  hw.SetLed(LED_1, 1.0f);
 */
enum Led
{
    LED_1,
    LED_2,
    LED_LAST,
};

/** @brief Hardware support class for the GuitarML Funbox (v3)
 *  Create and Init this object at the beginning of main() before running
 *  anything else. It takes the place of the core DaisySeed object.
 */
class Hardware
{
  public:
    /** @brief Empty Constructor. Call `Init` from main to initialize */
    Hardware() {}

    /** @brief Empty Destructor. This object should span the life of the program */
    ~Hardware() {}

    /** @brief Initialize the hardware.
     *  Call this at the start of main().
     *
     *  @param boost true runs the processor at the maximum 480MHz,
     *               false runs at 400MHz. Defaults to false.
     */
    void Init(bool boost = false)
    {
        seed.Init(boost);
        ConfigureAudio();
        ConfigureControls();
        ConfigureLeds();
        seed.adc.Start();
        UpdateHidRates();
    }

    /** @brief Starts a specified non-interleaved audio callback
     *         Data is non-interleaved
     *         (i.e. {{L0, L1, ... , LN},{R0, R1, ... , RN}})
     */
    void StartAudio(daisy::AudioHandle::AudioCallback cb)
    {
        current_cb_ = cb;
        seed.StartAudio(cb);
    }

    /** @brief Starts a specified interleaving audio callback */
    void StartAudio(daisy::AudioHandle::InterleavingAudioCallback cb)
    {
        seed.StartAudio(cb);
    }

    /** @brief Changes current callback to a new non-interleaved callback */
    void ChangeAudioCallback(daisy::AudioHandle::AudioCallback cb)
    {
        current_cb_ = cb;
        seed.ChangeAudioCallback(cb);
    }

    /** @brief Changes current callback to a new interleaved callback */
    void ChangeAudioCallback(daisy::AudioHandle::InterleavingAudioCallback cb)
    {
        seed.ChangeAudioCallback(cb);
    }

    /** @brief Stops Audio */
    void StopAudio() { seed.StopAudio(); }

    /** @brief sets the audio sample rate. Audio must be stopped for this to work properly */
    void SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate samplerate)
    {
        seed.SetAudioSampleRate(samplerate);
        UpdateHidRates();
    }

    /** @brief returns the sample rate in Hz of the audio engine */
    float AudioSampleRate() { return seed.AudioSampleRate(); }

    /** @brief sets the number of samples to process in each audio callback */
    void SetAudioBlockSize(size_t blocksize)
    {
        seed.SetAudioBlockSize(blocksize);
        UpdateHidRates();
    }

    /** @brief returns the number of samples to process in each audio callback */
    size_t AudioBlockSize() { return seed.AudioBlockSize(); }

    /** @brief returns the rate in Hz that the audio callback gets called */
    float AudioCallbackRate() const { return seed.AudioCallbackRate(); }

    /** @brief filters and debounces all controls.
     *  This should be run once per audio callback.
     */
    void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** @brief filters all analog controls (knobs and expression).
     *  This is called from ProcessAllControls, and should be run once per audio callback.
     */
    void ProcessAnalogControls()
    {
        for(int i = 0; i < KNOB_LAST; i++)
        {
            controls[i].Process();
        }
        expression.Process();
    }

    /** @brief filters and debounces digital controls (switches).
     *  This is called from ProcessAllControls, and should be run once per audio callback.
     */
    void ProcessDigitalControls()
    {
        for(int i = 0; i < SW_LAST; i++)
        {
            switches[i].Debounce();
        }
    }

    /** @brief returns a 0-1 value for the given knob control
     *  @param ctrl knob index to read from (one of the Knob enum, e.g. KNOB_1)
     */
    inline float GetKnobValue(int ctrl) const { return controls[ctrl].Value(); }

    /** @brief returns a 0-1 value for the expression pedal input (v3) */
    inline float GetExpression() const { return expression.Value(); }

    /** @brief returns a reference to a given switch
     *         Example Usage:
     *         bool state = hw.GetButton(FOOTSWITCH_1).Pressed();
     *  @param idx one of the Sw enum values
     */
    inline const daisy::Switch &GetButton(int idx) const
    {
        return switches[idx];
    }

    /** @brief Sets the brightness of one of the footswitch LEDs
     *  @param idx LED index (LED_1 or LED_2)
     *  @param bright 0-1 brightness value
     */
    void SetLed(int idx, float bright) { leds[idx].Set(bright); }

    /** @brief Sets all LEDs to off */
    void ClearLeds()
    {
        for(int i = 0; i < LED_LAST; i++)
        {
            leds[i].Set(0.0f);
        }
    }

    /** @brief Writes the current LED values out to the hardware.
     *  Call this once per loop/callback after setting LED values.
     */
    void UpdateLeds()
    {
        for(int i = 0; i < LED_LAST; i++)
        {
            leds[i].Update();
        }
    }

    /** @brief Initializes the MIDI UART (in on D30, out on D29).
     *  Call this from main() only if MIDI is used.
     */
    void InitMidi()
    {
        daisy::MidiUartHandler::Config midi_config;
        midi_config.transport_config.rx = daisy::seed::D30; // Funbox v2/v3
        midi_config.transport_config.tx = daisy::seed::D29; // Funbox v2 only
        midi.Init(midi_config);
    }

    /** @brief sets the state of the LED on the Daisy Seed itself */
    void SetTestLed(bool state) { seed.SetLed(state); }

    /** @brief delay function; same as System::Delay(). Do not call from any
     *         interrupt callbacks (LowPriorityCallback/AudioCallback).
     *  @param del number of milliseconds to delay
     */
    void DelayMs(size_t del) { seed.DelayMs(del); }

    /** @brief Update HID sample rates for new callback rate when samplerate/blocksize change */
    void UpdateHidRates()
    {
        for(int i = 0; i < KNOB_LAST; i++)
        {
            controls[i].SetSampleRate(AudioCallbackRate());
        }
        expression.SetSampleRate(AudioCallbackRate());
        for(int i = 0; i < SW_LAST; i++)
        {
            switches[i].SetUpdateRate(AudioCallbackRate());
        }
        for(int i = 0; i < LED_LAST; i++)
        {
            leds[i].SetSampleRate(AudioCallbackRate());
        }
    }

    /** Array of knob controls */
    daisy::AnalogControl controls[KNOB_LAST];

    /** Expression pedal input (v3) */
    daisy::AnalogControl expression;

    /** Array of switches (footswitches, toggles, dips) */
    daisy::Switch switches[SW_LAST];

    /** Array of footswitch LEDs */
    daisy::Led leds[LED_LAST];

    /** MIDI UART handle */
    daisy::MidiUartHandler midi;

    /** Daisy Seed base object */
    daisy::DaisySeed seed;

  private:
    /** @brief tracking current callback for recovery after samplerate change */
    daisy::AudioHandle::AudioCallback current_cb_;

    /** @brief Configure the audio engine (uses the Daisy Seed's onboard codec) */
    void ConfigureAudio() { seed.SetAudioBlockSize(48); }

    /** @brief Configure knobs (6), expression (1) and switches (12) */
    void ConfigureControls()
    {
        // ===== ADC: 6 knobs (D16-D21) + expression (D15) =====
        daisy::AdcChannelConfig cfg[KNOB_LAST + 1];
        cfg[KNOB_1].InitSingle(daisy::seed::D16);
        cfg[KNOB_2].InitSingle(daisy::seed::D17);
        cfg[KNOB_3].InitSingle(daisy::seed::D18);
        cfg[KNOB_4].InitSingle(daisy::seed::D19);
        cfg[KNOB_5].InitSingle(daisy::seed::D20);
        cfg[KNOB_6].InitSingle(daisy::seed::D21);
        cfg[KNOB_LAST].InitSingle(daisy::seed::D15); // expression (v3)
        seed.adc.Init(cfg, KNOB_LAST + 1);

        for(int i = 0; i < KNOB_LAST; i++)
        {
            controls[i].Init(seed.adc.GetPtr(i), AudioCallbackRate());
        }
        expression.Init(seed.adc.GetPtr(KNOB_LAST), AudioCallbackRate());

        // ===== switches =====
        daisy::Pin sw_pins[SW_LAST] = {
            daisy::seed::D25, // FOOTSWITCH_1
            daisy::seed::D26, // FOOTSWITCH_2
            daisy::seed::D14, // SWITCH_1_LEFT
            daisy::seed::D13, // SWITCH_1_RIGHT
            daisy::seed::D7,  // SWITCH_2_LEFT
            daisy::seed::D10, // SWITCH_2_RIGHT
            daisy::seed::D2,  // SWITCH_3_LEFT
            daisy::seed::D4,  // SWITCH_3_RIGHT
            daisy::seed::D1,  // SWITCH_DIP_1
            daisy::seed::D3,  // SWITCH_DIP_2
            daisy::seed::D5,  // SWITCH_DIP_3 (v2/v3)
            daisy::seed::D6,  // SWITCH_DIP_4 (v2/v3)
        };
        for(int i = 0; i < SW_LAST; i++)
        {
            switches[i].Init(sw_pins[i], AudioCallbackRate());
        }
    }

    /** @brief Configure the two footswitch LEDs (GPIO PWM on D22/D23) */
    void ConfigureLeds()
    {
        leds[LED_1].Init(daisy::seed::D22, false);
        leds[LED_2].Init(daisy::seed::D23, false);
        ClearLeds();
        UpdateLeds();
    }
};

} // namespace funbox

#endif
