#include <Arduino.h>
#include <esp_now.h>

class Wireless {
  public:
    struct alignas(1) packet_t {
        uint32_t color = 0;
        uint32_t hit_cnt = 0;
        uint8_t dbus[10] = {0};
    };
    
    static packet_t my_packet, peer_packet;
    static bool is_pairing;
    static bool pairing_found;
    static uint32_t last_receive_ms;

    static void begin();
    static void entryPairing();
    static void savePairing();
    static void onLoop();

  private:
    static const uint8_t empty_mac[6];
    static const uint8_t pairing_data[4];

    static esp_now_peer_info_t broadcast_peer;
    static esp_now_peer_info_t peer;

    static void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

    static void printMac(const uint8_t *mac);
};
