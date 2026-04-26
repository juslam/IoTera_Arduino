#include "IoTeraDevice.h"

CommandCallback IoTeraDevice::_cmdCallback = nullptr;

IoTeraDevice::IoTeraDevice() : 
    aClient(ssl_client, getNetwork(network)),
    streamClient(stream_ssl_client, getNetwork(network)),
    user_auth("AIzaSyAEBVcrMZ4V7nyAF5xtAgCtCsrncdAuoD0", "mesin@iotera.com", "mesin12345") {
    _streamStarted = false;
    _lastHeartbeat = 0;
}

void IoTeraDevice::begin(const char* ssid, const char* password, const char* deviceKey) {
    _deviceKey = String(deviceKey);

    // 1. Koneksi ke WiFi
    WiFi.begin(ssid, password);
    Serial.print("Menghubungkan ke Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nBerhasil terhubung ke Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Konfigurasi koneksi SSL/TLS yang Insecure
    ssl_client.setInsecure();
    stream_ssl_client.setInsecure();
#if defined(ESP8266)
    ssl_client.setBufferSizes(4096, 1024);
    stream_ssl_client.setBufferSizes(4096, 1024);
#endif

    // 2. Inisialisasi Firebase App
    initializeApp(aClient, app, getAuth(user_auth));
    app.getApp<RealtimeDatabase>(Database);
    Database.url("https://iotera-94b07-default-rtdb.asia-southeast1.firebasedatabase.app/");
}

void IoTeraDevice::loop() {
    app.loop();
    Database.loop();

    if (!app.ready()) return;

    // Heartbeat: Sinyal otomatis ke App bahwa alat ini masih hidup setiap 60 detik
    if (millis() - _lastHeartbeat > 60000 || _lastHeartbeat == 0) {
        _lastHeartbeat = millis();
        String hbPath = "/device_status/" + _deviceKey + "/heartbeat";
        Database.set<String>(aClient, hbPath, String(millis()));
    }

    if (!_streamStarted) {
        String streamPath = "/configurations/" + _deviceKey + "/states";
        
        // Pada FirebaseClient versi 1.1.x, callback dan parameter sse (true) dimasukkan langsung ke dalam get()
        Database.get(streamClient, streamPath.c_str(), AsyncResultCallback(IoTeraDevice::streamCallback), true);
        
        Serial.println("Siap menerima perintah dari Aplikasi IoTera...");
        _streamStarted = true;
    }
}

void IoTeraDevice::streamCallback(AsyncResult &aResult) {
    if (aResult.available()) {
        RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
        if (RTDB.isStream()) {
            if (_cmdCallback != nullptr) {
                String topic = RTDB.dataPath();
                if (topic.startsWith("/")) topic = topic.substring(1);
                
                String payload = RTDB.to<String>();
                _cmdCallback(topic, payload);
            }
        }
    }
}

void IoTeraDevice::sendSensorData(String topic, String value) {
    if (!app.ready()) {
        Serial.println("Gagal mengirim: Firebase belum siap (Menunggu Autentikasi / Internet).");
        return;
    }

    // Ganti garis miring (/) dengan underscore (_) sesuai dengan logic di Flutter app
    String safeTopic = topic;
    safeTopic.replace("/", "_");

    // Perbarui state di RTDB, maka HP akan seketika langsung merespon
    String path = "/configurations/" + _deviceKey + "/states/" + safeTopic;
    
    Serial.print("Mengirim data [");
    Serial.print(path);
    Serial.print("] -> ");
    Serial.println(value);

    Database.set<String>(aClient, path, value);
}

void IoTeraDevice::setCommandCallback(CommandCallback callback) {
    _cmdCallback = callback;
}

String IoTeraDevice::parseJson(String jsonString, String path) {
    FirebaseJson json;
    FirebaseJsonData jsonData;
    json.setJsonData(jsonString);
    
    if (json.get(jsonData, path)) {
        if (jsonData.type == "string") return jsonData.stringValue;
        else if (jsonData.type == "int") return String(jsonData.intValue);
        else if (jsonData.type == "float") return String(jsonData.floatValue);
        else if (jsonData.type == "double") return String(jsonData.doubleValue);
        else if (jsonData.type == "boolean") return jsonData.boolValue ? "true" : "false";
        else if (jsonData.type == "object" || jsonData.type == "array") return jsonData.stringValue; 
        else if (jsonData.type == "null") return "null";
    }
    return ""; // Kembalikan string kosong jika path tidak ditemukan
}
