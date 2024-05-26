#include "connect_wifi.h"
#include "connect_firebase.h"
#include "pin_map.h"

#define USER_EMAIL "samiot@gmail.com"
#define USER_PASSWORD "nopassword"

void streamCallback(FirebaseStream data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                String(data.intData()));

  if (data.dataType() == "int")
  {
    Serial.printf("Changing LED_BUILTIN: %d\n", data.intData());
    digitalWrite(LED_BUILTIN, data.intData());
  }
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void setup()
{
  pinMode(D1, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  connectWifi(WIFI_SSID, WIFI_PASSWORD);
  connectFirebase(USER_EMAIL, USER_PASSWORD);
  get("/status", streamCallback);
  listen("/status", streamCallback);
}

unsigned long sendDataPrevMillis = 0;
int count = 0;

int state_d1 = LOW;
bool is_d1 = true;

void loop()
{
  handleInputs(LED_BUILTIN, state_d1, is_d1);

  /* if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    count++;
    FirebaseJson json;
    json.add("data", "hello");
    json.add("num", count);
    Serial.printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, "/test/stream/data/json", &json) ? "ok" : fbdo.errorReason().c_str());
  } */
}

void handleInputs(int pin, int &state_d1, bool &is_d1)
{
  int newState_d1 = digitalRead(D1);
  if (newState_d1 == LOW && is_d1 == true)
  {
    is_d1 = false;
    state_d1 = state_d1 == LOW ? HIGH : LOW;
    Serial.print(newState_d1);
    Serial.println(state_d1);
    digitalWrite(pin, state_d1);
    pushStatus("/status/" + String(pin), state_d1);
    delay(300);
  }
  else if (newState_d1 == HIGH)
  {
    is_d1 = true;
  }
}
