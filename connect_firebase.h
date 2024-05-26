// https://github.com/mobizt/Firebase-ESP-Client
// Firebase Arduiono Client Library for ESP8266 and ESP32
// Version : 4.4.14
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "config.h"
#include "pin_map.h"

FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void connectFirebase(String userEmail, String userPassword)
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = userEmail;
  auth.user.password = userPassword;

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

// Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048, 512);
#endif

  /** Timeout options, below is default config.

  //WiFi reconnect timeout (interval) in ms (10 sec - 5 min) when WiFi disconnected.
  config.timeout.wifiReconnect = 10 * 1000;

  //Socket begin connection timeout (ESP32) or data transfer timeout (ESP8266) in ms (1 sec - 1 min).
  config.timeout.socketConnection = 30 * 1000;

  //ESP32 SSL handshake in ms (1 sec - 2 min). This option doesn't allow in ESP8266 core library.
  config.timeout.sslHandshake = 2 * 60 * 1000;

  //Server response read timeout in ms (1 sec - 1 min).
  config.timeout.serverResponse = 10 * 1000;

  //RTDB Stream keep-alive timeout in ms (20 sec - 2 min) when no server's keep-alive event data received.
  config.timeout.rtdbKeepAlive = 45 * 1000;

  //RTDB Stream reconnect timeout (interval) in ms (1 sec - 1 min) when RTDB Stream closed and want to resume.
  config.timeout.rtdbStreamReconnect = 1 * 1000;

  //RTDB Stream error notification timeout (interval) in ms (3 sec - 30 sec). It determines how often the readStream
  //will return false (error) when it called repeatedly in loop.
  config.timeout.rtdbStreamError = 3 * 1000;

  */
}

/// -------------------------- DB operations -----------------------------------
struct ResultBox
{
  int pin;
  int status;
  bool viaCloud;
};

typedef void(ResultCallback)(int pin, int status, bool viaCloud);
ResultCallback *globalCallback;

void pushStatus(String pinPath, int status, bool viaCloud = false)
{
  Serial.printf("Pushing status... %s\n\n", Firebase.RTDB.setString(&fbdo, "/dstatus/" + pinPath, String(status) + "_" + String(viaCloud)) ? "OK" : fbdo.errorReason().c_str());
  globalCallback(getPin(pinPath), status, true);
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

ResultBox getResult(FirebaseStream data, bool forceViaCloud = false)
{
  Serial.print("XXXXX------- GOT Data -------XXXXX \n");
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                String(data.intData()));

  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
  if (data.dataType() == "string")
  {
    String payload = data.stringData();
    ResultBox result = {getPin(data.dataPath()), payload.substring(0, 1).toInt(), forceViaCloud ? true : payload.substring(2).toInt()};
    Serial.printf("streamPath: %s, status: %d, viaCloud: %s\n\n", data.dataPath(), result.status, result.viaCloud ? "true" : "false");
    Serial.printf("XXXXX------- Received Data -------XXXXX \n");
    return result;
  }
  return {-1, -1, false};
}

void streamCallbackListen(FirebaseStream data)
{
  if (data.dataType() == "json")
  {
    Serial.print("YYYYYY------- GOT Data -------YYYYYY \n");
    FirebaseJson &json = data.jsonObject();
    FirebaseJsonData result;
    for (int i = 0; i < pinPaths->length(); i++)
    {
      if (json.get(result, pinPaths[i].substring(1)))
      {
        if (result.type != "string" || result.stringValue.length() < 3)
        {
          continue;
        }
        String payload = result.stringValue;
        ResultBox resultBox = {pins[i], payload.substring(0, 1).toInt(), true};
        Serial.printf("streamPath: %s, status: %d, viaCloud: %s\n\n", pinPaths[i], resultBox.status, "true");
        globalCallback(resultBox.pin, resultBox.status, true);
      }
    }
    Serial.print("YYYYYY------- Received Data -------YYYYYY \n");
  }
  else
  {
    ResultBox result = getResult(data);
    if (result.pin != -1)
    {
      globalCallback(result.pin, result.status, result.viaCloud);
    }
  }
}

void listen(ResultCallback callback)
{
  globalCallback = callback;
  if (!Firebase.RTDB.beginStream(&stream, "/dstatus"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallbackListen, streamTimeoutCallback);
}

void get(ResultCallback callback)
{
  globalCallback = callback;
  if (!Firebase.RTDB.get(&stream, "/dstatus"))
    Serial.printf("get error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallbackListen, streamTimeoutCallback);
}
