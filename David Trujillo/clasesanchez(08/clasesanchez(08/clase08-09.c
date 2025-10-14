#include "lpc17xx.h"

#define INPUT (uint8_t) 0
#define OUTPUT (uint8_t) 1

#define LEDverde (uint32_t)(1<<22)
#define LEDrojo (uint32_t)(1<<25)
#define LEDazul (uint32_t)(1<<24)

#define PORT_UNO (uint8_t)1

volatile uint16_t ADC_result=0;
volatile uint8_t muestras_altas=0;

void configGPIO(void);
void configTimer(void);
void configADC(void);

void configGPIO(void){
    PINSEL_CFG_TYPE PinCfg;
    PinCfg.Portnum = PINSEL_PORT_1;
    PinCfg.Pinnum = PINSEL_PIN_22;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PinCfg.Funcnum = PINSEL_FUNC_0;
    pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&PinCfg);
    // Configuro el pin P1.22 como salida(LED verde)
    GPIO_SetDir(PORT_UNO, LEDverde, OUTPUT );
}
void configTimer(void){
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    TIM_MATCHCFG_Type TIM_MatchConfigStruct;

    // Configuro el timer en modo de contador de tiempo
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue = 1000; // 1ms
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);//TIM_TIMER_MODE indica que es un timer no un contador

    // Configuro el valor de comparacion del timer
    TIM_MatchConfigStruct.MatchChannel = 1;
    TIM_MatchConfigStruct.IntOnMatch = ENABLE;
    TIM_MatchConfigStruct.ResetOnMatch = DISABLE;
    TIM_MatchConfigStruct.StopOnMatch = DISABLE;
    TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    TIM_MatchConfigStruct.MatchValue = 50; // por que tiene que ser doble para que sea 100ms 
    TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);

    NVIC_SetPriority(TIMER0_IRQn, 1); // Prioridad 1
    // Habilito la interrupcion del timer
    NVIC_EnableIRQ(TIMER0_IRQn);
    // Inicio el timer
    TIM_Cmd(LPC_TIM0, ENABLE);
    return;
}
void configADC(void){
    PINSEL_CFG_TYPE PinADC;
    PinADC.Portnum = PINSEL_PORT_0;
    ADC_Init(LPC_ADC, 200000); // Frecuencia de trabajo del ADC a 200KHz
    ADC_BurstCmd(LPC_ADC, DISABLE); // Deshabilito el modo rafaga
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE); // Habilito la interrupcion del canal 0
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE); // Habilito el canal 0
    NVIC_SetPriority(ADC_IRQn, 2); // Prioridad 2
    NVIC_EnableIRQ(ADC_IRQn); // Habilito las interrupciones del ADC
    ADC_StartCmd(LPC_ADC, ADC_START_NOW); // Inicio la conversion
}

void ADC_IRQHandler(void){
    TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT); // Limpio la interrupcion del timer

    uint8_t  temp=0;
    uint16_t consecutivas_rojo=0;

    if(temp<=40){

    }
    return;
}
falta terminar 