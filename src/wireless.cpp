#include "wireless.hpp"
#include <WiFi.h>
#include <esp_wifi.h>

// public
Wireless::packet_t Wireless::my_packet, Wireless::peer_packet;
bool Wireless::is_pairing = false;
bool Wireless::pairing_found = false;
// private
const uint8_t Wireless::empty_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t Wireless::pairing_data[4] = {0x11, 0x45, 0x14, 0x00};
esp_now_peer_info_t Wireless::broadcast_peer;
esp_now_peer_info_t Wireless::peer;
Preferences Wireless::preferences;

void Wireless::begin() {
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect();
    wifi_country_t config = {
        .cc = "JP",
        .schan = 1,
        .nchan = 14,
        .policy = WIFI_COUNTRY_POLICY_MANUAL,
    };
    esp_wifi_set_country(&config);
    esp_wifi_set_channel(14, WIFI_SECOND_CHAN_NONE);

    esp_now_init();
    esp_now_register_recv_cb(onRecv);

    // 添加广播peer
    memset(broadcast_peer.peer_addr, 0xFF, 6);
    esp_now_add_peer(&broadcast_peer);
    
    preferences.begin("wireless", false);
    if (preferences.isKey("peer_mac")) {
        preferences.getBytes("peer_mac", peer.peer_addr, 6);
        esp_now_add_peer(&peer);
    } else {
        memset(peer.peer_addr, 0x00, 6);
    }
}

void Wireless::entryPairing() {
    // 清除之前的配对
    esp_now_del_peer(peer.peer_addr);

    is_pairing = true;
    pairing_found = false;
}

void Wireless::savePairing() {
    if (pairing_found) {
        // 写入flash
        preferences.putBytes("peer_mac", peer.peer_addr, 6);

        // 添加peer
        esp_now_add_peer(&peer);
    } else {
        // 配对失败，重新从flash中读取mac地址
        if (preferences.isKey("peer_mac")) {
            preferences.getBytes("peer_mac", peer.peer_addr, 6);
            esp_now_add_peer(&peer);
        }
    }

    is_pairing = false;
}

void Wireless::onLoop() {
    if (is_pairing) {
        esp_now_send(broadcast_peer.peer_addr, pairing_data, 4); // 发送配对请求
    } else if (memcmp(peer.peer_addr, empty_mac, 6) != 0) {      // 已有配对设备
        esp_now_send(peer.peer_addr, (uint8_t *)&my_packet, sizeof(packet_t));
    }
}

void Wireless::onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (is_pairing) { // 在配对中
        if (data_len == 4 && memcmp(pairing_data, data, 4) == 0) {
            memcpy(peer.peer_addr, mac_addr, 6);
            pairing_found = true;
        }
    } else if (memcmp(peer.peer_addr, mac_addr, 6) == 0) { // 是从配对设备发过来的
        Serial.println("Message from paired device");
        if (data_len == sizeof(packet_t)) { // 数据长度正确
            memcpy(&peer_packet, data, sizeof(packet_t));
        }
    }
}

// 添加辅助函数用于调试
void Wireless::printMac(const uint8_t *mac) {
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
