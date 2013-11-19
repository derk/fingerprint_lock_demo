//

#include <Streaming.h>
#include <Servo.h> 


#define OpenDoor()  myservo.write(58);
#define CloseDoor() myservo.write(50);

#define ServoInit() myservo.attach(9);\
                    myservo.write(50)
                    
                    
Servo myservo;


int pos = 0;
int pos_buf = 0;

void servoCtrl(int pos)
{
    myservo.write(pos);

}

void setup()
{
    Serial.begin(115200);
    
    ServoInit();
    

    
    delay(1000);

    cout << "hello world" << endl;
}

void loop()
{

    OpenDoor();
    
    delay(1000);
    
    CloseDoor();
    delay(1000);
    

}