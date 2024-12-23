#include "msp430.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "DSPLib.h"

extern int16_t MODEL_ARRAY_TEMP[];
extern int16_t MULTIPLY_BUFFER[1892];
//extern int16_t MODEL_BLOCK_TEMP[100];


///* Input matrix A */
//DSPLIB_DATA(inputA, 4)
//_q15 inputA[1024];
//
///* Temporary storage for partial sums */
//DSPLIB_DATA(temp, 4)
//_q15 temp[32]={0};

/* Result of total sum */
//volatile int32_t totalSum = 0;  // ���� ������ �����Ͽ� �� Ȯ�� �����ϰ� ����
//
//void global_avg_pool(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *block)
//{
//
//    int16_t start_index; //�� ����� 1024(������ 32*32) ��� ũ��
//    _q15 *inputA = MULTIPLY_BUFFER;       // LEA RAM ������ �Ϻθ� inputA�� ���
//    _q15 *temp = &MULTIPLY_BUFFER[row * col];
//    for (int i = 0; i < row; i++) {
//        temp[i] = 0;
//    }
//
//    for(int ch=block_number; ch<channel; ch++){
//
//        start_index = ch * row * col;
//        dma_load(inputA, &MODEL_ARRAY_TEMP[start_index], row * col);
//
//        /* Set up parameters for matrix addition */
//        msp_status status;
//        msp_matrix_add_q15_params addParams;
//        addParams.rows = 1;            // �� �྿ ����
//        addParams.cols = col;  // ��ü �� �ջ�
//
//        totalSum = 0;
//
//        /* �� ���� ���� ����Ͽ� temp�� ���� */
//        for (uint16_t r = 0; r < row; r++) {
//            /* �� ��ü�� �� ���� �ջ��Ͽ� temp�� ���� */
//            status = msp_matrix_add_q15(&addParams, &inputA[r * col], temp, temp);
//            msp_checkStatus(status);
//        }
//
//        /* ��� ���� ���� ���Ͽ� ���� ���� ��� */
//        for (uint16_t r = 0; r < row; r++) {
//            totalSum += temp[r];
//        }
//        totalSum /= (row * col);
//        //totalSum = (_q15)(totalSum >> 15);
//        block[ch] = (_q15)totalSum;
//
//        _DBGUART("%x \r\n", block[ch]);
//    }
//}


#define Size 32*32

typedef struct {
    int16_t x_zero_point;
    int16_t y_zero_point;
    int16_t scale;
    int16_t shift;
} Global_avg_param;

Global_avg_param SE_1={
   .x_zero_point = -128,
   .y_zero_point = -128,
   .scale = 779,
   .shift = 9
};

Global_avg_param SE_2={
   .x_zero_point = -128,
   .y_zero_point = -128,
   .scale = 872,
   .shift = 9
};

Global_avg_param SE_3={   //koo: need to fix
   .x_zero_point = -128,
   .y_zero_point = -128,
   .scale = 779,
   .shift = 9
};



volatile int32_t totalSum = 0;

void global_avg_pool(int16_t row, int16_t col, int16_t block_number, int16_t channel, int16_t *block, int se_idx)
{
    int16_t start_index; // �� ����� 1024(������ 32x32) ��� ũ��
    int32_t temp[Size]={0,};
    Global_avg_param avg;
    switch(se_idx){
    case 1: avg = SE_1; break;
    case 2: avg = SE_2; break;
    case 3: avg = SE_3; break;
    }

    int16_t x_zero_point = avg.x_zero_point; //avg1
    int16_t y_zero_point = avg.y_zero_point; //avg1
    int16_t scale = avg.scale; //avg1 //shift=9
    int16_t shift = avg.shift;
    int16_t * idx = MODEL_ARRAY_TEMP;
    start_index = block_number * row * col;
    idx = idx + start_index;

    for (int ch = block_number; ch < channel; ch++) {
        //dma_load(inputA, &MODEL_ARRAY_TEMP[start_index], row * col);
        totalSum = 0;
        /* Set up parameters for matrix addition */
//        msp_status status;
//        msp_matrix_add_q15_params addParams;
//        addParams.rows = 1;        // �� �྿ ���� koo: ���� lea��µ� �� �ᵵ �ӵ��� �ȴ�����
//        addParams.cols = col;      // ��
        for (uint16_t r = 0; r < row; r++) { //koo: �� ���� ���� ����Ͽ� temp�� ����
            for (uint16_t c = 0; c < col; c++) { //koo:  �� ��ü�� �� ����ؼ� temp�� �����ؼ� ���� */
                totalSum += idx[r * col + c];
            }
        }
        totalSum /= (row * col);
        totalSum -= x_zero_point;
        totalSum *= scale;
        totalSum >>= shift;
        totalSum += y_zero_point;

        if (totalSum < -128) totalSum = -128;
        if (totalSum > 127) totalSum = 127;
        idx = idx + (row * col);

        block[ch] = (int8_t)totalSum;
    }
}

