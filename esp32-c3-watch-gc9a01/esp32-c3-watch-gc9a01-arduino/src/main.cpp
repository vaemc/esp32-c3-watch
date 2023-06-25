#include <lvgl.h>
#include <TFT_eSPI.h>
#include "Battery.h"

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

#define LCD_BL 5
#define LCD_BUTTON1 3
#define LCD_BUTTON2 10

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

Battery battery(3.3, 4.2);

lv_obj_t *batteryLabel;

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void lv_example_get_started_1(void)
{
  lv_obj_t *btn = lv_btn_create(lv_scr_act()); /*Add a button the current screen*/
  lv_obj_set_size(btn, 120, 50);               /*Set its size*/
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
  lv_label_set_text(label, "Button");     /*Set the labels text*/
  lv_obj_center(label);
}

void batteryTask(void *pvParameters)
{
  while (1)
  {

    char batteryChar[16];
    dtostrf(battery.getCurrentLevel(), 4, 2, batteryChar);
    char prefix[] = "battery: ";
    strcat(prefix, batteryChar);

    // char formattedNumber[20];
    // sprintf(formattedNumber, "battery: %.1f", battery.getCurrentLevel());

    lv_label_set_text(batteryLabel, prefix);

    delay(1000);
  }
}

void lv_example_btn_1(void)
{

  batteryLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_20, 0);
  lv_label_set_text(batteryLabel, "Hello, World!");
  lv_obj_align(batteryLabel, LV_ALIGN_TOP_MID, 0, 30);

  lv_obj_t *label;

  lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn1, 120, 50);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, 0);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  pinMode(LCD_BL, OUTPUT);
  pinMode(LCD_BUTTON1, INPUT);
  pinMode(LCD_BUTTON2, INPUT);
  digitalWrite(LCD_BL, HIGH);
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();        /* TFT init */
  tft.setRotation(0); /* Landscape orientation, flipped */

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  lv_example_btn_1();

  xTaskCreate(batteryTask, "batteryTask", 1000, NULL, 1, NULL);
  
  Serial.println("Setup done");
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
