#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SparkFun_BMI270_Arduino_Library.h>

#define IMU_SDA 8
#define IMU_SCL 9

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_ROATAION 2

#define SIM_W 128
#define SIM_H 96

#define BILLIARD

BMI270 imu;
TFT_eSPI tft(SCREEN_WIDTH, SCREEN_HEIGHT);
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68

void touch_calibrate();

#ifndef BILLIARD
uint16_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT] = {};
float velocity_buffer[SIM_W * SIM_H] = {};
float pressure_buffer[SIM_W * SIM_H] = {};

struct FluidShader 
{
    float time = 0;
    float dt = 5.f / 1000.f; // same as loop delay

    void readPixel(int X, int Y, float * r, float * g, float * b)
    {
        uint16_t c = tft.readPixel(X, Y);
        uint8_t rr = (c >> 11) & 0x1F;
        uint8_t gg = (c >> 5) & 0x3F;
        uint8_t bb = c & 0x1F;

        *r = rr / 31.0f;
        *g = gg / 63.0f;
        *b = bb / 31.0f;
    }

    uint16_t touchX,touchY;
    bool touched = false;

    uint16_t getColor(int X, int Y)
    {
        // ---------------- config ----------------
        constexpr int   GW = SIM_W, GH = SIM_H, N = GW * GH;      // fluid grid (5x5 pixels per cell)
        constexpr float SX = (float)GW / SCREEN_WIDTH;      // grid cells per pixel
        constexpr float SY = (float)GH / SCREEN_HEIGHT;

        // Flip these if the fluid falls the wrong way for your board orientation
        const float SIGN_X  = -1.f;
        const float SIGN_Y  = -1.f;
        const bool  SWAP_XY = false;

        const float GRAV      = 150.f;   // cells/s^2 per g of (apparent) gravity
        const float VEL_DAMP  = 0.998f;  // viscosity-ish
        const int   P_ITERS   = 15;      // pressure solver iterations
        const float MAX_V     = 50.f;    // cells/s

        // ------- memory: reuse the existing global buffers (all zero at boot) -------
        float* vx  = velocity_buffer;          // velocity x
        float* vy  = velocity_buffer + N;      // velocity y
        float* vx2 = velocity_buffer + 2 * N;  // scratch
        float* vy2 = velocity_buffer + 3 * N;  // scratch
        float* dn  = pressure_buffer;          // fluid density (1 = water, 0 = air)
        float* dn2 = pressure_buffer + N;      // scratch
        float* pr  = pressure_buffer + 2 * N;  // pressure (warm-started between frames)
        float* dv  = pressure_buffer + 3 * N;  // divergence

        static bool     inited = false;
        static uint32_t lastUs = 0;
        static bool     prevTouched = false;
        static float    prevTX = 0.f, prevTY = 0.f;

        auto clampf = [](float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); };

        auto sample = [&](const float* f, float x, float y) -> float {
            x = clampf(x, 0.f, GW - 1.001f);
            y = clampf(y, 0.f, GH - 1.001f);
            int x0 = (int)x, y0 = (int)y;
            float fx = x - x0, fy = y - y0;
            const float* p = f + y0 * GW + x0;
            return (p[0] * (1.f - fx) + p[1] * fx) * (1.f - fy) +
                (p[GW] * (1.f - fx) + p[GW + 1] * fx) * fy;
        };

        // ======================= once per frame: step the simulation =======================
        if (X == 0 && Y == 0)
        {
            if (!inited)
            {
                for (int y = 0; y < GH; y++)
                    for (int x = 0; x < GW; x++)
                        dn[y * GW + x] = (y >= GH / 2) ? 1.f : 0.f;   // start half full
                inited = true;
            }

            uint32_t now = micros();
            float dt = (lastUs == 0) ? 0.03f : (now - lastUs) * 1e-6f;
            lastUs = now;
            dt = clampf(dt, 0.005f, 0.04f);

            // ---- IMU: apparent gravity in the device frame = -(specific force) ----
            // This covers tilt AND shaking/acceleration in one go.
            float ax = clampf(imu.data.accelX, -3.f, 3.f);
            float ay = clampf(imu.data.accelY, -3.f, 3.f);
            if (SWAP_XY) { float t = ax; ax = ay; ay = t; }
            float gax = SIGN_X * ax * GRAV;
            float gay = SIGN_Y * ay * GRAV;

            // ---- 1. forces: dense fluid is pulled along apparent gravity, light fluid opposite ----
            float avg = 0.f;
            for (int i = 0; i < N; i++) avg += dn[i];
            avg /= N;
            for (int i = 0; i < N; i++)
            {
                float f = (dn[i] - avg) * dt;
                vx[i] += gax * f;
                vy[i] += gay * f;
            }

            // touch = stir with the finger's motion
            bool haveTouch = touched;
            float tcx = touchX * SX, tcy = touchY * SY;
            if (haveTouch && prevTouched)
            {
                float fvx = clampf((tcx - prevTX) / dt, -60.f, 60.f);
                float fvy = clampf((tcy - prevTY) / dt, -60.f, 60.f);
                const float R = 5.f;
                int x0 = max(0, (int)(tcx - R)), x1 = min(GW - 1, (int)(tcx + R));
                int y0 = max(0, (int)(tcy - R)), y1 = min(GH - 1, (int)(tcy + R));
                for (int y = y0; y <= y1; y++)
                    for (int x = x0; x <= x1; x++)
                    {
                        float dx = x - tcx, dy = y - tcy;
                        float d = sqrtf(dx * dx + dy * dy);
                        if (d < R)
                        {
                            float w = (1.f - d / R) * 0.6f;
                            vx[y * GW + x] += (fvx - vx[y * GW + x]) * w;
                            vy[y * GW + x] += (fvy - vy[y * GW + x]) * w;
                        }
                    }
            }
            prevTouched = haveTouch; prevTX = tcx; prevTY = tcy;

            // ---- 2. advect velocity (semi-Lagrangian) ----
            for (int y = 0; y < GH; y++)
                for (int x = 0; x < GW; x++)
                {
                    int i = y * GW + x;
                    float bx = x - vx[i] * dt, by = y - vy[i] * dt;
                    vx2[i] = sample(vx, bx, by);
                    vy2[i] = sample(vy, bx, by);
                }
            for (int i = 0; i < N; i++)
            {
                vx[i] = clampf(vx2[i] * VEL_DAMP, -MAX_V, MAX_V);
                vy[i] = clampf(vy2[i] * VEL_DAMP, -MAX_V, MAX_V);
            }

            // ---- 3. project: make velocity divergence-free (solid walls) ----
            auto enforceWalls = [&]() {
                for (int y = 0; y < GH; y++) { vx[y * GW] = 0.f; vx[y * GW + GW - 1] = 0.f; }
                for (int x = 0; x < GW; x++) { vy[x] = 0.f; vy[(GH - 1) * GW + x] = 0.f; }
            };
            enforceWalls();

            auto U = [&](int x, int y) -> float {
                if (x < 0 || x >= GW) return 0.f;
                y = y < 0 ? 0 : (y >= GH ? GH - 1 : y);
                return vx[y * GW + x];
            };
            auto V = [&](int x, int y) -> float {
                if (y < 0 || y >= GH) return 0.f;
                x = x < 0 ? 0 : (x >= GW ? GW - 1 : x);
                return vy[y * GW + x];
            };
            for (int y = 0; y < GH; y++)
                for (int x = 0; x < GW; x++)
                    dv[y * GW + x] = -0.5f * (U(x + 1, y) - U(x - 1, y) + V(x, y + 1) - V(x, y - 1));

            for (int it = 0; it < P_ITERS; it++)
                for (int y = 0; y < GH; y++)
                {
                    int ym = y > 0 ? y - 1 : y, yp = y < GH - 1 ? y + 1 : y;
                    for (int x = 0; x < GW; x++)
                    {
                        int xm = x > 0 ? x - 1 : x, xp = x < GW - 1 ? x + 1 : x;
                        pr[y * GW + x] = (dv[y * GW + x] + pr[y * GW + xm] + pr[y * GW + xp] +
                                        pr[ym * GW + x] + pr[yp * GW + x]) * 0.25f;
                    }
                }

            for (int y = 0; y < GH; y++)
            {
                int ym = y > 0 ? y - 1 : y, yp = y < GH - 1 ? y + 1 : y;
                for (int x = 0; x < GW; x++)
                {
                    int xm = x > 0 ? x - 1 : x, xp = x < GW - 1 ? x + 1 : x;
                    vx[y * GW + x] -= 0.5f * (pr[y * GW + xp] - pr[y * GW + xm]);
                    vy[y * GW + x] -= 0.5f * (pr[yp * GW + x] - pr[ym * GW + x]);
                }
            }
            enforceWalls();

            // ---- 4. advect the density (water/air) field ----
            for (int y = 0; y < GH; y++)
                for (int x = 0; x < GW; x++)
                {
                    int i = y * GW + x;
                    dn2[i] = sample(dn, x - vx[i] * dt, y - vy[i] * dt);
                }

            // keep the interface sharp (push toward 0/1) and conserve water volume
            float sum = 0.f, sw = 0.f;
            for (int i = 0; i < N; i++)
            {
                float d = clampf(dn2[i], 0.f, 1.f);
                d += 0.4f * d * (1.f - d) * (2.f * d - 1.f);
                d = clampf(d, 0.f, 1.f);
                dn2[i] = d;
                sum += d;
                sw  += 4.f * d * (1.f - d);
            }
            float k = (sw > 1.f) ? clampf((0.5f * N - sum) / sw, -0.2f, 0.2f) : 0.f;
            for (int i = 0; i < N; i++)
                dn[i] = clampf(dn2[i] + k * 4.f * dn2[i] * (1.f - dn2[i]), 0.f, 1.f);
        }

        // ======================= per pixel: render =======================
        float gx = clampf((X + 0.5f) * SX - 0.5f, 0.f, GW - 1.001f);
        float gy = clampf((Y + 0.5f) * SY - 0.5f, 0.f, GH - 1.001f);
        int   x0 = (int)gx, y0 = (int)gy;
        float fx = gx - x0, fy = gy - y0;

        const float* dp = dn + y0 * GW + x0;
        float d00 = dp[0], d10 = dp[1], d01 = dp[GW], d11 = dp[GW + 1];
        float d   = (d00 * (1.f - fx) + d10 * fx) * (1.f - fy) + (d01 * (1.f - fx) + d11 * fx) * fy;
        float gdx = (d10 - d00) * (1.f - fy) + (d11 - d01) * fy;   // density gradient = surface normal
        float gdy = (d01 - d00) * (1.f - fx) + (d11 - d10) * fx;

        int   ci  = (int)(gy + 0.5f) * GW + (int)(gx + 0.5f);
        float spd = sqrtf(velocity_buffer[ci] * velocity_buffer[ci] +
                        velocity_buffer[N + ci] * velocity_buffer[N + ci]);

        float w = clampf((d - 0.3f) / 0.4f, 0.f, 1.f);
        w = w * w * (3.f - 2.f * w);                                // smooth water mask

        float edge  = clampf(sqrtf(gdx * gdx + gdy * gdy) * 1.2f, 0.f, 1.f);
        float spec  = clampf(-(gdx + gdy) * 0.7f, 0.f, 1.f);        // light from the top-left
        float foam  = clampf(spd / 30.f, 0.f, 1.f) * 0.3f;

        float wr = 0.05f + 0.35f * edge + 0.45f * spec + foam;
        float wg = 0.35f + 0.30f * edge + 0.45f * spec + foam;
        float wb = 0.85f + 0.10f * edge + 0.15f * spec + foam;

        float r = 0.02f * (1.f - w) + wr * w;
        float g = 0.03f * (1.f - w) + wg * w;
        float b = 0.07f * (1.f - w) + wb * w;

        r = clampf(r, 0.f, 1.f); g = clampf(g, 0.f, 1.f); b = clampf(b, 0.f, 1.f);
        return tft.color565((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
    }

    /*uint16_t getColor(int X, int Y)
    {
        float tX = (touched) ? 2*((float)touchX / SCREEN_WIDTH) - 1: -2;
        float tY = (touched) ? 2*((float)touchY / SCREEN_WIDTH) - 1 : -2;
        auto data = imu.data;

        float uvx = (float)X / (float)SCREEN_WIDTH;
        float uvy = (float)Y / (float)SCREEN_HEIGHT;

        // lv_color_hsv_to_rgb((int)(time * 2) % 360, sin(time)*50+50, 80)
        float r = 0, g = 0,b = 0;


        return tft.color565((uint8_t)(r * 255),(uint8_t)(g * 255),(uint8_t)(b * 255));
    }*/

    void drawCall()
    {
        touched = tft.getTouch(&touchX, &touchY);

        for (int X = 0; X < SCREEN_WIDTH; X++)
        {
            for (int Y = 0; Y < SCREEN_HEIGHT; Y++)
            {
                buffer[Y * SCREEN_WIDTH + X] = getColor(X,Y);
            }
        }


        time += dt;
    }
};

FluidShader shader;
#else
struct Particle
{
    float x;
    float y;
    float rad;

    float vx = 0;
    float vy = 0;
    float ax = 0;
    float ay = 0;

    void setAccel(float x, float y)
    {
        ax = x; ay = y;
    }

    void update(float dt)
    {
        vy += ay * dt;
        y += vy * dt;
        
        vx += ax * dt;
        x += vx * dt;
    }

    void draw()
    {
        tft.drawCircle(x, y, rad, TFT_WHITE);
    }
};

Particle balls[8];
#endif

void setup() {
    Serial.begin(115200);
    Serial.println("Boot");
    
    Wire.begin(IMU_SDA, IMU_SCL);
    tft.begin();
    tft.setRotation(SCREEN_ROATAION);

    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString("Connecting IMU...",tft.width()/2, tft.height()/2, 2);
    
    while(imu.beginI2C(i2cAddress) != BMI2_OK)
    {
        tft.fillScreen(TFT_BLACK);
        tft.drawCentreString("ERROR",tft.width()/2, tft.height()/2, 2);

        // Wait a bit to see if connection is established
        delay(1000);
    }

    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString("Connected",tft.width()/2, tft.height()/2, 2);
    touch_calibrate();

    // init simulation
    #ifdef BILLIARD
    for (int i = 0; i < 8; i++)
    {
        int ra = esp_random() & 15;
        balls[i].x = ra * 60.f;
        balls[i].y = (i / 4.f) * 100.f;
        balls[i].rad = fmaxf(fminf((float)esp_random(), 60.f), 5.f);
    }
    #endif
}

void loop()
{
    imu.getSensorData();
#ifndef BILLIARD
    shader.drawCall();
    tft.pushPixelsDMA(buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
#else    
    for (int i = 0; i < 8; i++)
    {
        Particle& p = balls[i];
        p.draw();
    
        if (p.x + p.rad > SCREEN_WIDTH)
        {
            p.vx *= -1; p.x = SCREEN_WIDTH - p.rad;
        }
        else if (p.x - p.rad < 0)
        {
            p.vx *= -1; p.x = p.rad;
        }
        else if (p.y + p.rad > SCREEN_HEIGHT)
        {
            p.vy *= -1; p.y = SCREEN_HEIGHT - p.rad;
        }
        else if (p.y - p.rad < 0)
        {
            p.vy *= -1; p.y = p.rad;
        }

        for (int j = 0; j < 8; j++)
        {
            if ((p.x-balls[i].x)*(p.x-balls[i].x) + (p.y-balls[i].y)*(p.y-balls[i].y) <= (p.rad+balls[i].rad)*(p.rad+balls[i].rad))
            {
                p.ax = -p.vx;
                p.ay = -p.vy;

                balls[i].ax = -balls[i].vx;
                balls[i].ay = -balls[i].vy;
            }
        }

        float ax = -imu.data.accelX; // might need to change 
        float ay = -imu.data.accelY;

        p.setAccel(ax, ay);
        p.update(5.f / 1000.f);
    }
#endif
    delay(5);           // let this time pass
}

// Code to run a screen calibration, not needed when calibration values set in setup()
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // Calibrate
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  Serial.println(); Serial.println();
  Serial.println("// Use this calibration code in setup():");
  Serial.print("  uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();

  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete!");
  tft.println("Calibration code sent to Serial port.");

  delay(4000);
}