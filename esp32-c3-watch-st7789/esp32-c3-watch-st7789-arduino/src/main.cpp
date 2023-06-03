#define TOUCH_MODULES_CST_SELF

#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "TouchLib.h"
#include "Wire.h"

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 280;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

#define PIN_IIC_SDA 8
#define PIN_IIC_SCL 9
TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS816S_SLAVE_ADDRESS);

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

  if (touch.read())
  {

    TP_Point t = touch.getPoint(0);

    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = t.x;
    data->point.y = t.y;
    Serial.print("Data x ");
    Serial.println(t.x);
    Serial.print("Data y ");
    Serial.println(t.y);
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

void lv_example_get_started_1(void)
{
  lv_obj_t *btn = lv_btn_create(lv_scr_act()); /*Add a button the current screen*/
  lv_obj_set_size(btn, 120, 50);               /*Set its size*/
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

  lv_obj_t *label = lv_label_create(btn); /*Add a label to the button*/
  lv_label_set_text(label, "Button");     /*Set the labels text*/
  lv_obj_center(label);
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
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

  lv_example_get_started_1();

  Serial.println("Setup done");
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
