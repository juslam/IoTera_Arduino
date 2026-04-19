/*
 * PENTING / IMPORTANT:
 * Rekomendasi: Untuk solusi tercepat agar proyek IoTera Anda dapat berjalan 
 * (terutama jika Anda mengalami error 'override' pada library Firebase saat kompilasi), 
 * silakan turunkan (downgrade) versi core ESP32 ke 2.0.17 melalui Arduino Boards Manager.
 * (Recommendation: For the fastest solution to get your IoTera project running, especially 
 * if you encounter an 'override' error on the Firebase library during compilation, please 
 * downgrade the ESP32 core version to 2.0.17 via the Arduino Boards Manager.)
 *
 * Catatan Library: Jika library Firebase tidak terinstal secara otomatis, silakan cari dan 
 * instal "FirebaseClient" (oleh mobizt) di Library Manager. Pastikan Anda menggunakan library 
 * versi baru ini (bukan versi legacy), karena IoTera menggunakan arsitektur Async terbaru (FirebaseClient.h).
 * (Library Note: If the Firebase library is not installed automatically, please search and install 
 * "FirebaseClient" by mobizt in the Library Manager. Make sure to use this new version instead of 
 * the legacy one, as IoTera uses the latest Async architecture.)
 */

#include <IoTeraDevice.h>

// Konfigurasi Wi-Fi / Wi-Fi Configuration
#define WIFI_SSID "NAMA_WIFI_ANDA"
#define WIFI_PASS "PASSWORD_WIFI_ANDA"

// Masukkan API Key (Session ID) yang di-generate dari aplikasi mobile user
// Insert the API Key (Session ID) generated from the user's mobile app
#define DEVICE_KEY "masukkan-apikey-dari-aplikasi-disini"

// Definisikan pin yang terhubung ke LED / Relay
// Define the pin connected to the LED / Relay
#define LED_PIN 2 // Ganti dengan D1 jika menggunakan NodeMCU ESP8266 / Change to D1 if using NodeMCU ESP8266

IoTeraDevice iotDevice;
unsigned long previousMillis = 0;
bool isLedOn = false; // Variabel untuk menyimpan status LED saat ini / Variable to store current LED state

// Fungsi ini akan terpanggil otomatis saat Anda menekan tombol / slider di aplikasi mobile
// This function will be called automatically when you press a button / slider in the mobile app
void onAppCommand(String topic, String payload) {
  Serial.println("\n========== PESAN DARI APLIKASI / MESSAGE FROM APP ==========");
  Serial.println("Topic   : " + topic);
  Serial.println("Payload : " + payload);
  Serial.println("=========================================");
  
  // Contoh: Jika Anda menambahkan widget tombol di aplikasi dengan pin "D1"
  // Example: If you add a button widget in the app with pin "D1"
  // Menggunakan indexOf agar lebih aman terhadap format path Firebase (misal: "/pin_D1")
  // Using indexOf to be safer against Firebase path formats (e.g., "/pin_D1")
  if (topic.indexOf("pin_D1") != -1) {
    if (payload.indexOf("TOGGLE") != -1) {
      Serial.println("Sinyal diterima! Mengubah status LED. / Signal received! Changing LED status.");
      isLedOn = !isLedOn; // Balikkan status dari true ke false, atau sebaliknya / Toggle status from true to false, or vice versa
      digitalWrite(LED_PIN, isLedOn ? HIGH : LOW); // Menyalakan atau mematikan LED / Turn LED on or off
    }
  } 
  // Contoh jika menerima JSON: {"lampu":{"kamar":true, "teras":false}, "kecerahan":80}
  // Example if receiving JSON: {"lampu":{"kamar":true, "teras":false}, "kecerahan":80}
  else if (topic == "konfigurasi_ruangan") {
    String statusKamar = iotDevice.parseJson(payload, "lampu/kamar");
    String nilaiKecerahan = iotDevice.parseJson(payload, "kecerahan");
    
    Serial.println("Status Lampu Kamar / Room Light Status: " + statusKamar);
    Serial.println("Level Kecerahan / Brightness Level: " + nilaiKecerahan);
  } else {
    Serial.println("Info: Pesan diterima, namun topic '" + topic + "' belum ditangani di kode (unhandled).");
    // Info: Message received, but the topic is unhandled in the code.
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // Set pin sebagai OUTPUT / Set pin as OUTPUT
  
  // Mendaftarkan callback untuk menerima perintah dari HP
  // Register callback to receive commands from the phone
  iotDevice.setCommandCallback(onAppCommand);
  
  // Memulai koneksi IoTera dengan API Key unik dari Aplikasi
  // Start IoTera connection with the unique API Key from the App
  iotDevice.begin(WIFI_SSID, WIFI_PASS, DEVICE_KEY);
}

void loop() {
  // Pastikan WiFi selalu terhubung kembali jika tiba-tiba terputus
  // Ensure WiFi always reconnects if it suddenly disconnects
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Koneksi Wi-Fi terputus! Mencoba menyambung kembali... / Wi-Fi disconnected! Trying to reconnect...");
    WiFi.reconnect();
    delay(2000); // Tunggu sebentar agar tidak membebani siklus loop / Wait a moment to avoid overloading the loop cycle
    return;      // Lewati eksekusi Firebase di bawahnya hingga WiFi pulih / Skip Firebase execution below until WiFi recovers
  }

  // Wajib dipanggil untuk menjaga koneksi dan mendengarkan stream Firebase
  // Must be called to maintain connection and listen to Firebase stream
  iotDevice.loop();
  
  // Contoh: Kirim data sensor suhu setiap 10 detik (diperlambat agar stream lebih stabil)
  // Example: Send temperature sensor data every 10 seconds (slowed down for a more stable stream)
  if (millis() - previousMillis > 10000) {
    previousMillis = millis();
    float suhu = random(200, 350) / 10.0; // Simulasi suhu 20.0 - 35.0 C / Temperature simulation
    iotDevice.sendSensorData("iotera/sensor/suhu", String(suhu));
    
    // --- CONTOH MENGIRIM NILAI KE WIDGET SLIDER ---
    // --- EXAMPLE OF SENDING VALUE TO SLIDER WIDGET ---
    // Mengirim nilai persentase 0-100 (misalnya dari sensor Potensiometer / Water Level)
    // Sending percentage value 0-100 (e.g., from Potentiometer / Water Level sensor)
    // int nilaiAnalog = analogRead(A0); // Contoh jika membaca sensor nyata di pin A0 / Example if reading real sensor on pin A0
    // int persentase = map(nilaiAnalog, 0, 1023, 0, 100); // Konversi pembacaan ke 0-100 / Convert reading to 0-100
    int persentaseSlider = random(0, 101); // Simulasi angka acak 0 - 100 / Simulate random number 0 - 100
    iotDevice.sendSensorData("iotera/sensor/potensiometer", String(persentaseSlider));
  }
}