#ifndef BATTERY_H
#define BATTERY_H

#include "esp_adc_cal.h"

class Battery {
  private:
    float minLevel;  
    float maxLevel;  
  public:
    Battery(float min, float max);
    float getCurrentLevel();
};

#endif
