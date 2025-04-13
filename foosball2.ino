int away=0;
int home=0;
const int scoreToWin = 7;
//Display libraries
#include <Arduino.h>
#include <TM1637Display.h>

//Communication libraries
#include <WiFi.h>
#include <esp_now.h>
uint8_t peer_mac_address[] = {0x8C, 0x4F, 0x00, 0x28, 0xC0, 0xAC}; 
esp_now_peer_info_t peerInfo;

typedef struct struct_message {
    // Your data structure
    int home;
    int away;
    bool status;
} struct_message;

struct_message myData;


void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Home: ");
    Serial.println(myData.home);
    home=myData.away;
    Serial.print("Away: ");
    Serial.println(myData.away);
    away=myData.home;
    Serial.print("Status: ");
    Serial.println(myData.status);
    Serial.println();
}

//ToF libraries
#include <Wire.h>
#include "Adafruit_VL6180X.h"

// Module connection pins (Digital Pins)
#define CLK 15 //grey
#define DIO 2 //green wire



uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

TM1637Display display(CLK, DIO);
//Adafruit_VL6180X vl = Adafruit_VL6180X();

void resetGame()
{
  away = 0;
  home = 0;
}

void win() //flashes score on winning team's side
{
  int cycles = 10;
  int cycleDelay = 500;
  for(int i = 0; i < cycles; i++)
    {
      displayScore();
      delay(cycleDelay);
      display.clear();
      delay(cycleDelay);
    }

}

void lose() //disables display on losing teams side while winner's flashes
{
  display.clear();
}
void awayScore()
{
  away++;
  int cycles = 5;
  int cycleDelay = 250;
  for(int i = 0; i < cycles; i++) //flashes away score
    {
      displayScore();
      delay(cycleDelay);
      data[2] = 0x00;
      data[3] = 0x00;
      display.setSegments(data);
      delay(cycleDelay);
    }
}

void homeScore()
{
  home++;
  int cycles = 5;
  int cycleDelay = 250;
  for(int i = 0; i < cycles; i++) //flashes home score
    {
      displayScore();
      delay(cycleDelay);
      data[0] = 0x00;
      data[1] = 0x00;
      display.setSegments(data);
      delay(cycleDelay);
    }

}

void displayScore()
{
  


  if (home < 10) //Displays home score
  {
    data[0] = 0x00;
    data[1] = display.encodeDigit(home);
  }
  else
  {
    data[0] = display.encodeDigit(1);
    data[1] = display.encodeDigit(0);
  }

  if (away < 10) //Displays away score
  {
    data[2] = 0x00;
    data[3] = display.encodeDigit(away);
  }
  else
  {
    data[2] = display.encodeDigit(1);
    data[3] = display.encodeDigit(0);
  }
  display.setSegments(data);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  while (!Serial) {
    delay(1);
  }

  // Initialize ESP-NOW
    while (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        delay(200); // Stop setup if ESP-NOW initialization fails
    }

    // Register the callback function that will be called when data is sent
    esp_now_register_send_cb(onDataSent);

    // Register the callback function that will be called when data is received
    esp_now_register_recv_cb(onDataRecv);

    // Configure the peer information structure
    memcpy(peerInfo.peer_addr, peer_mac_address, 6); // Copy the peer's MAC address
    peerInfo.channel = 1; // Set the Wi-Fi channel (must be the same on both ESP32s)
    peerInfo.encrypt = false; // Set to true if you want to use encryption (more complex)

    // Add the peer device. Once added, we can send data to it.
    while (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        delay(300); // Stop setup if adding the peer fails
    }

    Serial.println("ESP-NOW initialized and peer added!");

  //while (! vl.begin()) //loops until ToF sensor is found
    //Serial.println("Failed to find sensor");
    //delay(100); //will not find the sensor without this delay

  //Serial.println("Sensor found!");
  display.setBrightness(6);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //uint8_t range = vl.readRange();
  //Serial.print("Range: "); Serial.println(range);
  //displayScore();
  //if (range < 10 && home  < 7) //ball rolls through the return pipe and max score has not been reached
    //homeScore();
  
  

  if(home >= scoreToWin)
    win();
  else if (away >= scoreToWin)
    lose();
  else
    displayScore();
    
  //delay(0);


}
