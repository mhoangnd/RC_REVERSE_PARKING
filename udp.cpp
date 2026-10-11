#include "udp.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdint.h>

// =====================================================
// WIFI & UDP CONFIG
// =====================================================
static const char* ssid = "SiCoLab_5G";
static const char* password = "SiCoLabb";

static const int UDP_PORT = 5005;

static WiFiUDP udp;

// =====================================================
// PACKET CONFIG
// =====================================================
#define HEADER_1        0xAA
#define HEADER_2        0x55

#define PACKET_SIZE     11

// Qua thoi gian nay (ms) khong nhan them goi HOP LE nao tu ban
// vo lang/ban dap -> coi nhu mat lien lac, khong dung gia tri cu.
#define MANUAL_FAILSAFE_MS   300UL

// =====================================================
// DATA
// =====================================================

// Góc đánh lái
// int16_t: -32768 → +32767
int16_t countValue = 0;

// Ga, lùi, phanh
// uint16_t: 0 → 65535
uint16_t gaValue = 0;
uint16_t luiValue = 0;
uint16_t phanhValue = 0;

// Thoi diem (millis) nhan duoc goi tin HOP LE gan nhat.
static uint32_t lastPacketMs = 0;
static bool everReceived = false;

// =====================================================
// CHECKSUM
// =====================================================
uint8_t calculateChecksum(uint8_t* data, int length) {
    uint8_t result = 0;
    for (int i = 0; i < length; i++) {
        result ^= data[i];
    }
    return result;
}

// =====================================================
// READ INT16 LITTLE ENDIAN
// =====================================================
int16_t readInt16LE(uint8_t* data) {
    uint16_t value = ((uint16_t)data[0]) | ((uint16_t)data[1] << 8);
    return (int16_t)value;
}

// =====================================================
// READ UINT16 LITTLE ENDIAN
// =====================================================
uint16_t readUInt16LE(uint8_t* data) {
    uint16_t value = ((uint16_t)data[0]) | ((uint16_t)data[1] << 8);
    return value;
}

// =====================================================
// DECODE PACKET
// =====================================================
bool decodePacket(uint8_t* packet, int length) {
    // Kiếm tra kích thước
    if (length != PACKET_SIZE) {
        return false;
    }

    // Kiểm tra HEADER
    if (packet[0] != HEADER_1 || packet[1] != HEADER_2) {
        return false;
    }

    // Kiểm tra CHECKSUM
    // byte 0 → byte 9: DATA
    // byte 10: CHECKSUM
    uint8_t checksumReceived = packet[10];
    uint8_t checksumCalculated = calculateChecksum(packet, 10);

    if (checksumReceived != checksumCalculated) {
        Serial.println("UDP checksum ERROR");
        return false;
    }

    // Decode
    // count = int16_t
    countValue = readInt16LE(&packet[2]);

    // ga = uint16_t
    gaValue = readUInt16LE(&packet[4]);

    // lui = uint16_t
    luiValue = readUInt16LE(&packet[6]);

    // phanh = uint16_t
    phanhValue = readUInt16LE(&packet[8]);

    return true;
}

// =====================================================
// INIT
// =====================================================
void udp_receiver_init(void) {
    WiFi.begin(ssid, password);

    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    udp.begin(UDP_PORT);
    Serial.print("UDP listening on port: ");
    Serial.println(UDP_PORT);
}

// =====================================================
// HANDLE
// =====================================================
void udp_receiver_handle(void) {
    int packetSize = udp.parsePacket();

    // Không có packet
    if (packetSize <= 0) {
        return;
    }

    // Packet phải đúng 11 byte
    if (packetSize != PACKET_SIZE) {
        while (udp.available()) {
            udp.read();
        }

        Serial.print("UDP packet size ERROR: ");
        Serial.println(packetSize);
        return;
    }

    // Đọc packet
    uint8_t packet[PACKET_SIZE];
    int len = udp.read(packet, PACKET_SIZE);

    if (len != PACKET_SIZE) {
        return;
    }

    // Decode
    if (!decodePacket(packet, len)) {
        return;
    }

    // Danh dau "con song" - dung cho udp_is_fresh()
    lastPacketMs = millis();
    everReceived = true;

    // DEBUG
    Serial.print("UDP -> ");
    Serial.print("count: ");
    Serial.print(countValue);
    Serial.print(" | ga: ");
    Serial.print(gaValue);
    Serial.print(" | lui: ");
    Serial.print(luiValue);
    Serial.print(" | phanh: ");
    Serial.println(phanhValue);
}

// =====================================================
// GETTERS
// =====================================================
bool udp_is_fresh(void) {
    if (!everReceived) {
        return false;
    }
    return (millis() - lastPacketMs) < MANUAL_FAILSAFE_MS;
}

int16_t udp_get_steering_angle(void) {
    return countValue;
}

uint16_t udp_get_speed(void) {
    return gaValue;
}

uint16_t udp_get_lui(void) {
    return luiValue;
}

uint16_t udp_get_phanh(void) {
    return phanhValue;
}