/*
 * timemeasure.c
 *
 *  Created on: 2024. 11. 11.
 *      Author: kooo
 */

//#include "timemeasure.h"
//
//volatile unsigned long overflowCount = 0;  // �����÷ο� Ƚ�� ����
//
//void TimerMeasure_start() {
//    TA0CTL = TASSEL_2 + MC_1 + TACLR;   // SMCLK, Clear, Up Mode
//    TA0CCR0 = 0xFFFF;                   // �ִ� ī��Ʈ ����
//    TA0CCTL0 = CCIE;                    // Ÿ�̸� ���ͷ�Ʈ Ȱ��ȭ
//    overflowCount = 0;                  // �����÷ο� ī��Ʈ �ʱ�ȭ
//}
//
//void TimerMeasure_stop() {
//    TA0CTL = MC_0;                      // Ÿ�̸� ����
//    TA0CCTL0 &= ~CCIE;                  // ���ͷ�Ʈ ��Ȱ��ȭ
//}

//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void Timer_A(void) {
//    overflowCount++;                    // �����÷ο� �߻� �� ����
//}
//
//unsigned long TimerMeasure_getElapsedTime_ms() {
//    unsigned long counts_per_ms = 16000;  // SMCLK�� 16MHz�� ��, �и��ʴ� ī��Ʈ ��
//
////    _DBGUART("overflowCount: %lu\r\n", overflowCount);
////    _DBGUART("TA0R: %u\r\n", TA0R);
//    return (overflowCount * 65536 + TA0R) / counts_per_ms;
//}



