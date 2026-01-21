#ifndef GLAZMO_PARAMETER_MAP_H
#define GLAZMO_PARAMETER_MAP_H

#include <array>
#include "common/config.h"
#include "common/core/Hardware.h"
#include "common/dsp/math/Fraction.h"
#include "common/dsp/math/math_utils.h"
#include "common/dsp/math/qmath.h"
#include "common/dsp/utility/Quantizer.h"
#include "common/peripherals/WS2812.h"

namespace kastle2
{

// ---PITCH---
// @author Marek Mach
// Stolen from the WaveBard. I aint writing a lot
static constexpr auto kMapPitch = MapDef<float, 3>{
    {pot(0.0f), pot(0.5f), pot(1.0f)},
    {0.25f, 1.0f, 4.f}};

static constexpr auto kMapPitchOctaves = MapDef<float, 5>{
    {pot(0.1f), pot(0.3f), pot(0.5f), pot(0.7f), pot(0.9f)},
    {0.25f, 0.5f, 1.f, 2.f, 4.f}};

// pot 0.0 = means -1V/Oct, pot 0.5 means no modulation, 1.0 = means +1V/Oct
// by mapping the values 0.15 to 0.3 etc. we keep more range for smaller values
// by mapping 0.005 to 0.0 etc we make sure we safely reach the end values
static constexpr auto kMapPitchMod = MapDef<int32_t, 7>{
    {pot(0.0f), pot(0.005f), pot(0.15f), pot(0.5f), pot(0.85f), pot(0.995f), pot(1.0f)},
    {pot(0.0f), pot(0.0f), pot(0.3f), pot(0.5f), pot(0.7f), pot(1.0f), pot(1.0f)}};

// this must match the number of octaves defined by the map above (kMapPitchOctaves)
static constexpr size_t kNumberOfOctaves = 5;
static constexpr int32_t kPitchSemitoneChangeThreshold = pot(1.0f) / (kNumberOfOctaves * 12);

static constexpr auto kMapPitchFine = MapDef<float, 4>{
    {pot(0.0f), pot(0.45f), pot(0.55f), pot(1.0f)},
    {0.943874313f, 1.f, 1.f, 1.059463094f}};

static constexpr auto kMapPitchRoot = MapDef<int32_t, 12>{
    {pot(0.04f), pot(0.12f), pot(0.2f), pot(0.28f), pot(0.36f), pot(0.44f), pot(0.52f), pot(0.6f), pot(0.68f), pot(0.76f), pot(0.84f), pot(0.92f)},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}};

static constexpr auto kMapPitchRootTranspose = MapDef<float, 12>{
    {pot(0.04f), pot(0.12f), pot(0.2f), pot(0.28f), pot(0.36f), pot(0.44f), pot(0.52f), pot(0.6f), pot(0.68f), pot(0.76f), pot(0.84f), pot(0.92f)},
    {1.0f, 1.05946f, 1.12246f, 1.18920f, 1.2599f, 1.33483f, 1.41421f, 1.49830f, 1.58740f, 1.68179f, 1.78179f, 1.88774f}};

static constexpr auto kPitchRootTransposeLut = std::array<float, 12>{1.0f, 1.05946f, 1.12246f, 1.18920f, 1.2599f, 1.33483f, 1.41421f, 1.49830f, 1.58740f, 1.68179f, 1.78179f, 1.88774f};

}
#endif 
