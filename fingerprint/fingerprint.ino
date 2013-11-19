/*
  fingerprint.ino

  Author:loovee
  2013-11-18
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <Servo.h>
#include <rgb_lcd.h>
#include <Wire.h>
#include <EEPROM.h>



#define ServoInit() myservo.attach(9);\
                    myservo.write(50)
                    

#define __Debug         1                               // if debug mode

#if __Debug
#define DBG(X)          Serial.println(X)
#else
#define DBG(X)
#endif


#define DOORCLOSE       1
#define DOOROPEN        0


#define ST_IDLE         1
#define ST_ENROLL       2
#define ST_OPEN         3


SoftwareSerial mySerial(4, 5);                                // tx, rx

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


rgb_lcd lcd;


Servo myservo;                                                  // create servo object to control a servo


const int pinButton     = 2;
const int pinIR         = 6;

int state = ST_IDLE;

int num_finger = 0;

void dispText(char *str, int r, int g, int b)
{
    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.setRGB(r, g, b);
    lcd.print(str);
}

unsigned char doorState()
{
    if(digitalRead(pinIR))
    {
        return DOOROPEN;
    }
    return DOORCLOSE;
}

void OpenDoor()
{
    myservo.attach(9);
    myservo.write(50);
    delay(100);
    myservo.detach();
}

void CloseDoor()
{   
    myservo.attach(9);
    myservo.write(58);
    delay(100);
    myservo.detach();
}

void stateMachine()
{
    
    int fId;
    
    switch(state)
    {
        case ST_IDLE:
        
        
        if(digitalRead(pinButton))
        {
            delay(20);
            if(digitalRead(pinButton))
            {
                
                for(int i=0; i<50; i++)
                while(digitalRead(pinButton));
                
                state = ST_ENROLL;
                
                cout << "goto enroll" << endl;
            }
            
        }
        fId = getFingerprintIDez();
        
        if(fId >= 0)
        {
            DBG("get right finger, open door now!!");
            OpenDoor();
              
            state = ST_OPEN;
  
            for(int i=0; i<50; i++)
            {
                while(DOORCLOSE == doorState());
                delay(1);
            }

        }
        else if(-4 == fId)
        {
            dispText("WrongFinger", 255, 0, 0);
            delay(1000);
            dispText("Working...", 255, 255, 255);        
        }
        delay(50);
        
        
        break;
        
        case ST_ENROLL:
        
        getFingerprintEnroll(++num_finger);
        
        EEPROM.write(100, num_finger);
        
        
        cout << "enroll ok, goto idle" << endl;
        state = ST_IDLE;
        break;
        
        
        case ST_OPEN:
        
        
        cout << "ST_OPEN" << endl;
        dispText("Door Open", 0, 0, 100);
        
        for(int i=0; i<50; i++)
        {
            while(DOOROPEN == doorState());
            delay(1);
        }
        

        CloseDoor();
        dispText("Working...", 255, 255, 255);
        
        cout << "Door close! goto IDLE" << endl;
        state = ST_IDLE;
        break;

    }
}

void setup()
{
    CloseDoor();
    Serial.begin(57600);
    finger.begin(57600);
    
    pinMode(pinIR, INPUT);
    pinMode(pinButton, INPUT);
    
    num_finger = EEPROM.read(100);
    
    lcd.begin(16, 2);
    dispText("Working...", 255, 255, 255);
    
    delay(500);
    DBG("setup ok!");
}


void loop()                     // run over and over again
{

    stateMachine();
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{

    if (!finger.verifyPassword())
    {
        DBG("Did not find fingerprint sensor :(");
        return -1;
    }

    unsigned char p = finger.getImage();
    
    if (p != FINGERPRINT_OK)
    {
        return -2;
    }

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
    {
        return -3;
    }

    p = finger.fingerFastSearch();
    
    if (p != FINGERPRINT_OK)
    {
        return -4;
    }

#if __Debug
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    DBG(finger.confidence);
#endif

    return finger.fingerID;

}

unsigned char getFingerprintEnroll(unsigned char id) 
{
    unsigned char p = -1;
    Serial.println("Waiting for valid finger to enroll");
    
    dispText("Put Finger", 0, 100, 100);
    
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
            case FINGERPRINT_NOFINGER:
            //Serial.println(".");
            break;
            case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
            case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
            default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p) {
        case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
        case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
        case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
        case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
        case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
        default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    
    dispText("Remove finger", 0, 200, 200);
    
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }

    p = -1;
    Serial.println("Place same finger again");
    
    dispText("Put Again", 0, 100, 100);
    
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
            case FINGERPRINT_NOFINGER:
            //Serial.print(".");
            break;
            case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
            case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
            default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
        case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
        case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
        case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
        case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
        case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
        default:
        Serial.println("Unknown error");
        return p;
    }


    // OK converted!
    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
        Serial.println("Prints matched!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");
        dispText("Stored!", 0, 255, 0);
        delay(2000);
        dispText("Working...", 255, 255, 255);
        
        return p;
        
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
    
    dispText("Error!", 0, 255, 0);
    delay(2000);
    dispText("Working...", 255, 255, 255);
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/