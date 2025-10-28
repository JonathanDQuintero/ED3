
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
/*ejemplo de configuracion de DMA para que mande datos 
de memoria a memoria
* ese codigo genera por datos obtenidos del adc 2 arrays de 10bits los cuales no puede ser mayor 1034 elemetos, los cuales el valor de estos 
son utilizados para decir desde que posicion se intercambian 100 elementos(posiciones) de table
*
*
*
*/
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h" 
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"

#define DMA_SWAP_SIZE 100 // cantidad de datos a transferir
#define TABLE_LEN 1034 // cantidad de datos en la tabla

void confDMA(void);
void confADC(void);//prototipo de la funcion de configuracion de interrupciones externas

GPDMA_Channel_CFG_Type GPDMACfg;//estructura de configuracion del canal DMA

uint32_t table[TABLE_LEN];//tabla de datos origen
uint32_t random[2];//tabla de datos destino

uint32_t aux_buffer[DMA_SWAP_SIZE];//buffer auxiliar para hacer la transferencia de datos

GPDMA_LLI_Type DMA_LLI_Struct1;//estructura de la primera lista de transferencia
GPDMA_LLI_Type DMA_LLI_Struct2;//estructura de la segunda lista de transferencia
GPDMA_LLI_Type DMA_LLI_Struct3;//estructura de la tercera lista de transferencia

uint32_t bitcount;//contador de bits transferidos

int main(void){
    uint32_t i;
    //les doy un valor inicial random para que no este con cualquier valor y pueda llegar a alterar el codigo
    random[0]=545;//tabla de datos destino
    random[1]=433;//tabla de datos destino
    bitcount=0;//inicializo el contador de bits transferidos
    //preparacion de la tabla de datos origen
    for(i=0;i<TABLE_LEN;i++){
        table[i]=i;//cargo la tabla de datos origen
    }
    confADC();//configuro las interrupciones externas
    while(1);
    return 0;
}
void confDMA(void) {
    // configuracion de la primera estructura LLI
    DMA_LLI_Struct1.SrcAddr = (uint32_t)table+4*random[0];//direccion de la fuente de datos (puntero hacia la direccion de memoria)
    DMA_LLI_Struct1.DstAddr = (uint32_t)aux_buffer;//direccion de destino (puntero hacia la direccion de memoria)
    DMA_LLI_Struct1.NextLLI = (uint32_t)&DMA_LLI_Struct2;//direccion de la siguiente estructura LLI
    DMA_LLI_Struct1.Control = DMA_SWAP_SIZE    
                            | (2 << 18)//ancho de burst de la fuente 32 bits 
                            | (2 << 21)//ancho de burst del destino 32 bits
                            | (1 << 26)//incremento de la fuente
                            | (1 << 27)//incremento del destino
                            ;
    // configuracion de la segunda estructura LLI
    DMA_LLI_Struct2.SrcAddr = (uint32_t)table+4*random[1];//direccion de la fuente de datos (puntero hacia la direccion de memoria)
    DMA_LLI_Struct2.DstAddr = (uint32_t)table+4*random[0];//direccion de destino (puntero hacia la direccion de memoria)
    DMA_LLI_Struct2.NextLLI = (uint32_t)&DMA_LLI_Struct3;//direccion de la siguiente estructura LLI
    DMA_LLI_Struct2.Control = DMA_SWAP_SIZE    
                            | (2 << 18)//ancho de burst de la fuente 32 bits 
                            | (2 << 21)//ancho de burst del destino 32 bits
                            | (1 << 26)//incremento de la fuente
                            | (1 << 27)//incremento del destino
                            ;
    // configuracion de la tercera estructura LLI
    DMA_LLI_Struct3.SrcAddr = (uint32_t)aux_buffer;//direccion de la fuente de datos (puntero hacia la direccion de memoria)
    DMA_LLI_Struct3.DstAddr = (uint32_t)table+4*random[1];//direccion de destino (puntero hacia la direccion de memoria)
    DMA_LLI_STRUCT3.NEXTLLI=0;//ultima transferencia no apunta a ninguna otra
    DMA_LLI_Struct3.Control = DMA_SWAP_SIZE    
                            | (2 << 18)//ancho de burst de la fuente 32 bits 
                            | (2 << 21)//ancho de burst del destino 32 bits
                            | (1 << 26)//incremento de la fuente
                            | (1 << 27)//incremento del destino
                            ;
    //GPDMA SECCION BLOQUE
    //inicializacion del GPDMA
    GPDMA_Init();
    //Configuracion del canal GPDMA
    //canal 7
    GPDMACfg.ChannelNum = 7; // canal 7
    //memoria fuente
    GPDMACfg.SrcMemAddr = DMA_LLI_STRUCT1.SrcAddr; // direccion de la fuente de datos (puntero hacia la direccion de memoria)
    //memoria destino
    GPDMACfg.DstMemAddr = DMA_LLI_STRUCT1.DstAddr; // direccion de destino (puntero hacia la direccion de memoria)
    //cantidad de datos a transferir
    GPDMACfg.TransferSize = DMA_SWAP_SIZE; // cantidad de datos a transferir(cuantas posiciones de memoria se van a transferir)
    //ancho de los datos a transferir (0=8bits,1=16bits,2=32bits)
    GPDMACfg.TransferWidth = GPDMA_WIDTH_WORD;// ancho de los datos a transferir (0=8bits,1=16bits,2=32bits)
    //tipo de transferencia (memoria a memoria)
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2M; // tipo de transferencia (memoria a memoria)
    //funete de conexion (no se usa porque la fuente es memoria)
    GPDMACfg.SrcConn = 0; // no se usa porque la fuente es memoria
    //destino de conexion (no se usa porque el destino es memoria)
    GPDMACfg.DstConn = 0; // no se usa porque el destino es memoria
    //LINKED LIST ITEM (no se usa porque no armamos varias listas entonces recorremos siempre la misma lista)
    GPDMACfg.DMALLI=(uint32_t)&DMA_LLI_STRUCT2; // puntero a la estructura LLI(esto por que no armamos varias listas entonces recorremos siempre la misma lista)
    // configuracion del canal con parametros dados
    GPDMA_Setup(&GPDMACfg); // configuracion del canal
    return;
}
void confADC(void){
    ADC_Init(LPC_ADC, 200000); // inicializo el ADC a 200kHz
    LPC_SC->PCONP |= (1 << 12); // Habilito el periferico ADC (PCADC)
    LPC_ADC->ADCR |= (1 << 21); // ADC activado
    LPC_SC->PCLKSEL0 |= (3 << 24);// PCLK_ADC = CCLK/8 = 12.5MHz (frecuencia reloj a la que puede trabajar el ADC)
    LPC_ADC->ADCR &= ~(255<<8); // ( bits [15:8] CLKDIV = 255 + 1 = 256 (secuencia de trabajo final)
    LPC_ADC->ADCR |= (1 << 16); // modo burst(si la frecuencia de muestreo es de 10khz si son 2 canales 5khz cada uno, como es 1 canal a 10khz)
    LPC_PINCON->PINMODE1 |= (1<<15);// P0.23 sin pull-up ni pull-down
    LPC_PINCON->PINSEL1 |= (1<<14);// P0.23 como funcion AD0.0
    LPC_ADC->ADINTEN = 1; // habilito la interrupcion por cada conversion
    NVIC_SetPriority(ADC_IRQn,3);//prioridad 3
    NVIC_EnableIRQ(ADC_IRQn); // habilito las interrupciones del ADC
    return;
}
void ADC_IRQHandler(void){
    stactic uint16_t ADC_Value=0;//variable estatica para que mantenga su valor entre interrupciones
    ADC_Value = ((LPC_ADC->ADDR0)>>4) & 0x001;// variable auxiliar para observar el valor convertido (hacemos una mascara para quedarnos con los bits [15:4] RESULT y no usar las otras funciones del ADC)
   //0x001 es una mascara para quedarnos con los 2bits menos significativos que sirven para elegir la posicion de memoria de la tabla
   if(bitcount<10){
        if(bitcount==0){
            random[0]=0;//inicializo la primer posicion de memoria
            }
            random[0]=(random[0]<<1)|ADC_Value;//carga el valor de la comparacion entre el valor de ADC_Value y random[0]
            bitcount++;
        
   }else{
        if(bitcount==10){
                random[1]=0;
        }
        random[1]=(random[1]<<1)|ADC_Value;
        bitcount++;
        if(bitcount==20){
            bitcount=0;
            random[1]=random[1]%1024;
            random[0]=random[0]%1024;
            NVIC_DisableIRQ(DMA_IRQn);
            confDMA();
            //habilitacion de GPDMA canal 5
            GPDMA_ChannelCmd(7,ENABLE);

        }
   }
   return;
}




















