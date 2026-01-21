/*
MIT License

Copyright (c) 2024 Michal Kripac (Bastl Instruments)

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

#include "Glazmo.hpp"
#include "common/core/Kastle2.h"
#include "common/utils.h"
#include "common/controls/FancyPot.h"
#include <cmath>
#include <math.h>
#include "samples.h"
#include "ParameterMap.h"
#include "common/core/MultiCore.h"

using namespace kastle2;

void Glazmo::Init()
{
    inited_ = false;

    player_.Init(SAMPLE_RATE, SAMPLE_RATE);
    player_.SetHifi(true);
    player_.SetSample(samples[0], samples_lengths[0], SamplePlayer16bit::STEREO);
    sample_ = 0;

//    clipper_.Init(SAMPLE_RATE);

    qnt_.Init();
    qnt_.SetEnabled(true);
    qnt_.SetScale(Quantizer::DefaultScale::MINOR);

    for (int index = 0; index < 2; index++) {
        density_[index].kMaxDelay = 5000 * (index + 2);
        density_[index].Init(SAMPLE_RATE);
        density_[index].SetFeedback(0, 0);
        density_[index].SetWet(Q15_MAX);
    }

//    Kastle2::base.SetFeatureEnabled(Base::Feature::LFO_OUT, false);

    pots_[Pot::HARMONY] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_1,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_MIN,
//         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::SCRUB] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_2,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_MIN,
//         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::DENSITY] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_3,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_MIN,
    //         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::PITCH] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_4,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_HALF,
    //         .midi_cc = cc::TIME_MOD,
         .deadzone = true,
         });

    pots_[Pot::FX] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_5,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_MIN,
    //         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::LENGTH] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_6,
         .layer = Hardware::Layer::NORMAL,
         .initial_value = POT_HALF,
         .deadzone = true,
    //         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::SCRUB_MOD] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_2,
         .layer = Hardware::Layer::SHIFT,
         .initial_value = POT_HALF,
//         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::PITCH_MOD] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_4,
         .layer = Hardware::Layer::SHIFT,
         .initial_value = POT_HALF,
    //         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::LENGTH_MOD] = FancyPot::Create(
        {.pot = Hardware::Pot::POT_6,
         .layer = Hardware::Layer::SHIFT,
         .initial_value = POT_HALF,
    //         .midi_cc = cc::TIME_MOD,
         });

    pots_[Pot::AUTOPLAY] = FancyPot::Create({
        .pot = Hardware::Pot::POT_4,
        .layer = Hardware::Layer::SETTINGS
    });
 
    
    for (auto &pot : pots_)
    {
        pot->Init(AUDIO_LOOP_RATE);
    }

    inited_ = true;
}

void Glazmo::DeInit()
{
    inited_ = false;
}

auto led_ = Hardware::Led::LED_1;
bool prev_trig_ = false;

FASTCODE void Glazmo::AudioLoop(q15_t *input, q15_t *output, size_t size)
{
    if (!inited_)
    {
        return;
    }

    for (auto &pot : pots_)
    {
        pot->Process();
    }

    output_buffer_ = output;
    buffer_size_ = size;

    bool trigger = 
        Kastle2::hw.GetTriggerIn() 
        || (Kastle2::hw.GetRawButtonState(Hardware::Button::SHIFT)
            && Kastle2::hw.GetLayer() != Hardware::Layer::MODE);

    if (trigger && !prev_trig_) 
    {
        player_.Play();
        Kastle2::hw.SetLed(led_, 0x000000);
        led_ = led_ == Hardware::Led::LED_1 ? Hardware::Led::LED_2 : Hardware::Led::LED_1;
    }

    prev_trig_ = trigger;

    MultiCore::SendMessage(MultiCore::BEGIN);

    for (size_t index = 0; index < size; index++)
    {
        if (!autoplay_ && !Kastle2::hw.GetTriggerIn()) {
            MultiCore::SendMessage(MultiCore::SAMPLE_REQUEST, index);
            continue; 
        }

        player_.SetSpeed(pitch_);
        if (pots_[Pot::SCRUB]->HasMoved()) {
            player_.Reset();
            Kastle2::hw.SetLed(led_, 0x000000);
            player_.SetStart(map(scrub_, 0, POT_MAX, 0, player_.GetLength() - 1));
            led_ = led_ == Hardware::Led::LED_1 ? Hardware::Led::LED_2 : Hardware::Led::LED_1;
        }

        if (pots_[Pot::LENGTH]->HasMoved()) {
            player_.SetReverse(pots_[Pot::LENGTH]->GetValue() < POT_HALF);
            player_.SetLength(map(abs((int) length_ - (int) POT_HALF), 0, POT_HALF, 0, samples_lengths[sample_]));
            player_.SetStart(map(scrub_, 0, POT_MAX, 0, player_.GetLength() - 1));
        }

        if (player_.IsPlaying()) {
            player_.Process();
            output[2 * index] = player_.GetLeft();
            output[2 * index + 1] = player_.GetRight();
            Kastle2::hw.SetLed(led_, 0x00FF00);
        } else {
            player_.Play();
            Kastle2::hw.SetLed(led_, 0x000000);
            led_ = led_ == Hardware::Led::LED_1 ? Hardware::Led::LED_2 : Hardware::Led::LED_1;
        }

        MultiCore::SendMessage(MultiCore::SAMPLE_REQUEST, index);
    }

    MultiCore::WaitForMessage(MultiCore::DONE);
}


void Glazmo::UiLoop()
{
    for (auto &pot : pots_)
    {
        pot->ReadValue();
    }
    scrub_ = pots_[Pot::SCRUB]->GetValue() + apply_pot_mod_attenuvert(Kastle2::hw.GetAnalogValue(CV_SCRUB), pots_[Pot::SCRUB_MOD]->GetValue());
    length_ = pots_[Pot::LENGTH]->GetValue() + apply_pot_mod_attenuvert(Kastle2::hw.GetAnalogValue(CV_LENGTH), pots_[Pot::LENGTH_MOD]->GetValue());
    
    pitch_ = qnt_.ProcessMultiplier(
            curve_map(pots_[Pot::PITCH]->GetValue()
                + apply_pot_mod_attenuvert(Kastle2::hw.GetAnalogValue(CV_NOTE), pots_[Pot::PITCH_MOD]->GetValue()),
            kMapPitch)
        );

    if (Kastle2::hw.GetLayer() == Hardware::Layer::SETTINGS)
    {
        if (pots_[Pot::AUTOPLAY]->HasChanged())
        {
            autoplay_ = pots_[Pot::AUTOPLAY]->GetValue() > POT_HALF;
        }
    }
}

FASTCODE void Glazmo::SecondCoreWorker()
{
    while (inited_)
    {
        if (MultiCore::HasMessage())
        {

            MultiCore::Message m = MultiCore::GetMessage();
            switch (m.type)
            {
            case MultiCore::BEGIN:
                second_core_processed_samples_ = 0;
                break;
            case MultiCore::SAMPLE_REQUEST:
                SecondCoreProcess(m.data);
                second_core_processed_samples_++;
                if (second_core_processed_samples_ == buffer_size_)
                {
                    MultiCore::SendMessage(MultiCore::DONE);
                }
                break;
            }

        }
    }
}

FASTCODE void Glazmo::SecondCoreProcess(size_t index)
{
    auto pot = pots_[Pot::DENSITY]->GetValue();

    if (pot > POT_MIN + 10) {
        auto l = output_buffer_[2 * index];
        auto r = output_buffer_[2 * index + 1];
        for (int j = 0; j < 2; j++) {
            int v = map(pot, 0, POT_MAX, 0, map(player_.GetLength(), 0, samples_lengths[sample_], 0, 4057 + j * 8311));
            density_[j].SetDelay(v, v);
            density_[j].Process(l, r);
            output_buffer_[2 * index] = q15_add(output_buffer_[2 * index], density_[j].GetLeft());
            output_buffer_[2 * index + 1] = q15_add(output_buffer_[2 * index + 1], density_[j].GetRight());
        }
    }

    int fx = pots_[Pot::FX]->GetValue();
    output_buffer_[2 * index] = Glazmo::TransientShape(fx, output_buffer_[2 * index]);
    output_buffer_[2 * index + 1] = Glazmo::TransientShape(fx, output_buffer_[2 * index + 1]);
}

void Glazmo::MidiCallback(midi::Message *msg)
{
    // Do something here...
}

// since its called only once, we can get over the usage of floats
q15_t state_ = 0;
float tau_ = 1.5f * 0.001f;
q15_t alpha_ = (q15_t) ((1.0f - expf(-1.0f/(tau_ * SAMPLE_RATE))) * 32767.0f + 0.5f);

q15_t Glazmo::TransientShape(int pot, q15_t x) {
    q15_t amnt = map(pot, 0, POT_MAX, 0, 32767);
    q15_t diff = q15_sub(x, state_);
    q15_t delta = q15_mult(alpha_, diff);
    q15_t env = q15_add(state_, delta);
    state_ = env;
    q15_t tr  = q15_sub(x, env);   // "transient" = x - lowpassed(x)
    q15_t adj = q15_mult(amnt, tr);                // scale by knob
    return q15_add(x, adj);
}
