/*
MIT License

Copyright (c) 2024 Marek Mach (Bastl Instruments)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cstdint>
#include "pico/stdlib.h"
#include "common/core/App.h"
#include "common/core/Hardware.h"
#include "common/core/Kastle2.h"
#include "common/dsp/math/qmath.h"
#include "common/controls/FancyPot.h"
#include "common/dsp/effects/StereoDelay.h"
#include "common/dsp/effects/SoftClipper.h"
#include "common/dsp/utility/Quantizer.h"


namespace kastle2
{

/**
 * @class Glazmo
 * @ingroup apps
 * @brief Simple template for creating new apps.
 * @author Majkel (Bastl Instruments, GLAZMO.RESEARCH)
 * @date 2024-11-14
 */
class Glazmo : public virtual App
{
public:
    /**
     * @brief Initializes all the parameters, memory, etc.
     */
    void Init();

    /**
     * @brief Deinitializes the app, stops all effects, etc.
     */
    void DeInit();

    /**
     * @brief Called each interrupt loop. Implements all the audio processing.
     * @param input Input buffer.
     * @param output Output buffer.
     * @param size Number of sample pairs in the buffer (real size of the buffer is 2*size).
     */
    FASTCODE void AudioLoop(q15_t *input, q15_t *output, size_t size);

    /**
     * @brief Called each time AudioLoop isn't busy.
     */
    void UiLoop();

    /**
     * @brief Called when the app is first loaded - initializes the memory values.
     */
    void MemoryInitialization() {}

    FASTCODE void SecondCoreWorker();

    /**
     * @brief Handles incoming MIDI messages.
     * @param msg The MIDI message to handle.
     */
    void MidiCallback(midi::Message *msg);

    q15_t TransientShape(int pot, q15_t x);

    /**
     * @brief Returns the app ID.
     * @return The app ID.
     */
    uint8_t GetId()
    {
        return Kastle2::kDefaultAppId;
    }

private:
    enum class Pot
    {
        HARMONY,
        SCRUB,
        PITCH,
        FX,
        LENGTH,
        DENSITY,
        SCRUB_MOD,
        PITCH_MOD,
        LENGTH_MOD,
        AUTOPLAY,
        COUNT           
    };
    bool inited_ = false;
    EnumArray<Pot, std::unique_ptr<FancyPot>> pots_;
        
    SamplePlayer16bit player_;

    StereoDelay density_[3];

    uint8_t sample_;

    size_t scrub_;
    float pitch_;
    size_t length_;

    bool autoplay_ = true;

    static constexpr Hardware::AnalogInput CV_SCRUB = Hardware::AnalogInput::PARAM_1;
    static constexpr Hardware::AnalogInput CV_LENGTH = Hardware::AnalogInput::PARAM_2;
    static constexpr Hardware::AnalogInput CV_GLAZMO = Hardware::AnalogInput::PARAM_3;
    static constexpr Hardware::AnalogInput CV_NOTE = Hardware::AnalogInput::PITCH_2;
    static constexpr Hardware::AnalogInput CV_FREE = Hardware::AnalogInput::PITCH_1;

    SoftClipper clipper_;
    Quantizer qnt_;
    q15_t* output_buffer_; 
    size_t buffer_size_;

    size_t second_core_processed_samples_ = 0;

    FASTCODE void SecondCoreProcess(size_t index);
};
}
