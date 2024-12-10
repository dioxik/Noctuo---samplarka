/*
Podlaczenie do ESP32:

**OLED Display (Adafruit_SH1106G):**
* VCC -> 3.3V 
* GND -> GND
* SCK -> GPIO 22 (SCL)
* SDA -> GPIO 21 (SDA)


**Rotary Encoder:**
* CLK (Encoder A) -> GPIO 32
* DT (Encoder B) -> GPIO 33
* SW (Button)     -> GPIO 34

**TMC2209 Stepper Driver:**
* VMOT -> Motor Power Supply (NOT connected to ESP32)
* GND -> GND
* STEP -> GPIO 26
* DIR -> GPIO 27
* EN -> GPIO 2  
* RX -> GPIO 13 (Serial1 RX) *Note: Using SoftwareSerial can sometimes be unreliable. Hardware Serial is preferred.*
* TX -> GPIO 12 (Serial1 TX) *Note: This pin is only needed if you are using UART communication for configuration or diagnostics and NOT for step/dir control.*


*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TMCStepper.h>
#include <ESP32RotaryEncoder.h>
// OLED Config
#define i2c_Address 0x3c // initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1	 //   QT-PY / XIAO
// end oled

// Rotary Encoder (Knob)
const uint8_t DI_ENCODER_A = 32;
const uint8_t DI_ENCODER_B = 33;
const int8_t DI_ENCODER_SW = 34;

// TMC2209 Stepper Driver
#define TMC2209_SERIAL_RX 13
#define TMC2209_SERIAL_TX 12
#define TMC2209_STEP_PIN        26 // Define the STEP pin
#define TMC2209_DIR_PIN         27 // Define the DIR pin
#define TMC2209_EN_PIN          2  // Define the ENABLE pin
#define R_SENSE 0.11f
#define TMC2209_SLAVE_ADDR 0b00

// States for screen management
enum class ScreenState {
    MAIN,
    LIST,
    EDIT
};
ScreenState currentScreen = ScreenState::MAIN;
int editValue = 0; // Value to be edited



volatile bool turnedRightFlag = false;
volatile bool turnedLeftFlag = false;
void OnLeft();
void OnRight();
void handleEncoder();
void knobCallback(long value);
void buttonCallback(unsigned long duration);
void changeScreen(int direction);
void drawScreen();
void moveStepper(int steps, bool direction);



RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW);
TMC2209Stepper driver(&Serial1, R_SENSE, TMC2209_SLAVE_ADDR); // Address 0 (default for TMC2209)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{

	Serial.begin(9600);
	delay(250); // wait for the OLED to power up
	rotaryEncoder.setEncoderType(EncoderType::HAS_PULLUP);

	rotaryEncoder.setBoundaries(-1, 1, false);
	rotaryEncoder.onTurned(&knobCallback);
	rotaryEncoder.onPressed(&buttonCallback);
	rotaryEncoder.begin();

    // TMC2209 Stepper Driver Setup
    pinMode(TMC2209_STEP_PIN, OUTPUT);
    pinMode(TMC2209_DIR_PIN, OUTPUT);
    pinMode(TMC2209_EN_PIN, OUTPUT);
    digitalWrite(TMC2209_EN_PIN, LOW); // Enable the driver (LOW is typically active)
    // Initialize TMC2209 settings (e.g., current, microsteps)
    driver.begin();  // Initiate the driver
    driver.rms_current(500); // Set RMS current (adjust as needed)
    driver.microsteps(8);   // Set microstepping (adjust as needed)

    Serial.println("TMC2209 initialized");



	display.begin(i2c_Address, true); // Address 0x3C default
									  // display.setContrast (0); // dim display
	display.clearDisplay();
	// text display tests
	display.setTextSize(1);
	display.setTextColor(SH110X_WHITE);
	display.setCursor(0, 0);
	display.println("glupia Berta");
	display.display();
	//  display.clearDisplay();

	Serial.print("online");
}

void loop()
{
	handleEncoder();
}

void moveStepper(int steps, bool direction) {
  digitalWrite(TMC2209_DIR_PIN, direction ? HIGH : LOW); // Set direction

  for (int i = 0; i < abs(steps); i++) {
    digitalWrite(TMC2209_STEP_PIN, HIGH);
    delayMicroseconds(5); // Adjust step pulse duration as needed
    digitalWrite(TMC2209_STEP_PIN, LOW);
    delayMicroseconds(5); // Adjust step pulse duration as needed
  }
}

void knobCallback(long value)
{

	if (turnedRightFlag || turnedLeftFlag)
		return;
	switch (value)
	{
	case 1:
		turnedRightFlag = true;
		break;

	case -1:
		turnedLeftFlag = true;
		break;
	}
	rotaryEncoder.setEncoderValue(0);
}

void buttonCallback(unsigned long duration)
{
	Serial.printf("boop! button was down for %lu ms\n", duration);
}
void handleEncoder()
{
	if (turnedRightFlag)
	{
		OnRight();
	}

	else if (turnedLeftFlag)
	{
		OnLeft();
	}
}
void OnLeft()
{
	turnedLeftFlag = false;
changeScreen(-1);
moveStepper(200, false); // Move 200 steps counter-clockwise

	// display.clearDisplay();
	// display.setCursor(2, 0);
	// display.println("left");
	// display.display();
	// Serial.println("left");
}
void OnRight()
{
	turnedRightFlag = false;
changeScreen(1);
moveStepper(200, true); // Move 200 steps counter-clockwise

	// display.clearDisplay();
	// display.setCursor(2, 0);
	// display.println("right");
	// display.display();
	// Serial.println("right");
}
void changeScreen(int direction) {
    int numScreens = 3;  // Number of screens
    int nextScreen = static_cast<int>(currentScreen) + direction;
    currentScreen = static_cast<ScreenState>((nextScreen + numScreens) % numScreens); // Wrap around
    drawScreen();
}

void drawScreen() {
    display.clearDisplay();
    display.setCursor(0, 0);

    switch (currentScreen) {
        case ScreenState::MAIN:
            display.println("Main Screen");
            break;
        case ScreenState::LIST:
            display.println("List Screen");
            break;
        case ScreenState::EDIT:
            display.print("Edit: ");
            display.println(editValue);
            break;
    }

    display.display();
}