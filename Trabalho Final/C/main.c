#include "dump.h"

#define NUM_SAMPLES FFT_TABLE_N
#define SAMPLE_FREQUENCY FFT_TABLE_FS
#define FREQ_SCALE FFT_FREQ_SCALE

//uint16_t samples[2][NUM_SAMPLES];
int sampling_buffer_index = 0;

int16_t real_buffer[NUM_SAMPLES];
int16_t imaginary_buffer[NUM_SAMPLES];
int16_t mag_buffer[NUM_SAMPLES/2 + 1]

void setup(){
    memset(&samples, 0, sizeof(samples));
    memset(&real_buffer, 0,sizeof(real_buffer));
    memset(&imaginary_buffer, 0,sizeof(imaginary_buffer));
    memset(&mag_buffer, 0,sizeof(mag_buffer));

    // setup ADC com DMA

    // setup timer

    // setup LCD16x2

}

void update_display()

void loop(){
    while(1){
        int32_t processing_buffer_index = sampling_buffer_index ^ 0b1;
        uint32_t peak_mag_index;
        fft(&samples[processing_buffer_index][0],
            &mag_buffer,
            NUM_SAMPLES,
            &real_buffer, 
            &imaginary_buffer,
            &peak_mag_index
        );
        
        const uint32_t frequency = frequency_bins[peak_mag_index];

        LCD16X2_Set_Cursor(LCD16X2_1,1,1,);
        
    }
}

int main() {
    setup();
    loop();
}

void HAL_ADC_ConversionComplete_Callback(){
    if(hadc == hadc1){
        sampling_buffer_index ^= 0b1;
        HAL_ADC_StartDMA(hadc,);
    }
}