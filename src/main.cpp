
/*
Podlaczenie do ESP32:
VCC -> 3.3V
GND -> GND
SCK -> gpio 22(SCL)
SDA -> gpio 21(SDA)
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
#define TMC2209_EN_PIN 2
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




RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW);
TMC2209Stepper driver(&Serial1, R_SENSE, 0b00); // Address 0 (default for TMC2209)
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