#include "connect_wifi.h"
#include "connect_firebase.h"
#define USER_EMAIL "samiot@gmail.com"
#define USER_PASSWORD "nopassword"

void callbackOut(int pin, int state, bool viaCloud)
{
  Serial.printf("Pin: %d, State: %d, Via Cloud: %s\n", pin, state, viaCloud ? "true" : "false");
  if (viaCloud)
  {
    digitalWrite(pin, state);
  }
}

void setup()
{
  pinMode(D1, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  connectWifi(WIFI_SSID, WIFI_PASSWORD);
  connectFirebase(USER_EMAIL, USER_PASSWORD);
  get(callbackOut);
  listen(callbackOut);
}

int state_d1 = LOW;
bool is_d1 = true;

void loop()
{
  for (int i = 0; i < 6; i++)
  {
    handleInputs(pinPaths[i], state_d1, is_d1);
  }
}

void handleInputs(String pinPath, int &state, bool &inPin)
{
  int newState = digitalRead(D1);
  if (newState == LOW && inPin == true)
  {
    inPin = false;
    state = state == LOW ? HIGH : LOW;
    Serial.print(newState);
    Serial.println(state);
    digitalWrite(getPin(pinPath), state);
    pushStatus(pinPath, state);
    delay(300);
  }
  else if (newState == HIGH)
  {
    inPin = true;
  }
}
