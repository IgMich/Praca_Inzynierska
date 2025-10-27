#include "wavformat.h"
#include "pitch_detection.h"
#include "audio_spectrum.h"

double compute_energy(complex_t* signal, int n){
    double energy = 0;
    for(int i=0;i<n;i++){
        energy +=pow(signal[i],2.0);
    }
    return energy;
}

bool check_new_pitch(double *pitches, double *curr_pitches, int idx){
    if(pitches[idx]!=curr_pitches[idx]){
        return 1;
    }
    return 0;

}
const char* methods[] = {"Maximum Peak", "HPS", "Autocorrelation"};

void display_current_pitch_wav(double energy,double *pitches, double confidence, int num_frame, const char * method){
    int idx = -1;
    for(int i; i< 3;i++){
        if(strcmp(method,methods[i]) == 0){
            idx = i;
            printf("Frame:%d\n",num_frame);
            printf("Energy: %.1f\n",energy);
            printf("Method: %s\n",methods[idx]);
            printf("Detected pitch: %.2f Hz\n", pitches[idx]);
            printf("Musical note: %s\n", frequency_to_note_name(pitches[idx]));
            printf("Confidence: %.1f%%\n\n", confidence * 100);
            break;
        }
    }
    if(idx == -1){
        printf("Cannot find method: %s\n\n",method);
    }

}

void analyze_wav_file(sound_t sound, int n, double sample_rate, const char* method){
    static uint32_t sound_position = 0; //Variable to iterate over the samples
    double *curr_pitches = calloc(3,sizeof(double)); // current pitches (estimated by each method)
    double curr_energy =0.0;    //Energy of last analyzed signal frame
    int num_frame = 0;
    int idx = 0;

    //Select method to dispay
    for(int i=0;i<3;i++){
        if(strcmp(method,methods[i]) == 0)
        idx = i;
    }
    // Main loop
    while(sound_position < sound.samples){
        complex_t* signal = allocate_complex_array(n);
        for(int i=0; i < n;i++){
            if(sound_position < sound.samples){
                signal[i] = (double)sound.data[sound_position++] + 0*I;
            }
            else{
                signal[i] = 0.0 + 0*I;
            }
        }
        num_frame++;
        double energy = compute_energy(signal,n);
        double energy_ratio = curr_energy/energy;
        
        //energy of next frames is rising == new note
        if(energy_ratio < 1.5){
            apply_window_hann(signal, n);
            complex_t* spectrum = allocate_complex_array(n);
            memcpy(spectrum, signal, n * sizeof(complex_t));
            radix2_dit_fft(spectrum, n, FFT_FORWARD);
            double *pitches = calloc(3,sizeof(double));
            // Method 1: Simple Maximum Peak
            pitches[0] = detect_pitch_peak(spectrum, n, sample_rate);
            // Method 2: HPS
            pitches[1] = detect_pitch_hps(spectrum, n, sample_rate, 3);
        
            // Method 3: Autocorrelation
            pitches[2] = detect_pitch_autocorr(signal, n, sample_rate);
            double avg_pitch = (pitches[0] + pitches[1] + pitches[1]) / 3;
            double variance = pow(pitches[0] - avg_pitch, 2) + 
                    pow(pitches[1] - avg_pitch, 2) + 
                    pow(pitches[2] - avg_pitch, 2);
            variance /= 3;

            double confidence = 1.0 / (1.0 + sqrt(variance) / avg_pitch);
            // if(check_new_pitch(pitches,curr_pitches,idx)){
            //     for(int i=0;i<3;i++){
            //         curr_pitches[i] = pitches[i];
            //     }
            //     display_current_pitch_wav(curr_pitches,confidence,num_frame, method);
            // }
            display_current_pitch_wav(energy_ratio,pitches,confidence,num_frame, method);
            free_complex_array(spectrum);
        }
        curr_energy = energy;
        free_complex_array(signal);
    }
}
// Main demonstration
int main() {
    printf("Music Pitch Detection using FFT\n");
    printf("================================\n\n");
    
    double sample_rate = 44100;
    int n = 4096;

    
    printf("\nWav file analyze test\n\n");
    sound_t sound;
	if(!LoadWav("wav/e4.wav", &sound)) {
		PRINT_ERROR("Failed to load cheers.wav");
	}
    else{
        printf("Wav file loaded succesfully\n");
    }
    analyze_wav_file(sound,n,sample_rate,methods[2]);
    
    // // Test 1: Pure sine wave
    // printf("Test 1: Pure Sine Wave (A4 = 440 Hz)\n");
    // printf("------------------------------------\n");
    
    // complex_t* signal = allocate_complex_array(n);
    // for (int i = 0; i < n; i++) {
    //     signal[i] = sin(2 * PI * 440 * i / sample_rate);
    // }
    
    // pitch_result_t result = detect_pitch_with_confidence(signal, n, sample_rate);
    // printf("Detected pitch: %.2f Hz\n", result.frequency);
    // printf("Musical note: %s\n", result.note);
    // printf("Confidence: %.1f%%\n\n", result.confidence * 100);
    
    // // Test 2: Complex tone with harmonics
    // printf("Test 2: Complex Tone (E4 with harmonics)\n");
    // printf("----------------------------------------\n");
    
    // double harmonic_amps[] = {1.0, 0.5, 0.3, 0.2, 0.1};
    // generate_musical_note(signal, n, 329.63, sample_rate, 5, harmonic_amps);
    
    // result = detect_pitch_with_confidence(signal, n, sample_rate);
    // printf("Detected pitch: %.2f Hz\n", result.frequency);
    // printf("Musical note: %s\n", result.note);
    // printf("Confidence: %.1f%%\n\n", result.confidence * 100);
    
    // // Test 3: Chord detection (challenging)
    // printf("Test 3: C Major Chord (C4, E4, G4)\n");
    // printf("-----------------------------------\n");
    
    // for (int i = 0; i < n; i++) {
    //     signal[i] = sin(2 * PI * 261.63 * i / sample_rate) +  // C4
    //                sin(2 * PI * 329.63 * i / sample_rate) +   // E4
    //                sin(2 * PI * 392.00 * i / sample_rate);    // G4
    // }
    
    // // Apply window to reduce spectral leakage
    // apply_window_hann(signal, n);
    
    // result = detect_pitch_with_confidence(signal, n, sample_rate);
    // printf("Detected pitch: %.2f Hz\n", result.frequency);
    // printf("Musical note: %s\n", result.note);
    // printf("Confidence: %.1f%%\n", result.confidence * 100);
    // printf("(Note: Chord detection is challenging - usually detects root note)\n\n");
    
    // // Test 4: Noisy signal
    // printf("Test 4: Noisy Signal (A4 with 20%% noise)\n");
    // printf("-----------------------------------------\n");
    
    // for (int i = 0; i < n; i++) {
    //     signal[i] = sin(2 * PI * 440 * i / sample_rate) + 
    //                0.2 * ((double)rand() / RAND_MAX - 0.5);
    // }
    
    // result = detect_pitch_with_confidence(signal, n, sample_rate);
    // printf("\nDetected pitch: %.2f Hz\n", result.frequency);
    // printf("Musical note: %s\n", result.note);
    // printf("Confidence: %.1f%%\n", result.confidence * 100);
    
    // // Compare pitch detection methods
    // printf("Method Comparison:\n");
    // printf("==================\n");
    
    // // Generate test tone
    // generate_musical_note(signal, n, 220, sample_rate, 3, NULL);  // A3
    
    // complex_t* spectrum = allocate_complex_array(n);
    // memcpy(spectrum, signal, n * sizeof(complex_t));
    // radix2_dit_fft(spectrum, n, FFT_FORWARD);
    
    // double pitch_peak = detect_pitch_peak(spectrum, n, sample_rate);
    // double pitch_hps = detect_pitch_hps(spectrum, n, sample_rate, 5);
    // double pitch_autocorr = detect_pitch_autocorr(signal, n, sample_rate);
    
    // printf("True frequency: 220.00 Hz (A3)\n");
    // printf("Peak detection: %.2f Hz (error: %.2f Hz)\n", 
    //        pitch_peak, pitch_peak - 220);
    // printf("HPS method: %.2f Hz (error: %.2f Hz)\n", 
    //        pitch_hps, pitch_hps - 220);
    // printf("Autocorrelation: %.2f Hz (error: %.2f Hz)\n", 
    //        pitch_autocorr, pitch_autocorr - 220);
    
    // // Performance considerations
    // printf("\n\nPerformance Considerations:\n");
    // printf("===========================\n");
    // printf("- FFT size: Larger = better frequency resolution\n");
    // printf("- Sample rate: Higher = detect higher frequencies\n");
    // printf("- Window function: Reduces spectral leakage\n");
    // printf("- HPS: Good for harmonic sounds\n");
    // printf("- Autocorrelation: Robust to noise\n");
    // printf("- Combine methods for better accuracy\n");
    
    // // Applications
    // printf("\n\nApplications:\n");
    // printf("=============\n");
    // printf("1. Musical instrument tuners\n");
    // printf("2. Vocal pitch correction (Auto-Tune)\n");
    // printf("3. Music transcription software\n");
    // printf("4. Speech analysis\n");
    // printf("5. Musical education tools\n");
    // printf("6. Frequency measurement instruments\n");
    
    // free_complex_array(signal);
    // free_complex_array(spectrum);
    return 0;
}
