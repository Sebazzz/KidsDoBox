const int PIN_ANALOG_MAX_VALUE = 1023;

// Inputs
// ... Some are analog because we need all the digital pins on this Uno
const int PIN_SW_BORING_BLACK_ANALOG = 0;
const int PIN_SW_MAJOR_RED_SHIELDED = 1;

// Outputs
const int PIN_OUT_PIEZO_DIGITAL = 8;

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

  Serial.write("Done!\n");
  Serial.write("\n");
}

bool isBoringBlackAnalogPressed() {
  // Though this value floats, unless the button is pressed, it never reaches 1024 (max value)
  return PIN_ANALOG_MAX_VALUE == analogRead(PIN_SW_BORING_BLACK_ANALOG);
}

bool isMainRedShieldedPressed() {
  //dbgInt("RED", analogRead(PIN_SW_MAJOR_RED_SHIELDED));
  // Though this value floats, unless the button is pressed, it never reaches 1024 (max value)
  return PIN_ANALOG_MAX_VALUE == analogRead(PIN_SW_MAJOR_RED_SHIELDED);
}

void dbgInt(const char* prefix, int val) {
  Serial.write("[");
  Serial.write(prefix);
  Serial.write("]");
  Serial.write(String(val).c_str());
  Serial.write("\n");  
}

void loop() {
  Serial.write("Running program loop...\n");
  
  while (true) {
    if (isBoringBlackAnalogPressed()) {
      Serial.write("Biep biep richie!");
      int pitch = random(50, 4000);
      tone(PIN_OUT_PIEZO_DIGITAL, pitch, 250);
    }

    while (isMainRedShieldedPressed()) {
      Serial.write("Emergency!");
      tone(PIN_OUT_PIEZO_DIGITAL, 2000, 250);
      delay(250);
      tone(PIN_OUT_PIEZO_DIGITAL, 4000, 250);
      delay(250);
      tone(PIN_OUT_PIEZO_DIGITAL, 2000, 250);
      delay(250);
      tone(PIN_OUT_PIEZO_DIGITAL, 4000, 250);
      delay(250);
    }
    
    delay(500);
  }
}
