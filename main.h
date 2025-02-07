#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "driverlib.h"

#include <msp430.h>
#include "DSPLib.h"
#include "myuart.h"
#include "FreeRTOS.h"
#include "task.h"

#define High_Threshold 0xCCC         // ~2V
#define Low_Threshold  0x886         // ~1.33v  //0x666         // ~1V


uint16_t UART_FG=0;

unsigned int FreqLevel = 8;
int uartsetup=0;
int uart2setup=0;
volatile uint32_t cycleCount;

int boardSetup(){
    WDTCTL = WDTPW + WDTHOLD;    /* disable watchdog timer  */
    PM5CTL0 &= ~LOCKLPM5;       // Disable the GPIO power-on default high-impedance mode

    P1DIR=0xff;P1OUT=0x00;
    P2DIR=0xff;P2OUT=0x00;
    P3DIR=0xff;P3OUT=0x00;
    P4DIR=0xff;P4OUT=0x00;
    P5DIR=0xff;P5OUT=0x00;
    P6DIR=0xff;P6OUT=0x00;
    P7DIR=0xff;P7OUT=0x00;
    P8DIR=0xff;P8OUT=0x00;
    PADIR=0xff;PAOUT=0x00;
    PBDIR=0xff;PBOUT=0x00;
    PCDIR=0xff;PCOUT=0x00;
    PDDIR=0xff;PDOUT=0x00;

    P8DIR=0xfd;P8REN=GPIO_PIN1;

    // setup uart
    uartsetup=0;
    setFrequency(8);
    CS_initClockSignal( CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    uartinit();

    /* clock setup */
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
    // operation beyond 8MHz _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_1;

    // Clock System Setup
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
    // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    // Per Device Errata set divider to 4 before changing frequency to
    // prevent out of spec operation from overshoot transient
//    CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
    CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz
    // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
    __delay_cycles(60);
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
    CSCTL0_H = 0;

    return 0;
};

void adc_setup(){
    //setup adc
    P1OUT &= ~BIT0;                         // Clear P1.0
    P1DIR |= BIT0;                          // Set P1.0 output direction
    P1SEL1 |= BIT2; // P1.2를 ADC 입력으로 설정
    P1SEL0 |= BIT2;
    PJSEL0 |= BIT4 | BIT5;                  // For XT1
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHS_0 | ADC12SSEL_0 | ADC12CONSEQ_0 | ADC12SHP;
    ADC12MCTL0 = ADC12INCH_2 | ADC12VRSEL_1 | ADC12WINC;
    ADC12HI = High_Threshold;
    ADC12LO = Low_Threshold;
    ADC12IER2 = ADC12HIIE | ADC12LOIE | ADC12INIE;



    // Configure internal reference
    while(REFCTL0 & REFGENBUSY);            // If ref generator busy, WAIT
    REFCTL0 |= REFVSEL_2|REFON;             // Select internal ref = 2.5V
                                            // Internal Reference ON
    while(!(REFCTL0 & REFGENRDY));          // Wait for reference generator
                                            // to settle
    ADC12CTL0 |= ADC12ENC | ADC12SC;    // Enable & start conversion  이거 켜야함.

    _enable_interrupt();
}
/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/* The heap is allocated here so the "persistent" qualifier can be used.  This
requires configAPPLICATION_ALLOCATED_HEAP to be set to 1 in FreeRTOSConfig.h.
See http://www.freertos.org/a00111.html for more information. */

#ifdef __ICC430__
    __persistent                    /* IAR version. */
#else
    #pragma PERSISTENT( ucHeap )    /* CCS version. */
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] = { 0 };

void vApplicationMallocFailedHook( void )
{
    /* Force an assert. */
    configASSERT( ( volatile void * ) NULL );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Force an assert. */
    configASSERT( ( volatile void * ) NULL );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    __bis_SR_register( LPM4_bits + GIE );
    __no_operation();
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER0_A0_VECTOR. */
void vApplicationSetupTimerInterrupt( void )
{
const unsigned short usACLK_Frequency_Hz = 32768;

    /* Ensure the timer is stopped. */
    TA0CTL = 0;

    /* Run the timer from the ACLK. */
    TA0CTL = TASSEL_1;

    /* Clear everything to start with. */
    TA0CTL |= TACLR;

    /* Set the compare match value according to the tick rate we want. */
    TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

    /* Enable the interrupts. */
    TA0CCTL0 = CCIE;

    /* Start up clean. */
    TA0CTL |= TACLR;

    /* Up mode. */
    TA0CTL |= MC_1;
}

void vConfigureTimerForRunTimeStats( void )
{
    /* Configure a timer that is used as the time base for run time stats.  See
    http://www.freertos.org/rtos-run-time-stats.html */

    /* Ensure the timer is stopped. */
    TA1CTL = 0;

    /* Start up clean. */
    TA1CTL |= TACLR;

    /* Run the timer from the ACLK/8, continuous mode, interrupt enable. */
    TA1CTL = TASSEL_1 | ID__8 | MC__CONTINUOUS | TAIE;
}


#pragma vector=TIMER1_A1_VECTOR
__interrupt void v4RunTimeStatsTimerOverflow( void )
{
    TA1CTL &= ~TAIFG;

    /* 16-bit overflow, so add 17th bit. */
    ulRunTimeCounterOverflows += 0x10000;
    __bic_SR_register_on_exit( SCG1 + SCG0 + OSCOFF + CPUOFF );
}






#endif /* MAIN_H_ */

