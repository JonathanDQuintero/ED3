/* codigo para la generiacion de una onda triangular

*/
#include "lpc17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"

#define NUM_WAVE_SAMPLES 100 // cantidad de muestras de la senoidal


void confWave(void){
    uint32_t WaveFormInit[NUM_WAVE_SAMPLES];
    for(uint16_t i=0; i<NUM_WAVE_SAMPLES; i++){
        WaveFormInit[i]=i;
    }
//Generacion forma de onda triangular(full wave)
for(uint16_t i=0; i<NUM_WAVE_SAMPLES; i++){
    //Primer Cuadrante
    if(i<(1*(NUM_WAVE_SAMPLES/4))){
        WaveFormInit[i]=512+512*WaveFormInit [i]/NUM_WAVE_SAMPLES;
    }
    if(i==(1*(NUM_WAVE_SAMPLES/4))){
        WaveFormInit[i]=1023;//para asegurar que el valor maximo sea 1023
    }
    //Segundo Cuadrante
    if(i<(2*(NUM_WAVE_SAMPLES/4))){
        WaveFormInit[i]=512+512*WaveFormInit[(NUM_WAVE_SAMPLES/4)-i]/NUM_WAVE_SAMPLES;
        }
    //Tercer Cuadrante
    if(i<(3*(NUM_WAVE_SAMPLES/4))){
        WaveFormInit[i]=512-512*WaveFormInit[i-((NUM_WAVE_SAMPLES/4))]/NUM_WAVE_SAMPLES;
    }
    //Cuarto Cuadrante
    if(i<(4*(NUM_WAVE_SAMPLES/4))){
        WaveFormInit[i]=512-512*WaveFormInit[(NUM_WAVE_SAMPLES)-i]/NUM_WAVE_SAMPLES;
    }
    WaveFormInit[i]=WaveFormInit[i]<<6;//corrimiento para que quede en el lugar correcto del registro del DAC
}
return;
}
/*corregido por chat gpt


*/
#define NUM_WAVE_SAMPLES 100
uint32_t WaveFormInit[NUM_WAVE_SAMPLES];

void confWave(void) {
    for (uint16_t i = 0; i < NUM_WAVE_SAMPLES; i++) {
        uint16_t value;
        if (i < NUM_WAVE_SAMPLES / 2) {
            // Subida lineal: de 0 a 1023
            value = (uint16_t)( (1023.0 / (NUM_WAVE_SAMPLES / 2)) * i );
        } else {
            // Bajada lineal: de 1023 a 0
            value = (uint16_t)( 1023 - (1023.0 / (NUM_WAVE_SAMPLES / 2)) * (i - (NUM_WAVE_SAMPLES / 2)) );
        }
        // Ajustar al formato del DAC (bits [15:6])
        WaveFormInit[i] = value << 6;
    }
}
