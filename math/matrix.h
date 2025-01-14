#include <stdint.h>

#ifndef MATRIX_GUARD
    #define MATRIX_GUARD

    typedef int16_t dtype;

    struct matrix {
        dtype *data;
        uint16_t numRows;
        uint16_t numCols;
    };
    typedef struct matrix matrix;

    struct matrix_8 {
            int8_t *data;
            uint16_t numRows;
            uint16_t numCols;
        };
        typedef struct matrix_8 matrix_8;


    struct matrix_32 {
            int32_t *data;
            uint16_t numRows;
            uint16_t numCols;
        };
        typedef struct matrix_32 matrix_32;

#endif
