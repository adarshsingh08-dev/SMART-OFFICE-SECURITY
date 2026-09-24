#define BLYNK_TEMPLATE_ID "TMPL32tQGxuEj"
#define BLYNK_TEMPLATE_NAME "SMART OFFICE SECURITY"
#define BLYNK_AUTH_TOKEN "gLAfQ6tLHPVHvjC3H7gqgJ-GfIg8FT0w"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


char ssid[] = "iPhone";
char pass[] = "john@123";


#define MQ2_PIN A0
#define FLAME_PIN D5
#define DHT_PIN D6
#define BUZZER_PIN D7

#define DHTTYPE DHT11


DHT dht(DHT_PIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;


int smokeThreshold = 400;
float temperatureThreshold = 40.0;


void readSensors()
{
  int smoke = analogRead(MQ2_PIN);
  int flame = digitalRead(FLAME_PIN);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("DHT11 ERROR");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT11 ERROR");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");

    return;
  }

  

  bool fireDetected = (flame == LOW);

  
  bool smokeDetected = (smoke > smokeThreshold);
  bool highTemperature = (temperature > temperatureThreshold);

  bool danger = fireDetected || smokeDetected || highTemperature;

 
  if (danger)
    digitalWrite(BUZZER_PIN, HIGH);
  else
    digitalWrite(BUZZER_PIN, LOW);


  

  lcd.clear();

  if (fireDetected)
  {
    lcd.setCursor(0, 0);
    lcd.print("FIRE ALERT");

  }
  else if (smokeDetected)
  {
    lcd.setCursor(0, 0);
    lcd.print("SMOKE ALERT");

  }
  else if (highTemperature)
  {
    lcd.setCursor(0, 0);
    lcd.print("HIGH TEMP");

  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("OFFICE SECURE");
  }

  lcd.setCursor(0, 1);

  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C ");

  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("%");

  delay(2000);



  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Smoke:");
  lcd.print(smoke);

  lcd.setCursor(0, 1);

  lcd.print("Flame:");
  lcd.print(flame);

  if (fireDetected)
    lcd.print(" FIRE");
  else
    lcd.print(" SAFE");

  delay(2000);



  Serial.print("Smoke: ");
  Serial.print(smoke);

  Serial.print(" | Temperature: ");
  Serial.print(temperature, 1);
  Serial.print(" C");

  Serial.print(" | Humidity: ");
  Serial.print(humidity, 0);
  Serial.print(" %");

  Serial.print(" | Flame: ");
  Serial.print(flame);

  if (fireDetected)
    Serial.println(" FIRE");
  else
    Serial.println(" SAFE");


  

  if (Blynk.connected())
  {
    Blynk.virtualWrite(V0, smoke);
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);

   
    Blynk.virtualWrite(V3, flame);

   
    if (fireDetected)
    {
      Blynk.virtualWrite(V4, "FIRE ALERT");
    }
    else if (smokeDetected)
    {
      Blynk.virtualWrite(V4, "SMOKE ALERT");
    }
    else if (highTemperature)
    {
      Blynk.virtualWrite(V4, "HIGH TEMPERATURE");
    }
    else
    {
      Blynk.virtualWrite(V4, "OFFICE SECURE");
    }

   

    if (fireDetected)
    {
      Blynk.logEvent(
        "fire_alert",
        "Fire detected in office!"
      );
    }

    if (smokeDetected)
    {
      Blynk.logEvent(
        "smoke_alert",
        String("Smoke level: ") + smoke
      );
    }

    if (highTemperature)
    {
      Blynk.logEvent(
        "temp_alert",
        String("Temperature: ") + temperature + " C"
      );
    }
  }
}




void setup()
{
  Serial.begin(115200);

  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);




  Wire.begin(D2, D1);

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SMART OFFICE");

  lcd.setCursor(0, 1);
  lcd.print("SECURITY SYSTEM");

  delay(2000);


 

  dht.begin();



  WiFi.begin(ssid, pass);

  Serial.println();
  Serial.print("Connecting WiFi");

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 20)
  {
    delay(500);
    Serial.print(".");
    count++;
  }

  Serial.println();


  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi Connected!");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // ---------- Blynk ----------

    Blynk.config(BLYNK_AUTH_TOKEN);

    if (Blynk.connect(3000))
    {
      Serial.println("Blynk Connected!");
    }
    else
    {
      Serial.println("Blynk Not Connected");
    }
  }
  else
  {
    Serial.println("WiFi Not Connected");
    Serial.println("Sensors will still work.");
  }


  timer.setInterval(2500L, readSensors);

  readSensors();
}


void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!Blynk.connected())
    {
      Blynk.connect(100);
    }

    Blynk.run();
  }

  timer.run();
}