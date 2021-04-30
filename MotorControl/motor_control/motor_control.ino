#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#include <ESP32Encoder.h>
#include <sys/time.h>



//===========================================================
//================== MOTOR CONTROLLER =======================

class MotorController
{
private:
    int m_led_channel = -1;
    int m_ena_pin = -1;
    int m_in1_pin = -1;
    int m_in2_pin = -1;
public:
    MotorController(int led_channel, int ena_pin, int in1_pin, int in2_pin);
    ~MotorController();

    // Spd is a double bounded from -1 to 1
    void drive(double spd);
};


MotorController::MotorController(int led_channel, int ena_pin, int in1_pin, int in2_pin)
{
    int freq = 490;
    int resolution = 8;
    ledcSetup(led_channel, freq, resolution);
    ledcAttachPin(ena_pin, led_channel);
    
    pinMode(in1_pin, OUTPUT);
    pinMode(in2_pin, OUTPUT);

    m_led_channel = led_channel;
    m_ena_pin = ena_pin;
    m_in1_pin = in1_pin;
    m_in2_pin = in2_pin;
}


MotorController::~MotorController()
{
}


void MotorController::drive(double spd)
{
    // Bound input
    if(spd > 1)
    {
        spd = 1;
    }
    if(spd < -1)
    {
        spd = -1;
    }
    
    if(spd > 0)
    {
        //Serial.print("Positive ");
        digitalWrite(m_in1_pin, HIGH);
        digitalWrite(m_in2_pin, LOW);
    }
    else
    {
        //Serial.print("Negative ");
        digitalWrite(m_in1_pin, LOW);
        digitalWrite(m_in2_pin, HIGH);
    }

    int written_val = 255*abs(spd);
    //Serial.println(written_val);
    ledcWrite(m_led_channel, written_val);
}

//===========================================================



//===========================================================
//======================== PID ==============================

class PID_Driver
{
private:
    double m_p = -1;
    double m_i = -1;
    double m_d = -1;

    double m_i_val = 0;
    double m_prev_val = 0;
    double m_prev_err = 0;
    double m_prev_time = 0;

    int m_prev_encoder_count = 0;

    int m_encoder_pin_a = -1;
    int m_encoder_pin_b = -1;
    ESP32Encoder m_encoder;
public:
    PID_Driver(double p, double i, double d, int enc_pin_a, int enc_pin_b);
    ~PID_Driver();

    // vel is a double bounded from 0 to inf
    void setVelocity(double vel, MotorController motor);
    double calcVelocity(int dt);
    double getCurrentVal();
    double timestamp();
    int calcDt();
};

PID_Driver::PID_Driver(double p, double i, double d, int enc_pin_a, int enc_pin_b)
{
    m_p = p;
    m_i = i;
    m_d = d;

    // Enable the weak pull up resistors
    ESP32Encoder::useInternalWeakPullResistors=UP;
    // Attach pins for use as encoder pins
    m_encoder.attachHalfQuad(enc_pin_a, enc_pin_b);
}

PID_Driver::~PID_Driver()
{
  
}

double PID_Driver::timestamp()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

int PID_Driver::calcDt()
{
    int current_time = timestamp();
    int dt = current_time - m_prev_time;
    m_prev_time = current_time;
    return dt;
}

double PID_Driver::calcVelocity(int dt)
{
    int current_cnt = m_encoder.getCount();
    int dx = current_cnt - m_prev_encoder_count;
    m_prev_encoder_count = current_cnt;

    return 1000*(double)dx/(double)dt;
}

double PID_Driver::getCurrentVal()
{
  return m_prev_val;
}

void PID_Driver::setVelocity(double vel_setpoint, MotorController motor)
{
    int dt = calcDt();
  
    double current_vel = calcVelocity(dt);
    double err = vel_setpoint - current_vel;

    double p_val = m_p*err;
    m_i_val += m_i*err*dt;
    double d_val = m_d*(err-m_prev_err)/dt;

    double setpoint = p_val + m_i_val + d_val;

    //Serial.print("CALD_PID_SETPOINT ");
    //Serial.println(current_vel);
    //Serial.println(m_encoder.getCount());

    if(vel_setpoint == 0)
    {
      m_i_val = 0;
      motor.drive(0);
    }else
    {
      motor.drive(setpoint);
    }
    
    // All values past here are just being updated in memory
    m_prev_val = current_vel;
    m_prev_err = err;
}

//===========================================================


//===========================================================
//====================== WRAP ANGLE =========================

class WrapAngle
{
private:
    int m_wrap_cnt = 0;
    double m_last_heading = 0;
public:
    WrapAngle();
    ~WrapAngle();

    // vel is a double bounded from 0 to inf
    double getWrappedAngle(double current_heading);
    void setLastHeading(double new_heading);
};

WrapAngle::WrapAngle()
{
}

WrapAngle::~WrapAngle()
{
}

double WrapAngle::getWrappedAngle(double current_heading)
{
    double err = current_heading - m_last_heading;
    m_last_heading = current_heading;
    
    if (err > 180)
    {
      m_wrap_cnt = m_wrap_cnt-1;
    }
    else if (err < -180)
    {
      m_wrap_cnt = m_wrap_cnt+1;
    }
    
    return current_heading + (double)m_wrap_cnt*360;
}

void WrapAngle::setLastHeading(double new_heading)
{
    m_last_heading = new_heading;
}

//===========================================================
//====================== MAIN CODE ==========================

// MotorController(int led_channel, int ena_pin, int in1_pin, int in2_pin)
// led_channel is used internally (must be different per motor controller)
// ena_pin: pwm controlled pin
// in1_pin: direction signal pin
// in2_pin: direction signal pin
MotorController MC_l(0,17,18,5);
MotorController MC_r(1,0,16,4);

// PID_Driver(double p, double i, double d, int enc_pin_a, int enc_pin_b)
PID_Driver pid_l(0.0004 , 0.0000002 , 0.0005, 32, 33);
PID_Driver pid_r(0.0004 , 0.0000005 , 0.0005, 14, 27);
//PID_Driver pid_l(0.0001,0,0, 32, 33);
//PID_Driver pid_r(0.0001,0,0, 14, 27);

// Gyroscope Compass
Adafruit_BNO055 bno = Adafruit_BNO055(55);
double held_heading = 0;
WrapAngle wrap_angle;

// This ecoder and MC variable is for testing purposes, delete later
ESP32Encoder encoder_L;
MotorController MC(10,99,99,99);


void sweep()
{
  int d = 100;
  
  // Up to 100% speed
  for(double i = 0; i < 50; i++)
  {
    double val = i/50;
    MC.drive(val);

    Serial.print("Encoder count = ");
    Serial.println(encoder_L.getCount());
    delay(d);
  }
  // Down from 100% to -100%
  for(double i = 0; i < 100; i++)
  {
    double val = 1-i/50;
    MC.drive(val);

    Serial.print("Encoder count = ");
    Serial.println(encoder_L.getCount());
    delay(d);
  }
  // Up to 0% from -100%
  for(double i = 0; i < 50; i++)
  {
    double val = i/50-1;
    MC.drive(val);

    Serial.print("Encoder count = ");
    Serial.println(encoder_L.getCount());
    delay(d);
  }
}

// VARIABLE INITIALIZATION
// Serial Related
int incomingByte = 0; // for incoming serial data
bool is_negative = false;
int setpoint_case = 0;
int num = 0;
// Setpoint Related
int l_wheel_vel = 0;
int r_wheel_vel = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);

  // First call to BNO
  sensors_event_t event; 
  bno.getEvent(&event);
  wrap_angle.setLastHeading(event.orientation.x);
}

void loop() {

  /* Get a new sensor event */ 
  sensors_event_t event; 
  bno.getEvent(&event);

  double current_heading = wrap_angle.getWrappedAngle(event.orientation.x);

  if(Serial.available() > 0)
  { 
    // Read the incoming byte:
    incomingByte = Serial.read();

    // Add the new value to num
    if (incomingByte != 10)
    {
      // If the byte is a comma, assign a new value and advance the switch case
      if (incomingByte == 44)
      {
        if (is_negative)
        {
          num = -1*num;
          is_negative = false;
        }
        
        switch (setpoint_case)
        {
          case 0:
            l_wheel_vel = num;
            Serial.print("l_wheel_vel:\t");
            Serial.println(l_wheel_vel);
            setpoint_case++;
            break;
          case 1:
            r_wheel_vel = num;
            Serial.print("r_wheel_vel:\t");
            Serial.println(r_wheel_vel);
            setpoint_case = 0;
            break;
        }
        num = 0;
      }
      // If a negative sign is transmitted, remember that
      else if (incomingByte == 45)
      {
        is_negative = true;
      }
      else
      {
        num = num*10 + (incomingByte - 48);
      }
    }
    else
    { 
      // say youre done:
      Serial.println("Full Command Recieved");
      if (l_wheel_vel == r_wheel_vel){
        held_heading = current_heading;
      }
      setpoint_case = 0;
      num = 0;
    }
  }
  else
  {

    if (l_wheel_vel == r_wheel_vel and l_wheel_vel != 0)
    {
      double p = 7;
      double heading_err = current_heading - held_heading;
      pid_l.setVelocity(l_wheel_vel - p*heading_err, MC_l);
      pid_r.setVelocity(r_wheel_vel + p*heading_err, MC_r);
      Serial.print("Setpoint R: ");
      Serial.println(r_wheel_vel + p*heading_err, 4);
      Serial.print("Setpoint L: ");
      Serial.println(l_wheel_vel - p*heading_err, 4);
      Serial.println();
    }
    else
    {
      pid_l.setVelocity(l_wheel_vel, MC_l);
      pid_r.setVelocity(r_wheel_vel, MC_r);
    }

    // Add delay in PID loop
    delay(50);
  }
}
