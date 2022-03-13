
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "config.h"

#define WIFI_SSID "Realme 9"
#define WIFI_PASSWORD "11111123"

#define USER_EMAIL "samiot@gmail.com"
#define USER_PASSWORD "nopassword"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DB_URL;
  config.token_status_callback = tokenStatusCallback;
  
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  delay(300);
  Serial.println("Starting Firebase Config");
  
  Firebase.begin(&config, &auth);
  
  Serial.println("Firebase Config DONE");
  Firebase.reconnectWiFi(true);
  
  Serial.println("Reconnect true");
  Firebase.setDoubleDigits(5);
  
  Serial.println("Set double digits");
}


bool taskCompleted = false;
void loop() {
  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.printf("Set timestamp... %s\n", Firebase.RTDB.setTimestamp(&fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());
    
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
    {
      
      //In setTimestampAsync, the following timestamp will be 0 because the response payload was ignored for all async functions.

      //Timestamp saved in millisecond, get its seconds from int value
      Serial.print("TIMESTAMP (Seconds): ");
      Serial.println(fbdo.to<int>());

      //Or print the total milliseconds from double value
      //Due to bugs in Serial.print in Arduino library, use printf to print double instead.
      printf("TIMESTAMP (milliSeconds): %lld\n", fbdo.to<uint64_t>());
    }

    Serial.printf("Get timestamp... %s\n", Firebase.RTDB.getDouble(&fbdo, "/test/timestamp") ? "ok" : fbdo.errorReason().c_str());
    if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
      printf("TIMESTAMP: %lld\n", fbdo.to<uint64_t>());

    //To set and push data with timestamp, requires the JSON data with .sv placeholder
    FirebaseJson json;

    json.set("Data", "Hello");
    //now we will set the timestamp value at Ts
    json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value

    //Set data with timestamp
    Serial.printf("Set data with timestamp... %s\n", Firebase.RTDB.setJSON(&fbdo, "/test/set/data", &json) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());

    //Push data with timestamp
    Serial.printf("Push data with timestamp... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/test/push/data", &json) ? "ok" : fbdo.errorReason().c_str());

    //Get previous pushed data
    Serial.printf("Get previous pushed data... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/push/data/" + fbdo.pushName()) ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());
  }
}
