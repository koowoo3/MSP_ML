#include "rtc.h"
#include <msp430.h>

//******************************************************************************
// General I2C State Machine ***************************************************
//******************************************************************************

#define MAX_BUFFER_SIZE 20

typedef enum I2C_ModeEnum {
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;

/* Used to track the state of the software state machine */
I2C_Mode MasterMode = IDLE_MODE;

/* The Register Address/Command to use */
uint8_t TransmitRegAddr = 0;

/* Buffers and counters for I2C communication */
uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};
uint8_t RXByteCtr = 0;
uint8_t ReceiveIndex = 0;
uint8_t TransmitBuffer[MAX_BUFFER_SIZE] = {0};
uint8_t TXByteCtr = 0;
uint8_t TransmitIndex = 0;

/* Function prototypes */
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count);
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count);
void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);
void AB1805_init();
uint8_t AB1805_read_rtc_register(uint8_t rtc_register);
uint8_t AB1805_write_rtc_register(uint8_t rtc_register, uint8_t data);
uint8_t AB1805_get_rtc_data(uint8_t rtc_register, uint8_t register_mask);
uint8_t AB1805_get_hundredth(void);
uint8_t AB1805_get_second(void);
uint8_t AB1805_get_minute(void);
uint8_t AB1805_get_hour(void);
uint8_t AB1805_get_day(void);
uint8_t AB1805_get_date(void);
uint8_t AB1805_get_month(void);
uint8_t AB1805_get_year(void);
void AB1805_get_datetime(AB1805 *rtc);
void AB1805_set_second(uint8_t value);
void AB1805_set_minute(uint8_t value);
void AB1805_set_hour(uint8_t value);
void AB1805_set_day(uint8_t value);
void AB1805_set_date(uint8_t value);
void AB1805_set_month(uint8_t value);
void AB1805_set_year(uint8_t value);
void AB1805_set_datetime(uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minutes, uint8_t seconds);

/* BCD 값을 10진수로 변환하는 함수 */
uint8_t bcd_to_decimal(uint8_t bcd_value) {
    return ((bcd_value >> 4) * 10) + (bcd_value & 0x0F);
}

/* I2C Read/Write Implementation */

I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count) {
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    RXByteCtr = count;
    TXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    /* Initialize slave address and interrupts */
    UCB2I2CSA = dev_addr;
    UCB2IFG &= ~(UCTXIFG + UCRXIFG);  // Clear any pending interrupts
    UCB2IE &= ~UCRXIE;                // Disable RX interrupt
    UCB2IE |= UCTXIE;                 // Enable TX interrupt

    UCB2CTLW0 |= UCTR + UCTXSTT;      // I2C TX, start condition
    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupts

    return MasterMode;
}

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count) {
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;

    // Copy register data to TransmitBuffer
    CopyArray(reg_data, TransmitBuffer, count);

    TXByteCtr = count;
    RXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    /* Initialize slave address and interrupts */
    UCB2I2CSA = dev_addr;
    UCB2IFG &= ~(UCTXIFG + UCRXIFG);  // Clear any pending interrupts
    UCB2IE &= ~UCRXIE;                // Disable RX interrupt
    UCB2IE |= UCTXIE;                 // Enable TX interrupt

    UCB2CTLW0 |= UCTR + UCTXSTT;      // I2C TX, start condition
    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupts

    return MasterMode;
}

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count) {
    uint8_t copyIndex=0;
    for (copyIndex = 0; copyIndex < count; copyIndex++) {
        dest[copyIndex] = source[copyIndex];
    }
}

/* AB1805 RTC Implementation */

void AB1805_init() {
    // I2C 초기화
    P7SEL0 |= BIT0 | BIT1;
    P7SEL1 &= ~(BIT0 | BIT1);

    UCB2CTLW0 = UCSWRST;                           // Enable SW reset
    UCB2CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK; // I2C master mode, SMCLK
    UCB2BRW = 10;                                  // fSCL = SMCLK/160 = ~100kHz
    UCB2I2CSA = AM1805_I2C_ADDRESS;                // Slave Address
    UCB2CTLW0 &= ~UCSWRST;                         // Clear SW reset, resume operation
    UCB2IE |= UCNACKIE;
}

uint8_t AB1805_read_rtc_register(uint8_t rtc_register) {
    I2C_Master_ReadReg(AM1805_I2C_ADDRESS, rtc_register, 1);
    return ReceiveBuffer[0];
}

uint8_t AB1805_write_rtc_register(uint8_t rtc_register, uint8_t data) {
    I2C_Master_WriteReg(AM1805_I2C_ADDRESS, rtc_register, &data, 1);
    return 1; // 성공 시 true 반환
}

uint8_t AB1805_get_rtc_data(uint8_t rtc_register, uint8_t register_mask) {
    return (AB1805_read_rtc_register(rtc_register) & register_mask);
}

uint8_t AB1805_get_hundredth(void) {
    uint8_t bcd_value = AB1805_get_rtc_data(HUNDRETH_REGISTER, 0xFF);
    return bcd_to_decimal(bcd_value); //
}

uint8_t AB1805_get_second() {
    uint8_t bcd_value = AB1805_get_rtc_data(SECOND_REGISTER, 0x7F);
    return bcd_to_decimal(bcd_value);
}

uint8_t AB1805_get_minute() {
    uint8_t bcd_value = AB1805_get_rtc_data(MINUTE_REGISTER, 0x7F);
    return bcd_to_decimal(bcd_value);
}

uint8_t AB1805_get_hour() {
    uint8_t bcd_value = AB1805_get_rtc_data(HOUR_REGISTER, 0x3F);
    return bcd_to_decimal(bcd_value);
}

uint8_t AB1805_get_day() {
    return AB1805_get_rtc_data(DAY_REGISTER, 0x07);
}

uint8_t AB1805_get_date() {
    uint8_t bcd_value = AB1805_get_rtc_data(DATE_REGISTER, 0x3F);
    return bcd_to_decimal(bcd_value);
}

uint8_t AB1805_get_month() {
    uint8_t bcd_value = AB1805_get_rtc_data(MONTH_REGISTER, 0x1F);
    return bcd_to_decimal(bcd_value);
}

uint8_t AB1805_get_year() {
    uint8_t bcd_value = AB1805_get_rtc_data(YEAR_REGISTER, 0xFF);
    return bcd_to_decimal(bcd_value);
}

void AB1805_get_datetime(AB1805 *rtc) {
    rtc->_seconds = AB1805_get_second();
    rtc->_minutes = AB1805_get_minute();
    rtc->_hour = AB1805_get_hour();
    rtc->_day = AB1805_get_day();
    rtc->_date = AB1805_get_date();
    rtc->_month = AB1805_get_month();
    rtc->_year = AB1805_get_year();
    rtc->_hundredth = AB1805_get_hundredth(); // millisec
}

void AB1805_set_second(uint8_t value) {
    value = value % 60;
    AB1805_write_rtc_register(SECOND_REGISTER, value);
}

void AB1805_set_minute(uint8_t value) {
    value = value % 60;
    AB1805_write_rtc_register(MINUTE_REGISTER, value);
}

void AB1805_set_hour(uint8_t value) {
    value = value % 24;
    AB1805_write_rtc_register(HOUR_REGISTER, value);
}

void AB1805_set_day(uint8_t value) {
    value = value % 7;
    AB1805_write_rtc_register(DAY_REGISTER, value);
}

void AB1805_set_date(uint8_t value) {
    value = value % 31;
    AB1805_write_rtc_register(DATE_REGISTER, value);
}

void AB1805_set_month(uint8_t value) {
    value = value % 12;
    AB1805_write_rtc_register(MONTH_REGISTER, value);
}

void AB1805_set_year(uint8_t value) {
    AB1805_write_rtc_register(YEAR_REGISTER, value);
}

void AB1805_set_datetime(uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t minutes, uint8_t seconds) {
    AB1805_set_year(year);
    AB1805_set_month(month);
    AB1805_set_date(date);
    AB1805_set_day(day);
    AB1805_set_hour(hour);
    AB1805_set_minute(minutes);
    AB1805_set_second(seconds);
}

//******************************************************************************
// I2C Interrupt ***************************************************************
//******************************************************************************

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{
  // Must read from UCB2RXBUF
  uint8_t rx_val = 0;
  switch(__even_in_range(UCB2IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
      break;
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 22: RXIFG0
        rx_val = UCB2RXBUF;
        if (RXByteCtr)
        {
          ReceiveBuffer[ReceiveIndex++] = rx_val;
          RXByteCtr--;
        }

        if (RXByteCtr == 1)
        {
          UCB2CTLW0 |= UCTXSTP;
        }
        else if (RXByteCtr == 0)
        {
          UCB2IE &= ~UCRXIE;
          MasterMode = IDLE_MODE;
          __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
        }
        break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
        switch (MasterMode)
        {
          case TX_REG_ADDRESS_MODE:
              UCB2TXBUF = TransmitRegAddr;
              if (RXByteCtr)
                  MasterMode = SWITCH_TO_RX_MODE;   // Need to start receiving now
              else
                  MasterMode = TX_DATA_MODE;        // Continue to transmision with the data in Transmit Buffer
              break;

          case SWITCH_TO_RX_MODE:
              UCB2IE |= UCRXIE;              // Enable RX interrupt
              UCB2IE &= ~UCTXIE;             // Disable TX interrupt
              UCB2CTLW0 &= ~UCTR;            // Switch to receiver
              MasterMode = RX_DATA_MODE;    // State state is to receive data
              UCB2CTLW0 |= UCTXSTT;          // Send repeated start
              if (RXByteCtr == 1)
              {
                  // Must send stop since this is the N-1 byte
                  while((UCB2CTLW0 & UCTXSTT));
                  UCB2CTLW0 |= UCTXSTP;      // Send stop condition
              }
              break;

          case TX_DATA_MODE:
              if (TXByteCtr)
              {
                  UCB2TXBUF = TransmitBuffer[TransmitIndex++];
                  TXByteCtr--;
              }
              else
              {
                  // Done with transmission
                  UCB2CTLW0 |= UCTXSTP;     // Send stop condition
                  MasterMode = IDLE_MODE;
                  UCB2IE &= ~UCTXIE;        // Disable TX interrupt
                  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
              }
              break;

          default:
              __no_operation();
              break;
        }
        break;
    default: break;
  }
}
