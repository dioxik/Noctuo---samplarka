#include <LiquidCrystal.h> // Biblioteka do obsługi wyświetlacza LCD
#include <Stepper.h> // Biblioteka do obsługi silnika krokowego

// Definicje pinów
const int pinRS = 12;
const int pinEN = 11;
const int pinD4 = 5;
const int pinD5 = 4;
const int pinD6 = 3;
const int pinD7 = 2;
const int pinKrok = 8;
const int pinKierunek = 9;
const int przycisk1Pin = 6; // Pin dla przycisku zwiększania długości
const int przycisk2Pin = 7; // Pin dla przycisku zatwierdzania

// Parametry silnika krokowego
const int krokiNaObrót = 200; // Zależy od typu silnika
const float srednicaSzpuli = 80; // Średnica szpuli w mm

// Utworzenie obiektu LCD
LiquidCrystal lcd(pinRS, pinEN, pinD4, pinD5, pinD6, pinD7);

// Utworzenie obiektu silnika krokowego
Stepper silnik(krokiNaObrót, pinKrok, pinKierunek);

// Zmienne globalne
int wybranaDlugosc = 0;
bool dlugoscWybrana = false;

void setup() {
  // Inicjalizacja LCD
  lcd.begin(16, 2);
  lcd.print("Wybierz dlugosc:");

  // Inicjalizacja silnika
  silnik.setSpeed(60); // Ustawienie prędkości obrotu silnika

  // Inicjalizacja przycisków
  pinMode(przycisk1Pin, INPUT_PULLUP);
  pinMode(przycisk2Pin, INPUT_PULLUP);
}

void loop() {
  // Wyświetlanie menu i odczytywanie wyboru użytkownika
  if (!dlugoscWybrana) {
    lcd.setCursor(0, 1);
    lcd.print("Dlugosc: ");
    lcd.print(wybranaDlugosc);
    lcd.print("mm  ");

    if (digitalRead(przycisk1Pin) == LOW) {
      wybranaDlugosc += 10; // Zwiększ długość o 10mm
      delay(200); // Opóźnienie aby uniknąć wielokrotnego zwiększania
    }

    if (digitalRead(przycisk2Pin) == LOW) {
      dlugoscWybrana = true;
      lcd.clear();
      lcd.print("Odwijanie...");
    }
  } else {
    // Obliczenie liczby kroków silnika
    float obwodSzpuli = PI * srednicaSzpuli;
    int liczbaKrokow = (wybranaDlugosc / obwodSzpuli) * krokiNaObrót;

    // Sterowanie silnikiem
    if (liczbaKrokow > 0) {
      silnik.step(liczbaKrokow);
      liczbaKrokow = 0; // Zresetuj liczbę kroków po odwinięciu
    }

    // Wyświetlanie informacji o odwiniętej długości
    lcd.setCursor(0, 1);
    lcd.print("Odwinieto: ");
    lcd.print(wybranaDlugosc);
    lcd.print("mm  ");

    delay(2000); // Odczekaj 2 sekundy po odwinięciu
    wybranaDlugosc = 0; // Zresetuj długość do kolejnego użycia
    dlugoscWybrana = false; // Wróć do menu wyboru długości
  }
}
