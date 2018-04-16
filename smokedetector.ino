#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int red_led = 0;
const int green_led = 5;
const char* ssid = "AndroidAP";
const char* password = "qxln5433";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}
void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("arzu", "hi SIT");
      // ... and resubscribe
      client.subscribe("arzu");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
    Serial.begin(115200);
    pinMode(red_led,OUTPUT);
    pinMode(green_led,OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected()) 
    {
      reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 2000) 
    {
      float sensor_volt;
      float RS_air; //  Get the value of RS via in a clear air
      float R0;  // Get the value of R0 via in Alcohol
      float sensorValue;

      /*--- Get a average data by testing 100 times ---*/
      for(int x = 0 ; x < 100 ; x++)
      {
        sensorValue = sensorValue + analogRead(A0);
      }
      sensorValue = sensorValue/100.0;
      /*-----------------------------------------------*/

      sensor_volt = sensorValue/1024*5.0;
      RS_air = (5.0-sensor_volt)/sensor_volt; // omit *RL
      R0 = RS_air/60.0; // The ratio of RS/R0 is 60 in a clear air from Graph (Found using WebPlotDigitizer)

      Serial.print("sensor_volt = ");
      Serial.print(sensor_volt);
      Serial.println("V");

      Serial.print("R0 = ");
      Serial.println(R0);
      if(R0>0.04)//you can set the value which exceed the normal value in your area in my case it was 0.04
    
      {
        snprintf (msg, 75, "Danger!!! Your house is on fire #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("arzu", msg);
        digitalWrite(red_led,HIGH);
        delay(500);
        digitalWrite(red_led,LOW);
        delay(500);
      }
      else
      {
        digitalWrite(green_led,HIGH);
        delay(500);
        digitalWrite(green_led,LOW);
        delay(500);
      }
      delay(1000);
      lastMsg = now;
      ++value;
     
    }
    

}
