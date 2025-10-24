/**
 * Ejercicio N 1: (40%)
Por un pin del ADC del microcontrolador LPC1769 ingresa una tensión de rango dinámico 0 a 3,3[v] 
proveniente de un sensor de temperatura. Debido a la baja tasa de variación de la señal,
se pide tomar una muestra cada 30[s]. Pasados los 2[min] se debe promediar las últimas 
 4 muestras y en función de este valor, tomar una decisión sobre una salida digital de la placa:
● Si el valor es <1 [V] colocar la salida en 0 (0[V]).
● Si el valor es >= 1[V] y <=2[V] modular una señal PWM con un Ciclo de trabajo que va desde 
el 50% hasta el 90% proporcional al valor de tensión, con un periodo de 20[KHz].
● Si el valor es > 2[V] colocar la salida en 1 (3,3[V]).
 */

#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"

#define FRECUENCIA 20

uint16_t valorADC=0;
uint16_t promedio=0;
uint32_t ciclo_trabajo=1;//POR AHORA IGUAL QUE TIMER
void configADC(void);
void configTimer00(void);
void configTimer01(void);
void configPCB(void);

int main(){
    
    SystemInit();
    configPCB();
    configADC();
    configTimer00();
    configTimer01();
    while(1) {}
    return 0;
}

void configPCB(void) {
    //SALIDA
    PINSEL_CFG_Type pinSALIDA;
    pinSALIDA.portNum = 1;
    pinSALIDA.pinNum = 9;
    pinSALIDA.funcNum = 0;
    pinSALIDA.pinMode = PINSEL_TRISTATE;
    pinSALIDA.openDrain = PINSEL_OD_NORMAL;
    PINSEL_ConfigPin(&pinSALIDA);

   /* PINSEL_CFG_Type pinTMR0;
    pinTMR0.portNum = 1;
    pinTMR0.pinNum = 28;    // match 0.0
    pinTMR0.funcNum = 3;
    pinTMR0.pinMode = PINSEL_PULLDOWN;
    pinTMR0.openDrain = PINSEL_OD_NORMAL;
    PINSEL_CFG_Type(&pinTMR0);*/
TIM_PinConfig(TIM_MAT0_0_P1_28);

    /*PINSEL_CFG_Type pinTMR1;
    pinTMR1.portNum = 1;
    pinTMR1.pinNum = 22;    // match 1.0
    pinTMR1.funcNum = 3;
    pinTMR1.pinMode = PINSEL_PULLDOWN;
    pinTMR1.openDrain = PINSEL_OD_NORMAL;
    PINSEL_ConfigPin(&pinTMR1);*/
 TIM_PinConfig(TIM_MAT0_0_P1_22);
}

//para leer adc
void configTimer00(void) {
    TIM_TIMERCFG_type timer0 = {0};
    TIM_MATCHCFG_Type match0={0};

    timer0.PrescaleOption = TIM_PRESCALE_USVAL ;
    timer0.PrescaleValue =  50//50US // COINCIDE CON EL PERIODO DE MI SEÑAL PWM
     //###########################
    match0.matchvalue = 600000;//30segundos // TIMEPO PARA CONVERTIR CON ADC
    match0.extMatchOutputType = TIM_NOTHING;
    match0.matchChannel;= TIM_MATCH_0;
    match0.IntOnMatch = ENABLE;//HABILITO INTERRUPCION
    match0.ResetOnMatch = ENABLE;
    match0.StopOnMatch = DISABLE;//que no se detenga
    //###########################
    match0.matchvalue = ciclo_trabajo;//TIEMPO QUE VARIO SEGUN EL CICLO DE TRABAJO
    match0.extMatchOutputType = TIM_NOTHING;
    match0.matchChannel;= TIM_MATCH_0;
    match0.IntOnMatch = ENABLE;//HABILITO INTERRUPCION
    match0.ResetOnMatch = ENABLE;
    match0.StopOnMatch = DISABLE;//que no se detenga


    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&timer0);
    TIM_ConfigMatch(LPC_TIM0,&match0);
    NVIC_EnableIRQn(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);
}    

void configADC(void) {
    ADC_Init(FRECUENCIA);
    ADC_ChannelCmd(ADC_CHANNEL_0,ENABLE);//HABILITO CANAL 
    ADC_PinConfig(ADC_CHANNEL_0);//CONFIGURO PIN
    ADC_BurstCmd(ENABLE);//3TIM_TIMER_MODE, &timer00000
    ADC_StartCmd(ADC_START_ON_MAT01);
}

void TIMER0_IRQHandler(void){
    static uint8_t cont = 0;
    static uint8_t salida=0;// toggle para pwm
    //cada vez que entra pasaron 30 segundos
    //pregutnar primero su fue por match0
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
        cont++;
            //leo adc
        valorADC += ADC_ChannelGetData(ADC_CHANNEL_0);
        if(cont == 4){
            //calculo promedio 
            promedio = valorADC / 4;
            valorADC = 0;
            cont = 0;
        }
        //si promedio es menor a 1v
        else if(promedio < 1241){//1v=  1241 ..  3.3v son  4095
            GPIO_ClrValue(1, 1<<9); //salida en 0v
        }
        else if(promedio > 2482){ // 2v = 2482
            GPIO_SetPins(1, 1<<9);//salida en 3.3v
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);//LIMPIO BANDERA MATCH 0
    }


        //si interruppio match 1 yesta entre kos rangos 1v y 2v
    else if (TIM_GetIntStatus(LPC_TIM0, TIM_MR1_INT) && promedio > 1241 && promedio < 2482){ 
            //sacar la pwm. entre el 50% y 90% proporcional al valor de tension
            ciclo_trabajo = (promedio - 1241) * (90 - 50) / (2482 - 1241) + 50;//PARA QUE ESTE ENTRE 50 Y 90%
            TIM_SetMatchExt(LPC_TIM0, TIM_MR1_INT, ciclo_trabajo); // configuro match. AJUSTO TIEMPO
            GPIO_SetPins(1, 1<<9);
            salida=1;
            TIM_ClearIntPending(LPC_TIM0, TIM_MR1_INT);//LIMPIO BANDERA MATCH 1

    }
    else if(salida){
        GPIO_ClearPins(1, 1<<9);
        salida=0;
    }

        //habilito en 0 la salida pwm
        GPIO0_ClrValue(1, 1<<9);


}