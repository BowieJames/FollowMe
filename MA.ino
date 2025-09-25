#include "DW1000Ranging.h"
#include "DW1000.h"
#include <SPI.h>
#include <math.h>
class DW1000Device; // fwd decl

// ---------- CONFIG ----------
char anchor_addr[] = "83:00:5B:D5:A9:9A:E2:9C"; // MASTER/front EUI
uint16_t Adelay    = 16600;

#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS  = 4;

// UART: LEFT TX=33 -> MASTER RX=32 ; RIGHT TX=17 -> MASTER RX=16
static const int UART1_RX = 32; // from LEFT
static const int UART1_TX = 33; // unused but must assign
static const int UART2_RX = 16; // from RIGHT
static const int UART2_TX = 17; // unused but must assign

// Geometry: equilateral, side 0.10 m
static const float s = 0.10f;
static const float h = 0.5f * 1.73205080757f * s;  // sqrt(3)/2 * s
// MASTER at (0,0); LEFT and RIGHT at the back
static const float xL = -0.5f*s, yL = -h;
static const float xR =  0.5f*s, yR = -h;

// Data containers
struct Sample { uint16_t tag; float dist; uint32_t t_ms; bool valid; };
static Sample localF  = {0,NAN,0,false}; // MASTER->TAG
static Sample remoteL = {0,NAN,0,false}; // LEFT ->TAG
static Sample remoteR = {0,NAN,0,false}; // RIGHT->TAG

static String bufL, bufR;

static inline float rad2deg(float r){ return r*57.2957795131f; }

// ========= 3s Averaging & window stats =========
#define AVG_WINDOW_MS 3000UL
static uint32_t winStartMs = 0;

// fused angle/dist accumulators
static uint32_t fusedCnt = 0;
static double   sumCos   = 0.0;   // vector-average angle
static double   sumSin   = 0.0;
static double   sumFused = 0.0;   // fused distance r = sqrt(x^2+y^2)

// per-source distance accumulators
static double sumS1 = 0.0; static uint32_t cntS1 = 0; // LEFT anchor → tag
static double sumS2 = 0.0; static uint32_t cntS2 = 0; // RIGHT anchor → tag
static double sumM  = 0.0; static uint32_t cntM  = 0; // MASTER → tag

static void windowReset(uint32_t now){
  winStartMs = now;
  fusedCnt = 0; sumCos = sumSin = sumFused = 0.0;
  sumS1 = sumS2 = sumM = 0.0; cntS1 = cntS2 = cntM = 0;
}

static void fusedAccumulate(float angle_deg, float fused_dist){
  const float ang_rad = angle_deg * (PI / 180.0f);
  sumCos   += cos(ang_rad);
  sumSin   += sin(ang_rad);
  sumFused += fused_dist;
  fusedCnt++;
}


static void maybeEmit(uint32_t now){
  if (winStartMs==0) { windowReset(now); return; }
  if ((now - winStartMs) < AVG_WINDOW_MS) return;

  // compute means
  float meanAngle = 0.0f;
  if (fusedCnt > 0) {
    meanAngle = rad2deg(atan2((float)sumSin, (float)sumCos)); // inputs ∈ [0,π]
    if (meanAngle < 0.0f)   meanAngle = 0.0f;
    if (meanAngle > 180.0f) meanAngle = 180.0f;
  }
  float meanFused = (fusedCnt>0) ? (float)(sumFused / (double)fusedCnt) : NAN;
  float meanS1    = (cntS1>0) ? (float)(sumS1 / (double)cntS1) : NAN;
  float meanS2    = (cntS2>0) ? (float)(sumS2 / (double)cntS2) : NAN;
  float meanM     = (cntM >0) ? (float)(sumM  / (double)cntM ) : NAN;

  // one clean line (angle, fused distance, s1, s2, master)
  // note: prints "nan" if a source had no samples in the window
  Serial.printf("angle = %.1f  distnace = %.3f  s1 = %.3f  s2 = %.3f  m = %.3f\n",
                meanAngle, meanFused, meanS1, meanS2, meanM);

  windowReset(now);
}

// ========= Geometry solver =========
static bool solveXY(float rF, float rL, float rR, float &x, float &y){
  const float A00=-2.0f*xL, A01=-2.0f*yL;
  const float A10=-2.0f*xR, A11=-2.0f*yR;
  const float b0 =(rL*rL - rF*rF) - (xL*xL + yL*yL);
  const float b1 =(rR*rR - rF*rF) - (xR*xR + yR*yR);
  const float det=A00*A11 - A01*A10;
  if (fabsf(det)<1e-9f) return false;
  x=( b0*A11 - b1*A01)/det;
  y=( A00*b1 - A10*b0)/det;
  return true;
}

// handle UART from slaves; isLeft distinguishes S1 vs S2 accumulators
static void handleUart(Stream &uart, String &buf, Sample &slot, bool isLeft) {
  while (uart.available()) {
    char c = uart.read();
    if (c=='\r') continue;
    if (c!='\n') { buf += c; continue; }
    if (buf.startsWith("ANCH,")) {
      char ahex[8]={0}, thex[8]={0};
      float d=NAN;
      if (sscanf(buf.c_str(), "ANCH,%7[^,],%7[^,],%f", ahex, thex, &d)==3) {
        unsigned tshort = (unsigned)strtoul(thex, nullptr, 16);
        slot.tag  = (uint16_t)tshort;
        slot.dist = d;
        slot.t_ms = millis();
        slot.valid= true;
        // accumulate per-source distance
        if (isLeft) { sumS1 += d; cntS1++; } else { sumS2 += d; cntS2++; }
      }
    }
    buf="";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Anchor (MASTER/front) start");
  Serial.print("Antenna delay "); Serial.println(Adelay);

  // UARTs from slaves
  Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000.setAntennaDelay(Adelay);

  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  DW1000Ranging.startAsAnchor(anchor_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);

  Serial.println("[MASTER] ready");
  windowReset(millis());
}

void loop() {
  DW1000Ranging.loop();

  // ingest slave UART lines
  handleUart(Serial1, bufL, remoteL, true);   // LEFT -> s1
  handleUart(Serial2, bufR, remoteR, false);  // RIGHT -> s2

  // if we have a fresh triple for the SAME TAG, compute fused pose
  const uint32_t now=millis(), maxAge=250;
  bool fresh = localF.valid && remoteL.valid && remoteR.valid &&
               (now-localF.t_ms)<=maxAge && (now-remoteL.t_ms)<=maxAge && (now-remoteR.t_ms)<=maxAge &&
               (localF.tag == remoteL.tag) && (localF.tag == remoteR.tag);

  if (fresh) {
    float x=0,y=0;
    if (solveXY(localF.dist, remoteL.dist, remoteR.dist, x, y)) {
      float r = sqrtf(x*x + y*y);  // fused distance from MASTER
      // angle in 0..180° (left=0, front=90, right=180)
      float angle_deg = 0.0f;
      if (r > 1e-6f) {
        float c = -x / r;                 // cos(theta) = -x/r
        if (c < -1.0f) c = -1.0f;
        if (c >  1.0f) c =  1.0f;
        angle_deg = rad2deg(acosf(c));    // 0..180
      }
      fusedAccumulate(angle_deg, r);
    }
  }

  // emit line every 3 seconds
  maybeEmit(now);
}

// MASTER's own range callback (acts as anchor too)
void newRange() {
  uint16_t tagShort = DW1000Ranging.getDistantDevice()->getShortAddress();
  float dist = DW1000Ranging.getDistantDevice()->getRange();

  localF.tag  = tagShort;
  localF.dist = dist;
  localF.t_ms = millis();
  localF.valid= true;

  // accumulate MASTER distance
  sumM += dist; cntM++;
}

void newDevice(DW1000Device *device) {
  Serial.print("Device added: ");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("Delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}

