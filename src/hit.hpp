#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class Hit {
  public:
    static uint32_t RED;
    static uint32_t BLUE;
    static uint32_t GREEN;
    static uint32_t YELLOW;

    static uint32_t color;
    static float blink_time;
    static bool detect_hit;
    static uint32_t hit_cnt;
    static float last_hit_time;

    static void begin();
    static void onLoop();

  private:
    static constexpr auto HIT_THRESHOLD = 50;
    static constexpr auto HIT_BLINK_TIME = 0.05f;

    static Adafruit_NeoPixel *ws2812s[4];
    static uint8_t adc_pins[4]; // 压电片ADC
};
