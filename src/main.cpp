#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS 3
#define HIT_THRESHOLD 50
#define BLINK_MS 50

// WS2812
Adafruit_NeoPixel ws2812_front(NUM_PIXELS, 10, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ws2812_right(NUM_PIXELS, 7, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ws2812_left(NUM_PIXELS, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ws2812_back(NUM_PIXELS, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel *ws2812s[4] = {
    &ws2812_front,
    &ws2812_right,
    &ws2812_left,
    &ws2812_back
};

// 压电片ADC
const uint8_t adc_front_pin = 0;
const uint8_t adc_right_pin = 1;
const uint8_t adc_left_pin = 3;
const uint8_t adc_back_pin = 4;
const uint8_t adc_pins[4] = {
    adc_front_pin,
    adc_right_pin,
    adc_left_pin,
    adc_back_pin
};

// 按键
const uint8_t key_pin = 9;

const uint32_t RED = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t BLUE = Adafruit_NeoPixel::Color(0, 0, 255);
uint32_t g_color = RED;

void key_task(void *pvParameters) {
    while (1) {
        // 按下为低电平
        if (!digitalRead(key_pin)) {
            delay(10); // 消抖
            
            // 等待按键释放
            while (!digitalRead(key_pin)) {
                delay(1);
            }
            delay(10); // 消抖
            
            // 切换颜色
            if (g_color == RED) {
                g_color = BLUE;
            } else if (g_color == BLUE) {
                g_color = RED;
            }
        }
        delay(1);
    }
}

void setup() {
    Serial.begin(115200);

    // 初始化WS2812
    for (int i = 0; i < 4; i++) {
        ws2812s[i]->begin();
        ws2812s[i]->clear();
        ws2812s[i]->show();
    }

    // 初始化按键
    pinMode(key_pin, INPUT_PULLUP);

    // 按键任务
    xTaskCreate(key_task, "key_task", 1024, NULL, 1, NULL);
}

void loop() {
    for (int i = 0; i < 4; i++) {
        Adafruit_NeoPixel *ws2812 = ws2812s[i];
        uint8_t adc = adc_pins[i];

        if (analogRead(adc) > HIT_THRESHOLD) {
            ws2812->clear();
            ws2812->show();
            delay(BLINK_MS);
            ws2812->fill(g_color, 0, NUM_PIXELS);
            ws2812->show();
        } else {
            ws2812->clear();
            ws2812->fill(g_color, 0, NUM_PIXELS);
            ws2812->show();
        }
    }
}