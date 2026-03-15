#define BLYNK_PRINT Serial


#define BLYNK_TEMPLATE_ID "TMPL3F52PHmtp"
#define BLYNK_TEMPLATE_NAME "Smart Refigerator stock monitoring"
#define BLYNK_AUTH_TOKEN "cxNKosjsA-OC5YARxkowB3cA9r5iYii7"

/* -------- INCLUDES -------- */
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>



/* -------- WIFI -------- */
char ssid[] = "SmartRefrigerator";
char pass[] = "12345678";

/* -------- RFID -------- */
#define SS_PIN 5
#define RST_PIN 22
#define REMOVE_DELAY 30000

/* -------- VIRTUAL PINS -------- */
#define VP_TOTAL     V0
#define VP_MILK      V1
#define VP_LAST_SCAN V2
#define VP_CHEESE    V3
#define VP_APPLE     V4
#define VP_POTATO    V5

MFRC522 rfid(SS_PIN, RST_PIN);

/* -------- ALERT FLAGS -------- */
bool milkAlert   = false;
bool cheeseAlert = false;
bool appleAlert  = false;
bool potatoAlert = false;
bool noStockAlert = false;

/* -------- FOOD STRUCT (NO String) -------- */
struct FoodItem {
  const char* uid;
  const char* name;
  int expDay, expMonth, expYear;
  bool isInside;
  unsigned long lastScan;
};

/* -------- FOOD DATABASE -------- */
FoodItem foods[] = {
  {"0141F605", "Milk",   9, 1, 2026, false, 0},
  {"FA373806", "Milk",   12, 1, 2026, false, 0},
  {"C4BEF605", "Cheese",  8, 1, 2026, false, 0},
  {"9A3CF905", "Apple",  15, 1, 2026, false, 0},
  {"BA63F605", "Potato",  10, 1, 2026, false, 0}
};

const int FOOD_COUNT = sizeof(foods) / sizeof(foods[0]);

/* -------- DATE UTILS -------- */
int daysFromDate(int d, int m, int y) {
  return y * 365 + m * 30 + d;
}

/* -------- DASHBOARD UPDATES -------- */
void updateCounts(int total, int milk, int cheese, int apple, int potato) {
  if (!Blynk.connected()) return;

  Blynk.virtualWrite(VP_TOTAL, total);
  Blynk.virtualWrite(VP_MILK, milk);
  Blynk.virtualWrite(VP_CHEESE, cheese);
  Blynk.virtualWrite(VP_APPLE, apple);
  Blynk.virtualWrite(VP_POTATO, potato);
}

void updateLastScan(const char* item, const char* action) {
  if (!Blynk.connected()) return;

  char msg[50];
  snprintf(msg, sizeof(msg), "%s %s", item, action);
  Blynk.virtualWrite(VP_LAST_SCAN, msg);
}

/* -------- ITEM ALERT HANDLER -------- */
void checkItemAlert(const char* name, int count, bool &flag, const char* eventID) {
  if (!Blynk.connected()) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int today = daysFromDate(
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
  );

  int nearestExpiry = 9999;

  for (int i = 0; i < FOOD_COUNT; i++) {
    if (foods[i].isInside && strcmp(foods[i].name, name) == 0) {
      int exp = daysFromDate(foods[i].expDay, foods[i].expMonth, foods[i].expYear);
      int diff = exp - today;
      if (diff < nearestExpiry) nearestExpiry = diff;
    }
  }

  char msg[60];

  if (count == 0 && !flag) {
    snprintf(msg, sizeof(msg), "%s stock is low", name);
    Blynk.logEvent(eventID, msg);
    flag = true;
  }
  else if (nearestExpiry < 0 && !flag) {
    snprintf(msg, sizeof(msg), "%s item has expired", name);
    Blynk.logEvent(eventID, msg);
    flag = true;
  }
  else if (nearestExpiry == 3 && !flag) {
    snprintf(msg, sizeof(msg), "%s expires in 3 days", name);
    Blynk.logEvent(eventID, msg);
    flag = true;
  }
  else if (nearestExpiry == 1 && !flag) {
    snprintf(msg, sizeof(msg), "%s expires in 24 hours", name);
    Blynk.logEvent(eventID, msg);
    flag = true;
  }
  else if (count > 0 && nearestExpiry > 3) {
    flag = false;
  }
}

/* -------- CHECK ALL ITEMS + NO STOCK -------- */
void checkAll() {
  int milk = 0, cheese = 0, apple = 0, potato = 0;

  for (int i = 0; i < FOOD_COUNT; i++) {
    if (foods[i].isInside) {
      if (strcmp(foods[i].name, "Milk") == 0) milk++;
      else if (strcmp(foods[i].name, "Cheese") == 0) cheese++;
      else if (strcmp(foods[i].name, "Apple") == 0) apple++;
      else if (strcmp(foods[i].name, "Potato") == 0) potato++;
    }
  }

  int total = milk + cheese + apple + potato;

  updateCounts(total, milk, cheese, apple, potato);

  /* ---- NO STOCK ALERT ---- */
  if (total == 0 && !noStockAlert && Blynk.connected()) {
    Blynk.logEvent("no_stock", "fridge is empty");
    noStockAlert = true;
  }
  if (total > 0) {
    noStockAlert = false;
  }

  checkItemAlert("Milk", milk, milkAlert, "low_milk");
  checkItemAlert("Cheese", cheese, cheeseAlert, "low_cheese");
  checkItemAlert("Apple", apple, appleAlert, "low_apple");
  checkItemAlert("Potato", potato, potatoAlert, "low_potato");
}

/* -------- SETUP -------- */
void setup() {
  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  configTime(19800, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) delay(500);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

/* -------- LOOP -------- */
void loop() {
  Blynk.run();

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  char uid[10];
  byte idx = 0;
  for (byte i = 0; i < rfid.uid.size; i++) {
    idx += sprintf(uid + idx, "%02X", rfid.uid.uidByte[i]);
  }

  unsigned long now = millis();

  for (int i = 0; i < FOOD_COUNT; i++) {
    if (strcmp(uid, foods[i].uid) == 0) {

      if (!foods[i].isInside) {
        foods[i].isInside = true;
        foods[i].lastScan = now;
        updateLastScan(foods[i].name, "added to fridge");
      }
      else if (now - foods[i].lastScan > REMOVE_DELAY) {
        foods[i].isInside = false;
        foods[i].lastScan = now;
        updateLastScan(foods[i].name, "removed from fridge");
      }

      checkAll();
      break;
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
