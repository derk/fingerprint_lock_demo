/*
  enroll.ino

  Author:Vigoss
  2013-10-5
 
  enroll your finger

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
#include <Streaming.h>
#include <SoftwareSerial.h>

uint8_t getFingerprintEnroll(uint8_t id);

SoftwareSerial mySerial(A4, A5);                    // rx, tx

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


void setup()
{

    Serial.begin(38400);
    finger.begin(57600);

    if (finger.verifyPassword()) 
    {
        Serial.println("Fingerprint sensor init ok");
    } 
    else 
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1);
    }
}

void loop()                     // run over and over again
{
    Serial.println("Type in the ID # you want to save this finger as...\r\n(end with an 'a', such as 15a)");
    uint8_t id = 0;
    
    while (Serial.available())
    {
        char c = Serial.read();
    }
            
    while (true) 
    {
        while (! Serial.available());
        char c = Serial.read();
        
        if (! isdigit(c)) 
        {
            while(Serial.available())
            {
                c = Serial.read();
            }
            break;
        }
        id *= 10;
        id += c - '0';
    }
    
    Serial.print("Enrolling ID #");
    Serial.println(id);

    while (!getFingerprintEnroll(id));
}

uint8_t getFingerprintEnroll(uint8_t id) 
{
    uint8_t p = -1;
    Serial.println("Waiting for valid finger to enroll");
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
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }

    p = -1;
    Serial.println("Place same finger again");
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
}
