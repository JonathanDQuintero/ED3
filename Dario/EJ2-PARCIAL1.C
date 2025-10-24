/*
Utilizando el timer0, un dac, interrupciones y el driver del LPC1769 ,
 escribir un código que permita generar una señal triangular periódica 
 simétrica, que tenga el mínimo periodo posible, la máxima excursión de 
 voltaje pico a pico posible y el mínimo incremento de señal posible por el dac.
  Suponer una frecuencia de core cclk de 100 Mhz.
   El código debe estar debidamente comentado.
*/
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"

volatile uint16_t dac_valor = 0;
volatile uint8_t direccion = 1; // 1 para subir, 0 para bajar

void cfgTMR0(void);
void cfgDAC(void);

int main(void){
    cfgTMR0();
    cfgDAC();
    while(1) {
        __WFI();
    }

    return 0;
}

void cfgTMR0(void) {
    TIM_TIMERCFG_Type t0;
    TIM_MATCHCFG_Type m0;

    t0.prescaleOption = TIM_USVAL;
    t0.prescaleValue = 1; // cuenta cada 1us

    m0.matchChannel = TIM_MATCH_0;
    m0.resetOnMatch = ENABLE;
    m0.matchValue = 1;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &t0);
    TIM_ConfigMatch(LPC_TIM0, &m0);
    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void cfgDAC(void) {
    DAC_Init(LPC_DAC);//ya configura el pin del dac
    DAC_UpdateValue(0);
    DAC_SetBias(DAC_700uA);//1 us max
}

void TIMER0_IRQHandler(void){
    // Verificar interrupción de Match0
    if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
        
        // Actualizar valor DAC según dirección
        if(direccion) {
            // Subiendo
            if(dac_valor >= 1023) {
                direccion = 0;  // Cambiar a bajada
            } else {
                dac_valor++;
            }
        } else {
            // Bajando
            if(dac_valor == 0) {
                direccion = 1;  // Cambiar a subida
            } else {
                dac_valor--;
            }
        }
        
        // Actualizar DAC (desplazar 6 bits a la izquierda)
        DAC_UpdateValue(dac_valor << 6);
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); // Limpio bandera para actualizar con un nuevo valor al DAC
    }
}