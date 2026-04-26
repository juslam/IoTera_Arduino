#ifndef IOTERA_DEVICE_H
#define IOTERA_DEVICE_H

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <FirebaseClient.h>
#include <FirebaseJson.h>
#include <WiFiClientSecure.h>

// Callback function type untuk menangani pesan/perintah yang masuk dari Aplikasi
typedef void (*CommandCallback)(String topic, String payload);

class IoTeraDevice {
  public:
    IoTeraDevice();
    
    // Inisialisasi koneksi WiFi dan Firebase
    void begin(const char* ssid, const char* password, const char* deviceKey);
    
    // Harus dipanggil di dalam fungsi void loop() Arduino
    void loop();
    
    // Mengirim data sensor ke Dashboard Aplikasi
    void sendSensorData(String topic, String value);
    
    // Mendaftarkan fungsi untuk menangani aksi tombol/slider dari Aplikasi
    void setCommandCallback(CommandCallback callback);

    // Helper untuk mem-parsing nilai spesifik dari string JSON menggunakan path
    String parseJson(String jsonString, String path);

  private:
    String _deviceKey;
    static CommandCallback _cmdCallback;
    bool _streamStarted;
    unsigned long _lastHeartbeat;

    WiFiClientSecure ssl_client;
    WiFiClientSecure stream_ssl_client;
    DefaultNetwork network;
    AsyncClientClass aClient;
    AsyncClientClass streamClient;
    
    FirebaseApp app;
    RealtimeDatabase Database;
    UserAuth user_auth;
    
    static void streamCallback(AsyncResult &aResult);
};

#endif