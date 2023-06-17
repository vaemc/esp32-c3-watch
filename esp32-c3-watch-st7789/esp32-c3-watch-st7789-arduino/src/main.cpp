#define TOUCH_MODULES_CST_SELF

#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "TouchLib.h"
#include "Wire.h"
#include "Battery.h"

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 280;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

#define LCD_BL 0
#define LCD_BUTTON1 1

TouchLib touch(Wire, SDA, SCL, CTS816S_SLAVE_ADDRESS);

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

Battery battery(3.3, 4.2);

lv_obj_t *batteryLabel;

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

  if (touch.read())
  {

    TP_Point t = touch.getPoint(0);

    data->state = LV_INDEV_STATE_PR;
    data->point.x = t.x;
    data->point.y = t.y;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

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

static void btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;
    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}

static void event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED)
  {
    LV_LOG_USER("Clicked");
  }
  else if (code == LV_EVENT_VALUE_CHANGED)
  {
    LV_LOG_USER("Toggled");
  }
}

void lv_example_btn_1(void)
{

  batteryLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(batteryLabel, &lv_font_montserrat_20, 0);
  lv_label_set_text(batteryLabel, "Hello, World!");
  lv_obj_align(batteryLabel, LV_ALIGN_TOP_MID, 0, 15);

  lv_obj_t *label;

  lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn1, 120, 50);
  lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2, 120, 50);
  lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Toggle");
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

void buttonTask(void *pvParameters)
{
  while (1)
  {
    Serial.println(digitalRead(LCD_BUTTON1));
    delay(500);
  }
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  pinMode(LCD_BL, OUTPUT);
  pinMode(LCD_BUTTON1, INPUT);
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
  touch.init();

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

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  lv_example_btn_1();

  Serial.println("Setup done");

  xTaskCreate(batteryTask, "batteryTask", 1000, NULL, 1, NULL);
  // xTaskCreate(buttonTask, "buttonTask", 1000, NULL, 1, NULL);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
