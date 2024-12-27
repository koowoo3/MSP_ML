/*
 * timemeasure.c
 *
 *  Created on: 2024. 11. 11.
 *      Author: kooo
 */

//#include "timemeasure.h"
//
//volatile unsigned long overflowCount = 0;  // 오버플로우 횟수 저장
//
//void TimerMeasure_start() {
//    TA0CTL = TASSEL_2 + MC_1 + TACLR;   // SMCLK, Clear, Up Mode
//    TA0CCR0 = 0xFFFF;                   // 최대 카운트 설정
//    TA0CCTL0 = CCIE;                    // 타이머 인터럽트 활성화
//    overflowCount = 0;                  // 오버플로우 카운트 초기화
//}
//
//void TimerMeasure_stop() {
//    TA0CTL = MC_0;                      // 타이머 정지
//    TA0CCTL0 &= ~CCIE;                  // 인터럽트 비활성화
//}

//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void Timer_A(void) {
//    overflowCount++;                    // 오버플로우 발생 시 증가
//}
//
//unsigned long TimerMeasure_getElapsedTime_ms() {
//    unsigned long counts_per_ms = 16000;  // SMCLK가 16MHz일 때, 밀리초당 카운트 수
//
////    _DBGUART("overflowCount: %lu\r\n", overflowCount);
////    _DBGUART("TA0R: %u\r\n", TA0R);
//    return (overflowCount * 65536 + TA0R) / counts_per_ms;
//}



