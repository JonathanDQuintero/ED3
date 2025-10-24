/*Ejercicio 1 - Parcial 2022*/
/*
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice 
 dos señales analógicas cuyos anchos de bandas son de 10 Khz cada una. 
 Los canales utilizados deben ser el 2 y el 4 y los datos deben ser guardados 
 en dos regiones de memorias distintas que permitan contar con los últimos 20 
 datos de cada canal. Suponer una frecuencia de core cclk de 100 Mhz. 
 El código debe estar debidamente comentado.
*/

#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"

/* ADC channels */
// AD0.2 P0.25
// Function 01
// AD0.4 P1.30 

// Function 11

/* LLIs */
#define DIRECCION1 0x 
#define DIRECCION2 0x
#define FRECUENCIA 40000 // 40 KHz F DE MUESTRO
uint16_t *bufferADC1 = (uint16_t *)DIRECCION1;
uint16_t *bufferADC2 = (uint16_t *)(DIRECCION2); 

void cfgPin(void);
void cfgADC(void);
void cfgDMA1(void);
void cfgDMA2(void);

int main(void){
    cfgPin();
    cfgADC();
    cfgDMA();
    while(1){
    }
    return 0;
} 
void cfgPin(void){
    /*PINSEL_CFG_Type PinCfg;
    //p0.25 AD0.2 y p1.30 AD0.4
    PinCfg.PINSEL_FUNC = PINSEL_FUNC_1;//para p0.25
    PinCfg.PINSEL_PIN = PINSEL_PIN_25;
    PinCfg.PINSEL_PORT = PINSEL_PORT_0;
    PinCfg.PINSEL_PINMODE = PINSEL_TRISTATE;
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.PINSEL_FUNC = PINSEL_FUNC_3;//para p1.30
    PinCfg.PINSEL_PIN = PINSEL_PIN_30;
    PinCfg.PINSEL_PORT = PINSEL_PORT_1;
    PinCfg.PINSEL_PINMODE = PINSEL_TRISTATE;
    PINSEL_ConfigPin(&PinCfg);

    */
   // ESTA ESTA OPCION O CONFIGURARLO DIRECTAMENTA CON DRIVERS DE ADC
}

void cfgADC(void) {
    ADC_init(FRECUENCIA); // para cumplir con el criterio de Nyquist: Fsh = 2*Fmax*Num_Channels
    ADC_BurstCmd(ENABLE);
    ADC_ChannelCmd(ADC_CHANNEL_2, ENABLE);//habilito los canales
    ADC_ChannelCmd(ADC_CHANNEL_4, ENABLE);
    
    ADC_PinConfig(ADC_CHANNEL_2);//CONFIGURA LOS PINES ESPECIFICO PARA TALES CANEALES
    ADC_PinConfig(ADC_CHANNEL_4);//AHORRO LINEAS DE CODIGO
//sin startcdd porque se activa por DMA
    
}


void cfgDMA1(void) {
    GPDMA_LLI_Type LLI;
    LLI.srcAddr = (uint16_t)&(LPC_ADC->ADDR2);
    LLI.dstAddr = (uint16_t)&bufferADC1;
    LLI.nextLLI = (uint16_t)&LLI;
    LLI.control = ;
    
    GPDMA_Channel_CFG_Type  cfgDMA; 
    cfgDMA.channelNum = GPDMA_CHANNEL_0;
    cfgDMA.transferSize = 320; // 20 muestras por canal
    cfgDMA.transferWidth = GPDMA_HALFWORD;
    cfgDMA.srcMemAddr = (uint32_t)&(LPC_ADC->ADDR2);
    cfgDMA.dstMemAddr =  (uint32_t)bufferADC1;
    cfgDMA.transferType = GPDMA_P2M;
    cfgDMA.srcConn = GPDMA_ADC;
    cfgDMA.dstConn = 0;
    cfgDMA.linkedList = LLI;
}

void cfgDMA2(void) {
    GPDMA_LLI_Type LLI;
    LLI.srcAddr = (uint16_t)&(LPC_ADC->ADDR4);
    LLI.dstAddr = (uint16_t)&bufferADC1;
    LLI.nextLLI = (uint16_t)&LLI;
    LLI.control = ;
    
    GPDMA_Channel_CFG_Type  cfgDMA; 
    cfgDMA.channelNum = GPDMA_CHANNEL_0;
    cfgDMA.transferSize = 320; // 20 muestras por canal
    cfgDMA.transferWidth = GPDMA_HALFWORD;
    cfgDMA.srcMemAddr = (uint32_t)&(LPC_ADC->ADDR4);
    cfgDMA.dstMemAddr =  (uint32_t)bufferADC2;
    cfgDMA.transferType = GPDMA_P2M;
    cfgDMA.srcConn = GPDMA_ADC;
    cfgDMA.dstConn = 0;
    cfgDMA.linkedList = LLI;
}