#include "auto_control.h"
#include "udp.h"           // dung lai calculateChecksum/readInt16LE/readUInt16LE

#include <WiFiUdp.h>
#include <Arduino.h>


// =====================================================
// CONG UDP RIENG CHO PC LUI CHUONG
// (khac cong cua udp.cpp - vo lang/ban dap - de 2 duong
//  du lieu khong bao gio dam vao nhau)
// =====================================================

#define AUTO_UDP_PORT           5006

static WiFiUDP autoUdp;


// =====================================================
// PACKET CONFIG
// =====================================================

#define AUTO_HEADER_1           0xA5
#define AUTO_HEADER_2           0x5A

#define AUTO_PACKET_SIZE        10


// =====================================================
// HIEU CHUAN QUY DOI DON VI
//
// - Goc lai: PC gui do (steer_deg) that, servor_control.cpp
//   lai nhan dai "count" -360..+360 (da "kiem chung o STM32
//   truoc do" - xem servor_control.cpp). Quy doi tuyen tinh
//   theo gioi han lai that su cua xe (AUTO_STEER_MAX_DEG).
//   PHAI khop voi VEHICLE.max_steer_deg ben config.py (Python).
//
// - Toc do: PC gui m/s co dau, esc_control.cpp nhan % (0-100)
//   tach rieng ga/lui. AUTO_MAX_SPEED_MPS = toc do (m/s) duoc
//   coi la 100% ga/lui - CAN DO THUC TE roi chinh lai hang so
//   nay, dung de nguyen gia tri vi du.
// =====================================================

#define AUTO_STEER_MAX_DEG      30.0f
#define AUTO_STEER_COUNT_MAX    360

#define AUTO_MAX_SPEED_MPS      0.30f
#define AUTO_SPEED_DEADBAND     0.01f   // |speed| duoi muc nay coi nhu dung han

// Qua thoi gian nay (ms) khong nhan them goi hop le nao
// -> coi nhu mat lien lac voi PC, khong dung lenh cu nua.
#define AUTO_FAILSAFE_MS        300UL


// =====================================================
// TRANG THAI NOI BO
// =====================================================

static bool everReceived = false;      // da tung nhan goi hop le nao chua
static bool hasValidSequence = false;  // da co "sequence goc" de so sanh chua
static uint16_t lastSequence = 0;

static uint32_t lastPacketMs = 0;

static float steerDeg = 0.0f;
static float speedMps = 0.0f;
static bool enableFlag = false;


// =====================================================
// SO SANH SEQUENCE CO XU LY TRAN SO (WRAP-AROUND)
//
// sequence la uint16_t, chay vong lai ve 0 sau 65535. Dung
// hieu so co dau 16-bit (chuan RFC1982/TCP) de biet "newSeq"
// co thuc su MOI HON "lastSeq" hay khong, ke ca khi da vong
// qua 0 - packet cu/den trung/den sai thu tu se bi loai.
// =====================================================

static bool sequence_is_newer(
    uint16_t newSeq,
    uint16_t lastSeq
)
{
    int16_t diff = (int16_t)(newSeq - lastSeq);

    return diff > 0;
}


// =====================================================
// GIAI MA GOI TIN
// =====================================================

static bool decodeAutoPacket(
    uint8_t* packet,
    int length
)
{
    // -------------------------------------------------
    // Kich thuoc
    // -------------------------------------------------

    if (length != AUTO_PACKET_SIZE)
    {
        return false;
    }


    // -------------------------------------------------
    // Header
    // -------------------------------------------------

    if (packet[0] != AUTO_HEADER_1)
    {
        return false;
    }

    if (packet[1] != AUTO_HEADER_2)
    {
        return false;
    }


    // -------------------------------------------------
    // Checksum: byte 0 -> byte 8 la DATA, byte 9 la checksum
    // -------------------------------------------------

    uint8_t checksumReceived = packet[9];

    uint8_t checksumCalculated =
        calculateChecksum(
            packet,
            9
        );

    if (checksumReceived != checksumCalculated)
    {
        Serial.println(
            "AUTO packet checksum ERROR"
        );

        return false;
    }


    // -------------------------------------------------
    // Sequence: chi chap nhan goi MOI HON goi hop le gan nhat.
    // Goi dau tien nhan duoc luon duoc chap nhan (chua co gi
    // de so sanh).
    // -------------------------------------------------

    uint16_t seq = readUInt16LE(&packet[2]);

    if (hasValidSequence && !sequence_is_newer(seq, lastSequence))
    {
        Serial.print(
            "AUTO packet CU/TRUNG, bo qua (seq="
        );
        Serial.print(seq);
        Serial.print(", da nhan seq=");
        Serial.print(lastSequence);
        Serial.println(")");

        return false;
    }

    lastSequence = seq;
    hasValidSequence = true;


    // -------------------------------------------------
    // Decode du lieu
    // -------------------------------------------------

    int16_t steer100 = readInt16LE(&packet[4]);
    int16_t speed1000 = readInt16LE(&packet[6]);

    steerDeg = (float)steer100 / 100.0f;
    speedMps = (float)speed1000 / 1000.0f;
    enableFlag = (packet[8] != 0);

    return true;
}


// =====================================================
// INIT
//
// LUU Y: khong tu goi WiFi.begin() o day - ham nay chay
// SAU udp_receiver_init() (xem esp32_vehicle.ino), luc do
// WiFi da ket noi roi, chi can mo them 1 socket UDP tren
// cong rieng.
// =====================================================

void auto_control_init(void)
{
    autoUdp.begin(AUTO_UDP_PORT);

    Serial.print(
        "AUTO (PC lui chuong) UDP listening on port: "
    );
    Serial.println(AUTO_UDP_PORT);
}


// =====================================================
// HANDLE
// =====================================================

void auto_control_handle(void)
{
    int packetSize = autoUdp.parsePacket();

    if (packetSize <= 0)
    {
        return;
    }

    if (packetSize != AUTO_PACKET_SIZE)
    {
        while (autoUdp.available())
        {
            autoUdp.read();
        }

        Serial.print(
            "AUTO packet size ERROR: "
        );
        Serial.println(packetSize);

        return;
    }

    uint8_t packet[AUTO_PACKET_SIZE];

    int len = autoUdp.read(packet, AUTO_PACKET_SIZE);

    if (len != AUTO_PACKET_SIZE)
    {
        return;
    }

    if (!decodeAutoPacket(packet, len))
    {
        return;
    }

    everReceived = true;
    lastPacketMs = millis();

    Serial.print("AUTO -> steer_deg: ");
    Serial.print(steerDeg);
    Serial.print(" | speed_mps: ");
    Serial.print(speedMps);
    Serial.print(" | enable: ");
    Serial.println(enableFlag ? 1 : 0);
}


// =====================================================
// TRANG THAI "CON SONG"
// =====================================================

bool auto_control_is_active(void)
{
    if (!everReceived)
    {
        return false;
    }

    if (!enableFlag)
    {
        return false;
    }

    return (millis() - lastPacketMs) < AUTO_FAILSAFE_MS;
}


// =====================================================
// GETTERS - GIA TRI THO (do, m/s)
// =====================================================

float auto_control_get_steer_deg(void)
{
    return steerDeg;
}

float auto_control_get_speed_mps(void)
{
    return speedMps;
}


// =====================================================
// GETTER - GOC LAI DA QUY DOI SANG "COUNT" (-360..+360)
// =====================================================

int16_t auto_control_get_steer_count(void)
{
    if (!auto_control_is_active())
    {
        return 0;
    }

    float deg = steerDeg;

    if (deg > AUTO_STEER_MAX_DEG)
    {
        deg = AUTO_STEER_MAX_DEG;
    }

    if (deg < -AUTO_STEER_MAX_DEG)
    {
        deg = -AUTO_STEER_MAX_DEG;
    }

    float count = (deg / AUTO_STEER_MAX_DEG) * (float)AUTO_STEER_COUNT_MAX;

    return (int16_t)lroundf(count);
}


// =====================================================
// GETTER - TOC DO DA QUY DOI SANG ga/lui/phanh (%)
// =====================================================

void auto_control_get_throttle(
    uint16_t *ga,
    uint16_t *lui,
    uint16_t *phanh
)
{
    *ga = 0;
    *lui = 0;
    *phanh = 0;

    if (!auto_control_is_active())
    {
        return;
    }

    float v = speedMps;

    if (fabsf(v) < AUTO_SPEED_DEADBAND)
    {
        return;
    }

    float pct = (fabsf(v) / AUTO_MAX_SPEED_MPS) * 100.0f;

    if (pct > 100.0f)
    {
        pct = 100.0f;
    }

    if (v > 0.0f)
    {
        *ga = (uint16_t)lroundf(pct);
    }
    else
    {
        *lui = (uint16_t)lroundf(pct);
    }
}
