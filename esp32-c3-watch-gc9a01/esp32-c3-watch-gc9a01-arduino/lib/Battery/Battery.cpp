#include "Arduino.h"
#include "Battery.h"
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define ADC_CHANNEL ADC1_CHANNEL_1
#define ADC_ATTEN_DB ADC_ATTEN_DB_11
#define ADC_WIDTH_BIT ADC_WIDTH_BIT_12
#define ADC_NUM ADC_UNIT_1

static esp_adc_cal_characteristics_t *adcChar;


const float R1 = 100400.0;
const float R2 = 100400.0;
const float adc_offset = -0.01;
const float vol_offset = 0.00;

uint32_t adc_read_val()
{

  int samplingFrequency = 500; // 采样频率（可调）
  long sum = 0;                // 采样和
  float samples = 0.0;         // 采样平均值

  for (int i = 0; i < samplingFrequency; i++)
  {
    sum += adc1_get_raw(ADC_CHANNEL); // Take an ADC1 reading from a single channel.
    delayMicroseconds(1000);
  }
  samples = sum / (float)samplingFrequency;

  uint32_t voltage = esp_adc_cal_raw_to_voltage(samples, adcChar); // 2.调用 API 函数将 ADC 值转换为电压（在调用此函数之前，必须初始化特征结构，也就是步骤 1）
  // uint32_t voltage = (samples * 2.6) / 4096.0; //通过公式转换为电压

  return voltage; // 单位(mV)
}

Battery::Battery(float min, float max)
{
  minLevel = min;
  maxLevel = max;

  adc1_config_width(ADC_WIDTH_BIT);
  adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB);

  adcChar = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t cal_mode = esp_adc_cal_characterize(ADC_NUM, ADC_ATTEN_DB, ADC_WIDTH_BIT, ESP_ADC_CAL_VAL_DEFAULT_VREF, adcChar);
}

float Battery::getCurrentLevel()
{
  float adc_voltage = (float)adc_read_val() / 1000.0f + adc_offset;
  float battery_voltage = (R1 + R2) / R2 * adc_voltage + vol_offset;
  return battery_voltage;
}

