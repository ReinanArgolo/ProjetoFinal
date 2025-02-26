#include "convert.h"

int converterJoyToUmid(int valorADC) {
    return (int) (valorADC * 100) / 4095;
}

int converterJoyToCelsius(int valorADC) {
    return (int) ((valorADC * 34) / 2048);
}