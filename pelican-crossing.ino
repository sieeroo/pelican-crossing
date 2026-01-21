#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_7segment sevenSeg = Adafruit_7segment();

// ===== TRAFFIC =====
#define TRAFFIC_GREEN 8
#define TRAFFIC_YELLOW 9
#define TRAFFIC_RED 10

// ===== PEDESTRIAN =====
#define PED_L_GREEN 4
#define PED_L_RED 3
#define PED_R_GREEN 12
#define PED_R_RED 11

// ===== BUTTON =====
#define BUTTON1 5
#define BUTTON2 6

// ===== BUZZER =====
#define BUZZER 2

// ===== POTENTIOMETER =====
#define POT_PIN A0

bool systemBusy = false;

// ======= BUZZER ======
void pelicanBeepNormal(){
    tone(BUZZER, 2200, 70);
}
void pelicanBeepFast(){
    tone(BUZZER, 2200, 40);
}
void pelicanBeepStart(){
    tone(BUZZER, 1800, 500);
    delay(100);
    noTone(BUZZER);
}

// =========================
void setup(){
    pinMode(TRAFFIC_GREEN, OUTPUT);
    pinMode(TRAFFIC_YELLOW, OUTPUT);
    pinMode(TRAFFIC_RED, OUTPUT);

    pinMode(PED_L_GREEN, OUTPUT);
    pinMode(PED_L_RED, OUTPUT);
    pinMode(PED_R_GREEN, OUTPUT);
    pinMode(PED_R_RED, OUTPUT);

    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);

    Wire.begin();
    lcd.init();
    lcd.backlight();

    sevenSeg.begin(0x70);
    sevenSeg.clear();
    sevenSeg.writeDisplay();

    // KONDISI AWAL
    digitalWrite(TRAFFIC_GREEN, HIGH);
    digitalWrite(PED_L_RED, HIGH);
    digitalWrite(PED_R_RED, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("Lalu Lintas");
    lcd.setCursor(0, 1);
    lcd.print("Aktif");
}

void loop(){
    bool button1Pressed = (digitalRead(BUTTON1) == LOW);
    bool button2Pressed = (digitalRead(BUTTON2) == LOW);

    if (!systemBusy && (button1Pressed || button2Pressed)){
        pelicanBeepStart();
        systemBusy = true;

        int activeButton = button1Pressed ? 1 : 2;
        pelicanCrossing(activeButton);

        systemBusy = false;
    }
}

void pelicanCrossing(int startButton){
    int potRaw = analogRead(POT_PIN);
    int potLevel = map(potRaw, 0, 1023, 0, 30);
    potLevel = constrain(potLevel, 0, 30);

    String status;
    int trafficDelay;

    if (potLevel <= 7){
        status = "SEPI";
        trafficDelay = map(potLevel, 0, 7, 10, 20);
    }else if (potLevel <= 15){
        status = "NORMAL";
        trafficDelay = map(potLevel, 8, 15, 20, 25);
    }else if (potLevel <= 22){
        status = "RAMAI";
        trafficDelay = map(potLevel, 16, 22, 30, 40);
    }else{
        status = "PADAT";
        trafficDelay = map(potLevel, 23, 30, 50, 60);
    }

    // tampilkan kondisi traffic
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kondisi: ");
    lcd.print(status);
    lcd.setCursor(0, 1);
    lcd.print("Mohon Tunggu");

    for (int i = trafficDelay; i >= 0; i--){
        sevenSeg.print(i);
        sevenSeg.writeDisplay();

        if (i <= 7){
            digitalWrite(TRAFFIC_GREEN, LOW);
            digitalWrite(TRAFFIC_YELLOW, HIGH);
            delay(500);
            digitalWrite(TRAFFIC_YELLOW, LOW);
            delay(500);
        }else{
            digitalWrite(TRAFFIC_GREEN, HIGH);
            digitalWrite(TRAFFIC_YELLOW, LOW);
            delay(1000);
        }
    }

    // === TRAFFIC BERHENTI ===
    digitalWrite(TRAFFIC_GREEN, LOW);
    digitalWrite(TRAFFIC_YELLOW, LOW);
    digitalWrite(TRAFFIC_RED, HIGH);

    // === PEDESTRIAN AKTIF (KEDUA SISI) ===
    digitalWrite(PED_L_RED, LOW);
    digitalWrite(PED_R_RED, LOW);
    digitalWrite(PED_L_GREEN, HIGH);
    digitalWrite(PED_R_GREEN, HIGH);

    lcd.clear();
    lcd.print("Penyeberangan");
    lcd.setCursor(0, 1);
    lcd.print("Silakan Jalan");

    int crossTime = 20;
    unsigned long lastTime = 0;

    while (crossTime >= 0){
        if (digitalRead(BUTTON1) == LOW || digitalRead(BUTTON2) == LOW){
            if (crossTime < 20){
                crossTime = 20; // tambah waktu jadi 25 detik
                lcd.setCursor(0, 1);
                lcd.print("WAKTU DITAMBAH");
                delay(10);
            }
        }

        if (millis() - lastTime >= 1000){
            sevenSeg.print(crossTime);
            sevenSeg.writeDisplay();

            if (crossTime <= 5){
                pelicanBeepFast();
                digitalWrite(PED_L_GREEN, !digitalRead(PED_L_GREEN)); // Toggle kedip
                digitalWrite(PED_R_GREEN, !digitalRead(PED_R_GREEN));
            }else{
                pelicanBeepNormal();
                digitalWrite(PED_L_GREEN, HIGH);
                digitalWrite(PED_R_GREEN, HIGH);
            }

            crossTime--;
            lastTime = millis();
        }
        delay(10);
    }

    // === SELESAI MENYEBERANG ===
    noTone(BUZZER);
    sevenSeg.clear();
    sevenSeg.writeDisplay();

    digitalWrite(PED_L_GREEN, LOW);
    digitalWrite(PED_R_GREEN, LOW);
    digitalWrite(PED_L_RED, HIGH);
    digitalWrite(PED_R_RED, HIGH);

    lcd.clear();
    lcd.print("Pedestrian");
    lcd.setCursor(0, 1);
    lcd.print("Non Aktif");

    delay(1000);

    // === TRAFFIC AKTIF KEMBALI ===
    digitalWrite(TRAFFIC_RED, LOW);
    digitalWrite(TRAFFIC_YELLOW, HIGH);
    delay(1000);
    digitalWrite(TRAFFIC_YELLOW, LOW);
    digitalWrite(TRAFFIC_GREEN, HIGH);

    lcd.clear();
    lcd.print("Lalu Lintas");
    lcd.setCursor(0, 1);
    lcd.print("Aktif");
}
