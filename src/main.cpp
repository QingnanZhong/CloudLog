#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "examples/lv_examples.h"

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#define DF_GFX_BL 21

/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

#if 0
/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();
#else
Arduino_DataBus *bus = new Arduino_ESP32SPI(23 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, -1 /* MISO */, HSPI /* spi_num */);
#endif

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
#if 0
Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);
#else
Arduino_GFX *gfx = new Arduino_GC9A01(bus, 22 /* RST */, 0 /* rotation */, true /* IPS */);
#endif

#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

const char *ssid = "HKUST SHCIRI";
const char *password = "Ku7J9ITUO4Lt";

HTTPClient http;

String payload = "{}";

StaticJsonDocument<100> doc;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;

// put function declarations here:
int myFunction(int, int);

char timef[50];
bool get_time_is_ok;
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    get_time_is_ok = false;
    return;
  }
  get_time_is_ok = true;
  // Serial.println(&timeinfo, "%F %T %A"); // 格式化输出
  sprintf(timef, "%4d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  // Serial.println(timef);
}

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);

  lv_disp_flush_ready(disp);
}

void setup()
{
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->setCursor(110, 110);
  gfx->setTextColor(RED);
  gfx->println("Hello World!");
  Serial.println("Hello World!");
  gfx->drawCircle(119, 119, 119, GREEN);

  delay(1000); // 1 seconds

  lv_init();

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * 10);
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
    return;
  }

  // 最早HoloCubic的代码中并未注意到这一设置，buffer只设置为了240x10，因此导致画面刷新缓慢且有撕裂感。
  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 10);

  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);

/* Create simple label */
#if 0
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Arduino-Niceday");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
#endif

  lv_example_meter_2();

  Serial.println("Setup done");
  // // put your setup code here, to run once:
  // int result = myFunction(2, 3);
  // Serial.begin(115200);
  // delay(4000);
  // WiFi.begin(ssid, password);

  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi...");
  // }

  // Serial.println("Connected to the WiFi network");

  // http.begin("https://www.zqn.ink/cloudlog/transfer.php");
  // http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // http.end(); // Free the resources

  // // 从网络时间服务器上获取并设置时间
  // // 获取成功后芯片会使用RTC时钟保持时间的更新
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();
}

uint8_t buffer[100];

void loop()
{
  // gfx->setCursor(random(gfx->width()), random(gfx->height()));
  // gfx->setTextColor(random(0xffff), random(0xffff));
  // gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
  // gfx->println("Hello World!");
  // Serial.println("Hello World!");

  // delay(1000); // 1 second

  lv_timer_handler(); /* let the GUI do its work */
  delay(5);

  int c = Serial.read();
  if (c != -1)
  {
    if (c == 0xAA)
    {
      uint8_t len = 17;
      while (len != 0)
      {
        buffer
      }
    }
  }
  // put your main code here, to run repeatedly:
  //   delay(1000);
  //   printLocalTime();
  //   doc["time"] = timef;
  //   doc["level"] = "Warning";
  //   doc["intext"] = "cloud log test";
  //   serializeJson(doc, Serial);
  //   String sendjson;
  //   serializeJson(doc, sendjson);
  //   int httpResponseCode = http.POST(sendjson);
  //   if (httpResponseCode <= 0)
  //   { // Check for the returning code
  //     Serial.println("Error on HTTP request");
  //   }
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}