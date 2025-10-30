#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pwm.h"

#define ADC_CHANNEL    1     // Por ejemplo: P0.24 (AD1)
#define ADC_PIN        24
#define ADC_PORT       0

#define PWM_PORT       2
#define PWM_PIN        0     // P2.0 = PWM1.1
#define PWM_CHANNEL    1     // PWM1.1

#define OUTPUT_PORT    1
#define OUTPUT_PIN     (1<<28)    // GPIO como salida digital

#define ADC_SAMPLES    4
#define SAMPLE_PERIOD_S 30  // 30 segundos

uint16_t samples[ADC_SAMPLES];
uint8_t sample_index = 0;
uint32_t sample_count = 0;

void configADC(void);
void configTimer(void);
void configPWM(void);
void TIMER0_IRQHandler(void);
void processSamples(void);

int main(void) {
    configADC();
    configPWM();
    configTimer();

    while (1) {
        // Nada aquí, todo se maneja con interrupciones.
    }
}
void configADC(void) {
    PINSEL_CFG_Type pinCfg;
    pinCfg.Portnum = ADC_PORT;
    pinCfg.Pinnum = ADC_PIN;
    pinCfg.Funcnum = 1; // AD1
    pinCfg.Pinmode = 0;
    pinCfg.OpenDrain = 0;
    PINSEL_ConfigPin(&pinCfg);
    GPIO_SetDir(OUTPUT_PORT,  OUTPUT_PIN, 1); // Configuro como salida digital

    ADC_Init(LPC_ADC, 200000); // 200 kHz
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL, ENABLE);
}
void configTimer(void) {
    TIM_TIMERCFG_Type timerCfg;
    TIM_MATCHCFG_Type matchCfg;

    timerCfg.PrescalerOption = TIM_PRESCALE_USVAL;
    timerCfg.PrescaleValue = 100; // 100 us

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCfg);

    matchCfg.MatchChannel = TIM_MATCH_CHANNEL_0;
    matchCfg.IntOnMatch = TRUE;
    matchCfg.ResetOnMatch = TRUE;
    matchCfg.StopOnMatch = FALSE;
    matchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    matchCfg.MatchValue = (SAMPLE_PERIOD_S * 10000); // 30 segundos

    TIM_ConfigMatch(LPC_TIM0, &matchCfg);
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);
}
void configPWM(void) {
    PINSEL_CFG_Type pwmPin;
    pwmPin.Portnum = PWM_PORT;
    pwmPin.Pinnum = PWM_PIN;
    pwmPin.Funcnum = 1;
    pwmPin.Pinmode = 0;
    pwmPin.OpenDrain = 0;
    PINSEL_ConfigPin(&pwmPin);

    PWM_TIMERCFG_Type pwmCfg;
    pwmCfg.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
    pwmCfg.PrescaleValue = 1;
    PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &pwmCfg);

    PWM_MatchUpdate(LPC_PWM1, 0, (CLK_FREQ / 20000), PWM_MATCH_UPDATE_NOW); // 20 kHz
    PWM_MatchUpdate(LPC_PWM1, PWM_CHANNEL, 0, PWM_MATCH_UPDATE_NOW);

    PWM_ChannelCmd(LPC_PWM1, PWM_CHANNEL, ENABLE);
    PWM_ResetCounter(LPC_PWM1);
    PWM_CounterCmd(LPC_PWM1, ENABLE);
    PWM_Cmd(LPC_PWM1, ENABLE);
}
void TIMER0_IRQHandler(void) {
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

    ADC_StartCmd(LPC_ADC, ADC_START_NOW);
    while (!(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL, ADC_DATA_DONE)));

    uint16_t adcValue = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL);
    samples[sample_index] = adcValue;
    sample_index = (sample_index + 1) % ADC_SAMPLES;
    sample_count++;

    if (sample_count >= ADC_SAMPLES) {
        processSamples();
    }
}
void processSamples(void) {
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += samples[i];
    }

    float avg_adc = sum / (float)ADC_SAMPLES;
    float voltage = (avg_adc * 3.3) / 4095;

    if (voltage < 1.0) {
        GPIO_ClearValue(OUTPUT_PORT, (1 << OUTPUT_PIN));
        PWM_MatchUpdate(LPC_PWM1, PWM_CHANNEL, 0, PWM_MATCH_UPDATE_NOW);
    } else if (voltage >= 1.0 && voltage <= 2.0) {
        GPIO_ClearValue(OUTPUT_PORT, (1 << OUTPUT_PIN));

        float duty = 0.5 + (voltage - 1.0) * (0.4); // de 50% a 90%
        uint32_t match = (uint32_t)((CLK_FREQ / 20000) * (1 - duty));
        PWM_MatchUpdate(LPC_PWM1, PWM_CHANNEL, match, PWM_MATCH_UPDATE_NOW);
    } else {
        GPIO_SetValue(OUTPUT_PORT, (1 << OUTPUT_PIN));
        PWM_MatchUpdate(LPC_PWM1, PWM_CHANNEL, 0, PWM_MATCH_UPDATE_NOW);
    }
}

