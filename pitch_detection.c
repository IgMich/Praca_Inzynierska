#include "fft_common.h"
#include "fft_algorithms.h"
#include "pitch_detection.h"

musical_note_t notes[] = {
    {"C0", 16.35}, {"C#0", 17.32}, {"D0", 18.35}, {"D#0", 19.45},
    {"E0", 20.60}, {"F0", 21.83}, {"F#0", 23.12}, {"G0", 24.50},
    {"G#0", 25.96}, {"A0", 27.50}, {"A#0", 29.14}, {"B0", 30.87},
    {"C1", 32.70}, {"C#1", 34.65}, {"D1", 36.71}, {"D#1", 38.89},
    {"E1", 41.20}, {"F1", 43.65}, {"F#1", 46.25}, {"G1", 49.00},
    {"G#1", 51.91}, {"A1", 55.00}, {"A#1", 58.27}, {"B1", 61.74},
    {"C2", 65.41}, {"C#2", 69.30}, {"D2", 73.42}, {"D#2", 77.78},
    {"E2", 82.41}, {"F2", 87.31}, {"F#2", 92.50}, {"G2", 98.00},
    {"G#2", 103.83}, {"A2", 110.00}, {"A#2", 116.54}, {"B2", 123.47},
    {"C3", 130.81}, {"C#3", 138.59}, {"D3", 146.83}, {"D#3", 155.56},
    {"E3", 164.81}, {"F3", 174.61}, {"F#3", 185.00}, {"G3", 196.00},
    {"G#3", 207.65}, {"A3", 220.00}, {"A#3", 233.08}, {"B3", 246.94},
    {"C4", 261.63}, {"C#4", 277.18}, {"D4", 293.66}, {"D#4", 311.13},
    {"E4", 329.63}, {"F4", 349.23}, {"F#4", 369.99}, {"G4", 392.00},
    {"G#4", 415.30}, {"A4", 440.00}, {"A#4", 466.16}, {"B4", 493.88},
    {"C5", 523.25}, {"C#5", 554.37}, {"D5", 587.33}, {"D#5", 622.25},
    {"E5", 659.25}, {"F5", 698.46}, {"F#5", 739.99}, {"G5", 783.99},
    {"G#5", 830.61}, {"A5", 880.00}, {"A#5", 932.33}, {"B5", 987.77},
    {"C6", 1046.50}, {"C#6", 1108.73}, {"D6", 1174.66}, {"D#6", 1244.51},
    {"E6", 1318.51}, {"F6", 1396.91}, {"F#6", 1479.98}, {"G6", 1567.98},
    {"G#6", 1661.22}, {"A6", 1760.00}, {"A#6", 1864.66}, {"B6", 1975.53},
    {"C7", 2093.00}, {"C#7", 2217.46}, {"D7", 2349.32}, {"D#7", 2489.02},
    {"E7", 2637.02}, {"F7", 2793.83}, {"F#7", 2959.96}, {"G7", 3135.96},
    {"G#7", 3322.44}, {"A7", 3520.00}, {"A#7", 3729.31}, {"B7", 3951.07},
    {"C8", 4186.01}
};

int num_notes = sizeof(notes) / sizeof(notes[0]);

// Find closest musical note
const char* frequency_to_note_name(double freq) {
    int closest_idx = 0;
    double min_cents = 1200;  // Max cents difference
    
    for (int i = 0; i < num_notes; i++) {
        double cents = 1200 * log2(freq / notes[i].frequency);
        if (fabs(cents) < fabs(min_cents)) {
            min_cents = cents;
            closest_idx = i;
        }
    }
    
    static char result[32];
    if (fabs(min_cents) < 1) {
        snprintf(result, sizeof(result), "%s (in tune)", notes[closest_idx].name);
    } else {
        snprintf(result, sizeof(result), "%s (%+.0f cents)", 
                notes[closest_idx].name, min_cents);
    }
    
    return result;
}

// Simple peak detection for fundamental frequency
double detect_pitch_peak(complex_t* spectrum, int n, double sample_rate) {
    double* magnitude = compute_magnitude(spectrum, n);
    
    // Find peak in reasonable frequency range (80-2000 Hz)
    int min_bin = (int)(80 * n / sample_rate);
    int max_bin = (int)(2000 * n / sample_rate);
    if (max_bin > n/2) max_bin = n/2;
    
    double max_mag = 0;
    int peak_bin = 0;
    
    for (int i = min_bin; i < max_bin; i++) {
        if (magnitude[i] > max_mag) {
            max_mag = magnitude[i];
            peak_bin = i;
        }
    }
    
    // Quadratic interpolation for more accurate frequency
    if (peak_bin > 0 && peak_bin < n/2 - 1) {
        double y1 = magnitude[peak_bin - 1];
        double y2 = magnitude[peak_bin];
        double y3 = magnitude[peak_bin + 1];
        
        double delta = 0.5 * (y1 - y3) / (y1 - 2*y2 + y3);
        peak_bin += delta;
    }
    
    free(magnitude);
    
    return peak_bin * sample_rate / n;
}
double detect_pitch_peak_v2(complex_t* spectrum, int k,double *fundamentals){
    double* magnitude = compute_magnitude(spectrum,k);
    
    double max_mag = 0;
    int freq_idx = 0;
    
    for (int i = 0; i < k; i++) {
        if (magnitude[i] > max_mag) {
            max_mag = magnitude[i];
            freq_idx = i;
        }
    }
    free(magnitude);
    return fundamentals[freq_idx%12]*pow(2,(int)freq_idx/12);
}

// Harmonic Product Spectrum (HPS) method
double detect_pitch_hps(complex_t* spectrum, int n, double sample_rate, int harmonics) {
    double* magnitude = compute_magnitude(spectrum, n);
    double* hps = (double*)malloc((n/2 + 1) * sizeof(double));
    
    // Initialize HPS with original spectrum
    for (int i = 0; i <= n/2; i++) {
        hps[i] = magnitude[i];
    }
    
    // Multiply with downsampled versions
    for (int h = 2; h <= harmonics; h++) {
        for (int i = 0; i <= n/(2*h); i++) {
            hps[i] *= magnitude[i * h];
        }
    }
    
    // Find peak in HPS
    int min_bin = (int)(80 * n / sample_rate);
    int max_bin = (int)(1000 * n / sample_rate);
    if (max_bin > n/(2*harmonics)) max_bin = n/(2*harmonics);
    
    double max_hps = 0;
    int peak_bin = 0;
    
    for (int i = min_bin; i < max_bin; i++) {
        if (hps[i] > max_hps) {
            max_hps = hps[i];
            peak_bin = i;
        }
    }
    
    free(magnitude);
    free(hps);
    
    return peak_bin * sample_rate / n;
}



// Autocorrelation-based pitch detection
double detect_pitch_autocorr(complex_t* signal, int n, double sample_rate) {
    // Compute autocorrelation using FFT
    complex_t* signal_copy = allocate_complex_array(n);
    memcpy(signal_copy, signal, n * sizeof(complex_t));
    
    // FFT
    radix2_dit_fft(signal_copy, n, FFT_FORWARD);
    
    // Power spectrum
    for (int i = 0; i < n; i++) {
        signal_copy[i] = signal_copy[i] * conj(signal_copy[i]);
    }
    
    // Inverse FFT to get autocorrelation
    radix2_dit_fft(signal_copy, n, FFT_INVERSE);
    
    // Find peak in autocorrelation (excluding lag 0)
    int min_lag = (int)(sample_rate / 1000);  // 1000 Hz max
    int max_lag = (int)(sample_rate / 80);     // 80 Hz min
    if (max_lag > n/2) max_lag = n/2;
    
    double max_corr = 0;
    int peak_lag = 0;
    
    for (int lag = min_lag; lag < max_lag; lag++) {
        double corr = creal(signal_copy[lag]);
        if (corr > max_corr) {
            max_corr = corr;
            peak_lag = lag;
        }
    }
    
    free_complex_array(signal_copy);
    
    if (peak_lag > 0) {
        return sample_rate / peak_lag;
    }
    
    return 0;
}

pitch_result_t detect_pitch_with_confidence(complex_t* signal, int n, double sample_rate) {
    pitch_result_t result = {0};
    
    // Method 1: Peak detection
    complex_t* spectrum = allocate_complex_array(n);
    memcpy(spectrum, signal, n * sizeof(complex_t));
    radix2_dit_fft(spectrum, n, FFT_FORWARD);
    
    double pitch1 = detect_pitch_peak(spectrum, n, sample_rate);
    
    // Method 2: HPS
    double pitch2 = detect_pitch_hps(spectrum, n, sample_rate, 5);
    
    // Method 3: Autocorrelation
    double pitch3 = detect_pitch_autocorr(signal, n, sample_rate);
    
    // Combine results
    result.frequency = pitch2;  // HPS is often most reliable
    
    // Estimate confidence based on agreement between methods
    double avg_pitch = (pitch1 + pitch2 + pitch3) / 3;
    double variance = pow(pitch1 - avg_pitch, 2) + 
                     pow(pitch2 - avg_pitch, 2) + 
                     pow(pitch3 - avg_pitch, 2);
    variance /= 3;
    
    result.confidence = 1.0 / (1.0 + sqrt(variance) / avg_pitch);
    
    // Find musical note
    result.note = frequency_to_note_name(result.frequency);
    
    free_complex_array(spectrum);
    
    return result;
}

// Generate test signals
void generate_musical_note(complex_t* signal, int n, double freq, double sample_rate, 
                          int num_harmonics, double* harmonic_amps) {
    for (int i = 0; i < n; i++) {
        signal[i] = 0;
        for (int h = 1; h <= num_harmonics; h++) {
            double amp = (harmonic_amps && h <= num_harmonics) ? 
                        harmonic_amps[h-1] : 1.0 / h;
            signal[i] += amp * sin(2 * PI * freq * h * i / sample_rate);
        }
    }
}

