#include <main.h>
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define BUZZER 2
#define BUTTON_1 3
#define BUTTON_2 4
#define BUTTON_3 8       // Pin connected to the push button
#define BUTTON_4 9
#define LED 13
#define SLAVE_ADDRESS 0x40
#define MAX_DATA_COUNT 30



RTC_DS3231 rtc;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




bool lastButton1State = HIGH; // Last state of button 1
bool lastButton2State = HIGH;
bool lastButton3State = HIGH;
bool lastButton4State = HIGH;

char mlxValueStr[10];
float mlxValueAll;
int counter = 0;
float mlxValueToSend;
char buffer[10];

int eepromAddress = 0;
int mode = 1;

void setup() {
  Serial.begin(9600);
  Wire.begin(10);
  mlx.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize OLED display
  EEPROM.begin();       

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  while (!Serial) {
    ; // wait for serial port to connect
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1) {}
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, resetting the date and time...");
    rtc.adjust(DateTime(2024, 3, 12, 21, 56, 10)); // Example for current date and time
  }
}

void saveToEEPROM(float mlxValue) {
  EEPROM.put(eepromAddress, mlxValue);
  eepromAddress += sizeof(float);
}

void printEEPROM() {
  Serial.println("EEPROM Data:");
  for (int addr = 0; addr < eepromAddress; addr += sizeof(float)) {
    EEPROM.get(addr, mlxValueAll);
    Serial.print("Address ");
    Serial.print(addr);
    Serial.print(": ");
    Serial.println(mlxValueAll);
  }
}

void count(int countValue) {
   // Display the count number in the top right corner with smaller font
  display.setTextSize(1);
  display.setCursor(display.width() - 30, 0); // Set cursor to top right corner
  display.print(countValue); // Print the current count
  
  display.print("/"); // Print separator
  display.print(MAX_DATA_COUNT); // Print the maximum count
}

void scanOnce() {
  counter += 1;
  // display.fillRect(0, display.height() - 8, display.width(), 8, SSD1306_BLACK); 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  float mlxValue = mlx.readObjectTempC(); // Read temperature from MLX sensor
  dtostrf(mlxValue, 4, 2, mlxValueStr); // Convert float to string
  String celcius = " C";

  String tempCelcius = mlxValueStr + celcius;

  int16_t textWidth = tempCelcius.length() * 12; // Assuming a 12-pixel width per character in text size 2
  int16_t textHeight = 16; // Assuming a height of 16 pixels for text size 2
  
  // Calculate the position to center the text horizontally and vertically
  int16_t x = (display.width() - textWidth) / 2;
  int16_t y = (display.height() - textHeight) / 2;

  display.setCursor(x, y);
  display.print(tempCelcius);


  count(counter);

  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);

  saveToEEPROM(mlxValue); // Save value to EEPROM
  Serial.println(mlxValue); // Print value to Serial Monitor

  float emissivityValue = 5;
  Serial.print("Emissivity: ");
  Serial.println(emissivityValue);

  display.display();

}

void displayMode(String text) {
  
  display.fillRect(0, display.height() - 9, display.width(), 8, SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(0, display.height() - 9); // Set cursor to bottom left corner
  display.print(text);
  
  display.display();
}

void switchMode() {
  mode++; // Tambah mode
  
  // Jika mode melebihi jumlah mode yang diinginkan, kembali ke mode pertama
  if (mode > 3) {
    mode = 1;
  }

  switch (mode) {
    case 1:
      displayMode("Vendy");
      break;
    case 2:
      displayMode("Sanjaya");
      break;
    case 3:
      displayMode("Pranoto");
      break;
  }
}

void requestEvent() {
  dtostrf(mlxValueToSend, 4, 2, buffer);
  Wire.write(buffer);
}

void sending(){
  for (int addr = 0; addr < eepromAddress; addr += sizeof(float)) {
      EEPROM.get(addr, mlxValueToSend);  // Read mlxValueAll from EEPROM
      // Send data to slave
      Wire.onRequest(requestEvent);
    }
    Serial.println("Data dikirim ke slave.");

    // Reset EEPROM to address 0
    eepromAddress = 0;
    counter = 0; 
    Serial.println("EEPROM direset.");
  
    count(counter);

  display.clearDisplay();

  display.setTextSize(2); // Change text size to 2
  display.setTextColor(SSD1306_WHITE);
  
  // Teks yang akan ditampilkan
  String reset = "Senpai!";
  
  // Calculate the width and height of the text
  int16_t textWidth = reset.length() * 12; // Assuming a 12-pixel width per character in text size 2
  int16_t textHeight = 16; // Assuming a height of 16 pixels for text size 2
  
  // Calculate the position to center the text horizontally and vertically
  int16_t x = (display.width() - textWidth) / 2;
  int16_t y = (display.height() - textHeight) / 2;
  
  // Set the cursor position and print the text
  display.setCursor(x, y);
  display.print(reset); 
  
  // Display the content
  display.display();
} 


void loop() {
  bool button1State = digitalRead(BUTTON_1);
  bool button2State = digitalRead(BUTTON_2);
  bool button3State = digitalRead(BUTTON_3);
  bool button4State = digitalRead(BUTTON_4);

  if (button1State == LOW && lastButton1State == HIGH) { // Button 1 pressed
    lastButton1State = button1State; // Update lastButton1State
    if (eepromAddress < MAX_DATA_COUNT * sizeof(float)) { // Check if maximum data count is not exceeded
      scanOnce(); // Perform scanning once
      delay(500);
    } else {
      Serial.println("Maximum data count exceeded. New data not saved."); // Print error message
    }
  } else {
    lastButton1State = button1State; // Update lastButton1State
  }

  if (button2State == LOW && lastButton2State == HIGH) {
    lastButton2State = button2State;
    printEEPROM();
    delay(1000);
  }else {s
  
    lastButton2State = button2State; // Update lastButton1State
  }

  if (button3State == LOW && lastButton3State == HIGH) { 
    lastButton3State = button3State;
    sending();
    delay(500);
    
  }else {
    lastButton3State = button3State; // Update lastButton1State
  }

  if (button4State == LOW && lastButton4State == HIGH) {
    lastButton4State = button4State;
    switchMode();
    delay(500);
  }else {
    lastButton4State = button4State; // Update lastButton1State
  }


}