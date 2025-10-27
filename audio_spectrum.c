#include "audio_spectrum.h"


// Window functions for spectral analysis
void apply_window_hann(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        double window = 0.5 * (1.0 - cos(TWO_PI * i / (n - 1)));
        signal[i] *= window;
    }
}

void apply_window_hamming(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        double window = 0.54 - 0.46 * cos(TWO_PI * i / (n - 1));
        signal[i] *= window;
    }
}

void apply_window_blackman(complex_t* signal, int n) {
    for (int i = 0; i < n; i++) {
        double window = 0.42 - 0.5 * cos(TWO_PI * i / (n - 1)) 
                      + 0.08 * cos(4 * PI * i / (n - 1));
        signal[i] *= window;
    }
}

// Generate test audio signal
void generate_test_audio(complex_t* signal, int n, double sample_rate) {
    // Generate a signal with multiple frequency components
    double f1 = 440.0;   // A4 note
    double f2 = 554.37;  // C#5 note
    double f3 = 659.25;  // E5 note
    
    for (int i = 0; i < n; i++) {
        double t = i / sample_rate;
        signal[i] = 0.5 * sin(TWO_PI * f1 * t) +
                   0.3 * sin(TWO_PI * f2 * t) +
                   0.2 * sin(TWO_PI * f3 * t) +
                   0.1 * ((double)rand() / RAND_MAX - 0.5);  // Add noise
    }
}

// Compute frequency for each FFT bin
double bin_to_frequency(int bin, int fft_size, double sample_rate) {
    return bin * sample_rate / fft_size;
}


void find_peaks(double* magnitude, int n, double sample_rate, 
                peak_t* peaks, int* num_peaks, int max_peaks) {
    *num_peaks = 0;
    double threshold = 0.1;  // Minimum magnitude threshold
    
    for (int i = 1; i < n/2 - 1 && *num_peaks < max_peaks; i++) {
        // Check if local maximum
        if (magnitude[i] > magnitude[i-1] && 
            magnitude[i] > magnitude[i+1] &&
            magnitude[i] > threshold) {
            
            peaks[*num_peaks].bin = i;
            peaks[*num_peaks].frequency = bin_to_frequency(i, n, sample_rate);
            peaks[*num_peaks].magnitude = magnitude[i];
            (*num_peaks)++;
        }
    }
    
    // Sort peaks by magnitude (simple bubble sort)
    for (int i = 0; i < *num_peaks - 1; i++) {
        for (int j = 0; j < *num_peaks - i - 1; j++) {
            if (peaks[j].magnitude < peaks[j+1].magnitude) {
                peak_t temp = peaks[j];
                peaks[j] = peaks[j+1];
                peaks[j+1] = temp;
            }
        }
    }
}

// Display spectrum as ASCII art
void display_spectrum_ascii(double* magnitude, int n, double sample_rate) {
    int display_bins = 64;  // Number of bins to display
    if (display_bins > n/2) display_bins = n/2;
    
    // Find maximum magnitude for scaling
    double max_mag = 0;
    for (int i = 0; i < display_bins; i++) {
        if (magnitude[i] > max_mag) max_mag = magnitude[i];
    }
    
    printf("\nFrequency Spectrum:\n");
    printf("==================\n");
    
    // Display each frequency bin
    for (int i = 0; i < display_bins; i++) {
        double freq = bin_to_frequency(i, n, sample_rate);
        int bar_length = (int)(50 * magnitude[i] / max_mag);
        
        printf("%5.0f Hz |", freq);
        for (int j = 0; j < bar_length; j++) {
            printf("█");
        }
        printf(" %.3f\n", magnitude[i]);
    }
}

// Analyze audio spectrum
void analyze_audio_spectrum(complex_t* signal, int n, double sample_rate, 
                          const char* window_type) {
    // Apply window function
    if (strcmp(window_type, "hann") == 0) {
        apply_window_hann(signal, n);
    } else if (strcmp(window_type, "hamming") == 0) {
        apply_window_hamming(signal, n);
    } else if (strcmp(window_type, "blackman") == 0) {
        apply_window_blackman(signal, n);
    }
    
    // Compute FFT
    radix2_dit_fft(signal, n, FFT_FORWARD);
    
    // Compute magnitude spectrum
    double* magnitude = compute_magnitude(signal, n);
    
    // Find and display peaks
    peak_t peaks[10];
    int num_peaks;
    find_peaks(magnitude, n, sample_rate, peaks, &num_peaks, 10);
    
    printf("\nDetected Peaks:\n");
    printf("===============\n");
    for (int i = 0; i < num_peaks && i < 5; i++) {
        printf("%d. %.1f Hz (magnitude: %.3f)\n", 
               i+1, peaks[i].frequency, peaks[i].magnitude);
    }
    
    // Display spectrum
    display_spectrum_ascii(magnitude, n, sample_rate);
    
    free(magnitude);
}


