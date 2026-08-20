#include "LPC17xx.h"
#include <stdio.h>

/* -------------------- CONSTANTES -------------------- */

#define PIEDRA  0
#define PAPEL   1
#define TIJERA  2

/* LEDs */
#define LED_EMPATE      (1 << 4)   // P2.4
#define LED_JUGADOR     (1 << 5)   // P2.5
#define LED_CPU         (1 << 6)   // P2.6

/* Máscara para leer P2.0, P2.1 y P2.2 */
#define MASCARA_BOTONES 0x07


/* -------------------- VARIABLES -------------------- */

/*
 * Mapea el valor leído del puerto a una jugada.
 *
 * entrada = 0 -> -1  inválida
 * entrada = 1 ->  0  Piedra
 * entrada = 2 ->  1  Papel
 * entrada = 3 -> -1  inválida
 * entrada = 4 ->  2  Tijera
 */
int mapa[8] = {
    -1, 0, 1, -1, 2, -1, -1, -1
};


/*
 * resultado[jugador][cpu]
 *
 *  1 -> gana jugador
 *  0 -> empate
 * -1 -> gana CPU
 */
int resultado[3][3] = {

    /* CPU:
       Piedra  Papel  Tijera
    */
    {  0,     -1,      1 },   // Jugador: Piedra
    {  1,      0,     -1 },   // Jugador: Papel
    { -1,      1,      0 }    // Jugador: Tijera
};


/* Para mostrar los nombres por consola */
char *nombreJugada[3] = {
    "Piedra",
    "Papel",
    "Tijera"
};


/* -------------------- FUNCIONES -------------------- */

void configurarGPIO(void);
void demora(void);
void apagarLEDs(void);


/* -------------------- MAIN -------------------- */

int main(void)
{
    int entrada;
    int jugador;
    int cpu;
    int ganador;

    /*
     * Esta variable funciona como contador para generar
     * una jugada variable para la CPU.
     */
    unsigned int contador = 0;


    configurarGPIO();

    printf("\n--- PIEDRA PAPEL TIJERA ---\n");


    while(1)
    {
        /*
         * Leemos solamente P2.0, P2.1 y P2.2.
         *
         * 000 = nadie presiona
         * 001 = Piedra
         * 010 = Papel
         * 100 = Tijera
         */
        entrada = LPC_GPIO2->FIOPIN & MASCARA_BOTONES;


        /*
         * Mientras no haya botón presionado,
         * incrementamos contador.
         *
         * Esto nos sirve para variar la jugada
         * de la CPU.
         */
        if(entrada == 0)
        {
            contador++;
        }
        else
        {
            /*
             * Usamos el valor leído directamente
             * como índice del arreglo mapa.
             */
            jugador = mapa[entrada];


            /*
             * jugador será:
             *
             *  0 -> Piedra
             *  1 -> Papel
             *  2 -> Tijera
             * -1 -> entrada inválida
             */
            if(jugador != -1)
            {
                /* Apagamos resultado anterior */
                apagarLEDs();


                /*
                 * Generamos la jugada de la CPU.
                 *
                 * % 3 garantiza:
                 * 0, 1 o 2
                 */
                cpu = contador % 3;


                /*
                 * Consultamos la matriz.
                 */
                ganador = resultado[jugador][cpu];


                /* ---------------- CONSOLA ---------------- */

                printf("\nJugador: %s\n", nombreJugada[jugador]);

                printf("CPU: %s\n", nombreJugada[cpu]);


                /* ---------------- RESULTADO ---------------- */

                if(ganador == 1)
                {
                    printf("Resultado: GANA EL JUGADOR\n");

                    LPC_GPIO2->FIOSET = LED_JUGADOR;
                }
                else if(ganador == 0)
                {
                    printf("Resultado: EMPATE\n");

                    LPC_GPIO2->FIOSET = LED_EMPATE;
                }
                else
                {
                    printf("Resultado: GANA LA CPU\n");

                    LPC_GPIO2->FIOSET = LED_CPU;
                }


                /*
                 * Demora para evitar rebotes del botón.
                 */
                demora();


                /*
                 * Esperamos a que el usuario SUELTE
                 * el botón antes de permitir otra partida.
                 */
                while((LPC_GPIO2->FIOPIN & MASCARA_BOTONES) != 0)
                {
                    contador++;
                }


                demora();


                /* Apagamos LEDs para la próxima ronda */
                apagarLEDs();
            }
        }
    }

    return 0;
}


/* ---------------------------------------------------- */
/* CONFIGURACIÓN GPIO                                   */
/* ---------------------------------------------------- */

void configurarGPIO(void)
{
    /*
     * P2.0 a P2.6 como GPIO.
     *
     * PINSEL4 controla P2.0 a P2.15.
     *
     * Cada pin utiliza 2 bits.
     * 00 = GPIO
     *
     * Limpiamos los bits correspondientes
     * a P2.0 ... P2.6.
     */
    LPC_PINCON->PINSEL4 &= ~0x00003FFF;


    /*
     * P2.0, P2.1, P2.2 = ENTRADAS
     *
     * FIODIR:
     * 0 = entrada
     * 1 = salida
     */
    LPC_GPIO2->FIODIR &= ~((1 << 0) |
                           (1 << 1) |
                           (1 << 2));


    /*
     * P2.4, P2.5, P2.6 = SALIDAS
     */
    LPC_GPIO2->FIODIR |= ((1 << 4) |
                          (1 << 5) |
                          (1 << 6));


    /* Comenzamos con todos los LEDs apagados */
    apagarLEDs();
}


/* ---------------------------------------------------- */
/* APAGAR LEDs                                          */
/* ---------------------------------------------------- */

void apagarLEDs(void)
{
    LPC_GPIO2->FIOCLR = LED_EMPATE |
                        LED_JUGADOR |
                        LED_CPU;
}


/* ---------------------------------------------------- */
/* DEMORA POR SOFTWARE                                  */
/* ---------------------------------------------------- */

void demora(void)
{
    volatile unsigned int i;

    for(i = 0; i < 1000000; i++)
    {
        /* Espera */
    }
}