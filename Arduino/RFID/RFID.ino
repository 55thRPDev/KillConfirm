#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <hd44780.h>                        // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header

#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above

hd44780_I2Cexp lcd;  // Declare LCD object

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
int blueScore = 0; // Blue team score
int redScore = 0;  // Red team score

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

const int MAX_CARDS = 10;          // Maximum number of score cards to store
byte storedUIDs[MAX_CARDS][4];     // Array to store score card UIDs, each with 4 bytes
int uidCount = 0;                  // Number of stored score card UIDs

byte accessUID[4] = {0xB3, 0xDB, 0xF9, 0x2C};          // Predefined access UID for BLUE team
byte redteamAccess[4] = {0x43, 0xC5, 0xCB, 0x2C};      // Predefined access UID for RED team
bool accessGranted = false;         // Access flag to allow scoring
String lastTeam = "";               // Track which team has the access
unsigned long accessStartTime = 0;  // Time when the access card was last successfully scanned
const unsigned long ACCESS_WINDOW = 60000;  // 1 minute window (60000 milliseconds)

int greenPin = 2;

void setup() {
  pinMode(greenPin, OUTPUT);

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  lcd.print("Scan Access Card..");
  lcd.setCursor(0, 1);
  lcd.print("BLUE: " + String(blueScore) + " RED: " + String(redScore));

  Serial.begin(9600);
  while (!Serial); // Wait for serial port to open

  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  delay(4); // Optional delay

  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  // Check if the access window has expired
  if (accessGranted && millis() - accessStartTime >= ACCESS_WINDOW) {
    // Access window expired, require the access card again
    accessGranted = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Access Card");
    lcd.setCursor(0, 1);
    lcd.print("BLUE: " + String(blueScore) + " RED: " + String(redScore));
    delay(2000);
  }

  // Check for new card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return; // No new card or error reading card
  }

  // Print out the scanned UID
  printScannedUID();

  if (isAccessCard(accessUID)) {
    grantAccess("BLUE");  // Grant access when the BLUE team access card is scanned
  } else if (isAccessCard(redteamAccess)) {
    grantAccess("RED");  // Grant access when the RED team access card is scanned
  } else if (accessGranted && !isDuplicate()) {
    storeScoreCard();  // Store the score card if access is granted and it's not a duplicate
  } else if (accessGranted && isDuplicate()) {
    cardExists();  // Notify if the score card already exists
  } else {
    unauthorizedCard();  // Unauthorized card action
  }

  mfrc522.PICC_HaltA(); // Halt PICC
}

bool isAccessCard(byte teamUID[4]) {
  // Ensure the card UID size matches the expected size
  if (mfrc522.uid.size != 4) {
    Serial.println("Incorrect UID size, expected 4 bytes.");
    return false; // If the card UID size is not 4 bytes, it can't be the access card
  }

  // Check if the UID matches the specified team UID
  for (byte i = 0; i < 4; i++) {
    Serial.print("Comparing UID byte ");
    Serial.print(i);
    Serial.print(": scanned=");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" expected=");
    Serial.println(teamUID[i], HEX);

    if (mfrc522.uid.uidByte[i] != teamUID[i]) {
      Serial.println("Byte mismatch detected.");
      return false;  // Return false if any byte does not match
    }
  }
  Serial.println("Access card recognized.");
  return true;  // Return true if all bytes match
}

bool isDuplicate() {
  for (int i = 0; i < uidCount; i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (storedUIDs[i][j] != mfrc522.uid.uidByte[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return true;
    }
  }
  return false;
}

void printScannedUID() {
  Serial.print("Scanned UID: ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""); // Ensure two-digit format
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    lcd.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""); // Ensure two-digit format
    lcd.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) {
      lcd.print(":"); // Add a colon between UID bytes
    }
  }
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print("Checking...");
  delay(2000);
}

void grantAccess(String team) {
  accessGranted = true;
  accessStartTime = millis(); // Set the access start time
  lastTeam = team;  // Remember which team's access card was last scanned
  Serial.println(team + " Access card scanned. Ready to scan score cards.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(team + " Access Granted");
  lcd.setCursor(0, 1);
  lcd.print("Scan Score Card");
  delay(2000);
}

void storeScoreCard() {
  if (uidCount < MAX_CARDS) {
    for (byte i = 0; i < 4; i++) {
      storedUIDs[uidCount][i] = mfrc522.uid.uidByte[i];
    }
    uidCount++;

    // Increment the correct team's score
    if (lastTeam == "BLUE") {
      blueScore++;
    } else if (lastTeam == "RED") {
      redScore++;
    }

    accessGranted = false;  // Reset access after one score card is scanned
    Serial.println("New score card scanned and stored.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Score Card Stored");
    lcd.setCursor(0, 1);
    lcd.print("BLUE: " + String(blueScore) + " RED: " + String(redScore));

    digitalWrite(greenPin, HIGH);
    delay(2000);
    digitalWrite(greenPin, LOW);

    resetDisplay();
  } else {
    Serial.println("Max score cards reached.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Max Cards Scanned");
    delay(2000);
    resetDisplay();
  }
}

void cardExists() {
  Serial.println("Score card already exists.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Card Already");
  lcd.setCursor(0, 1);
  lcd.print("Exists :(");


  delay(2000);
  resetDisplay();
}

void unauthorizedCard() {
  Serial.println("Unauthorized card.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unauthorized Card");
  delay(2000);
  resetDisplay();
}

void resetDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Access Card...");
  lcd.setCursor(0, 1);
  lcd.print("BLUE: " + String(blueScore) + " RED: " + String(redScore));
}
