#include "convert.h"

int converterJoyToUmid(int valorADC) {
    return (int) (valorADC * 100) / 4095;
}