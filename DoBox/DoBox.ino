#include <Servo.h>

const int ANALOG_WRITE_MAX = 255;
const int PIN_ANALOG_CONN_VALUE = 1000;
const int PIN_SW_MAJOR_RED_SHIELDED_CONN_VALUE = 600; // This switch appears to be bad so lets assume everything above this value to be connected

// Inputs
// ... Some are analog because we need all the digital pins on this Uno
const int PIN_SW_BORING_BLACK_ANALOG = 0;
const int PIN_SW_MAJOR_RED_SHIELDED_ANALOG = 1;
const int PIN_SW_BLACK_W_LIGHT_ANALOG = 2;
const int PIN_SW_BLACK_ON_OFF_SWITCH_ANALOG = 3;

const int PIN_SW_BLACK_LIGHT_DIGITAL = 4;

const int PIN_SW_MAJOR_RED_LEFT_LIGHT_DIGITAL = 7;
const int PIN_SW_MAJOR_RED_RIGHT_LIGHT_DIGITAL = 2;

const int PIN_SW_BLACK_ON_OFF_LIGHT_YELLOW = 10;
const int PIN_SW_BLACK_ON_OFF_LIGHT_BLUE = 11;

// Outputs
const int PIN_OUT_PIEZO_DIGITAL = 8;

typedef void (*loopingDelayCallback)(void*);

void loopDelay(int milliseconds, loopingDelayCallback callback, void* ctx = nullptr) {
  unsigned long targetTime = millis() + milliseconds;

  do {
    callback(ctx);
    delay(25);
  } while (targetTime < millis());
}

class FlashyLight {
  private:
    int m_pin;
    unsigned long m_toggleTime;
    unsigned long m_lastTime;
    bool m_state_initial;
    bool m_state;
    bool m_enabled;

    void do_output() {
      digitalWrite(this->m_pin, this->m_state ? HIGH : LOW);
    }
 
  public:
    FlashyLight(int pin, int toggleTime, bool isOnInitially): m_pin(pin), m_toggleTime(toggleTime), m_state_initial(isOnInitially) {
      this->m_enabled = false;
      pinMode(pin, OUTPUT);
    }

    void enable() {
      if (this->m_enabled) {
        return;
      }
      
      this->m_lastTime = millis();
      this->m_enabled = true;
      this->m_state = this->m_state_initial;

      this->do_output();
    }

    void disable() {
      this->m_enabled = false;
      this->m_state = false;

      this->do_output();
    }

    void update() {
      if (!this->m_enabled) {
        return;
      }
      
      unsigned long time = millis();
      unsigned long diff = time - this->m_lastTime;

      if (diff > this->m_toggleTime) {
        this->m_state = !this->m_state;
        this->m_lastTime = time;
        this->do_output();
      }
    }
};

class FadingLight {
  private:
    int m_pin;
    unsigned long m_speed;
    float m_steps_per_milli;
    unsigned long m_lastTime;
    bool m_state_initial;
    bool m_is_fading_out;
    float m_val;
    bool m_enabled;

    void do_output() {
      analogWrite(this->m_pin, (int) this->m_val);
    }
    
 
  public:
    FadingLight(int pin, int speed, bool isOnInitially): m_pin(pin), m_speed(speed), m_steps_per_milli((float)ANALOG_WRITE_MAX / (float)speed), m_state_initial(isOnInitially) {
      this->m_enabled = false;
      pinMode(pin, OUTPUT);
    }

    void enable() {
      if (this->m_enabled) {
        return;
      }
      
      this->m_lastTime = millis();
      this->m_enabled = true;
      this->m_val = this->m_is_fading_out ? ANALOG_WRITE_MAX : 0;
      this->m_is_fading_out = this->m_state_initial;

      this->do_output();
    }

    void disable() {
      this->m_enabled = false;
      this->m_val = !this->m_state_initial ? ANALOG_WRITE_MAX : 0;

      this->do_output();
    }

    void update() {
      if (!this->m_enabled) {
        return;
      }
      
      unsigned long time = millis();
      unsigned long diff = time - this->m_lastTime;

      if (diff >= this->m_speed) {
        this->m_is_fading_out = !this->m_is_fading_out;
        this->m_lastTime = time; 
      }
 
      float pos = ((float) min(diff, this->m_speed) * this->m_steps_per_milli);
      if (this->m_is_fading_out) pos = ANALOG_WRITE_MAX - pos;
      this->m_val = pos;
      this->do_output();

      Serial.write(String(pos).c_str());
      Serial.write("\n");
    }

    
};

// Globals
FlashyLight blackLightDigital(PIN_SW_BLACK_LIGHT_DIGITAL, 200, true);
FlashyLight majorRedLeftLight(PIN_SW_MAJOR_RED_LEFT_LIGHT_DIGITAL, 150, true);
FlashyLight majorRedRightLight(PIN_SW_MAJOR_RED_RIGHT_LIGHT_DIGITAL, 150, false);
FlashyLight yellowLeftRight(PIN_SW_BLACK_ON_OFF_LIGHT_YELLOW, 400, true);

FadingLight blueLight(PIN_SW_BLACK_ON_OFF_LIGHT_BLUE, 500, true);

Servo smileyServo;

void setup() {
  // DBG
  Serial.begin(9600);
  Serial.write("Hello world - kids do-box v1.0\n");
  Serial.write("\n");
  Serial.write("Initialization...\n");

  // RNG
  Serial.write("... RNG\n");
  unsigned long rngSeed = analogRead(PIN_SW_BORING_BLACK_ANALOG) /* This value floats and is random*/;
  randomSeed(rngSeed);
  
  // Pins
  Serial.write("... pin modes\n");
  pinMode(PIN_OUT_PIEZO_DIGITAL, OUTPUT);

  smileyServo.attach(9);

  Serial.write("Done!\n");
  Serial.write("\n");
}

bool isBoringBlackAnalogPressed() {
  // Though this value floats, unless the button is pressed, it never reaches 1023 (max value)
  return PIN_ANALOG_CONN_VALUE < analogRead(PIN_SW_BORING_BLACK_ANALOG);
}

bool isBlackOnOffSwitchSwitched() {
  return PIN_ANALOG_CONN_VALUE < analogRead(PIN_SW_BLACK_ON_OFF_SWITCH_ANALOG);  
}

bool isBlackSwitchWithLightSwitchedOn() {
  dbgInt("B_WITH_LIGHT", analogRead(PIN_SW_BLACK_W_LIGHT_ANALOG));
  return PIN_ANALOG_CONN_VALUE < analogRead(PIN_SW_BLACK_W_LIGHT_ANALOG);
}

bool isMainRedShieldedPressed() {
  //dbgInt("RED", analogRead(PIN_SW_MAJOR_RED_SHIELDED_ANALOG));
  
  // Though this value floats, unless the button is pressed, it never reaches 1024 (max value)
  return PIN_SW_MAJOR_RED_SHIELDED_CONN_VALUE < analogRead(PIN_SW_MAJOR_RED_SHIELDED_ANALOG);
}

void dbgInt(const char* prefix, int val) {
  Serial.write("[");
  Serial.write(prefix);
  Serial.write("]");
  Serial.write(String(val).c_str());
  Serial.write("\n");  
}

void updateCommonItems() {
  blackLightDigital.update();
  
  majorRedLeftLight.update();
  majorRedRightLight.update();

  yellowLeftRight.update();
  blueLight.update();
}

void loop() {
  Serial.write("BEGIN LOOP \n");

  updateCommonItems();
  
  if (isBoringBlackAnalogPressed()) {
    Serial.write("Biep biep richie!");
    int pitch = random(50, 4000);
    tone(PIN_OUT_PIEZO_DIGITAL, pitch, 250);

    smileyServo.write(180);
  }

  while (isMainRedShieldedPressed()) {
    Serial.write("Emergency!\n");
    majorRedLeftLight.enable();
    majorRedRightLight.enable();

    smileyServo.write(0);
    tone(PIN_OUT_PIEZO_DIGITAL, 2000, 250);
    loopDelay(250, [](void*) { updateCommonItems(); });
    
    tone(PIN_OUT_PIEZO_DIGITAL, 4000, 250);
    loopDelay(250, [](void*) { updateCommonItems(); });
    smileyServo.write(180);
    
    tone(PIN_OUT_PIEZO_DIGITAL, 200, 250);
    loopDelay(250, [](void*) { updateCommonItems(); });
    
    tone(PIN_OUT_PIEZO_DIGITAL, 4000, 250);
    loopDelay(250, [](void*) { updateCommonItems(); });
  }

  majorRedLeftLight.disable();
  majorRedRightLight.disable();

  if (isBlackSwitchWithLightSwitchedOn()) {
    tone(PIN_OUT_PIEZO_DIGITAL, 1000, 250);
    blackLightDigital.enable();
    smileyServo.write(90);
  } else {
    blackLightDigital.disable();
    smileyServo.write(0);
  }

  if (isBlackOnOffSwitchSwitched()) {
    yellowLeftRight.enable();
    blueLight.disable();
  } else {
    yellowLeftRight.disable();
    blueLight.enable(); 
  }

  loopDelay(500, [](void*) { updateCommonItems(); });

  Serial.write("END LOOP \n");
}
