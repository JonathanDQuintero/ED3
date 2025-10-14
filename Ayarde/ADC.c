void confADC(void) {
    LPC_SC->PCONP |= (1 << 12); // Habilito el periferico ADC (PCADC)
    LPC_ADC->ADCR |= (1 << 21); // ADC activado
    LPC_SC->PCLKSEL0 |= (3 << 24);// PCLK_ADC = CCLK/8 = 12.5MHz (frecuencia reloj a la que puede trabajar el ADC)
    LPC_ADC->ADCR &= ~(255<<8); // ( bits [15:8] CLKDIV = 255 + 1 = 256 (secuencia de trabajo final)
    LPC_ADC->ADCR |= (1 << 16); // modo burst(si la frecuencia de muestreo es de 10khz si son 2 canales 5khz cada uno, como es 1 canal a 10khz)
    LPC_PINCON->PINMODE1 |= (1<<15)// P0.23 sin pull-up ni pull-down
    LPC_PINCON->PINSEL1 |= (1<<14);// P0.23 como funcion AD0.0
    LPC_ADC->ADINTEN |= 1; // habilito la interrupcion por cada conversion
    LPC_ADC->ADINTEN &= ~(1<<8); // para el modo burst tiene que estar en 0 el bit 8 (interrupcion por secuencia de canales) por que como resetea en 1 solo lee a el global
    NVIC_EnableIRQ(ADC_IRQn); // habilito las interrupciones del ADC
    return;
}
void ADC_IRQHandler(void) {
    //float volt; // variable auxiliar para calcular el voltaje (demanda un poco mas de tiempo de procesamiento)
    ADC0Value = ((LPC_ADC->ADDR0)>>4) & 0xFFF; // variable auxiliar para observar el valor convertido (hacemos una mascara para quedarnos con los bits [15:4] RESULT y no usar las otras funciones del ADC)
    //volt = (ADC0Value*3.3)/4096; // si quiero calcular el voltaje en base al valor digital (3.3V es la referencia y 4096=2^12)
    if (ADC0Value<2054) { 
        LPC_GPIO0->FIOSET = (1<<22); // P0.22 en alto
    } else {
        LPC_GPIO0->FIOCLR = (1<<22); // P0.22 en bajo
    }
    return;
}
/* ejercicio de ADC utilizando el canal 2 sin utilizar interrupciones para observar 
el dato digitalizado sino que vamos a hacer una retardo por software para que apartir de determinado
tiempo tome el valor del ADC y lo guarde en una variable global

*/
#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_pinsel.h"

#define _ADC_INT        ADC_ADINTEN2  // interrupcion por cada conversion
#define _ADC_CHANNEL    ADC_CHANNEL_2 // canal 2 del ADC (AD0.2 P0.25)

_IO uint32_t adc_value;/* variable global para almacenar el valor convertido del ADC
(_IO para que no la optimice el compilador es una variable "volatile") la utilizamos por que los distintos
valores no lo toma por software si no por hardware y por eso el micro puede optimizarla sino ve
que se modifica por software y con esto siempre lo vaya a buscar a registro correspondiente */

void ADC_IRQHandler(void);
void confADC(void);
void confPin(void);

int main(void) {
    uint32_t tmp;
    
    confPin(); // configuro el pin
    confADC(); // configuro el ADC
    while(1){
        ADC_StartCmd(LPC_ADC, ADC_START_NOW); // inicio la conversion
        NVIC_EnableIRQ(ADC_IRQn); // habilito las interrupciones del ADC
        for(tmp=0; tmp<100000; tmp++); // retardo por software para que alcance a hacer la conversion

    }
    return 0;
}
void confPin(void){
    PINSEL_CFG_Type PinCfg;// 
    PinCfg.Funcnum =PINSEL_FUNC_1; // funcion 1 del pin
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL; // sin open drain
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE; // sin pull-up ni pull-down
    PinCfg.Pinnum = 25; // pin 25 del puerto 0
    PinCfg.Portnum = 0; // puerto 0
    PINSEL_ConfigPin(&PinCfg); // llamo a la funcion que configura el pin
    

    return;
}
void configADC(void) {
    ADC_Init(LPC_ADC, 200000); // inicializo el ADC a 200kHz
    ADC_IntConfig(LPC_ADC, _ADC_INT, ENABLE); // habilito la interrupcion por cada conversion
    ADC_ChannelCmd(LPC_ADC, _ADC_CHANNEL, ENABLE); // habilito el canal 2 del ADC

    NVIC_Setpriority(ADC_IRQn, (9)); // seteo la prioridad de la interrupcion del ADC como 9
    return;
}
void ADC_IRQHandler(void) {
    adc_value = 0;
    if (ADC_ChannelGetStatus(LPC_ADC, _ADC_CHANNEL, ADC_DATA_DONE)) { // si la conversion del canal 2 esta lista
        adc_value = ADC_ChannelGetData(LPC_ADC, _ADC_CHANNEL); // leo el valor convertido y lo guardo en la variable global
        NVIC_DisableIRQ(ADC_IRQn); // deshabilito las interrupciones del ADC para no entrar nuevamente hasta que vuelva a iniciar una conversion cuando finalice el retardo
    }
     
    return;
}