// ============ ANCHOR (RECEIVER) — Fast Distance/Angle (no filtering) ============
// Angle convention (front-only):
//   0°   = toward LEFT anchor
//   90°  = straight ahead (perpendicular to baseline)
//   180° = toward RIGHT anchor
//
// Geometry: Sender (LEFT) at (-b/2, 0), Receiver (this board, RIGHT) at (+b/2, 0)

#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// ---- Identify THIS anchor (Receiver / RIGHT) ----
char anchor_addr[] = "83:00:5B:D5:A9:9A:E2:9C";  // change if needed
uint16_t Adelay = 16607;

// ---- Baseline between anchors (meters) ----
const float BASELINE_B = 0.10f; // 10 cm

// ---- DW1000 wiring (as before) ----
#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS     4
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS  = 4;

// ---- UART2 wiring: RX2 <- Sender TX2 ----
const int UART2_TX_PIN = 17; // not used but required by begin()
const int UART2_RX_PIN = 16;
const int UART0_TX_PIN = 1;
const int UART0_RX_PIN = 3;

struct Sample {
  uint16_t tag = 0;
  float dist = NAN;
  uint32_t t_ms = 0;
  bool valid = false;
};
Sample localS, remoteS;

String uartLine;

// Pair samples if within this window (lower = faster/stricter)
const uint32_t MATCH_WINDOW_MS = 150;

static inline bool nearlySimultaneous(const Sample& a, const Sample& b, uint32_t window_ms=MATCH_WINDOW_MS) {
  if (!a.valid || !b.valid) return false;
  if (a.tag != b.tag) return false;
  uint32_t dt = (a.t_ms > b.t_ms) ? (a.t_ms - b.t_ms) : (b.t_ms - a.t_ms);
  return dt <= window_ms;
}

static void computeAndPrint(const Sample& remoteL, const Sample& localR) {
  const float b  = BASELINE_B;
  const float rL = remoteL.dist; // LEFT (Sender) range
  const float rR = localR.dist;  // RIGHT (Receiver) range

  // Solve for (x,y) with midpoint at (0,0), baseline along x
  float x = (rL*rL - rR*rR) / (2.0f * b);

  // y from left-circle: (x + b/2)^2 + y^2 = rL^2
  float term = rL*rL - (x + 0.5f*b)*(x + 0.5f*b);
  if (term < -0.05f) return;     // reject clearly invalid geometry
  if (term < 0.0f) term = 0.0f;  // clamp tiny neg due to noise
  float y = sqrtf(term);         // FRONT-ONLY: take y >= 0

  float distance = sqrtf(x*x + y*y);

  // Angle from LEFT direction (0..180):
  // theta = atan2(y, x) [deg] is bearing from +x; LEFT is 180°, so angleL = 180 - theta
  float theta  = atan2f(y, x) * 180.0f / 3.14159265f;   // in [0,180] since y>=0
  float angleL = 180.0f - theta;
  if (angleL < 0.0f)   angleL = 0.0f;
  if (angleL > 180.0f) angleL = 180.0f;

  Serial.printf("distance = %.3f m, angle = %.2f deg\n", distance, angleL);
}

void setup() {
  Serial.begin(115200);
  delay(120);
  Serial.println("Receiver started (fast, no filtering)");
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000.setAntennaDelay(Adelay);

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  DW1000Ranging.startAsAnchor(anchor_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
}

void loop() {
  DW1000Ranging.loop();

  // Read LEFT anchor lines: ANCH,AAAA,TTTT,DD.ddd
  while (Serial2.available()) {
    char c = (char)Serial2.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (uartLine.startsWith("ANCH,")) {
        uint16_t aShort=0, tShort=0;
        float d=0.f;
        int parsed = sscanf(uartLine.c_str(), "ANCH,%hx,%hx,%f", &aShort, &tShort, &d);
        if (parsed == 3) {
          remoteS.tag  = tShort;
          remoteS.dist = d;
          remoteS.t_ms = millis();
          remoteS.valid = true;

          if (nearlySimultaneous(remoteS, localS)) computeAndPrint(remoteS, localS);
        }
      }
      uartLine = "";
    } else {
      uartLine += c;
      if (uartLine.length() > 120) uartLine = ""; // overflow guard
    }
  }
}

void newRange() {
  uint16_t tagShort = DW1000Ranging.getDistantDevice()->getShortAddress();
  float dist = DW1000Ranging.getDistantDevice()->getRange();

  localS.tag  = tagShort;
  localS.dist = dist;
  localS.t_ms = millis();
  localS.valid = true;

  if (nearlySimultaneous(remoteS, localS)) computeAndPrint(remoteS, localS);
}

void newDevice(DW1000Device *device) {}
void inactiveDevice(DW1000Device *device) {}

