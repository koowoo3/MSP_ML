/*
 * rtc.h
 *
 *  Created on: 2024. 7. 23.
 *      Author: PC
 */

#ifndef AB1850_H
#define AB1850_H

#include "stdint.h"
#include <stdbool.h>

#define AM1805_I2C_ADDRESS 0x69

#define HUNDRETH_REGISTER           0x00
#define SECOND_REGISTER             0x01
#define MINUTE_REGISTER             0x02
#define HOUR_REGISTER               0x03
#define DATE_REGISTER               0x04
#define MONTH_REGISTER              0x05
#define YEAR_REGISTER               0x06
#define DAY_REGISTER                0x07
#define HUNDRETH_ALARM_REGISTER     0x08
#define SECOND_ALARM_REGISTER       0x09
#define MINUTE_ALARM_REGISTER       0x0A
#define HOUR_ALARM_REGISTER         0x0B
#define DATE_ALARM_REGISTER         0x0C
#define MONTH_ALARM_REGISTER        0x0D
#define DAY_ALARM_REGISTER          0x0E
#define STATU_REGISTER              0x0F

#define CONTROL1_REGISTER           0x10
#define CONTROL2_REGISTER           0x11
#define INT_MASK_REGISTER           0x12
#define SQW_REGISTER                0x13
#define CAL_XT_REGISTER             0x14
#define CAL_RC_HI_REGISTER          0x15
#define CAL_RC_LOW_REGISTER         0x16
#define INT_POLARITY_REGISTER       0x17
#define TIMER_CONTROL_REGISTER      0x18
#define TIMER_REGISTER              0x19
#define TIMER_INITIAL_REGISTER      0x1A
#define WDT_REGISTER                0x1B
#define OSC_CONTROL_REGISTER        0x1C
#define OSC_STATUS_REGISTER         0x1D

#define CONFIG_KEY_REGISTER         0x1F
#define TRICKLE_REGISTER            0x20
#define BREF_CONTROL_REGISTER       0x21

#define AFCTRL_REGISTER             0x26
#define BATMODE_REGISTER            0x27

#define ID0_REGISTER                0x28
#define ID1_REGISTER                0x29
#define ID2_REGISTER                0x2A
#define ID3_REGISTER                0x2B
#define ID4_REGISTER                0x2C
#define ID5_REGISTER                0x2D
#define ID6_REGISTER                0x2E
#define ASTAT_REGISTER              0X2F

#define OCTRL_REGISTER              0x30
#define EXTENSION_ADDRESS_REGISTER  0x3F

#define TENTH_MASK                  0xF0
#define HUNDRETH_MASK               0x0F
#define SECOND_MASK                 0x7F
#define MINUTE_MASK                 0x7F
#define HOUR_24_MASK                0x3F
#define HOUR_12_MASK                0x1F
#define AM_PM_MASK                  0x20
#define DATE_MASK                   0x3F
#define MONTH_MASK                  0x1F
#define YEAR_MASK                   0xFF
#define DAY_MASK                    0x07

#define TENTH_ALARM_MASK            0xF0
#define HUNDRETH_ALARM_MASK         0xF0
#define SECOND_ALARM_MASK           0x7F
#define MINUTE_ALARM_MASK           0x7F
#define HOUR_24_ALARM_MASK          0x3F
#define HOUR_12_ALARM_MASK          0x1F
#define DATE_ALARM_MASK             0x3F
#define MONTH_ALARM_MASK            0x1F
#define WEEKDAY_ALARM_MASK          0x07

#define MONDAY                      0
#define TUESDAY                     1
#define WEDNESDAY                   2
#define THURSDAY                    3
#define FRIDAY                      4
#define SATURDAY                    5
#define SUNDAY                      6

#define MIN_SECOND                  0
#define MAX_SECOND                  0x60
#define MIN_MINUTE                  0
#define MAX_MINUTE                  0x60
#define MIN_HOURS                   0
#define MAX_HOURS                   0x24
#define MIN_DAY                     1
#define MAX_DAY                     0x08
#define MIN_DATE                    0
#define MAX_DATE                    0x32
#define MIN_MONTH                   0
#define MAX_MONTH                   0x13
#define MIN_YEAR                    0
#define MAX_YEAR                    0x100

typedef struct {
    uint8_t _ss_pin;
    volatile uint8_t _hundredth;
    volatile uint8_t _year;
    volatile uint8_t _month;
    volatile uint8_t _date;
    volatile uint8_t _day;
    volatile uint8_t _hour;
    volatile uint8_t _minutes;
    volatile uint8_t _seconds;

    volatile uint8_t _alarm_year;
    volatile uint8_t _alarm_month;
    volatile uint8_t _alarm_date;
    volatile uint8_t _alarm_day;
    volatile uint8_t _alarm_hour;
    volatile uint8_t _alarm_minutes;
    volatile uint8_t _alarm_seconds;
} AB1805;

void AB1805_init();

uint8_t AB1805_read_rtc_register(const uint8_t rtc_register);
uint8_t AB1805_write_rtc_register(const uint8_t rtc_register, const uint8_t data);

uint8_t AB1805_get_rtc_data(const uint8_t rtc_register, const uint8_t register_mask);

uint8_t AB1805_get_hundredth();
uint8_t AB1805_get_second();
uint8_t AB1805_get_minute();
uint8_t AB1805_get_hour();
uint8_t AB1805_get_day();
uint8_t AB1805_get_date();
uint8_t AB1805_get_month();
uint8_t AB1805_get_year();

void AB1805_get_datetime(AB1805 *rtc);

uint8_t AB1805_get_seconds_alarm();
uint8_t AB1805_get_minutes_alarm();
uint8_t AB1805_get_hour_alarm();
uint8_t AB1805_get_day_alarm();
uint8_t AB1805_get_date_alarm();
uint8_t AB1805_get_month_alarm();

void AB1805_set_second(const uint8_t value);
void AB1805_set_minute(const uint8_t value);
void AB1805_set_hour(const uint8_t value);
void AB1805_set_day(const uint8_t value);
void AB1805_set_date(const uint8_t value);
void AB1805_set_month(const uint8_t value);
void AB1805_set_year(const uint8_t value);

void AB1805_set_datetime(uint8_t year, uint8_t month, uint8_t date,
                         uint8_t day, uint8_t hour, uint8_t minutes, uint8_t seconds);

bool AB1805_set_seconds_alarm(uint8_t value);
bool AB1805_set_minutes_alarm(uint8_t value);
bool AB1805_set_hour_alarm(uint8_t value);
bool AB1805_set_day_alarm(uint8_t value);
bool AB1805_set_date_alarm(uint8_t value);
bool AB1805_set_month_alarm(uint8_t value);

#endif
