// Main.cpp  (portable)
// Build on macOS: clang++ -std=c++17 -O2 Main.cpp Com_posix.cpp wit_c_sdk.c -lpthread -o WitSimulate

#include <cstdio>
#include <cstdint>
#include <cstring>
// #include <cmath>
#include <string.h>

#include <unistd.h>
#include "serialport.h"
#include "BleUart.h"
#include "wit_c_sdk.h"

#include <math.h>

typedef struct { double L, F, D; } LFD;

static char s_cDataUpdate = 0;
ulong iComPort  = 4;
int iAddress  = 0x50;

static ComQt *serialPorts = nullptr;
static ComBt *serialPortb = nullptr;

bool AutoScanSensor();
void AutoSetBaud(int);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum);
//static void DelayMs(uint16_t ms);

using RxCallbackINS = void(*)(void *handler, char* data, uint32_t length); //char*, uint32_t);
static RxCallbackINS  callback_ = nullptr;
static void setINSRxCallback(RxCallbackINS cb) { callback_ = cb; }
static void *handle = nullptr;

bool INS_driver(void *handler,ComQt *serPorts, ComBt *serPortb, void *func)
{
    serialPorts = serPorts;
    serialPortb = serPortb;
    handle = handler;

    setINSRxCallback((RxCallbackINS)func);
    WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    WitSerialWriteRegister(SensorUartSend);
    WitRegisterCallBack(CopeSensorData);
    WitDelayMsRegister(posix_delay_ms);
    s_cDataUpdate = 0;
    return 1;
}

void AutoSetBaud(int baud)
{
#ifndef Q_OS_IOS
    if(baud == QSerialPort::Baud9600){
        WitSetUartBaud(WIT_BAUD_9600);
        serialPorts->setBaudrate(QSerialPort::Baud9600);
    }
    if(baud == QSerialPort::Baud115200){
        WitSetUartBaud(WIT_BAUD_115200);
        serialPorts->setBaudrate(QSerialPort::Baud115200);
    }
#endif
}

bool AutoScanSensor()
{
    WitReadReg(AX, 3);
    SLEEP_MS(200);
    if(s_cDataUpdate == 1) return 1;
    else return 0;
}


static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    if(serialPorts != nullptr)
        serialPorts->send(reinterpret_cast<const char*>(p_data),static_cast<unsigned short>(uiSize));
    else if(serialPortb != nullptr)
        serialPortb->send(reinterpret_cast<const char*>(p_data),static_cast<unsigned short>(uiSize));
}

static inline int32_t join32(uint16_t lo, uint16_t hi) {
    return (int32_t)(((uint32_t)hi << 16) | lo);
}


static double nmea_ddmm_to_deg(int32_t raw) {
    // raw is ddmm.mmmmmm with decimal removed; may be signed
    int sign = (raw < 0) ? -1 : 1;
    uint32_t v = (raw < 0) ? (uint32_t)(-raw) : (uint32_t)raw;

    uint32_t dd        = v / 10000000U;       // whole degrees
    uint32_t mm_x1e6   = v % 10000000U;       // minutes * 1e6
    double minutes     = mm_x1e6 / 100000.0;  // mm.mmmmmm

    double deg = (double)dd + minutes / 60.0;
    return sign * deg;
}

/*
 Inputs:
   vN, vE, vD   : velocity in NED (m/s)
   roll_deg     : φ  (right-wing down positive)  [deg]
   pitch_deg    : θ  (nose up positive)          [deg]
   yaw_deg      : ψ  (heading from North, CW to East) [deg]
 Output:
   LFD velocity components (Left, Forward, Down) in m/s
 Notes:
   Uses standard aerospace ZYX Euler convention:
     C_n_b = Rz(yaw) * Ry(pitch) * Rx(roll)  (body → NED)
   Then v_b = C_b_n * v_n = C_n_b^T * v_n
*/
// deg→rad helper
static inline double deg2rad(double deg){ return deg * M_PI / 180.0; }

// If your course is TRUE and yaw is MAGNETIC, pass decl_deg = +declination (east positive)
// to rotate magnetic -> true. If both already TRUE, pass 0.
static LFD body_vel_from_speed_course_yaw(double speed_ms,
                             double course_deg_true,
                             double yaw_deg,            // IMU heading
                             double decl_deg,           // usually 0.0
                             double vDown_ms)           // use 0.0 if unknown
{
    // Correct yaw if it’s magnetic but course is true
    double yaw_true = yaw_deg + decl_deg;

    // Relative angle between motion and body forward
    double d = deg2rad(course_deg_true - yaw_true);

    // Forward/Left in body frame (Down passed through)
    LFD out;
    out.F = speed_ms * cos(d);
    out.L = -speed_ms * sin(d);   // Left = -Right
    out.D = vDown_ms;             // If no vertical speed, set 0 or compute from alt-rate
    return out;
}
static void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum)
{
    float  a[3], w[3], Angle[3], h[3], temp;
    int version;
    double GPS[3];
    char   data[512] = {0};
    size_t off = 0;

    s_cDataUpdate = 1;
    if(uiReg == Roll){
        for (int i = 0; i < 3; ++i) {
            a[i]     = (float)sReg[AX   + i] / 32768.0f * 16.0f;
            w[i]     = (float)sReg[GX   + i] / 32768.0f * 2000.0f;
            Angle[i] = (float)sReg[Roll + i] / 32768.0f * 180.0f;
            h[i]     = (float)sReg[HXi  + i];
        }

        GPS[0] = nmea_ddmm_to_deg(join32(sReg[LonL],sReg[LonH]));
        GPS[1] = nmea_ddmm_to_deg(join32(sReg[LatL],sReg[LatH]));
        GPS[2] = (float)sReg[GPSHeight]/10.0; // Get altitude...
        temp = (float)sReg[TEMP]/100.0;
        version = sReg[VERSION];

        double Preassure = join32(sReg[PressureL],sReg[PressureH]);
        double Alt = join32(sReg[HeightL],sReg[HeightH]);

        uint32_t raw = join32(sReg[GPSVL], sReg[GPSVH])/100.0;
        int32_t yaw = sReg[GPSYAW] / 100.0;     // deg (use /10.0 if your fw is 0.1°/LSB)

        LFD vel = body_vel_from_speed_course_yaw(raw, yaw, Angle[2], 0,0);


        auto app = [&](const char* fmt, auto... args) {
            int n = std::snprintf(data + off, sizeof(data) - off, fmt, args...);
            if (n < 0 || off + (size_t)n >= sizeof(data)) { /* handle overflow */ }
            off += (size_t)n;
        };


//        "AccX -0.00	AccY 0.00	AccZ 0.66	AsX -0.24	AsY -0.79	AsZ -0.37	AngleX 30.5	AngleY 38.4	AngleZ 30.0	HX 0.0	HY 0.0	HZ 0.0	VER 15821	TEMP 33.5	LAT 10.491192	LON 60.409745	ALT 256.300000"
        app("AccX %.2f\tAccY %.2f\tAccZ %.2f\t", a[0], a[1], a[2]);
        app("AsX %.2f\tAsY %.2f\tAsZ %.2f\t", w[0], w[1], w[2]);
        app("AngleX %.1f\tAngleY %.1f\tAngleZ %.1f\t", Angle[0], Angle[1], Angle[2]);
        app("HX %.1f\tHY %.1f\tHZ %.1f\t", h[0], h[1], h[2]);
        app("VER %.1d\tTEMP %.1f\t",version,temp);
        app("LAT %.6f\tLON %.6f\tALT %.6f\t", GPS[0], GPS[1], GPS[2]);
        app("PRES %.1d\tBALT %.1f\t",Preassure,Alt);
        app("FW %.1d\tDOWN %.1f\t",vel.F,vel.D);

        callback_(handle,data,strlen(data));
    }
}

