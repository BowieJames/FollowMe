// ============ ANCHOR (SENDER) ============
// Sends its ranging data to the other anchor over UART2
// Change ONLY the address/Adelay to match this anchor's calibration.
// Leave SPI/DW1000 pins as-is.

#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// ---- Identify THIS anchor (example = your #4) ----
char anchor_addr[] = "84:00:5B:D5:A9:9A:E2:9C"; // #4
uint16_t Adelay = 16615; // your calibrated delay for this anchor

// ---- Calibration distance (not used in logic, just printed) ----
float dist_m = 1; // meters

// ---- DW1000 wiring (as in your code) ----
#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS    4

const uint8_t PIN_RST = 27; // reset
const uint8_t PIN_IRQ = 34; // irq (input-only)
const uint8_t PIN_SS  = 4;  // CS

// ---- UART2 wiring (change if needed) ----
const int UART2_TX_PIN = 17; // Sender TX2 -> Receiver RX2
const int UART2_RX_PIN = 16; // not used by sender but must be valid

// Simple helper: last 2 bytes of MAC become "short address"
static uint16_t shortFromAddr(const char *addr) {
  // addr like "84:00:5B:D5:A9:9A:E2:9C" -> short = 0xE29C
  uint16_t hi=0, lo=0;
  // parse last two bytes
  sscanf(addr + 15, "%2hx:%2hx", &hi, &lo);
  return (uint16_t)((hi << 8) | lo);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Anchor (SENDER) start");
  Serial.printf("Antenna delay %u\n", Adelay);
  Serial.printf("Calibration distance %.2f m\n", dist_m);

  // UART2 for streaming to Receiver
  Serial2.begin(115200, SERIAL_8N1, UART2_RX_PIN, UART2_TX_PIN);

  // DW1000 init
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
}

void newRange() {
  uint16_t tagShort = DW1000Ranging.getDistantDevice()->getShortAddress();
  float dist = DW1000Ranging.getDistantDevice()->getRange();

  // Local print
  Serial.printf("TAG 0x%04X, dist=%.2f m\n", tagShort, dist);

  // Send one CSV line over UART2:
  // ANCH,<anchorShortHex>,<tagShortHex>,<distanceMeters>\n
  uint16_t myShort = shortFromAddr(anchor_addr);
  Serial2.printf("ANCH,%04X,%04X,%.3f\n", myShort, tagShort, dist);
}

void newDevice(DW1000Device *device) {
  Serial.print("Device added: 0x");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("Delete inactive device: 0x");
  Serial.println(device->getShortAddress(), HEX);
}
