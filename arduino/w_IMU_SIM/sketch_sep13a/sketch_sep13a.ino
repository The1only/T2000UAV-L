// Very simple WitMotion WTGAHRS3 simulator (UART/TTL).
// Sends Angle (0x53) every 20 ms, plus optional Accel (0x51) & Gyro (0x52).
#include <Adafruit_NeoPixel.h> // LED controller...
#define SIMULATE

int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// === CONFIG ===
const long BAUD = 9600;            // Set to what your receiver expects (e.g., 9600 or 115200)
const uint16_t PERIOD_MS = 20;     // ~50 Hz, common output rate

// Helpers to pack scaled values ------------------------------------------------

// clamp to int16 range
int16_t clamp16(long v) { if (v > 32767) return 32767; if (v < -32768) return -32768; return (int16_t)v; }

// scale degrees -> protocol int16 (±180°)
int16_t angle_to_q15(float deg) { return clamp16((long)(deg * 32768.0f / 180.0f)); }
// scale dps -> protocol int16 (±2000 dps)
int16_t dps_to_q15(float dps)   { return clamp16((long)(dps * 32768.0f / 2000.0f)); }
// scale g -> protocol int16 (±16 g)
int16_t g_to_q15(float g)       { return clamp16((long)(g * 32768.0f / 16.0f)); }

void setup() {
  Serial.begin(BAUD);

  #ifdef DEBUG
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
#else
  for(int x = 0; x < 50; x++){
    delay(100);
    if(Serial) break;
  }
#endif
  
  pixels.begin();
  pinMode(Power,OUTPUT);
  digitalWrite(Power, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));    
  pixels.show();

}

// Temperature encoding varies by model; many Wit docs put raw temp in TL/TH with T=((TH<<8)|TL)/340+36.53.
// We'll just send 25 °C equivalent from the classic formula: raw ≈ (T-36.53)*340.
uint16_t make_temp_raw(float celsius = 25.0f) {
  float raw = (celsius - 36.53f) * 340.0f;
  long r = (long)(raw);
  if (r < 0) r = 0; if (r > 65535) r = 65535;
  return (uint16_t)r;
}

// Build and send a standard 11-byte packet
void sendPacket(uint8_t pid, int16_t x, int16_t y, int16_t z, uint16_t tRaw) {
  uint8_t pkt[11];
  pkt[0] = 0x55;
  pkt[1] = pid;
  pkt[2] = (uint8_t)(x & 0xFF);
  pkt[3] = (uint8_t)((x >> 8) & 0xFF);
  pkt[4] = (uint8_t)(y & 0xFF);
  pkt[5] = (uint8_t)((y >> 8) & 0xFF);
  pkt[6] = (uint8_t)(z & 0xFF);
  pkt[7] = (uint8_t)((z >> 8) & 0xFF);
  pkt[8] = (uint8_t)(tRaw & 0xFF);
  pkt[9] = (uint8_t)((tRaw >> 8) & 0xFF);
  uint16_t sum = 0;
  for (int i = 0; i < 10; ++i) sum += pkt[i];
  pkt[10] = (uint8_t)(sum & 0xFF);

  Serial.write(pkt, 11);
}
// Simple motion generator: yaw spins slowly, small pitch/roll sway.
void loop() {
  static unsigned int l_sleep = 0;
  static bool active = false;
  static uint32_t t0 = millis();
  float t = (millis() - t0) * 0.001f; // seconds

//-----------------------------------------
#ifdef SIMULATE
  if(l_sleep++ > 50)
#else
  if(l_sleep++ > 50000)
#endif
  {
    l_sleep = 0;
    pixels.clear();
    if(active == false)
    {
      pixels.setPixelColor(0, pixels.Color(10, 0, 0));
    }else
    {
        pixels.setPixelColor(0, pixels.Color(0, 80, 0));
    }
    pixels.show();
  }

  if(active == true){
    // --- Angles (0x53) ---
    float roll  = 60.0f * sinf(2.0f * 3.14159f * 0.2f * t);   // ±5°
    float pitch = 30.0f * sinf(2.0f * 3.14159f * 0.1f * t);   // ±3°
    float yaw   = fmodf(t * 20.0f, 360.0f) - 180.0f;         // -180..+180°, rotates ~20°/s

    int16_t r_q = angle_to_q15(roll);
    int16_t p_q = angle_to_q15(pitch);
    int16_t y_q = angle_to_q15(yaw);
    uint16_t t_raw = make_temp_raw(25.0f);

    sendPacket(0x53, r_q, p_q, y_q, t_raw);  // Angle packet

    // --- OPTIONAL: Acceleration (0x51) ---
    // float ax = 0.0f, ay = 0.0f, az = 1.0f; // 1 g on Z
    // sendPacket(0x51, g_to_q15(ax), g_to_q15(ay), g_to_q15(az), t_raw);

    // --- OPTIONAL: Angular velocity (0x52) ---
    // float wx = 0.0f, wy = 0.0f, wz = 20.0f; // dps
    // sendPacket(0x52, dps_to_q15(wx), dps_to_q15(wy), dps_to_q15(wz), t_raw);
  }

  while (Serial.available()) {
    static int state = 0;
    char inChar = (char)Serial.read();

    switch(state){
      case 0:
      if(inChar == 0xFF) state = 1;
      break;
      case 1:
      if(inChar == 0xAA) state = 2;
      else state = 0;
      break;
      case 2:
      switch(inChar){
        case 0x63:
          Serial.begin(15200);
          state = 0;
        break;
        case 0x64:
          Serial.begin(9600);
          state = 0;
        break;
        case 0x27:
          state = 3;
        break;
        default:
          state = 0;
        break;
      } 
      break;
      case 3:
      if(inChar == 0x34) active = true;
      else active = false;
      state = 4;
      break;
      case 4:
      state = 0;
      break;
      default: break;
    }
  }


  delay(PERIOD_MS);
}