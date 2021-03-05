#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define LED_PIN 2     //On-board LED

#define DHTPIN 18     //DHT PIN
#define DHTTYPE DHT11 //DHT TYPE

char temperatureTopic[] = "dht11_temperature";
char humidityTopic[] = "dht11_humidity";
char subTopic[] = "dht11_ledControl";     //payload[0] will control/set LED
char pubTopic[] = "dht11_ledState";       //payload[0] will have ledState value

//Message variables
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int ledState = 0;

DHT_Unified dht(DHTPIN, DHTTYPE);

//Connects to specified network
void setup_wifi(){
  WiFi.begin(my_ssid, my_pass);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting to connect...");
    delay(1000);
  }
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

//Callback to sub topics
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("topic: ");
  Serial.print(topic);
  Serial.print(" message: ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //Set LED high/low
  if ((char)payload[0] == '1') 
  {
    digitalWrite(LED_PIN, HIGH);   
    ledState = 1;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);
  } 
  else if((char)payload[0] == '0')
  {
    digitalWrite(LED_PIN, LOW); 
    ledState = 0;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);
  }
  else
  {
  
  }
}

//Publishes the same message multiple times depending on the potentiometers resistance value
void multipleMessages(char topic[], int loopCount, float numericVal){
  float timeStart = millis();
  
  //Potentiometer 0 Ohm -> 1 message at a time
  if(loopCount == 0)
    loopCount = 1;
    
  for(int i=0; i < loopCount; i++)
  {
    client.publish(topic, String(numericVal).c_str());
  }
  
  float timePassed = millis() - timeStart;
  Serial.print(timePassed / 1000);
  Serial.print(" seconds passed for ");
  Serial.print(loopCount + 1);
  Serial.print(" messages with a frequency of ");
  Serial.print(1 / ((timePassed / 1000) / loopCount));
  Serial.println("Hz");
}

//Broker connection
void reconnect(){
  while(!client.connected())
  {
    //"mqtt-test" is the arduino's device name shown in the localhost:15672
    if(client.connect("mqtt-test", mqtt_user, mqtt_pass))
    {
      Serial.println("Connected!");
      client.subscribe(subTopic);
    } else{
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println("Retrying in 5 seconds\n");
      for(int i=0; i<5; i++){
        delay(1000);
        Serial.print(".");
      }
      Serial.print("\n");
    }
  }
}

//Client and sensor settings
void setup() {
  //Baud rate, client and broker setup
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
}

void loop() {
  int Poti = analogRead(34);
  char msg[8];
  if(!client.connected())
  {
    Serial.println("Connecting to Broker...");
    reconnect();
  }
  client.loop();

  long now = millis();
  if(now - lastMsg > 5000)
  {
    lastMsg = now;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);

    //Temperature and humidity event
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      float celsius = event.temperature;
      Serial.print(celsius);
      Serial.println(F("°C"));
      //Publish the same message times potentiometer resistance value
      multipleMessages(temperatureTopic, Poti, celsius);
    }

    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      float humidity = event.relative_humidity;
      Serial.print(humidity);
      Serial.println(F("%"));
      //Publish the same message times potentiometer resistance value
      multipleMessages(humidityTopic, Poti, humidity);
    }
    Serial.println("\n");
  }
}
