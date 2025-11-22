// TODO: Include the appropriate daisy board header file for your platform
#include "daisy_pod.h"
#include "q/pitch/pitch_detector.hpp"
#include "q/support/literals.hpp"

using namespace daisy;
using namespace cycfi;
using namespace q::literals;

DaisyPod hw;

const size_t sample_rate = 32000;
const auto sr = daisy::SaiHandle::Config::SampleRate::SAI_32KHZ;
const q::frequency min_freq = q::frequency{50.0f};
const q::frequency max_freq = q::frequency{3000.0f};
const auto hysteresis = -45_dB;
q::pitch_detector pd(min_freq, max_freq, sample_rate, hysteresis);


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    hw.ProcessAllControls();

    for(size_t i = 0; i < size; ++i ) {
        float input_sample = in[0][i]; 
        pd(input_sample);
    }
    float pitch = pd.get_frequency();
    hw.seed.PrintLine("Detected pitch: " + std::to_string(pitch) + " Hz");
}

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(16); // number of samples handled per callback
    hw.SetAudioSampleRate(sr);
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    /*
    bool led_state = false;

    while (1) {
        hw.DelayMs(500);
        led_state = !led_state;
        hw.seed.SetLed(led_state);
    }
    */
}
