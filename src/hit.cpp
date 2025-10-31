#include "hit.hpp"
#include <Preferences.h>
#include <driver/adc.h>

extern Preferences preferences;

// public
uint32_t Hit::RED = Adafruit_NeoPixel::Color(255, 0, 0);
uint32_t Hit::BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
uint32_t Hit::GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
uint32_t Hit::YELLOW = Adafruit_NeoPixel::Color(255, 255, 0);
uint32_t Hit::color = 0;
bool Hit::detect_hit = true;
uint32_t Hit::hit_cnt = 0;
uint32_t Hit::last_hit_ms = 0;

// private
Adafruit_NeoPixel *Hit::ws2812s[4] = {
    new Adafruit_NeoPixel(3, 10, NEO_GRB + NEO_KHZ800), // front
    new Adafruit_NeoPixel(3, 5, NEO_GRB + NEO_KHZ800),  // back
    new Adafruit_NeoPixel(3, 6, NEO_GRB + NEO_KHZ800),  // left
    new Adafruit_NeoPixel(3, 7, NEO_GRB + NEO_KHZ800),  // right
};
uint8_t Hit::adc_pins[4] = {0, 4, 3, 1}; // front, back, left, right

void Hit::begin() {
    // 从flash读取颜色
    color = preferences.getUInt("color", RED);

    // 初始化WS2812
    for (int i = 0; i < 4; i++) {
        ws2812s[i]->begin();
    }

    // 设置ADC增益
    for (int i = 0; i < 4; i++) {
        adc1_config_channel_atten(adc1_channel_t(adc_pins[i]), ADC_ATTEN_DB_0);
    }
}

void Hit::onLoop() {
    // Serial.println(analogRead(adc_pins[0]));

    for (int i = 0; i < 4; i++) {
        Adafruit_NeoPixel *ws2812 = ws2812s[i];
        uint8_t adc_pin = adc_pins[i];

        if (detect_hit) {
            if (millis() - last_hit_ms < HIT_BLINK_MS) { // 还在上一次击打冷却时间内
                ws2812->clear();
                ws2812->show();
            } else {
                if (analogRead(adc_pin) > HIT_THRESHOLD) { // 检测到击打
                    hit_cnt++;
                    last_hit_ms = millis();
                    ws2812->clear();
                    ws2812->show();
                } else {
                    // 持续刷新ws2812状态
                    ws2812->clear();
                    ws2812->fill(color, 0, 3);
                    ws2812->show();
                }
            }
        } else {
            // 持续刷新ws2812状态
            ws2812->clear();
            ws2812->fill(color, 0, 3);
            ws2812->show();
        }
    }
}
