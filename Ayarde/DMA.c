
/*utilzamos el DMA para hacer una transferencia de memoria a periferico 
que en este caso es le dac y no vamos a utilizar ninuga interrupcion y handler de DMA*/

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"

#define DMA_SIZE 60 // cantidad de datos a transferir
#define NUM_SINE_SAMPLES 60 // cantidad de muestras de la senoidal
#define SINE_FREQ_IN_HZ 50 // frecuencia de la senoidal
#define PLCK_DAC_IN_MHZ 25 // frecuencia del CCLK/4 

void confPin(void);
void confDMA(void);
void confDac(void);

GPDMA_Channel_CFG_Type GPDMACfg;
uint32_t dac_sine_lut[NUM_SINE_SAMPLES];
int main(void){
    uint32_t i;
    uint32_t sin_0_to_90_16_samples[16] = {\
        0,1045,2079,3090,4067,5000,5877,6691,\
        7431,8090,8660,9135,9510,9781,9945,10000
    };//posiciones de memoria que contienen los valores de la senoidal
    confPin();
    confDac();
    for (i=0;i<NUM_SINE_SAMPLES;i++){
        if(i<=15)
        {
            dac_sine_lut[i]=512+512*sin_0_to_90_16_samples[i]/10000;//0 a 90
            if(i==15) dac_sine_lut[i]=1023;//para asegurar que el valor maximo sea 1023
        }
        else if(i<=30)
        {
            dac_sine_lut[i]=512+512*sin_0_to_90_16_samples[30-i]/10000;//90 a 180
            }
        else if(i<=45)
        {
            dac_sine_lut[i]=512-512*sin_0_to_90_16_samples[i-30]/10000;//180 a 270
        }
        else
        {
            dac_sine_lut[i]=512-512*sin_0_to_90_16_samples[60-i]/10000;//270 a 360
        }
        dac_sine_lut[i]=dac_sine_lut[i]<<6;//corrimiento para que quede en el lugar correcto del registro del DAC
    }
    confDMA();
     // habilito el canal 0
    GPDMA_ChannelCmd(0, ENABLE);//Comienza la transferencia de informacion
    while(1);
    return 0;
}
void confPin(void){
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2; // funcion 2 para el DAC
    PinCfg.OpenDrain = 0; // no es open drain
    PinCfg.Pinmode = 0; // sin pull up ni pull down
    PinCfg.Portnum = 0; // puerto 0
    PinCfg.Pinnum = 26; // pin 26
    PINSEL_ConfigPin(&PinCfg);
    return;
}
void confDMA(void) {
    GPDMA_LLI_Type DMA_LLI_Struct;
    // direccion de la fuente de datos(puntero hacia la direccion de memoria)
    DMA_LLI_Struct.SrcAddr = (uint32_t)dac_sine_lut;
    //direccion de destino (puntero hacia el periferico) que es la direccion asociada al DAC(tambien puede ser memoria)
    DMA_LLI_Struct.DstAddr = (uint32_t)&(LPC_DAC->DACR);
    // indicamos que la nueva direccion de la siguiente estructura es ella misma (ciclo infinito)
    DMA_LLI_Struct.NextLLI = (uint32_t)&DMA_LLI_Struct;
    // configuramos el tamaño de los datos a transferir (8,16,32 bits) y el registro de control 
    DMA_LLI_Struct.Control = DMA_SIZE    
                            | (2 << 18)//ancho de burst de la fuente 32 bits 
                            | (2 << 21)//ancho de burst del destino 32 bits
                            | (1 << 26)//incremento de la fuente
                            ;
    GPDMA_Init();
    //Configuracion del canal GPDMA
    GPDMACfg.ChannelNum = 0; // canal 0
    GPDMACfg.SrcMemAddr = (uint32_t)(dac_sine_lut); // direccion de la fuente de datos (puntero hacia la direccion de memoria)
    GPDMACfg.DstMemAddr = 0; // no se usa porque el destino es un periferico
    GPDMACfg.TransferSize = DMA_SIZE; // cantidad de datos a transferir(cuantas posiciones de memoria se van a transferir al DAC) 
    GPDMACfg.TransferWidth = 2; // ancho de los datos a transferir (0=8bits,1=16bits,2=32bits)
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P; // tipo de transferencia (memoria a periferico)
    GPDMACfg.SrcConn = 0; // no se usa porque la fuente es memoria
    GPDMACfg.DstConn = GPDMA_CONN_DAC; // conexion del destino (DAC)
    GPDMACfg.DMALLI = (uint32_t)&DMA_LLI_Struct; // puntero a la estructura LLI( esto por que no armamos varias listas entonces recorremos siempre la misma lista)
    GPDMA_Setup(&GPDMACfg); // configuracion del canal
    return;
}
void confDac(void){
    uint32_t tmp;
    DAC_CONVERTER_CFG_Type DAC_ConverteConfigStruct;//inicializacion del dac junto con el DMA
    /* para que el dac funcione con DMA y pase esas muetras hay que asociar al DAC un timer 
    o el reloj interno del dac el cual organiza el tiempo con el que se envian las muestras al DAC 
    que es lo que hacemos a continuacion*/
    DAC_ConverteConfigStruct.CNT_ENA = SET; // Habilito el contador
    DAC_ConverteConfigStruct.DMA_ENA = SET; // Habilito el DMA
    DAC_INIT(LPC_DAC); // setea el que genera mayor frecuencia de funcionamiento que le maximo es 1MHz
    //seteo el tiempo de la salida del DAC
    tmp = (PLCK_DAC_IN_MHZ * 1000000 / (SINE_FREQ_IN_HZ * NUM_SINE_SAMPLES));//calculo el tiempo en us que tiene que tener cada muestra para obtener la frecuencia deseada
    DAC_SetDMATimeOut(LPC_DAC, tmp);//tiempo al cual el timer va a desbordar y generar la nueva muestra
    DAC_ConfigDAConverterControl(LPC_DAC, &DAC_ConverteConfigStruct);
    return;
}

























/*confirguracion de DMA con drivers*/
int main(){
    confDMA();
     // habilito el canal 0
    GPDMA_ChannelCmd(0, ENABLE);//Comienza la transferencia de informacion
    while(1);
    return 0;
}
