#include <WiFi.h> 
#include <HTTPClient.h> 
#include "DHT.h" 
#define WIFI_SSID "One Plus N"
  // WiFi password
#define WIFI_PASSWORD "Sky niyaz"
  
#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "ZX5p2E2SlVqjdnAvDb8LySty8PsuEdpv8GXI4zYCBUIRew7jkw7R6Dn22vQHS0FJU_oVA05gRiS0NNYuZ6vB1A=="
#define INFLUXDB_ORG "a0945f5f51432fd6"
#define INFLUXDB_BUCKET "PlanMon"

#define SOIL_SENSOR_PIN 36
#define MOISTURE_THRESHOLD 30

#define LUX_CALC_SCALAR 12518931
#define LUX_CALC_EXPONENT  -1.405
int ldrPin1 = 34;
float V_R=0.0, V_L=0.0,lux=0.0, vout = 0.0, R_L = 0.0, ldrValue = 0.0, ft = 0.0;

#define DHTPIN 26 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE); 
void setup() { 
  Serial.begin(115200); 

  Serial.println("Starting DHT and LDR Sensors with ThingSpeak...");
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(ldrPin1, INPUT);
  dht.begin(); 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
    }
  Serial.println("WiFi connected"); 
  } 
  void loop() {
    float temperature = dht.readTemperature(); 
    float humidity = dht.readHumidity(); 
    int sensorValue = analogRead(SOIL_SENSOR_PIN);
    int moisturePercent = map(sensorValue, 0, 4095, 100, 0);

    ldrValue = analogRead(ldrPin1);//reading sensor output
    V_R = ldrValue/4096.0*5;
    R_L = (V_R*10000)/(1-V_R/5);
    lux = pow(50*1e3*pow(10,0.7)/R_L,(1/0.7));
    if (lux > 1076 && lux < 2690)
    {
      Serial.print("Light intensity is optimal. ");
      Serial.println(lux*10);
    }
    else if (lux > 2690)
    {
      //Serial.print("Light intensity is more, please move to shady place. ");
      //Serial.println(lux*10);
    }
    else
    {
      //Serial.print("Light intensity is low, please change the position yo brighter or give compliment light. ");
      //Serial.println(lux*10);
    }

    if (!isnan(temperature) && !isnan(humidity)) { 
      String postData = "temperature,location=room value=" + String(temperature) + "\n";
      Serial.print(F("Temperature: ")); 
      Serial.println(temperature);
      postData += "humidity,location=room value=" + String(humidity) + "\n"; 
      postData += "lux,location=room value=" + String(lux) + "\n";
      postData += "moisturePercent,location=room value=" + String(moisturePercent);
      if (WiFi.status() == WL_CONNECTED) { 
        HTTPClient http; 
        String url = String(INFLUXDB_URL) + "/api/v2/write?org=" + INFLUXDB_ORG + "&bucket=" + INFLUXDB_BUCKET + "&precision=s"; 
        http.begin(url); 
        http.addHeader("Content-Type", "text/plain"); 
        http.addHeader("Authorization", "Token " + String(INFLUXDB_TOKEN)); 
        int httpResponseCode = http.POST(postData); 
        if (httpResponseCode > 0) { 
          //Serial.println("Data sent successfully"); 
        } 
        else { 
          Serial.print("Error sending data: "); 
          Serial.println(httpResponseCode); 
          } 
      http.end(); 
             
      } 
    else { 
        Serial.println("Failed to read from DHT sensor"); 
        }
      delay(60000);
    Serial.print(F("Temperature: ")); 
    Serial.print(temperature);
    //Serial.print(F("Humidity: ")); 
    //Serial.print(humidity);
    }
  
  }
