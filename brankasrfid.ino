#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// --- Konstanta Pin ---
#define RST_PIN       6
#define SS_PIN        10
#define LED_PIN       2
#define BUZZER_PIN    4
#define SERVO_PIN     18

// --- Inisialisasi Komponen ---
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;

// --- UID Kartu yang Diizinkan ---
byte allowedUIDs[3][4] = {
  {0x82, 0x1E, 0x8B, 0x3F},
  {0x89, 0x62, 0xD1, 0xB9},
  {0x62, 0xDD, 0xB8, 0x7C}
};
const int numAllowedUIDs = 3;

// --- Setup Awal ---
void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  myservo.attach(SERVO_PIN, 1000, 2000);
  myservo.write(90); // Posisi awal = tertutup

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println("Loker Aman RFID Siap...");
}

// --- Fungsi Mengecek UID ---
bool isAuthorizedCard(byte *uid, byte size) {
  if (size != 4) return false;
  for (int i = 0; i < numAllowedUIDs; i++) {
    bool match = true;
    for (int j = 0; j < 4; j++) {
      if (uid[j] != allowedUIDs[i][j]) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}

// --- Loop Utama ---
void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID Kartu:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (isAuthorizedCard(mfrc522.uid.uidByte, mfrc522.uid.size)) {
    // --- Akses Diberikan ---
    Serial.println("Akses Diberikan!");
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
    delay(200);
    noTone(BUZZER_PIN);

    myservo.write(0);  // Buka loker
    delay(5000);       // Tunggu 5 detik

    myservo.write(90); // Tutup loker
    delay(1000);       // Tunggu hingga servo selesai

    digitalWrite(LED_PIN, LOW);
    Serial.println("Loker Tertutup, Siap...");
  } else {
    // --- Akses Ditolak ---
    Serial.println("Akses Ditolak!");
    digitalWrite(LED_PIN, LOW);
    tone(BUZZER_PIN, 500);
    delay(500);
    noTone(BUZZER_PIN);
  }

  // Stop komunikasi kartu
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
