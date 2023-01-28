#include <Arduino.h>
#include <HomeSpan.h>

// Pairing Code: 466-37-726

const int STATUS_PIN = 2;
const int CONTROL_PIN = 27;
const int RELAY_PIN = 13;
const long UNLOCK_TIME_MILLIS = 5000;

const int STATE_SECURED = 1;
const int STATE_UNSECURED = 0;

struct DoorBuzzer : Service::LockMechanism {

  SpanCharacteristic *currentState;
  SpanCharacteristic *targetState;
  int relayPin;
  long unlockedUntilMillis = 0;
  
  DoorBuzzer(int relayPin) : Service::LockMechanism() {
    currentState = new Characteristic::LockCurrentState(1);
    targetState = new Characteristic::LockTargetState(1);
    this->relayPin=relayPin;

    pinMode(relayPin,OUTPUT);
    digitalWrite(relayPin, LOW);
  }

  boolean update() {       
    if (targetState->getNewVal() == STATE_UNSECURED) {
      unlockedUntilMillis = millis() + UNLOCK_TIME_MILLIS;

      digitalWrite(relayPin, HIGH);
      this->currentState->setVal(STATE_UNSECURED);
    } else {
      unlockedUntilMillis = 0;

      digitalWrite(relayPin, LOW);
      this->currentState->setVal(STATE_SECURED);
    }

    return true;
  
  }

  void loop() {
    long currentTimeMillis = millis();

    boolean relayUnlocked = digitalRead(relayPin) == HIGH;

    boolean shouldBeLocked = unlockedUntilMillis < currentTimeMillis;

    if (shouldBeLocked && relayUnlocked) {
      digitalWrite(relayPin, LOW);
      this->targetState->setVal(STATE_SECURED);
      this->currentState->setVal(STATE_SECURED);
    }
  }
};

DoorBuzzer *doorBuzzer;
TaskHandle_t h_HK_poll;

void setup()
{
  Serial.begin(115200);

  homeSpan.setLogLevel(1);
  if (STATUS_PIN > 0) homeSpan.setStatusPin(STATUS_PIN);
  if (CONTROL_PIN > 0) homeSpan.setControlPin(CONTROL_PIN);
  homeSpan.begin(Category::Locks);

  new SpanAccessory();

  new Service::AccessoryInformation();
  new Characteristic::Name("Door Buzzer");
  new Characteristic::Manufacturer("HomeSpan");
  new Characteristic::SerialNumber("AK-0001");
  new Characteristic::Model("DoorBuzzer");
  new Characteristic::FirmwareRevision("0.9");
  new Characteristic::Identify();

  new Service::HAPProtocolInformation();
  new Characteristic::Version("1.1.0");

  doorBuzzer = new DoorBuzzer(RELAY_PIN);

  // delay(1000);
}

void loop()
{
    homeSpan.poll();
}
