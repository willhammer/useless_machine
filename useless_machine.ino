// Include the Servo library 
#include <Servo.h> 

#define DEBUG

Servo servoMove;
Servo servoPress;

enum move_servo_state
{
  move_idle = 0,
  moving = 1
  
};

enum press_servo_state
{
  press_idle = 0,
  press_returning = 1,
  toggling = 2
};

press_servo_state press_state = press_servo_state::press_idle;
move_servo_state move_state = move_servo_state::move_idle;


const int numSwitches = 5;

//angles are hardcoded to the test build.
const int valPressStartPosition = 30;
const int valPressEndPosition = 1;

int servoMin = 0;
int servoMax = 180;

int valMin = 0;
int valMax = 180;

//angles are hardcoded to the test build.
const int switchAngles[numSwitches] = {8,30,58,78,96};
bool switchStates[numSwitches] = {false, false, false, false, false};

int valMove = switchAngles[2];
int valPress = valPressStartPosition;

const int pinOffset = 2;

const int ledPin = 13;

int goToToggle = -1;

void update_states()
{
  for(int i = 0; i < numSwitches; ++i)
  {
    bool newState = (digitalRead(i + pinOffset) == HIGH);
    
#ifdef DEBUG
    Serial.write(i);
    //Serial.write(newState);
#endif
    switchStates[i] = newState;

    if(newState == true && move_state == move_servo_state::move_idle)
    {
      move_state = move_servo_state::moving;
    }
  }
  
  
}

void setup() 
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

#ifdef DEBUG
  Serial.write(1);
#endif
  pinMode(ledPin, OUTPUT);

  for(int i = 0; i < numSwitches; ++i)
  {
    pinMode(i + pinOffset, INPUT_PULLUP);
  }

  move_state = move_servo_state::move_idle;
  update_states();
  
  servoMove.attach(10);  
  servoMove.write(valMove); 

  servoPress.attach(9);
  servoPress.write(valPress);
}

int find_closest_toggle_index()
{
  int closestIndex = -1;
  int minDist = 200;
  for(int i = 0; i < numSwitches; ++i)
  {
    if(switchStates[i] == false)
      continue;
    
    int dist = abs(switchAngles[i] - valMove);
    if(dist < minDist)
    {
      minDist = dist;
      closestIndex = i;
    }
  }
  
  return closestIndex;
}


void loop() 
{
  const int speed = 1; 

  //current state
  valPress = servoPress.read();
  valMove = servoMove.read();
    
  update_states();
  
  if(move_state == move_servo_state::moving && press_state == press_servo_state::press_idle)
  {
    digitalWrite(ledPin, HIGH);

    goToToggle = find_closest_toggle_index();
    
    if(valMove == switchAngles[goToToggle])
    {
      press_state = press_servo_state::toggling;
      return;
    }
    
    valMove = switchAngles[goToToggle];
    servoMove.write(valMove);
    
    delay(15);
    return;
  }
  
  if(move_state == move_servo_state::moving && press_state == press_servo_state::toggling)
  {
    if(valPress > valPressEndPosition)
    {
      valPress -= speed;
      servoPress.write(valPress);

      delay(15);
    }
    
    else
    {
      digitalWrite(ledPin, LOW);

      update_states();
      
      if(switchStates[goToToggle] == false)
      {
        press_state = press_servo_state::press_returning;
      }      
    }
    
    
    return;
  }

  if(move_state == move_servo_state::moving && press_state == press_servo_state::press_returning)
  {
    digitalWrite(ledPin, LOW);
    servoMove.write(switchAngles[3]);

    valPress = servoPress.read();
    
    if(valPress != valPressStartPosition)
    {
      valPress = valPressStartPosition;
      servoPress.write(valPress);
    }
    
    else
    {
      move_state = move_servo_state::move_idle;
      press_state = press_servo_state::press_idle;
    }

    delay(15);
    return;
  }
  
}
