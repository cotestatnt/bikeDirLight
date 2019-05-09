#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

#define MESSAGE_TIME 15000
#define LEFT 2
#define RIGHT 3
#define BUZZER A2

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

uint8_t leftArrow[] = {8, 0x18, 0x3c, 0x7e, 0xe7, 0xc3, 0x81, 0x0, 0x0}; // char 3C
uint8_t rightArrow[] = {8, 0x0, 0x0, 0x81, 0xc3, 0xe7, 0x7e, 0x3c, 0x18}; // char 3E
uint8_t vertLine[] = {8, 0x0, 0xff, 0xff, 0x0, 0x0, 0xff, 0xff, 0x0 }; // char 7C
uint8_t dashLine[] = {8, 0x0, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0x0 }; // char 2D
 
// Global variables
typedef struct
{
  uint8_t spacing;  // character spacing
  char  *msg;   // message to display
} msgDef_t;

msgDef_t  Messages[] ={    
  { 0, "<<<<" },
  { 0, ">>>>" },
  { 0, "|--|" },
  { 1, "STOP" }
};

#define PAUSE_TIME  350
uint32_t  leftStart, rightStart, brakeTime;
bool Left, Right, Brake = false;
float AccX = 0.0F;

void setup(void){
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  //pinMode(BUZZER, OUTPUT);
  //digitalWrite(BUZZER, LOW);
  
  Serial.begin(115200);
  Serial.println("START");
  Wire.begin();
  mpu6050.begin();

  P.begin();
  P.addChar('>', rightArrow);
  P.addChar('<', leftArrow);
  P.addChar('|', vertLine);
  P.addChar('-', dashLine);
  P.setCharSpacing(Messages[0].spacing);
  P.setIntensity(15);
  P.setSpeed(9);  
  
}


void loop(void){  
   
  mpu6050.update();
  AccX = mpu6050.getAccX();  
  bool fast = false;
  if(AccX < -0.50 && AccX > -1.00){
    Brake = true;
    brakeTime = millis();
    Serial.println(AccX);
  } else if(AccX < -1.00){
     Brake = true;
     brakeTime = millis();
     Serial.print ("F " );
     Serial.println(AccX);    
     fast = true;
  }
   
  check_left();
  check_right();
  check_brake(fast); 
}



void check_left(){  
  if(digitalRead(LEFT) == LOW){
    if(!Left){
      leftStart = millis();  
      Serial.println("Left start");
      P.displayClear(); 
      Right = false;
    }
    Left = true;      
  }
  // Start left signaling
  if(Left){  
   if (P.displayAnimate() ) {        
      P.displayText(Messages[0].msg, PA_LEFT, P.getSpeed(), PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);       
      P.displayClear();        
      delay(250);
    }        
  }   
  // End Left after some seconds
  if(millis() - leftStart > MESSAGE_TIME && Left){    
    if (P.displayAnimate() ) {
      P.displayClear(); 
      Left = false;  
      Serial.println("Left Stop");
    }
  }
}


void check_right(){  
  if(digitalRead(RIGHT) == LOW){
    if(!Right){
      rightStart = millis();  
      Serial.println("Right start");
      P.displayClear(); 
      Left = false;
    }
    Right = true;      
  }
  // Start right signaling
  if(Right){
    if (P.displayAnimate() ) {        
      P.displayText(Messages[1].msg, PA_RIGHT, P.getSpeed(), PAUSE_TIME, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);       
      P.displayClear();        
      delay(250);
    }
  }
  // End Right after some seconds
  if(millis() - rightStart > MESSAGE_TIME && Right){    
    if (P.displayAnimate() ) {
      P.displayClear(); 
      Right = false;  
      Serial.println("Right Stop");
    }
  }
}


void check_brake(bool fast){  
  while(Brake){
    
    if (P.displayAnimate()) {    
      P.displayText(Messages[2].msg, PA_CENTER, P.getSpeed(), 50, PA_OPENING, PA_CLOSING);          
      P.displayClear();     
    }
    
    if (fast){
      while(millis() - brakeTime < 200){        
        if (P.displayAnimate() ) {        
          P.displayText(Messages[3].msg, PA_CENTER, P.getSpeed(), 50, PA_PRINT, PA_CLOSING); 
          P.displayClear();       
        }  
      }
    }
    if(millis() - brakeTime > 1000)
      Brake = false;     
  }

  if(!Left && !Right)
    P.displayClear();       
}
