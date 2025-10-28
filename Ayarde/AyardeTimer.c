#include "LPC17xx.h"
// connfigracion de Timer0
void confTimer(void) {
    // Power up Timer0
    LPC_SC->PCONP |= (1 << 1); // disminuye el consumo de energia y prende el periferico

    // Set PCLK for Timer0 to CCLK
    LPC_SC->PCLKSEL0    |= (1 << 2);// indica el reloj que va a llegar al periferico PLCK=CCLK
    LPC_PINCON->PINSEL3 |= (3 <<24); // P1.28 como funcion de Timer0 el MAT0.0
    
    LPC_TIM0->EMR       |= (3 << 4); // match genera una señal externa toggle en match0
    LPC_TIM0->MR0 = 70000000; // Seteo el match valuado en 0.7seg a 100MHz=PCLK // T=(1/PCLK)*(PR+1)*(MR0+1)=0.7seg; PCLK=CCLK=100MHz, PR=0, MR0=70M

    // Interrupt and reset on MR0
    LPC_TIM0->MCR       |= (1<<1);// reset on match0
    LPC_TIM0->MCR       &= ~(1<<0);// interrupcion deshabilitada que genere el timer
    //LPC_TIM0->MCR       |= ~(1<<0);// pongo en 1 si quiero que cuando el valor de timer sea igual al valor del match 0 se genere una interrupcion
    // Start Timer0
    LPC_TIM0->TCR       = 3; // Reset and enable timer
    LPC_TIM0->TCR       &=~(1<<1);// quitar el reset
    // Enable Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);
 

}

/*timer por cap
*
*/
void confTimer(void) {
    // Power up Timer0
    LPC_SC->PCONP |= (1 << 1); // disminuye el consumo de energia y prende el periferico

    // Set PCLK for Timer0 to CCLK/4
    LPC_SC->PCLKSEL0 &= ~(1 << 2);// indica el reloj que va a llegar al periferico
    LPC_PINCON->PINSEL3 |= (1 << 20); // P1.26 como funcion de CAP0.0 
    
    LPC_TIM0->CCR       |= (3 << 1)|(1<<2)// CAP0.0 rising and falling edge and interrupt
    // Start Timer0
    LPC_TIM0->TCR       = 3; // Reset and enable timer
    LPC_TIM0->TCR       &=~(1<<1);// quitar el reset
    // Enable Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);


}


void TIMER0_IRQHandler(void) {
   static uint32_t i= 0;
   //si uso cap: aux = LPC_TIM0->CR0; // leo el valor del registro de captura
   if (i==0){
    LPC_GPIO0->FIOSET |= (1<<22); // P0.22 en alto
    i=1;    
    }else if (i==1){
    LPC_GPIO0->FIOCLR |= (1<<22); // P0.22
    i=0;    
    }
    LPC_TIM0->IR    |= 1; // Clear interrupt flag
    return;
   
}
/*otro ejercicio
*
*
*
**/
#include "lpc17xx_timer.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_pinsel.h"

#define OUTPUT     (uint8_t) 1
#define INPUT      (uint8_t) 0

#define PIN_22    ((uint32_t)(1<<22)) // pin 22 del puerto 0
#define PORT_ZERO  (uint8_t) 0
#define PORT_ONE   (uint8_t) 1
#define PORT_TWO   (uint8_t) 2
#define PORT_THREE (uint8_t) 3

void config_GPIO(void);
void config_timer(void);

int main(void) {
    config_GPIO();
    config_timer();
    GPIO_Setvalue(PORT_ZERO, PIN_22); // P0.22 en alto
    while(1);
    return 0;
}
void config_GPIO(void) {
    PINSEL_CFG_Type pin_configuration;
    /* Configuramos la variable de configuracion del pin que despues 
    vamos a pasar a la funcion PINSEL_ConfigPin() que si configura el periferico */
    pin_configuration.Portnum = PINSEL_PORT_0;
    pin_configuration.Pinnum  = PINSEL_PIN_22;
    pin_configuration.Pinmode = PINSEL_PINMODE_PULLUP;
    pin_configuration.Funcnum = PINSEL_FUNC_0; // GPIO
    pin_configuration.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&PinCfg);
    // Configuro el pin P0.22 como salida
    GPIO_SetDir(PORT_ZERO, PIN_22, OUTPUT); // P0.22 como salida
    return;
}

void GPIO_SetDir(uint8_t portNum, uint32_t bitValue, uint8_t dir) {
    LPC_GPIO_TypeDef *pGPIO = GPIO_GetPointer(portNum);
    if (pGPIO != NULL) {
        if(dir){
        pGPIO->FIODIR |= bitValue; // pongo en 1 el/los bits que quiero que sean salida
    } else {
        pGPIO->FIODIR &= ~bitValue; // pongo en 0 el/los bits que quiero que sean entrada
    }
    return;
}
void config_timer(void) {
    TIM_TIMERCFG_Type struct_config;/* Considera como miembro de la estructura el campo PrescaleOption y PrescaleValue
    (configura si el prescaler es en microsegundos(TIM_PRESCALE_USVAL) o en ciclos de reloj(TIM_PRESCALE_TICKVAL) y el valor del prescaler)*/
    TIM_MATCHCFG_Type struct_match;

    struct_config.PrescaleOption = TIM_PRESCALE_USVAL; // valor del prescaler en microsegundos
    struct_config.PrescaleValue  = 100; // cada 100us se incrementa el timer

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config); // inicializo el timer0 en modo timer con la configuracion dada

    struct_match.MatchChannel = 0; // match0
    struct_match.IntOnMatch   = TRUE; // habilito la interrupcion por match
    struct_match.ResetOnMatch = TRUE; // reseteo el timer cuando ocurre el match
    struct_match.StopOnMatch  = FALSE; // no detengo el timer cuando ocurre el match
    struct_match.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE; // hago un toggle en la salida externa cuando ocurre el match
    struct_match.MatchValue   = 5000; // cada 500ms hago un toggle (T=(1/PCLK)*(PR+1)*(MR0+1)=0.5seg; PCLK=CCLK=100MHz, PR=100, MR0=5000)
    
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &struct_config); // inicializo el timer0 en modo timer con la configuracion dada
    TIM_ConfigMatch(LPC_TIM0, &struct_match); // configuro el match del timer0 con la configuracion dada

    // Enable Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);

    TIM_Cmd(LPC_TIM0, ENABLE); // habilito el timer0
    return;
}
void TIMER0_IRQHandler(void) {
   if (GPIO_ReadValue(PORT_ZERO) &PIN_22)// leo el valor del pin P0.22 y selecciono el bit 22
   {
    GPIO_Clearvalue(PORT_ZERO, PIN_22); // P0.22 en bajo    
    }else {
    GPIO_SetValue(PORT_ZERO, PIN_22); // P0.22 en alto    
    }
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT); // Clear interrupt flag
    return;
   
}
} 
/*nuevo ejercicio


*/
void confGPIO(void);
void confADC(void);
void confTimer(void);
void retardo(void);

int main(void) {
    confGPIO();
    confADC();
    confTimer();
    while(1){
        LPC_GPIO0->FIOSET = (1<<22); // P0.22 en alto
        retardo();
        LPC_GPIO0->FIOCLR = (1<<22); // P0.22 en bajo
        retardo();
    }
    return 0;
}
void retardo(void){
    uint32_t conta;
    for(conta=0; conta<1000000; conta++);
    return;
}
void confGPIO(void){
    LPC_GPIO0->FIODIR |= (1<<22) | (1<<9); // P0.22 como salida y P0.9 como salida
    return;
}
void confADC(void) {
    LPC_SC->PCONP |= (1 << 12); // Habilito el periferico ADC (PCADC)
    LPC_ADC->ADCR |= (1 << 21); // ADC activado
    LPC_SC->PCLKSEL0 |= (3 << 24);// PCLK_ADC = CCLK/8 = 12.5MHz (frecuencia reloj a la que puede trabajar el ADC)
    LPC_ADC->ADCR &= ~(255<<8); // ( bits [15:8] CLKDIV = 255 + 1 = 256 (para que el ADC trabaje a 48.8kHz, secuencia de trabajo final) (divido por 1 )
    LPC_ADC->ADCR &= ~(1 << 16);// modo burst desactivado
    //Los bits [26:24] son los que seleccionan la fuente de inicio de la conversion (START)
    LPC_ADC->ADCR |= (1<<26);//utilizo el match01 del timer0 para iniciar la conversion(comienza cuando hay flanco de subida en el bit 27)
    LPC_ADC->ADCR &= ~(3<<24);// selecciono el canal 0 del ADC (AD0.0)
    LPC_ADC->ADCR |= (1 << 27);// EDGE=1 para que tome el flanco de subida del match0 del timer0
    LPC_PINCON->PINMODE1 |= (1<<15)// P0.23 sin pull-up ni pull-down(entrada analogica)
    LPC_PINCON->PINSEL1 |= (1<<14);// P0.23 como funcion AD0.0
    LPC_ADC->ADINTEN = 1; // Habilito la interrupcion por cada conversion
    NVIC_EnableIRQ(ADC_IRQn); // habilito las interrupciones del ADC
    return;
}
void confTimer(void) {
    // Power up Timer0
    LPC_SC->PCONP |= (1 << 1); // disminuye el consumo de energia y prende el periferico

    // Set PCLK for Timer0 to CCLK
    LPC_SC->PCLKSEL0    |= (1 << 2);// indica el reloj que va a llegar al periferico PLCK=CCLK
    //LPC_PINCON->PINSEL3 |= (1 << 24); // P0.25 como funcion de Timer0  (no la voy  sacar al valor en la salida de un pin por eso la dejo comentada)
    
    LPC_TIM0->EMR       |= (3 << 6); // match1 genera una señal externa toggle en match1
    LPC_TIM0->MR1 = 100000000; // Seteo el match valuado en 1seg a 100MHz=PCLK // T=(1/PCLK)*(PR+1)*(MR0+1)=1seg; PCLK=CCLK=100MHz, PR=0, MR1=100M

    // Interrupt and reset on MR0
    LPC_TIM0->MCR       |= (1<<4);// reset on match1
    //LPC_TIM0->MCR       &= ~(1<<0);// interrupcion deshabilitada que genere el timer
    //LPC_TIM0->MCR       |= ~(1<<0);// pongo en 1 si quiero que cuando el valor de timer sea igual al valor del match 0 se genere una interrupcion
    // Start Timer0
    LPC_TIM0->TCR       = 3; // Reset and enable timer
    LPC_TIM0->TCR       &=~(1<<1);// quitar el reset
    // Enable Timer0 interrupt
    NVIC_EnableIRQ(TIMER0_IRQn);
}
/* en este codigo hicimos que la tiempo de muestreo sea de 2segundos y la frecuencia de 0.5Hz 
por que el timer0 hace un toggle cada 1seg y cada vez que hace un toggle inicia una conversion del ADC*/
 void ADC_IRQHandler(void) {
    static uint16_t ADC0Value=0;
    float volt=0;
    ADC0Value = ((LPC_ADC->ADDR0)>>4) & 0xFFF; // variable auxiliar para observar el valor convertido (hacemos una mascara para quedarnos con los bits [15:4] RESULT y no usar las otras funciones del ADC)
    if (ADC0Value<2055) { 
        LPC_GPIO0->FIOSET |= (1<<9); // P0.9 en alto
    } else {
        LPC_GPIO0->FIOCLR |= (1<<9); // P0.9 en bajo
    }
    volt=(ADC0Value/4096)*3.3; // si quiero calcular el voltaje en base al valor digital (3.3V es la referencia y 4096=2^12)
    return;
}