#include <Arduino.h>

#include "config.h"

#include "SPI.h"
#include "WiFi.h"
#include "esp_now.h"
#include "LittleFS.h"
#include "pmk.h"

#include "variables.h"

#include "espNowHandle.h"

void loopCount();


void setup() 
{

  gamepadPacket.deviceID = 4; 

  //-----Serial
  Serial.begin(115200);

  //-----Shift register
  pinMode(annularPin, INPUT_PULLUP);
  pinMode(middlePin, INPUT_PULLUP);
  pinMode(indexPin, INPUT_PULLUP);
  pinMode(thumb1Pin, INPUT_PULLUP);
  pinMode(thumb2Pin, INPUT_PULLUP);
  pinMode(thumb3Pin, INPUT_PULLUP);
  pinMode(switchDownPin, INPUT_PULLUP);
  pinMode(switchUpPin, INPUT_PULLUP);

  //Initialize SPI for SR

  //-----Leds


  //-----ESP NOW
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  if(esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, dongleAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  //-----PMK

}




void loop() 
{
  //Serial.println(WiFi.macAddress());
  //Serial.println("loop");
  loopCount();

  //------------------------------------------------------gpioTask
  if(micros() - gpioTask.beginTime >= gpioTask.interval)
  {
    gpioTask.beginTime = micros();
    gpioTask.inBetweenTime = gpioTask.beginTime - gpioTask.endTime;

      gamepadPacket.z = map(constrain(analogRead(throttlePin), 0, 4095), 0, 4095, -128, 127);
      gamepadPacket.buttons[0] = !digitalRead(annularPin) << 4 | !digitalRead(middlePin) << 5 | !digitalRead(indexPin) << 6 | !digitalRead(thumb1Pin) << 7;
      gamepadPacket.buttons[1] = !digitalRead(thumb2Pin) | !digitalRead(thumb3Pin) << 1 | !digitalRead(switchDownPin) << 2 | !digitalRead(switchUpPin) << 3;

    gpioTask.endTime = micros();
    gpioTask.counter++;
    gpioTask.duration = gpioTask.endTime - gpioTask.beginTime;
  }
  
  //send packet
  //------------------------------------------------------espnowTask
  if(micros() - espnowTask.beginTime >= espnowTask.interval)
  {
    espnowTask.beginTime = micros();
    espnowTask.inBetweenTime = espnowTask.beginTime - espnowTask.endTime;

//------------------
      
    //Send all pressed keys to packet
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(dongleAddress, (uint8_t *) &gamepadPacket, sizeof(gamepadPacket));
      
    if (result == ESP_OK) 
    {
      //Serial.println("Sent with success");
    }
    else 
    {
      //Serial.println("Error sending the data");
    }
//------------------
    espnowTask.endTime = micros();
    espnowTask.counter++;
    espnowTask.duration = espnowTask.endTime - espnowTask.beginTime;

  }
}

void loopCount()
{
  //Task frequency counter

  //gpioTask frequency counter
  if(gpioTask.counter == 0)
  {
    gpioTask.startCounterTime = micros();
  }
  if(micros() - gpioTask.startCounterTime > 1000000)
  {
    gpioTask.frequency = gpioTask.counter;
    //Serial.println(gpioTask.counter);
    gpioTask.counter = 0;
  }

  //Task frequency counter
  if(espnowTask.counter == 0)
  {
    espnowTask.startCounterTime = micros();
  }
  if(micros() - espnowTask.startCounterTime > 1000000)
  {
    espnowTask.frequency = espnowTask.counter;
    //Serial.println(espnowTask.counter);
    espnowTask.counter = 0;
  }
}

/* Typical task outline

  //------------------------------------------------------Task
  if(micros() - Task.beginTime >= Task.interval)
  {
    Task.beginTime = micros();
    Task.inBetweenTime = Task.beginTime - Task.endTime;

    **functions

    Task.endTime = micros();
    Task.counter++;
    Task.duration = Task.endTime - Task.beginTime;

  }

  //Task frequency counter
  if(Task.counter == 0)
  {
    Task.startCounterTime = micros();
  }
  if(micros() - Task.startCounterTime > 1000000)
  {
    Task.frequency = Task.counter;
    debug.println(Task.counter);
    Task.counter = 0;
  }

*/