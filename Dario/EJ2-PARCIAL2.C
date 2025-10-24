/**
Programar el microcontrolador LPC1769 en un código de lenguaje C para que mediante su ADC
digitalice una señal analógica cuyo ancho de banda es de 16 khz. La señal analógica tiene una
amplitud de pico máxima positiva de 3.3 voltios. Los datos deben ser guardados utilizando el
Hardware GDMA en la primera mitad de la memoria SRAM ubicada en el bloque AHB SRAM -
bank 0, de manera tal que permita almacenar todos los datos posibles que esta memoria nos
permita. Los datos deben ser almacenados como un buffer circular conservando siempre las últimas
muestras.
Por otro lado se tiene una forma de onda como se muestra en la imagen a continuación. Esta señal
debe ser generada por una función y debe ser reproducida por el DAC desde la segunda mitad de
AHB SRAM - bank 0 memoria utilizando DMA de tal forma que se logre un periodo de 614us
logrando la máxima resolución y máximo rango de tensión.
Durante operación normal se debe generar por el DAC la forma de onda mencionada como
wave_form. Se debe indicar cuál es el mínimo incremento de tensión de salida de esa forma de onda.
Cuando interrumpe una extint conectada a un pin, el ADC configurado debe completar el ciclo de
conversión que estaba realizando, y ser detenido, a continuación se comienzan a sacar las muestras del ADC 
por el DAC utilizando DMA
y desde las posiciones de memoria originales.
Cuando interrumpe nuevamente en el mismo pin, se vuelve a repetir la señal del DAC generada por la forma de 
onda de wave_form 
previamente almacenada y se arranca de nuevo la conversión de datos del adc. Se alterna así entre
los dos estados del sistema 
con cada interrupción externa.
Suponer una frecuencia de core cclk de 80 Mhz. El código debe estar debidamente comentado.
 */

#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_exti.h"

#define FRECUENCIA  32000;  //F=16Khz ==>f_muestreo= 32Khz
uint32_t *bufferStart = (uint16_t *)0x2007C000; //Inicio de la SRAM bank 0   .. duda si va 2007 nomas
uint32_t *bufferEnd = (uint16_t *)0x2007DFFF; //Final de la 1ra mitad de la SRAM bank 0
uint32_t *buffer1Start = (uint16_t *)0x2007E000; //Inicio de la 2da mitad de la SRAM bank 0
uint32_t *buffer1End = (uint16_t *)0x2007FFFF; //Final de la 2da mitad de la SRAM bank 0
 uint8_t cont=1;
#define muestras  1024; //cantidad de muestras de la señal
uint16_t onda[muestras]{
//señal  de la figura 0     1024
    for(int i=512; i<1023;i++){
        DAC_UpdateValue(i);
    }

    for(int i=0; i<512;i++){
        DAC_UpdateValue(i);
    }
}
uint16_t onda* = onda;


void cfgpin(void);
void cfgAdc(void);
void cfgDac(void);
void DmaAdc(void);
void cfgExtInt(void);
void DmaDac1(void);
void DmaDac2(void);

int main(void){
    cfgAdc();
    cfgDac();
    DmaAdc();
    DmaDac1();
    DmaDac2();
    cfgpin();
    while (1) {
        __WFI();
    }
}

void cfgAdc(void){
    //p0.23 adc 
    ADC_Init(FRECUENCIA);//ya lo enciende 
    ADC_PinConfig(ADC_CHANNEL_0);
    ADC_BurstCmd(ENABLE);
    //CONECTAR CON DMA, NO USO START CMD ACA
}

void DmaAdc(void){
    //CANAL 0 .TRANSFIERE DE ADC A MEMORIA BANCO 0
    GPDMA_Init();
    GPDMA_LLI_Type LLIdac;
    LLIdac.srcAddr = (uint32_t)LPC_ADC->ADGDR;
    LLIdac.dstAddr = (uint16_t)bufferStart;
    LLIdac.nextLLI = LLIdac;
    LLIdac.control =   4096 
                    | (1<<15)//recibo  de a 4 datos 
                    | (1<<18) //16bits fuente
                    | (1<<21) //16 bits destino
                    | (0<<26) //no incremento la fuente
                    | (1<<27)//incremento el destino

    GPDMA_Channel_CFG_Type CfgDmaAdc{0};
    CfgDmaAdc.channelNum = GPDMA_CHANNEL_0;
    CfgDmaAdc.transferSize = 4096;
    CfgDmaAdc.srcMemAddr = (uint32_t)LPC_ADC->ADGDR;
    CfgDmaAdc.dstMemAddr = (uint16_t)bufferStart;
    CfgDmaAdc.srcConn = GPDMA_ADC;
    CfgDmaAdc.dstConn = 0;
    CfgDmaAdc.transferWidth = GPDMA_HALFWORD;
    CfgDmaAdc.transferType = GPDMA_P2M;
    CfgDmaAdc.linkedList = LLIdac;

    GPDMA_Setup(&cfgDmaAdc);
    //empiezo habilitado?
    GPDMA_ChannelCmd(ENABLE);
}

void DmaDac1(void){
    //CANAL 1 TRANSFIERE DE MEMORIA BANCO 1 A DAC
    GPDMA_Init();
    GPDMA_LLI_Type LLIdac;
    LLIdac.srcAddr = (uint16_t)buffer1Start;
    LLIdac.dstAddr = (uint32_t)LPC_DAC->DACR;
    LLIdac.nextLLI = (uint32_t)LLIdac;
    LLIdac.control =   1024 
                    | (1<<15)//recibo  de a 4 datos 
                    | (1<<18) //16bits fuente
                    | (1<<21) //16 bits destino
                    | (0<<26) //no incremento la fuente
                    | (0<<27)//NO incremento el destino

    GPDMA_Channel_CFG_Type CfgDmaDac{0};
    CfgDmaDac.channelNum = GPDMA_CHANNEL_1;
    CfgDmaDac.transferSize = 1024;
    CfgDmaDac.srcMemAddr = (uint16_t)buffer1Start;//BANCO 1 SACA LA SEÑAL
    CfgDmaDac.dstMemAddr = (uint32_t)LPC_DAC->DACR;
    CfgDmaDac.srcConn = 0;
    CfgDmaDac.dstConn = LPC_DAC->DACR;
    CfgDmaDac.transferWidth = GPDMA_HALFWORD;
    CfgDmaDac.transferType = GPDMA_M2P;
    CfgDmaDac.linkedList = LLIdac;

    GPDMA_Setup(&cfgDmaDac);
    //empiezo habilitado?
    GPDMA_ChannelCmd(ENABLE);
}
void DmaDac2(void){
    //CANAL 2 TRANSFIERE DE MEMORIA BANCO 0 A DAC
    GPDMA_Init();
    GPDMA_LLI_Type LLIdac2;
    LLIdac2.srcAddr = (uint16_t)bufferStart;//BANCO 0
    LLIdac2.dstAddr = (uint32_t)LPC_DAC->DACR;
    LLIdac2.nextLLI = (uint32_t)LLIdac2;
    LLIdac2.control =   1024 
                    | (1<<15)//recibo  de a 4 datos 
                    | (1<<18) //16bits fuente
                    | (1<<21) //16 bits destino
                    | (0<<26) //no incremento la fuente
                    | (1<<27)//NO incremento el destino.. 

    GPDMA_Channel_CFG_Type CfgDmaDac2{0};
    CfgDmaDac2.channelNum = GPDMA_CHANNEL_2;
    CfgDmaDac2.transferSize = 4096;
    CfgDmaDac2.srcMemAddr = (uint16_t)bufferStart;//BANCO 0 SACA LO QUE DEJO EL ADC
    CfgDmaDac2.dstMemAddr = (uint32_t)LPC_DAC->DACR;
    CfgDmaDac2.srcConn = 0;
    CfgDmaDac2.dstConn = LPC_DAC->DACR;
    CfgDmaDac2.transferWidth = GPDMA_HALFWORD;
    CfgDmaDac2.transferType = GPDMA_M2P;
    CfgDmaDac2.linkedList = LLIdac2;

    GPDMA_Setup(&CfgDmaDac2);
    //empieza desabilitado porque aun no interrumpio
    //GPDMA_ChannelCmd(ENABLE);
}

void cfgDac(void){
    DAC_Init();
    DAC_CONVERTER_CFG_Type DacCfg;
    DacCfg.counterEnable = ENABLE;
    DacCfg.dmaEnable = ENABLE;
    DacCfg.doubleBufferEnable= ENABLE;
    DAC_ConfigDAConverterControl(&DacCfg);
    DAC_SetDMATimeOut(12);// me pide 614us de periodo-->f = 1628hz
    //clock es de 80MHZ
    //(F_CLOCK/4) / (FRECUENCIA * MUESTRAS) = (80000000/4) / (1628*1024)
    // = 12 
}

void cfgExtInt(void){
    EXTI_CFG_Type pinEINT0;
    pinEINT0.line = EXTI_EINT0;
    pinEINT0.mode = EXTI_EDGE_SENSITIVE;
    pinEINT0.polarity = EXTI_RISING_EDGE;
    EXTI_Config(&pinEINT0);
    NVIC_EnableIRQ(EINT0_IRQn);
    NVIC_SetPriority(EINT0_IRQn, 0);
}

void EINT0_IRQHandler (void){
    if(cont){//si es 1. empieza con 1
        //interruppe 1 vez y cambia de canal. saca por el dac lo que cargo en el banco 0.
        // canal 0 de desabilitado. canal 1 desabilitado y canal 2 habilitado.
        GPDMA_ChannelCmd(GPDMA_CHANNEL_0, DISABLE);
        GPDMA_ChannelCmd(GPDMA_CHANNEL_1, DISABLE);
        GPDMA_ChannelCmd(GPDMA_CHANNEL_2, ENABLE);
        cont=0;
    }
    else{
    //interrumpe otra vez. vuelve como antes. canal 3 desabilitado y 1 y 2 habilitado
        GPDMA_ChannelCmd(GPDMA_CHANNEL_0, ENABLE);
        GPDMA_ChannelCmd(GPDMA_CHANNEL_1, ENABLE);
        GPDMA_ChannelCmd(GPDMA_CHANNEL_2, DISABLE);
        cont=1;
    }
    //limpio bandera
    EXTI_ClearFlag(EXTI_EINT0);
}