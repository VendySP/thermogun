#include <main.h>

#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define BUZZER 2
#define BUTTON_1 3
#define BUTTON_2 4
#define LED 13


RTC_DS3231 rtc;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mlx.begin();




  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
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
    // You can set the date and time here if you want to initialize it on power loss
    rtc.adjust(DateTime(2024, 3, 12, 21, 56, 10)); // Example for current date and time
  }

  


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
}

void loop() {
  bool button1State = digitalRead(BUTTON_1);        // default state is HIGH because the button is INPUT_PULL_UP, when pressed become LOW
  // bool button2State = digitalRead(BUTTON_2);


  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);


  DateTime now = rtc.now();
  String timeStr = "";
  timeStr += String(now.year(), DEC) + "-";
  timeStr += String(now.month(), DEC) + "-";
  timeStr += String(now.day(), DEC) + " ";
  timeStr += String(now.hour(), DEC) + ":";
  timeStr += String(now.minute(), DEC) + ":";
  timeStr += String(now.second(), DEC);

  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x, &y, &w, &h);
  x = (display.width() - w) / 2;
  y = (display.height() - h) / 2;
  display.setCursor(x, y);
  display.println(timeStr);
  display.display();



  if (!button1State) {
    digitalWrite(BUZZER, HIGH);
    Serial.println(timeStr);
    delay(100);

  } else {
    digitalWrite(BUZZER, LOW);
  }

  // delay(1000);
}





