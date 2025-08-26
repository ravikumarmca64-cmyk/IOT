#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define WIFI_SSID "@Rk"
#define WIFI_PASS "ravi@123"
#define THINGSPEAK_API_KEY "HMTWKM7HAT1UADJF"
#define THINGSPEAK_URL "https://api.thingspeak.com/update?api_key=HMTWKM7HAT1UADJF&field1=0"


#define DHT_PIN 15        // GPIO for DHT Sensor
#define DHT_TYPE DHT11   // Change to DHT22 if using DHT22
#define MQ_SENSOR 34     // GPIO for Gas Sensor
#define SOUND_SENSOR 35  // GPIO for Sound Sensor
#define BUZZER_PIN 2     // GPIO for Buzzer (Optional)

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// Initialize LCD with I2C address 0x27 and 16 columns x 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Wire.begin(22, 23); 

    lcd.init();
    lcd.backlight();
    
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer initially
    
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Connecting..");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected!");
    delay(1000);

    dht.begin();
}
    //display values in output serial monitor
void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        int gasValue = analogRead(MQ_SENSOR);
        int soundLevel = analogRead(SOUND_SENSOR);
        
        Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
        Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
        Serial.print("Gas Level: "); Serial.println(gasValue);
        Serial.print("Sound Level: "); Serial.println(soundLevel);

        //Live input on LCD STARTS
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: "); 
        lcd.print(temperature); 
        lcd.print(" C");

        lcd.setCursor(0, 1);
        lcd.print("Humidity: "); 
        lcd.print(humidity); //Value
        lcd.print(" %");
        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Gas Level: "); 
        lcd.print(gasValue); 

        lcd.setCursor(0, 1);
        lcd.print("Sound Level:"); 
        lcd.print(soundLevel);
        delay(2000);

        //Live input on LCD ENDS
      
        String alertMessage = "";

        // Check threshold values
        if (temperature > 40) {
            alertMessage += "Temp High! ";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Temp High!");
            delay(2000);
        }
        if (humidity < 30 || humidity > 80) {
            alertMessage += "Humidity Alert! ";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Humidity!");
            lcd.setCursor(1, 1);
            lcd.print("Alert!");
            delay(2000);
        }
        if (gasValue > 1000) {
            alertMessage += "Air Pollution ";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Air Pollution");
            lcd.setCursor(1, 1);
            lcd.print("Detected!!");
            delay(2000);
        }
        if (soundLevel > 1500) {
            alertMessage += "Noise Pollution High! ";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Noise Pollution!");
            lcd.setCursor(1, 1);
            lcd.print("High!");
            delay(2000);

        }

        // Activate buzzer if alert exists
        if (alertMessage.length() > 0) {
         for (int i = 0; i < 5; i++) {  // Blink 5 times
         digitalWrite(BUZZER_PIN, HIGH);
         delay(500);  // LED ON for 0.5 sec //Change this to change buzzer sound time 1sec = 1000
         digitalWrite(BUZZER_PIN, LOW);
         delay(500);  // LED OFF for 0.5 sec //Change this to change buzzer sound time 1sec = 1000
        }
        Serial.println("ALERT: " + alertMessage);
          } else {
        digitalWrite(BUZZER_PIN, LOW);  // Keep LED OFF if no alert
}


        // Send data to ThingSpeak
        HTTPClient http;
        String url = THINGSPEAK_URL;
        url += "?api_key=" + String(THINGSPEAK_API_KEY);
        url += "&field1=" + String(soundLevel);
        url += "&field2=" + String(temperature);
        url += "&field3=" + String(humidity);
        url += "&field4=" + String(gasValue);
        if (alertMessage.length() > 0) {
            url += "&field5=" + alertMessage;
        }

        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("Data sent to ThingSpeak!");
        } else {
            Serial.print("Error sending data: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected. Reconnecting...");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Disconnected");
        delay(2000);
    }

    delay(5000);  // Update every 5 seconds
}