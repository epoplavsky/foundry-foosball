int away;
int home;
const int scoreToWin = 5;

//Display libraries
#include <Arduino.h>
#include <TM1637Display.h>

//Communication libraries
#include <WiFi.h>
#include <esp_now.h>
uint8_t peer_mac_address[] = {0x8C, 0x4F, 0x00, 0x28, 0xC6, 0xC0};
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
    home=myData.home;
    Serial.print("Away: ");
    Serial.println(myData.away);
    away=myData.away;
    Serial.print("Status: ");
    Serial.println(myData.status);
    Serial.println();
    if (away != 0 && away != scoreToWin) //ensures that we don't blink on game reset signal
      blink_away(5, 250);
}



//ToF libraries
#include <Wire.h>
#include "Adafruit_VL6180X.h"


// Module connection pins (Digital Pins)
#define CLK 19 //grey
#define DIO 18 //green wire

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};



uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

TM1637Display display(CLK, DIO);
Adafruit_VL6180X vl = Adafruit_VL6180X();

void resetGame()
{
  away = 0;
  home = 0;
}

void win() //flashes score on winning team's side
{
  int cycles = 5;
  int cycleDelay = 500;
  for(int i = 0; i < cycles; i++)
    {
      displayScore();
      delay(cycleDelay);
      display.clear();
      delay(cycleDelay);
    }

  home = 0;
  away = 0;
  sendScores(); //just to make sure both reset
}

void lose() //disables display on losing teams side while winner's flashes
{
  home = 0;
  away = 0;
  display.clear();
  delay(5500);
  sendScores();
}


void blink_home(int cycles, int cycleDelay)
{
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
void blink_away(int cycles, int cycleDelay)
{
  for(int i = 0; i < cycles; i++) //flashes home score
    {
      displayScore();
      delay(cycleDelay);
      data[2] = 0x00;
      data[3] = 0x00;
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
    data[0] = display.encodeDigit(home / 10);
    data[1] = display.encodeDigit(home % 10);
  }

  if (away < 10) //Displays away score
  {
    data[2] = 0x00;
    data[3] = display.encodeDigit(away);
  }
  else
  {
    data[2] = display.encodeDigit(away / 10);
    data[3] = display.encodeDigit(away % 10);
  }
  display.setSegments(data);
}
void sendScores(){
    myData.home = away;
    myData.away = home;
    esp_err_t result = esp_now_send(peer_mac_address, (const uint8_t *) &myData, sizeof(myData));
    // Check the result of the sending operation
    if (result == ESP_OK) {
      Serial.println("Score data sent successfully");
    } else {
      Serial.print("Error sending score data: ");
      Serial.println(esp_err_to_name(result));
    }
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

  while (! vl.begin()) //loops until ToF sensor is found
    Serial.println("Failed to find sensor");
    delay(100); //will not find the sensor without this delay

  Serial.println("Sensor found!");

  display.setBrightness(6);
  away = 0;
  home = 0;
  sendScores();
}

void loop() {
  // put your main code here, to run repeatedly:
  displayScore();

  if (away >= scoreToWin)
    lose();

  uint8_t range = vl.readRange();
  delay(100);
  range = vl.readRange(); //if you don't read twice, it will double count the goal for some reason
  Serial.print("Range: "); Serial.println(range);
  
  if (range >= 5)
  {
    delay(100);
    return;
  }

  if (home  < scoreToWin - 1) //ball rolls through the return pipe and max score has not been reached
  {
    home++;
    sendScores();
    blink_home(5,250);
  }
  else if (home = scoreToWin - 1)
  {
    home++;
    sendScores();
    win();
  }
  //code losing

    
  delay(100);


}
