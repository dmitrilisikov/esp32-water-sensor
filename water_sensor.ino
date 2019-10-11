#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID     = "XXXX";
const char* WIFI_PASSWORD = "XXXX";

const char* ZAP_WEB_HOOK_URL = "https://hooks.zapier.com/hooks/catch/XXXX";

const char* ZAPIER_ROOT_CA = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n" \
                      "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
                      "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n" \
                      "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n" \
                      "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n" \
                      "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n" \
                      "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n" \
                      "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n" \
                      "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n" \
                      "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n" \
                      "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n" \
                      "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n" \
                      "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n" \
                      "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n" \
                      "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n" \
                      "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n" \
                      "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n" \
                      "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
                      "-----END CERTIFICATE-----";

const int SENSOR_PIN = 33;
const int ONBOARD_LED_PIN = 2;

const int SEND_NOTIFICATION_INTERVAL_SECONDS= 3600;
const int CYCLE_DURATION_MS = 5000;

bool notificationSent = false;
int notificationSentSecondsAgo = -1;

void connectToWiFi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWaterSensorPins() {
  pinMode(SENSOR_PIN, INPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  Serial.println("");
  Serial.println("Water sensor listening");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  connectToWiFi();
  setupWaterSensorPins();
}

void sendNotification() {
  HTTPClient http;
  http.begin(ZAP_WEB_HOOK_URL, ZAPIER_ROOT_CA);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    notificationSent = true;
    // Serial.println(payload);
  } else {
    Serial.println("Error on HTTP request");
  }

  Serial.println();
  Serial.println("closing connection");
  http.end();
}

bool isWaterDetected() {
  int sensorState = analogRead(SENSOR_PIN);
  if (sensorState > 70) {
    String message = "Water detected - ";
    message += sensorState;
    Serial.println(message);
    digitalWrite(ONBOARD_LED_PIN, HIGH);
    return true;
  }
  else {
    String message = "All dry - ";
    message += sensorState;
    Serial.println(message);
    digitalWrite(ONBOARD_LED_PIN, LOW);
    return false;
  }
}

void incrementNotificationCounter() {
  if (notificationSent 
    && notificationSentSecondsAgo < SEND_NOTIFICATION_INTERVAL_SECONDS) {
    notificationSentSecondsAgo += CYCLE_DURATION_MS / 1000;
  } else {
    notificationSent = false;
    notificationSentSecondsAgo = -1;
  }
}

void loop() {
  delay(CYCLE_DURATION_MS);
  incrementNotificationCounter();

  if (isWaterDetected() && !notificationSent) {
    sendNotification();
  }
}
