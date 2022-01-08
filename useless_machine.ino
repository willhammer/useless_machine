
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


const short numSwitches = 5;

//angles are hardcoded to the test build.
const short valPressStartPosition = 45;//60;
const short valPressEndPosition = 29;
const short valPressRestPosition = 90;

short servoMin = 0;
short servoMax = 180;

short valMin = 0;
short valMax = 180;

//angles are hardcoded to the test build.
const short switchAngles[numSwitches] = {138, 120, 95, 68, 49}; //switches 1 to 5, right to left
bool switchStates[numSwitches] = {false, false, false, false, false};

short valMoveRestPosition = switchAngles[2];
short valMove = valMoveRestPosition;
short valPress = valPressRestPosition;//switch close angle: 31 //switch idle position: 90 //switch start position: 45

const short pinOffset = 2;

const short ledPin = 13;

short goToToggle = -1;

short moveDirection = 0;
short moveDiff = 360;

bool overallPinState = false;

unsigned int idleFrames = 0;

void update_states()
{
  //current state
  valPress = servoPress.read();
  valMove = servoMove.read();

  if(goToToggle != -1)
  {
    moveDiff = switchAngles[goToToggle] - valMove;
    moveDirection = moveDiff > 0 ? 1 : -1;  
  }  
  
  bool overallState = false;
  
  for(int i = 0; i < numSwitches; ++i)
  {
    bool newState = (digitalRead(i + pinOffset) == HIGH);    

    switchStates[i] = newState;
    
    if(newState == true)
    {
      overallState = true;
    }
  }

  if(overallState != overallPinState)
  {
#ifdef DEBUG
    Serial.write(overallState);
#endif
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
  int minDist = 360;
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
  const int speed = 3; 


    
  update_states();

  switch(move_state)
  {
    case move_servo_state::move_idle:
      
      if(overallPinState == true)
      {
        idleFrames = 0;
        move_state = move_servo_state::moving;
      }
    
      if(press_state == press_servo_state::press_idle)
      {
        ++idleFrames;
    
        if(idleFrames > 10)
        {
          servoPress.write(valPressRestPosition);
        }
      }
    break;

    
    case move_servo_state::moving:
      switch(press_state)
      {
        case press_servo_state::press_idle:
          if(goToToggle == -1)
          {
            goToToggle = find_closest_toggle_index();  
          }          

          else if(abs(moveDiff) < 2)
          {
            press_state = press_servo_state::toggling;       
          }
          else
          {
            servoMove.write(valMove + moveDirection * speed);
          }
        break;
    
    
        case press_servo_state::toggling:  
          if(valPress > valPressEndPosition)
          {
            servoPress.write(valPress - speed);
          }
          
          else
          {
            if(switchStates[goToToggle] == false)
            {
              press_state = press_servo_state::press_returning;
            }      
          }
        break;
        
        case press_servo_state::press_returning:    
          if(valPress < valPressStartPosition)
          {
            servoPress.write(valPress + speed);
          }
          
          else
          {
            if(abs(moveDiff) < 2)
            {
              servoMove.write(valMove + moveDirection * speed);
            }
            
            //servoPress.write(valPressStartPosition);
            
            move_state = move_servo_state::move_idle;
            goToToggle = -1;
            
            press_state = press_servo_state::press_idle;
          }
        break;
    }
    break;
  }

  delay(15);
  
  
  
}
