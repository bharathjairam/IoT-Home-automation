

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#define lRbulb 16
#define lRfan 5
#define kRpump 4
#define kRfreezer 0
#define BRac D4

//Provide the token generation process info.
#include <addons/TokenHelper.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "MAJOR PROJECT"
#define WIFI_PASSWORD "12345678"

//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyAxR7RgKvyBIm24CZeGoBD-4GJ9QeUNlSI"

/* 3. Define the RTDB URL */
#define DATABASE_URL "iot-home-automation-c22e6-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "avuku12334@gmail.com"
#define USER_PASSWORD "Nani2618@"

//Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;
FirebaseJson json;
FirebaseAuth auth;
FirebaseJsonData result;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

void streamCallback(StreamData data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  //Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server. 
  //Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{

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

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  //Or use legacy authenticate method
  //config.database_url = DATABASE_URL;
  //config.signer.tokens.legacy_token = "<database secret>";

  //To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

//Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.beginStream(stream, "/appliance"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);

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

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(lRbulb, OUTPUT);
  pinMode(lRfan, OUTPUT);
  pinMode(kRpump, OUTPUT);
  pinMode(kRfreezer, OUTPUT);
  pinMode(BRac, OUTPUT);
  
}

void loop()
{
//if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
// {
//   sendDataPrevMillis = millis();
//   FirebaseJson json;
//    Serial.printf("Set json... %s\n\n", Firebase.setJSON(fbdo, "/appliance", json) ? "ok" : fbdo.errorReason().c_str());
// }

  if (dataChanged)
  {
    dataChanged = false;
    //When stream data is available, do anything here...
    Serial.println("data changed");
    Serial.println(stream.stringData());
    Serial.print(stream.dataPath()); 

    
    if(stream.dataPath()=="/BRac"){
      if(stream.stringData()=="false"){
        digitalWrite(BRac, LOW);
        }else{
        digitalWrite(BRac, HIGH);
        }
    }
    else if(stream.dataPath()=="/BRbulb"){
      if(stream.stringData()=="false"){
        digitalWrite(lRbulb, LOW);
        }else{
        digitalWrite(lRbulb, HIGH);
        }
    }
    else if(stream.dataPath()=="/BRfan"){
      if(stream.stringData()=="false"){
        digitalWrite(lRfan, LOW);
        }else{
        digitalWrite(lRfan, HIGH);
        }
    }
    else if(stream.dataPath()=="/BRlamp"){
      if(stream.stringData()=="true"){
        digitalWrite(LED_BUILTIN, LOW);
        }else{
        digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    else if(stream.dataPath()=="/kRbulb"){
      if(stream.stringData()=="false"){
        digitalWrite(lRbulb, LOW);
        }else{
        digitalWrite(lRbulb, HIGH);
        }
    }
    else if(stream.dataPath()=="/kRfreezer"){
      if(stream.stringData()=="false"){
        digitalWrite(kRfreezer, LOW);
        }else{
        digitalWrite(kRfreezer, HIGH);
        }
    }
    else if(stream.dataPath()=="/kRfan"){
      if(stream.stringData()=="false"){
        digitalWrite(lRfan, LOW);
        }else{
        digitalWrite(lRfan, HIGH);
        }
    }
    else if(stream.dataPath()=="/kRpump"){
      if(stream.stringData()=="false"){
        digitalWrite(kRpump, LOW);
        }else{
        digitalWrite(kRpump, HIGH);
        }
    }
    else if(stream.dataPath()=="/lRbulb"){
      if(stream.stringData()=="false"){
        digitalWrite(lRbulb, LOW);
        }else{
        digitalWrite(lRbulb, HIGH);
        }
    }
    else if(stream.dataPath()=="/kRpump"){
      if(stream.stringData()=="false"){
        digitalWrite(kRpump, LOW);
        }else{
        digitalWrite(kRpump, HIGH);
        }
    }
    else if(stream.dataPath()=="/lRfan"){
      if(stream.stringData()=="false"){
        digitalWrite(lRfan, LOW);
        }else{
        digitalWrite(lRfan, HIGH);
        }
    }
    else if(stream.dataPath()=="/lRtv"){
      if(stream.stringData()=="true"){
        digitalWrite(LED_BUILTIN, LOW);
        }else{
        digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    else{
      if(stream.stringData()=="true"){
        digitalWrite(LED_BUILTIN, LOW);
        }else{
        digitalWrite(LED_BUILTIN, HIGH);
        }
    }
}
}
