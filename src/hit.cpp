#include "hit.hpp"
#include <driver/adc.h>

// public
uint32_t Hit::RED = Adafruit_NeoPixel::Color(255, 0, 0);
uint32_t Hit::BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
uint32_t Hit::GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
uint32_t Hit::YELLOW = Adafruit_NeoPixel::Color(255, 255, 0);
uint32_t Hit::color = Hit::RED;
float Hit::blink_time = 0.0f;
bool Hit::detect_hit = true;
uint32_t Hit::hit_cnt = 0;
float Hit::last_hit_time = 0.0f;

// private
Adafruit_NeoPixel *Hit::ws2812s[4] = {
    new Adafruit_NeoPixel(3, 10, NEO_GRB + NEO_KHZ800), // front
    new Adafruit_NeoPixel(3, 5, NEO_GRB + NEO_KHZ800),  // back
    new Adafruit_NeoPixel(3, 6, NEO_GRB + NEO_KHZ800),  // left
    new Adafruit_NeoPixel(3, 7, NEO_GRB + NEO_KHZ800),  // right
};
uint8_t Hit::adc_pins[4] = {0, 4, 3, 1}; // front, back, left, right

void Hit::begin() {
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

        float now_time = millis() / 1000.0f;
        if (detect_hit) {
            if (now_time - last_hit_time < HIT_BLINK_TIME) { // 还在上一次击打冷却时间内
                ws2812->clear();
                ws2812->show();
            } else {
                if (analogRead(adc_pin) > HIT_THRESHOLD) { // 检测到击打
                    hit_cnt++;
                    last_hit_time = now_time;
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
            // 固定周期闪烁模式
            if (blink_time != 0 && fmodf(now_time, blink_time) / blink_time < 0.5f) {
                ws2812->clear();
                ws2812->show();
            } else {
                ws2812->clear();
                ws2812->fill(color, 0, 3);
                ws2812->show();
            }
        }
    }
}
