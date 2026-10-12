#include "IoTeraDevice.h"

CommandCallback IoTeraDevice::_cmdCallback = nullptr;

IoTeraDevice::IoTeraDevice() : 
    aClient(ssl_client, getNetwork(network)),
    streamClient(stream_ssl_client, getNetwork(network)),
    user_auth("AIzaSyAEBVcrMZ4V7nyAF5xtAgCtCsrncdAuoD0", "mesin@iotera.com", "mesin12345") {
    _streamStarted = false;
    _lastHeartbeat = 0;
    _lastWifiAttempt = 0;
}

void IoTeraDevice::begin(const char* ssid, const char* password, const char* deviceKey) {
    _deviceKey = String(deviceKey);
    _ssid = String(ssid);
    _password = String(password);

    // 1. Koneksi ke WiFi (NON-BLOCKING: hanya trigger, tidak while-timeout)
    WiFi.mode(WIFI_STA); // Set ESP agar hanya menjadi Station (Client), mematikan Access Point
    WiFi.setAutoReconnect(true);
    WiFi.disconnect();
    WiFi.begin(_ssid.c_str(), _password.c_str());
    _lastWifiAttempt = millis();
    Serial.println("Menghubungkan ke Wi-Fi (non-blocking, proses lain tetap jalan)...");

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

    // WiFi belum nyambung (SSID salah / password salah / router mati):
    // jangan block, beri kesempatan proses user lain, retry tiap 5 detik.
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - _lastWifiAttempt >= 5000) {
            _lastWifiAttempt = millis();
            Serial.print("WiFi belum terhubung (status=");
            Serial.print(WiFi.status());
            Serial.println("). Mencoba lagi, proses lain tetap jalan...");
            WiFi.disconnect();
            WiFi.begin(_ssid.c_str(), _password.c_str());
        }
        // Stream harus didaftarkan ulang setelah WiFi pulih
        _streamStarted = false;
        return;
    }

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
    // Deteksi jika terjadi error atau terputus pada Stream
    if (aResult.isError()) {
        Serial.println("⚠️ Stream Error/Disconnected: " + aResult.error().message());
    }
    
    // Event info dari Firebase Client dikomentari karena isEvent() tidak didukung di versi ini
    // if (aResult.isEvent()) {
    //     Serial.println("ℹ️ Stream Event: " + aResult.appEvent().message());
    // }

    if (aResult.available()) {
        RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
        if (RTDB.isStream()) {
            if (_cmdCallback != nullptr) {
                String topic = RTDB.dataPath();
                if (topic.startsWith("/")) topic = topic.substring(1);
                
                // Otomatis hapus awalan "pin_" agar user bisa langsung mencocokkan nama asli (DX Improvement)
                if (topic.startsWith("pin_")) {
                    topic = topic.substring(4);
                }

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

void IoTeraDevice::sendPinState(String pin, String value) {
    // Topik untuk pin secara konsisten menggunakan prefix "pin_" 
    // agar sesuai dengan yang diharapkan oleh aplikasi mobile.
    sendSensorData("pin_" + pin, value);
}

void IoTeraDevice::setCommandCallback(CommandCallback callback) {
    _cmdCallback = callback;
}

bool IoTeraDevice::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool IoTeraDevice::waitForConnect(unsigned long timeoutMs) {
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
        delay(100);
        yield();
    }
    return WiFi.status() == WL_CONNECTED;
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
