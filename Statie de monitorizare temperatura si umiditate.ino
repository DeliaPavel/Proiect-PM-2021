#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Wire.h>
#include <LCD.h>

#define REAL_FEEL_BUTTON 7
#define FAN 5
#define BUZZER 6
#define MAX_START 200
#define DHTPIN 2
#define DHTPIN2 3
#define RELAY 4
#define RED_LED 11
#define GREEN_LED 10
#define BLUE_LED 9
#define MAX_TEMP 24.8
#define START_BUTTON 12
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Initializam senzorii DHT pentru Arduino
DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// valorile pentru primul senzor DHT
int chk;
float hum;
float temp;
float realfeel;

// valorile pentru al doilea senzor DHT
int chk2;
float hum2;
float temp2;
float realfeel2;

// constante pentru calcul RealFeel
float c1 = -8.78469475556;
float c2 = 1.61139411;
float c3 = 2.33854883889;
float c4 = -0.14611605;
float c5 = -0.012308094;
float c6 = -0.0164248277778;
float c7 = 0.002211732;
float c8 = 0.00072546;
float c9 = -0.000003582;

// daca vrem sau nu sa afisam RealFeel
bool buttonValue = 0;
// daca vrem sau nu sa pornim statia de monitorizare
bool startButtonValue = 0;
// viteza cu care merge ventilatorul
int fan_speed = 0;

// initializare LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  // initializam lcd-ul
  lcd.begin();
  lcd.clear();
  lcd.backlight();

  // setam ventilatorul ca output
  pinMode(FAN, OUTPUT);

  // initializam senzorii
  dht.begin();
  dht2.begin();

  // setam releul ca output
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);

  // setam led-ul rgb ca output
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // setam cele 2 butoane ca input
  pinMode(REAL_FEEL_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);

}

// functie care calculeaza realfeel dupa
// formula de aici
// https://en.wikipedia.org/wiki/Heat_index
float get_realfeel(float temp, float hum) {
  return c1 + c2 * temp + c3 * hum + c4 * temp * hum 
        + c5 * temp * temp + c6 * hum * hum + c7 * temp * temp 
        * hum + c8 * temp * hum * hum + c9 * temp * temp * hum * hum;
}

// functie care primeste ca parametru valorile pe care trebuie
// sa le scriem pentru a aprinde led-ul rgb cu o anumita culoare
void setColorRGB(unsigned int red, unsigned int green, unsigned int blue) {
  analogWrite(RED_LED, red);
  analogWrite(GREEN_LED, green);
  analogWrite(BLUE_LED, blue);
}


void loop() {

  // citim daca intrerupatorul este apasat sau nu
  startButtonValue = digitalRead(START_BUTTON);

  // daca nu este
  if (!startButtonValue) {
    // oprim ventilatorul daca acesta era pornit
    if(fan_speed > 0) fan_speed -= 20;
    analogWrite(FAN, fan_speed);

    // oprim sirena
    digitalWrite(RELAY, HIGH);  

    setColorRGB(0, 0, 0);
    
    lcd.setCursor(0, 0);
    lcd.print("Proiect PM 2021");
    lcd.setCursor(0, 1);
    lcd.print("Pavel Delia 335CC");
  } else {
    lcd.clear();
    // verificam daca vrem sa afisam RealFeel sau doar temperatura
    // si umiditatea curenta
    buttonValue = digitalRead(REAL_FEEL_BUTTON);

    // citim umiditatea si temperatura de pe ambii senzori
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    hum2 = dht2.readHumidity();
    temp2 = dht2.readTemperature();

    lcd.setCursor(0, 0);
    // daca butonul de RealFeel nu este apasat
    // afisam temperatura si umiditatea
    // altfel afisam RealFeel-ul
    if (!buttonValue){
      lcd.print("T:");
      lcd.print(temp);
      lcd.print("C");
      lcd.print("; ");
  
      lcd.print(temp2);
      lcd.print("C");
    } else {
      lcd.print("RealFeel1: ");
      realfeel = get_realfeel(temp, hum);
      lcd.print(realfeel);
    }
    // scriem pe randul urmator
    lcd.setCursor(0, 1);
    if (!buttonValue) {
      lcd.print("U:");
      lcd.print(hum);
      lcd.print("%");
      lcd.print("; ");
      lcd.print(hum2);
      lcd.print("%");
    } else {
      lcd.print("RealFeel2: ");
      realfeel2 = get_realfeel(temp2, hum2);
      lcd.print(realfeel2);
    }

    delay(1000);

    // daca ambele temperaturi depasesc valoarea maxima
    if (temp >= MAX_TEMP && temp2 >= MAX_TEMP) {
      // pornim ventilatorul
      if(fan_speed < 200) fan_speed += 20;
      analogWrite(FAN, fan_speed);
      
      // va suna sirena
      digitalWrite(RELAY, LOW);

      setColorRGB(255, 0, 0);
     // daca doar o temperatura depaseste valoarea maxima 
    } else if (temp >= MAX_TEMP || temp2 >= MAX_TEMP) {
      // oprim sirena
      digitalWrite(RELAY, HIGH);
      
      // pornim ventilatorul
      if(fan_speed<150) fan_speed += 20;
      analogWrite(FAN, fan_speed);

      //pornim buzzer-ul
      tone(BUZZER, 1245, 1000);

      setColorRGB(255, 0, 255);
    } else {
      // oprim ventilatorul
      if(fan_speed > 0) fan_speed -= 20;
      analogWrite(FAN, fan_speed);
      
      // oprim releul
      digitalWrite(RELAY, HIGH);
      
      setColorRGB(0, 255, 0);
    }
  }


}
