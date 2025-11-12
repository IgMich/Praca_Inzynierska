#ifndef PITCH_DETECTION_H
#define PITCH_DETECTION_H

// # MIT License

// Copyright (c) 2024 FFT Study Repository Contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "fft_common.h"
#include "fft_algorithms.h"

// Musical note frequencies (A4 = 440 Hz)
typedef struct {
    const char* name;
    double frequency;
} musical_note_t;

// Pitch detection with confidence estimation
typedef struct {
    double frequency;
    double confidence;
    const char* note;
    double cents_off;
} pitch_result_t;

const char* frequency_to_note_name(double freq);
double detect_pitch_peak(complex_t* spectrum, int n, double sample_rate);
double detect_pitch_hps(complex_t* spectrum, int n, double sample_rate, int harmonics);
double detect_pitch_autocorr(complex_t* signal, int n, double sample_rate);
double detect_pitch_peak_v2(complex_t* spectrum, int n, double *fundamentals);
double detect_pitch_hps_v2(complex_t* spectrum, int n, double sample_rate, int harmonics);
double detect_pitch_autocorr_v2(complex_t* signal, int n, double sample_rate);
pitch_result_t detect_pitch_with_confidence(complex_t* signal, int n, double sample_rate);
void generate_musical_note(complex_t* signal, int n, double freq, double sample_rate, int num_harmonics, double* harmonic_amps);

#endif