#include "convert.h"

int converterJoyToUmid(int adc, int *umid) {

    if(adc > 3500 && *umid < 100)
        *umid += 1;
    else if(adc < 1500 && *umid > 0)
        *umid -= 1;
    
    
    return *umid;
}

int converterJoyToCelsius(int adc, int *temp) {
    if(adc > 3500)
        *temp += 1;
    else if(adc < 1500)
        *temp -= 1;
    
    return *temp;
}