// TODO: Include the appropriate daisy board header file for your platform
#include "daisy_pod.h"
#include "q/pitch/pitch_detector.hpp"
#include "q/support/literals.hpp"
#include "daisysp.h"

using namespace daisy;
using namespace cycfi;
using namespace q::literals;

DaisyPod hw;

const size_t sample_rate = 48000;
const auto sr = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
const q::frequency min_freq = q::frequency{50.0f};
const q::frequency max_freq = q::frequency{3000.0f};
const auto hysteresis = -45_dB;
q::pitch_detector pd(min_freq, max_freq, sample_rate, hysteresis);

daisysp::Oscillator osc;

void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size) {
    hw.ProcessAllControls();
    float f = hw.GetKnobValue(DaisyPod::KNOB_1);
    //osc.SetFreq(50.0f + f * 2950.0f); 

    for(size_t i = 0; i < size; i += 2)
    {
        float smp = osc.Process();
        out[i] = smp;
        out[i+1] = smp;
    }
    /*
    for(size_t i = 0; i < size; ++i ) {
        float input_sample = in[0][i]; 
        pd(input_sample);
    }
    float pitch = pd.get_frequency();
    hw.seed.PrintLine("Detected Pitch : %f Hz", pitch);
    */
}

int main(void) {
    osc.Init(static_cast<float>(sample_rate));
    osc.SetWaveform(daisysp::Oscillator::WAVE_SAW);
    osc.SetFreq(440.0f);
    osc.SetAmp(0.5f);

    hw.Init();
    //hw.seed.StartLog();
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(sr);
    //hw.StartAdc();
    //hw.StartAudio(AudioCallback);

    bool led_state = false;

    while (1) {
        hw.DelayMs(100);
        led_state = !led_state;
        hw.seed.SetLed(led_state);
        float v = (led_state) ? 1.0f : 0.0f;
        hw.led1.Set(v,v,v);
        hw.led2.Set(v,v,v);
        hw.UpdateLeds();
    }

    return 0;
}
