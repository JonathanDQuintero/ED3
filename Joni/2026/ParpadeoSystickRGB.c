#include "LPC17xx.h"

#define RED_LED     22
#define GREEN_LED   25
#define BLUE_LED    26

volatile uint32_t i = 250;
volatile uint8_t color = 0;

void confPCB(void);
void confSysTick(void);
void SysTick_Handler(void);

int main(void)
{
    confPCB();
    confSysTick();

    while(1)
    {
        // Todo lo hace la interrupción SysTick
    }
}


/*---------------- CONFIGURACIÓN GPIO ----------------*/

void confPCB(void)
{
    // -------- ROJO P0.22 --------
    // Función GPIO
    LPC_PINCON->PINSEL1 &= ~(3 << 12);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE1 &= ~(3 << 12);
    LPC_PINCON->PINMODE1 |=  (2 << 12);

    // Salida
    LPC_GPIO0->FIODIR |= (1 << RED_LED);


    // -------- VERDE P3.25 --------
    // P3.25 corresponde a bits 18-19 de PINSEL7
    LPC_PINCON->PINSEL7 &= ~(3 << 18);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE7 &= ~(3 << 18);
    LPC_PINCON->PINMODE7 |=  (2 << 18);

    // Salida
    LPC_GPIO3->FIODIR |= (1 << GREEN_LED);


    // -------- AZUL P3.26 --------
    // P3.26 corresponde a bits 20-21
    LPC_PINCON->PINSEL7 &= ~(3 << 20);

    // Sin pull-up / pull-down
    LPC_PINCON->PINMODE7 &= ~(3 << 20);
    LPC_PINCON->PINMODE7 |=  (2 << 20);

    // Salida
    LPC_GPIO3->FIODIR |= (1 << BLUE_LED);


    // Apagamos los tres LEDs inicialmente
    LPC_GPIO0->FIOSET = (1 << RED_LED);

    LPC_GPIO3->FIOSET = (1 << GREEN_LED) |
                        (1 << BLUE_LED);
}


/*---------------- CONFIGURACIÓN SYSTICK ----------------*/

void confSysTick(void)
{
    // Interrupción cada 1 ms
    SysTick->LOAD = (SystemCoreClock / 100) - 1;

    // Limpia contador actual
    SysTick->VAL = 0;

    // Clock CPU + interrupción + enable
    SysTick->CTRL =
          SysTick_CTRL_CLKSOURCE_Msk |
          SysTick_CTRL_TICKINT_Msk   |
          SysTick_CTRL_ENABLE_Msk;
}


/*---------------- INTERRUPCIÓN ----------------*/

void SysTick_Handler(void)
{
    i--;

    if(i == 0)
    {
        i = 100;   // cambio de color cada 250 ms


        // Primero apagamos TODOS
        LPC_GPIO0->FIOSET = (1 << RED_LED);

        LPC_GPIO3->FIOSET = (1 << GREEN_LED) |
                            (1 << BLUE_LED);


        // Elegimos cuál prender
        if(color == 0)
        {
            // ROJO
            LPC_GPIO0->FIOCLR = (1 << RED_LED);
        }
        else if(color == 1)
        {
            // VERDE
            LPC_GPIO3->FIOCLR = (1 << GREEN_LED);
        }
        else if(color == 2)
        {
            // AZUL
            LPC_GPIO3->FIOCLR = (1 << BLUE_LED);
        }


        color++;

        if(color == 3)
        {
            color = 0;
        }
    }
}
