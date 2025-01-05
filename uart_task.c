/*
 * uart_task.c
 *
 *  Created on: 2025. 1. 3.
 *      Author: kooo
 */

/* Standard includes. */
//#include <stdlib.h>
//#include <string.h>
///* Scheduler includes. */
//#include "FreeRTOS.h"
//#include "queue.h"
//#include "task.h"
//
///* Demo application includes. */
//#include "serial.h"
//#include "semphr.h"
///* TI includes. */
//#include "driverlib.h"
//
///* Local includes. */
//#include "task_common.h"
//
///* Misc. constants. */
//#define serNO_BLOCK             ( ( TickType_t ) 0 )
//
///* The queue used to hold received characters. */
//static QueueHandle_t xRxedChars;
//
///* The queue used to hold characters waiting transmission. */
//static QueueHandle_t xCharsForTx;
//
///* Dimensions the buffer into which input characters are placed. */
//#define cmdMAX_INPUT_SIZE       50
//
///* Dimentions a buffer to be used by the UART driver, if the UART driver uses a
//buffer at all. */
//#define cmdQUEUE_LENGTH         50
//
///* DEL acts as a backspace. */
//#define cmdASCII_DEL        ( 0x7F )
//
///* The maximum time to wait for the mutex that guards the UART to become
//available. */
//#define cmdMAX_MUTEX_WAIT       pdMS_TO_TICKS( 300 )
//
//#ifndef configCLI_BAUD_RATE
//    #define configCLI_BAUD_RATE 115200
//#endif
//
///*
// * The task that implements the command console processing.
// */
//static void UART_Task( void *pvParameters );
//
///*-----------------------------------------------------------*/
//
///* Const messages output by the command console. */
//static const char * const pcWelcomeMessage = "Let's start\r\n>";
//
///* Used to guard access to the UART in case messages are sent to the UART from
//more than one task. */
//static SemaphoreHandle_t xTxMutex = NULL;
//
///* The handle to the UART port, which is not used by all ports. */
//static xComPortHandle xPort = 0;
//
//
///*-----------------------------------------------------------*/
//
//xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
//{
//unsigned long ulBaudRateCount;
//
//    /* Initialise the hardware. */
//
//    /* Generate the baud rate constants for the wanted baud rate. */
//    ulBaudRateCount = configCPU_CLOCK_HZ / ulWantedBaud;
//
//    portENTER_CRITICAL();
//    {
//        /* Create the queues used by the com test task. */
//        xRxedChars = xQueueCreate( uxQueueLength, ( UBaseType_t ) sizeof( signed char ) );
//        xCharsForTx = xQueueCreate( uxQueueLength, ( UBaseType_t ) sizeof( signed char ) );
//
//        /* Reset UART. */
//        UCA0CTL1 |= UCSWRST;
//
//        /* Use SMCLK. */
//        UCA0CTL1 = UCSSEL0 | UCSSEL1;
//
//        /* Setup baud rate low byte. */
//        UCA0BR0 = ( unsigned char ) ( ulBaudRateCount & ( unsigned long ) 0xff );
//
//        /* Setup baud rate high byte. */
//        ulBaudRateCount >>= 8UL;
//        UCA0BR1 = ( unsigned char ) ( ulBaudRateCount & ( unsigned long ) 0xff );
//
//        /* Enable interrupts. */
//        UCA0IE |= UCRXIE;
//
//        /* Take out of reset. */
//        UCA0CTL1 &= ~UCSWRST;
//    }
//    portEXIT_CRITICAL();
//
//    /* Note the comments at the top of this file about this not being a generic
//    UART driver. */
//    return NULL;
//}
///*-----------------------------------------------------------*/
//
//signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime )
//{
//    /* Get the next character from the buffer.  Return false if no characters
//    are available, or arrive before xBlockTime expires. */
//    if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
//    {
//        return pdTRUE;
//    }
//    else
//    {
//        return pdFALSE;
//    }
//}
///*-----------------------------------------------------------*/
//
//signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime )
//{
//BaseType_t xReturn;
//
//    /* Send the next character to the queue of characters waiting transmission,
//    then enable the UART Tx interrupt, just in case UART transmission has already
//    completed and switched itself off. */
//    xReturn = xQueueSend( xCharsForTx, &cOutChar, xBlockTime );
//    UCA0IE |= UCTXIE;
//
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength )
//{
//UBaseType_t uxChar;
//const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 100 );
//
//    /* The driver only supports one port so the pxPort parameter is not used. */
//    ( void ) pxPort;
//
//    for( uxChar = 0; uxChar < usStringLength; uxChar++ )
//    {
//        if( xQueueSend( xCharsForTx, &( pcString[ uxChar ] ), xMaxBlockTime ) == pdFALSE )
//        {
//            break;
//        }
//        else
//        {
//            UCA0IE |= UCTXIE;
//        }
//    }
//}
//
//void UartStart( uint16_t usStackSize, UBaseType_t uxPriority )
//{
//    /* Create the semaphore used to access the UART Tx. */
//    xTxMutex = xSemaphoreCreateMutex();
//    configASSERT( xTxMutex );
//
//    /* Create that task that handles the console itself. */
//    xTaskCreate(    UART_Task,  /* The task that implements the command console. */
//                    "CLI",                      /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
//                    usStackSize,                /* The size of the stack allocated to the task. */
//                    NULL,                       /* The parameter is not used, so NULL is passed. */
//                    uxPriority,                 /* The priority allocated to the task. */
//                    NULL );                     /* A handle is not required, so just pass NULL. */
//}
///*-----------------------------------------------------------*/
//
//static void UART_Task( void *pvParameters )
//{
//    signed char cRxedChar;
//    uint8_t ucInputIndex = 0;
//    char *pcOutputString;
//    static char cInputString[ cmdMAX_INPUT_SIZE ], cLastInputString[ cmdMAX_INPUT_SIZE ];
//    BaseType_t xReturned;
//    xComPortHandle xPort;
//
//    ( void ) pvParameters;
//
//    /* Obtain the address of the output buffer.  Note there is no mutual
//    exclusion on this buffer as it is assumed only one command console interface
//    will be used at any one time. */
//    pcOutputString = FreeRTOS_CLIGetOutputBuffer();
//
//    /* Initialise the UART. */
//    xPort = xSerialPortInitMinimal( configCLI_BAUD_RATE, cmdQUEUE_LENGTH );
//
//    /* Send the welcome message. */
//    vSerialPutString( xPort, ( signed char * ) pcWelcomeMessage, ( unsigned short ) strlen( pcWelcomeMessage ) );
//
//    for( ;; )
//    {
//        if(task_1_complete == 0 || task2_complete == 0){
//            /* Wait to receive a character from UART. */
//            while( xSerialGetChar( xPort, &cRxedChar, portMAX_DELAY ) != pdPASS );
//
//            /* Ensure exclusive access to the UART Tx. */
//            if( xSemaphoreTake( xTxMutex, cmdMAX_MUTEX_WAIT ) == pdPASS )
//            {
//                /* Echo the received character back to the UART. */
//                xSerialPutChar( xPort, cRxedChar, portMAX_DELAY );
//
//                /* Must ensure to give the mutex back. */
//                xSemaphoreGive( xTxMutex );
//            }
//        }
//        else{
//            yield;
//        }
//     }
//
//}
//
//
//#pragma vector=USCI_A0_VECTOR
//__interrupt void prvUSCI_A0_ISR( void )
//{
//signed char cChar;
//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//
//    while( ( UCA0IFG & UCRXIFG ) != 0 )
//    {
//        /* Get the character from the UART and post it on the queue of Rxed
//        characters. */
//        cChar = UCA0RXBUF;
//        xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
//    }
//
//    /* If there is a Tx interrupt pending and the tx interrupts are enabled. */
//    if( ( UCA0IFG & UCTXIFG ) != 0 )
//    {
//        /* The previous character has been transmitted.  See if there are any
//        further characters waiting transmission. */
//        if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
//        {
//            /* There was another character queued - transmit it now. */
//            UCA0TXBUF = cChar;
//        }
//        else
//        {
//            /* There were no other characters to transmit - disable the Tx
//            interrupt. */
//            UCA0IE &= ~UCTXIE;
//        }
//    }
//
//    __bic_SR_register_on_exit( SCG1 + SCG0 + OSCOFF + CPUOFF );
//
//    /* If writing to a queue caused a task to unblock, and the unblocked task
//    has a priority equal to or above the task that this interrupt interrupted,
//    then lHigherPriorityTaskWoken will have been set to pdTRUE internally within
//    xQueuesendFromISR(), and portEND_SWITCHING_ISR() will ensure that this
//    interrupt returns directly to the higher priority unblocked task.
//
//    THIS MUST BE THE LAST THING DONE IN THE ISR. */
//    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//
//}
