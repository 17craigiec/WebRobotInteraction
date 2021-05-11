
// VARIABLE INITIALIZATION

// Serial Related
int incomingByte = 0; // for incoming serial data
bool is_negative = false;
bool command_sent = false;
bool motors_enabled = true;
int setpoint_case = 0;
int num = 0;

// Arm Joint Setpoints
int j1_setpoint = 0;
int j2_setpoint = 0;

// State Machine Enum
enum State{ HOMING, INTERPOLATING };
State state = INTERPOLATING;

// Pin Declarations
int j1_lim_sw = 4;
int j2_lim_sw = 3;

// Stepper Class
class Stepper
{
  private:
    int m_ena_pin;
    int m_dir_pin;
    int m_pulse_pin;
    bool m_enabled = true;

    int m_cur_pulse = 0;
    int m_pulse_time = 200;

   public:
    Stepper(int ena_pin, int dir_pin, int pulse_pin)
    {
      m_ena_pin = ena_pin;
      m_dir_pin = dir_pin;
      m_pulse_pin = pulse_pin;

      pinMode(m_ena_pin, OUTPUT);
      pinMode(m_dir_pin, OUTPUT);
      pinMode(m_pulse_pin, OUTPUT);
    }

    int getPulseTime()
    {
      return m_pulse_time;
    }

    int getPulseErr(int setpoint_pulse)
    {
      return setpoint_pulse - m_cur_pulse;
    }

    void zeroCurrentPulse()
    {
      m_cur_pulse = 0;
    }
    
    void enable()
    {
      digitalWrite(m_ena_pin, LOW);
      m_enabled = true;
    }

    void disable()
    {
      digitalWrite(m_ena_pin, HIGH);
      m_enabled = false;
    }

    // Lowest level movement command, dir should be either 1 or -1
    void pulse_raw(int dir)
    {
      if(dir == 1)
      {
        digitalWrite(m_dir_pin, LOW);
      }
      else if (dir == -1)
      {
        digitalWrite(m_dir_pin, HIGH);
      }
      else
      {
        Serial.println("ERROR: Wrong input stepper.pulse_raw(int dir)");
      }
      digitalWrite(m_pulse_pin, HIGH);
      delayMicroseconds(m_pulse_time);
      digitalWrite(m_pulse_pin, LOW);
    }

    // Higher level movement using the global pulse setpoint recieved from UART
    void pulse(int* pulse_setpoint)
    {
      int err = *pulse_setpoint - m_cur_pulse;

      if(err == 0)
      {
        // The setpoint has been reached... Cool.
      }
      else if(err > 0)
      {
        pulse_raw(1);
        m_cur_pulse++;
      }
      else if(err < 0)
      {
        pulse_raw(-1);
        m_cur_pulse--;
      }
      //Serial.println(err);
    }
};

//Stepper Class Declaration
Stepper j1_stepper(17, 18, 16);
Stepper j2_stepper(5, 7, 6);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Limit Sw Setup
  pinMode(j1_lim_sw, INPUT_PULLUP);
  pinMode(j2_lim_sw, INPUT_PULLUP);

  // Stepper Setup
  j1_stepper.enable();
  j2_stepper.enable();

  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:
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
            if(!command_sent && motors_enabled)
            {
              j1_setpoint = num;
              Serial.print("j1_setpoint:\t");
              Serial.println(j1_setpoint);
            }
            setpoint_case++;
            break;
          case 1:
            if(!command_sent && motors_enabled)
            {
              j2_setpoint = num;
              Serial.print("j2_setpoint:\t");
              Serial.println(j2_setpoint);
            }
            setpoint_case = 0;
            break;
        }
        command_sent = false;
        num = 0;
      }
      // If a negative sign is transmitted, remember that
      else if (incomingByte == '-')
      {
        is_negative = true;
      }
      else if (incomingByte == 'H')
      {
        command_sent = true;
        Serial.println("Homing Request Detected");
        state = HOMING;
      }
      else if (incomingByte == 'D')
      {
        command_sent = true;
        motors_enabled = false;
        Serial.println("Disable Request Detected");
        j1_stepper.disable();
        j2_stepper.disable();
      }
      else if (incomingByte == 'E')
      {
        command_sent = true;
        motors_enabled = true;
        Serial.println("Enable Request Detected");
        j1_stepper.enable();
        j2_stepper.enable();
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
      setpoint_case = 0;
      num = 0;
    }
  }
  // MAIN CODE WHICH UPDATED THE STEPPER PULSES
  else
  {
    bool j1_sw_triggered = !digitalRead(j1_lim_sw);
    bool j2_sw_triggered = !digitalRead(j2_lim_sw);
    switch(state)
    {
      case HOMING:
        // If either switch is still not engaged, the system will take homing actions
        if(!j1_sw_triggered || !j2_sw_triggered)
        {
          // If only the j1 switch is triggered, retract j2
          if(j1_sw_triggered)
          {
            j2_stepper.pulse_raw(-1);
          }
          // If only the j2 switch is triggered, retract j1 and move j2 along with the assembly
          else if(j2_sw_triggered)
          {
            j1_stepper.pulse_raw(-1);
            j2_stepper.pulse_raw(1);
          }
          // If neither switch is triggered, retract both j1 and j2
          else
          {
            j1_stepper.pulse_raw(-1);
            j2_stepper.pulse_raw(-1);
          }
        }
        else
        {
          Serial.println("Homing Complete");
          j1_setpoint = 0;
          j2_setpoint = 0;
          j1_stepper.zeroCurrentPulse();
          j2_stepper.zeroCurrentPulse();
          state = INTERPOLATING;
        }
        break;

      case INTERPOLATING:
        if( !(j1_stepper.getPulseErr(j1_setpoint) < 0 && j1_sw_triggered) )
        {
          j1_stepper.pulse(&j1_setpoint);
        }
        else
        {
          Serial.println("ERROR: Joint 1 is attempting to actuate into the system");
        }
        if( !(j2_stepper.getPulseErr(j2_setpoint) < 0 && j2_sw_triggered) )
        {
          j2_stepper.pulse(&j2_setpoint);
        }
        else
        {
          Serial.println("ERROR: Joint 2 is attempting to actuate into the system");
        }
        break;
    }
//    Serial.print("j1_lim_sw: ");
//    Serial.println(digitalRead(j1_lim_sw));
//    Serial.print("j2_lim_sw: ");
//    Serial.println(digitalRead(j2_lim_sw));
//    Serial.println();
    //delay(pulse_delay - 2*j1_stepper.getPulseTime());
  }
}
