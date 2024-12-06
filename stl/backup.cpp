#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP_Knob.h>
#include <TMCStepper.h>

// OLED Display
#define OLED_RESET 4     // May not be needed for some OLED connections
Adafruit_SSD1306 display(128, 64);
#define SCREEN_ADDRESS 0x3C // I2C address of the OLED display

// Rotary Encoder (Knob)
#define ROTARY_PIN_A 32
#define ROTARY_PIN_B 33
// NO BUTTON PIN defined for this version of ESP32_Knob

// TMC2209 Stepper Driver
#define TMC2209_SERIAL_RX 13
#define TMC2209_SERIAL_TX 12
#define TMC2209_EN_PIN  2
#define R_SENSE 0.11f
#define TMC2209_SLAVE_ADDR 0b00
TMC2209Stepper driver(&Serial1, R_SENSE, 0b00); // Address 0 (default for TMC2209)


// Buttons
const int przycisk1Pin = 6;  // Confirmation button
const int przycisk2Pin = 7;   // Reset button


// Stepper Motor Parameters
const int krokiNaObrot = 200;
const float srednicaSzpuli = 80;  // Diameter of spool in mm

// Global Variables
volatile int wybranaDlugosc = 0;
bool dlugoscWybrana = false;

ESP_Knob knob(ROTARY_PIN_A, ROTARY_PIN_B);
void onKnobLeftEventCallback(int count, void *usr_data)
{
    Serial.printf("Detect left event, count is %d\n", count);
}

void onKnobRightEventCallback(int count, void *usr_data)
{
    Serial.printf("Detect right event, count is %d\n", count);
}

void setup() {
  Serial.begin(9600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Initialize TMC2209 driver
  Serial1.begin(115200, SERIAL_8N1, TMC2209_SERIAL_RX, TMC2209_SERIAL_TX);
  pinMode(TMC2209_EN_PIN, OUTPUT);
  digitalWrite(TMC2209_EN_PIN, LOW); // Enable the driver

  driver.begin();
  driver.toff(5);
  driver.rms_current(500);  // Adjust current as needed
  driver.microsteps(16);    // Adjust microsteps as needed

  // Initialize buttons
  pinMode(przycisk1Pin, INPUT_PULLUP);
  pinMode(przycisk2Pin, INPUT_PULLUP);

  // Initial display message
  display.setCursor(0, 0);
  display.println("Wybierz dlugosc:");
  display.display();
}


void loop() {    // Must call this frequently to update knob state!
  wybranaDlugosc = knob.getCountValue() * 10;  // Use getCountValue()

  if (wybranaDlugosc < 0) {
    wybranaDlugosc = 0;
  }

  if (!dlugoscWybrana) {
    // ... (Display code remains the same)

    if (digitalRead(przycisk1Pin) == LOW) { // ONLY button starts the process now
      dlugoscWybrana = true;
      // ... (rest of the code inside this conditional block is the same)
    }

    if (digitalRead(przycisk2Pin) == LOW) {
      wybranaDlugosc = 0;
      delay(200);  // Debouncing
    }

  } else {
    float obwodSzpuli = PI * srednicaSzpuli;
    int liczbaKrokow = (wybranaDlugosc / obwodSzpuli) * krokiNaObrot;

    if (liczbaKrokow > 0) {
     // driver.move(liczbaKrokow);  // Use TMC2209 driver
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Odwinieto: ");
    display.print(wybranaDlugosc);
    display.println("mm");
    display.display();

    delay(2000);
    wybranaDlugosc = 0;
    dlugoscWybrana = false;
  }
}
