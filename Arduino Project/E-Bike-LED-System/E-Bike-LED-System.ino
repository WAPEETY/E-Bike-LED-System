#include "BluetoothSerial.h"
#include "FastLED.h"
#include "Wire.h"
#include "MPU6050_tockn.h"

#define NUM_LEDS 17
#define PIN 2

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial BT;

MPU6050 mpu(Wire);

CRGB leds[60];

TaskHandle_t BTfunction;
TaskHandle_t LEDfunction;
TaskHandle_t Gyroscope;

String modeS, redS, greenS, blueS, brightnessS;

int counter, mode;
int red, green, blue, brightness;

bool done, connection = false;

float temp, accY, accX, accZ, acceleration, prevAcceleration;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial Started, waiting for cores");
  FastLED.addLeds<NEOPIXEL, PIN>(leds, 17);
  
  xTaskCreatePinnedToCore(
                    BTserver,
                    "Bluetooth",
                    10000,
                    NULL,
                    1,
                    &BTfunction,
                    0);
  delay(500);

  xTaskCreatePinnedToCore(
                    I2Cgyroscope,
                    "Gyroscope",
                    10000,
                    NULL,
                    1,
                    &Gyroscope,
                    0);
  delay(500);
  
  xTaskCreatePinnedToCore(
                    LEDmanager,
                    "LEDmanger",
                    10000,
                    NULL,
                    1,
                    &LEDfunction,
                    1);
  delay(500);
}

void BTserver(void * pvParameters){

  Serial.print("Bluetooth Function running on core ");
  Serial.println(xPortGetCoreID());

  BT.begin("E-Bike LED System");
  Serial.println("The device started, now you can pair it with bluetooth!");

  bool newMessage;
  
  while(true){
    String content = "";
    char character;
    redS = "";
    greenS = "";
    blueS = "";
    brightnessS = "";
    
    while(BT.available()) {
      character = BT.read();
      if(character != '\0'){
        connection = true;
      }
      content.concat(character);
      newMessage = true;
    }

    for(int i=0; i<content.length(); i++){
      if(counter == 0 and content[i]!= ','){
        modeS = content[i];
      }
      if(counter == 1 and content[i]!= ','){
        redS += content[i];
      }
      if(counter == 2 and content[i]!= ','){
        greenS += content[i];
      }
      if(counter == 3 and content[i]!= ','){
        blueS += content[i];
      }
      if(counter == 4 and content[i]!= ','){
        brightnessS += content[i];
      }
      if(content[i] == ','){
        counter ++;
        done = true;
      }
    }
    if(newMessage == true){
      mode = modeS.toInt();
      red = redS.toInt();
      green = greenS.toInt();
      blue = blueS.toInt();
      brightness = brightnessS.toInt();
      newMessage = false;
    }
    
    counter = 0;

    if(done == true){
      Serial.print("mode: ");Serial.println(mode);
      Serial.print("R: ");Serial.println(red);
      Serial.print("G: ");Serial.println(green);
      Serial.print("B: ");Serial.println(blue);
      done = false;
    }
    delay(100);
  }
}

void I2Cgyroscope(void * pvParameters){
  Serial.print("Gyroscope Data Function running on core ");
  Serial.println(xPortGetCoreID());
  Wire.begin(21,22);
  mpu.begin();
  mpu.calcGyroOffsets(true);
  while(true){
    mpu.update();
    //Serial.println(mpu.getAccY());
    //Serial.println(mpu.getTemp());
    prevAcceleration = acceleration;
    acceleration = mpu.getAccY();
    delay(500);
  }
}

void LEDmanager(void * pvParameters){
  Serial.print("LED Control Function running on core ");
  Serial.println(xPortGetCoreID());

  while(true){
    
      while(mode == 0){
        Serial.println("Solid color");

        if(connection == false){
            FastLED.setBrightness(255);
            fill_solid( leds, NUM_LEDS, CRGB(random(255) , random(255), random(255)));
            FastLED.show();
            delay(1000);
        }
        else{
            FastLED.setBrightness(brightness);
            fill_solid( leds, NUM_LEDS, CRGB(red , green , blue));
            FastLED.show();
            delay(1000);
        }
      }
      while(mode == 1){
        Serial.println("Walking");

        FastLED.setBrightness(brightness);
        for(int i = 0 ; i < NUM_LEDS; i++ ) {
          memset(leds, 0, NUM_LEDS * 3);
          leds[i].setRGB( red , green , blue );
          FastLED.show();
          delay(15);
        }
        
      }
      while(mode == 2){
        Serial.println("Breath");

        memset(leds, 0, NUM_LEDS * 3);
        for(int k = 0; k < 256; k++) {
          for(int i = 0; i < NUM_LEDS; i++ ) {
            leds[i].setRGB( red, green , blue );
            FastLED.setBrightness(k);
          }
        FastLED.show();
        delay(10);
        }
        for(int k = 255; k >= 0; k--) {
          for(int i = 0; i < NUM_LEDS; i++ ) {
            leds[i].setRGB( red, green , blue );
            FastLED.setBrightness(k);
          }
        FastLED.show();
        delay(10);
        }
        delay(500);

      }
      while(mode == 3){
        Serial.println("Rainbow");

        FastLED.setBrightness(brightness);
        for (int j = 0; j < 255; j++) {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV(i - (j * 2), 255, 255); 
          }
          FastLED.show();
          delay(25); 
        }
        
      }
      while(mode == 4){
        Serial.println("Driving Mode");
        if(prevAcceleration > acceleration){
            FastLED.setBrightness(150);
            fill_solid( leds, NUM_LEDS, CRGB(255 , 0 , 0));
            FastLED.show(); 
          }
        else{
            FastLED.setBrightness(150);
            fill_solid( leds, NUM_LEDS, CRGB(0 , 255 , 0));
            FastLED.show();
          }
        delay(25);
      }
    }
    delay(100);
}

void loop() {
  //not needed
}
