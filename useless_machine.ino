
// Include the Servo library 
#include <Servo.h> 

//#define DEBUG

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
const int valPressStartPosition = 60;
const int valPressEndPosition = 29;
const int valPressRestPosition = 90;

int servoMin = 0;
int servoMax = 180;

int valMin = 0;
int valMax = 180;

//angles are hardcoded to the test build.
const int switchAngles[numSwitches] = {138, 120, 95, 68, 49}; //switches 1 to 5, right to left
bool switchStates[numSwitches] = {false, false, false, false, false};

int valMove = switchAngles[2];
int valPress = valPressRestPosition;//switch close angle: 31 //switch idle position: 90 //switch start position: 45

const int pinOffset = 2;

const int ledPin = 13;

int goToToggle = -1;

bool overallPinState = false;

int idleFrames = 0;

void update_states()
{
  bool overallState = false;
  
  for(int i = 0; i < numSwitches; ++i)
  {
    bool newState = (digitalRead(i + pinOffset) == HIGH);
    
#ifdef DEBUG
    Serial.write(i);
    //Serial.write(newState);
#endif
    switchStates[i] = newState;
    
    if(newState == true)
    {
      overallState = true;
      
      if(move_state == move_servo_state::move_idle)
      {
        idleFrames = 0;
        move_state = move_servo_state::moving;
      }
    }
  }

  if(overallState != overallPinState)
  {
    Serial.write(overallState);
    overallPinState = overallState;    
  }

  digitalWrite(ledPin, overallPinState ? HIGH : LOW);
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

    goToToggle =  find_closest_toggle_index();
    
    if(valMove == switchAngles[goToToggle])
    {
      press_state = press_servo_state::toggling;
      return;
    }
    
    servoMove.write(switchAngles[goToToggle]);
    
    delay(15);
    return;
  }
  
  if(move_state == move_servo_state::moving && press_state == press_servo_state::toggling)
  {
    if(valPress > valPressEndPosition)
    {
      servoPress.write(valPress - speed);
      delay(15);
    }
    
    else
    {
      digitalWrite(ledPin, LOW);
      
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
    
    valPress = servoPress.read();
    
    if(valPress != valPressStartPosition)
    {
      servoPress.write(valPressStartPosition);
    }
    
    else
    {
      servoMove.write(switchAngles[2]);
      servoPress.write(valPressStartPosition);
      move_state = move_servo_state::move_idle;
      press_state = press_servo_state::press_idle;
    }

    delay(15);
    return;
  }
  if(move_state == move_servo_state::move_idle && press_state == press_servo_state::press_idle)
  {
    ++idleFrames;

    if(idleFrames > 10)
      servoPress.write(valPressRestPosition);

    delay(15);
    return;
  }
}
