#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
WiFiClient client;
#define velocity 0.034  // cm/micro s
#define DUST_SENSOR_PIN A0
#define ULTRASONIC_TRIG D5
#define ULTRASONIC_ECHO D6
#define RED_LED_PIN     D8
#define YELLOW_LED_PIN  D4
#define GREEN_LED_PIN   D3
#define RELAY_PIN       D7
float waterLevel, distance, duration;



volatile float dustDensity;
bool sprinler_status;

// Timer variables
//unsigned long previousTime = 0;
//unsigned long currentTime=0;



LiquidCrystal_I2C lcd(0x27, 16, 2);


const char* ssid = "yourwifiname";
const char* password = "strongwifipassword";
const char* host = "your cloud platform";
const int httpsPort = 80;

const char Thing[] = "PM_SS";
const char Property1[] = "Pm_Level";
const char Property2[] = "Water_level";
const char Property3[] = "Sprinkling_St";



void Put(String ThingName, String ThingProperty, int Value)
{

  Serial.println(host);
  if (!client.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  } else

  {
    Serial.println("Connected to ThingWorx.");
  }
  String url = "/Thingworx/Things/" + ThingName + "/Properties/" + ThingProperty;
  Serial.print("requesting URL: ");
  Serial.println(url);

  String strPUTReqVal = "{\"" + ThingProperty + "\":\"" + Value + "\"}";
  Serial.print("PUT Value: ");
  Serial.println(strPUTReqVal);

  client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "appKey: 2e5e33db-0f43-4568-9de5-179d3e3f561e" + "\r\n" +
               "x-thingworx-session: false" + "\r\n" +
               "Accept: application/json" + "\r\n" +
               "Connection: close" + "\r\n" +
               "Content-Type: application/json" + "\r\n" +
               "Content-Length: " + String(strPUTReqVal.length()) + "\r\n\r\n" +
               strPUTReqVal + "\r\n\r\n");

  while (client.connected())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  client.stop();
}

void setup()
{

  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  delay(1000);

  Wire.begin(D1, D2);

  lcd.home();
  lcd.init(); // initialize the lcd
  lcd.backlight();

  dustDensity = 0;
  sprinler_status = 0;
  Serial.begin(9600);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);


}
void loop()
{
  float dustDensity = readDustDensity();
  float waterLevel = readWaterLevel();
  Serial.print("dustDensity:");
  Serial.println(dustDensity);
  Serial.print("waterLevel:");
  Serial.println(waterLevel);

  readDustDensity() ;
  readWaterLevel();

  if (dustDensity > 7)
  {
    printLCD();

    digitalWrite(RELAY_PIN, LOW);  // Turn on the water sprinkling
    Serial.print("flag 1");
    sprinler_status = 1;

    delay(500);

  readWaterLevel();

  if (waterLevel < 20)
  {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    readWaterLevel();
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:Very LOW");
  }
  else if (waterLevel > 25 && waterLevel <= 55)
  {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    readWaterLevel();
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:Medium ");
  }
  else
  { 
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:HIGH ");

  }

  delay(1000);

  Put(Thing, Property1, dustDensity);
  Put(Thing, Property2, waterLevel);
  Put(Thing, Property3, sprinler_status);

  delay(1000);
}

  
  else
  {
    digitalWrite(RELAY_PIN, HIGH);   // Turn off the water sprinkling

    delay(500);

    sprinler_status = 0;
    Serial.print("flag 0");
    printLCD();
    
  readWaterLevel();

  if (waterLevel < 20)
  {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    readWaterLevel();
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:Very LOW");
  }
  else if (waterLevel > 25 && waterLevel <= 55)
  {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    readWaterLevel();
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:Medium ");
  }
  else
  { 
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    printLCD();
    lcd.setCursor(0, 1);
    lcd.print("Level:HIGH ");

  }

  delay(1000);

  Put(Thing, Property1, dustDensity);
  Put(Thing, Property2, waterLevel);
  Put(Thing, Property3, sprinler_status);

  delay(1000);
}
    
  }






  
float readDustDensity()
{
  int rawValue = analogRead(DUST_SENSOR_PIN);
  float voltage = 5.0 * float (rawValue) / 1024.00;  /* The amount of dust particles by using the linear
                                                 equation provided by Chris Nafis; the unit is in ug/m3 */

  dustDensity = voltage * 0.17 * 1000; // Conversion factor for GP2Y dust sensor
  return dustDensity;
}


float readWaterLevel()
{
  //SLEEPING TRIG
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);
  duration = pulseIn(ULTRASONIC_ECHO, HIGH);
  distance = duration * velocity / 2;
  Serial.print("Distance:");
  Serial.println(distance);
  waterLevel = map(distance, 0, 4, 100, 0);  // Assuming 0cm to 100cm range
  waterLevel = waterLevel;
  return waterLevel;
}
void printLCD()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PM2.5:");
  lcd.print(dustDensity, 2);
  lcd.print(" mg/m3");


}
