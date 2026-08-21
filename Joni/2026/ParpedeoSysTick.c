#include "LPC17xx.h"

#define RED_LED 22

volatile uint32_t i = 100;

void confPCB(void);
void confSysTick(void);
void SysTick_Handler(void);

int main(void)
{
    confPCB();
    confSysTick();

    while(1)
    {
        // El micro espera.
        // SysTick_Handler se ejecuta automáticamente.
    }
}

void confPCB(void)
{
    // P0.22 como GPIO
    LPC_PINCON->PINSEL1 &= ~(3 << 12);

    // P0.22 sin pull-up ni pull-down
    LPC_PINCON->PINMODE1 &= ~(1 << 12);
    LPC_PINCON->PINMODE1 |=  (1 << 13);

    // P0.22 como salida
    LPC_GPIO0->FIODIR |= (1 << 22);

    // LED inicialmente apagado
    LPC_GPIO0->FIOSET = (1 << 22);
}

void confSysTick(void)
{
    // Ejemplo: interrupción cada 10 ms
    SysTick->LOAD = (SystemCoreClock / 100) - 1;

    // Limpia el contador
    SysTick->VAL = 0;

    // Clock del CPU + interrupción + habilitar SysTick
    SysTick->CTRL =
          SysTick_CTRL_CLKSOURCE_Msk |
          SysTick_CTRL_TICKINT_Msk   |
          SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
    i--;
    //La interrupcion se hara cada 1sg
    if(i == 0)
    {
        i = 100;

        if ((1 << 22) & LPC_GPIO0->FIOPIN)
        {
            LPC_GPIO0->FIOCLR = (1 << 22);
        }
        else
        {
            LPC_GPIO0->FIOSET = (1 << 22);
        }
    }
}
/*
*
* Mismo codigo pero que ahora usando al funcion de la libreria de CMSIS, para
configurar el Systick. SysTick_config(Ticks)
*/

#include "LPC17xx.h"

#define RED_LED 22

volatile uint32_t i = 100;

void confPCB(void);
void SysTick_Handler(void);

int main(void)
{
    confPCB();

    // Configura SysTick para interrumpir cada 10 ms
    SysTick_Config(SystemCoreClock / 100);

    while(1)
    {
        // El programa queda acá.
        // SysTick_Handler se ejecuta automáticamente.
    }
}

void confPCB(void)
{
    // P0.22 como GPIO
    LPC_PINCON->PINSEL1 &= ~(3 << 12);

    // P0.22 sin pull-up ni pull-down
    LPC_PINCON->PINMODE1 &= ~(1 << 12);
    LPC_PINCON->PINMODE1 |=  (1 << 13);

    // P0.22 como salida
    LPC_GPIO0->FIODIR |= (1 << RED_LED);

    // LED inicialmente apagado
    LPC_GPIO0->FIOSET = (1 << RED_LED);
}

void SysTick_Handler(void)
{
    i--;

    if(i == 0)
    {
        i = 100;

        if ((1 << RED_LED) & LPC_GPIO0->FIOPIN)
        {
            LPC_GPIO0->FIOCLR = (1 << RED_LED);
        }
        else
        {
            LPC_GPIO0->FIOSET = (1 << RED_LED);
        }
    }
}