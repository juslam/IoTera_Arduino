# 🌐 IoTera Ecosystem

![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=flat-square&logo=Arduino&logoColor=white)
![Flutter](https://img.shields.io/badge/-Flutter-02569B?style=flat-square&logo=Flutter&logoColor=white)
![Firebase](https://img.shields.io/badge/-Firebase-FFCA28?style=flat-square&logo=firebase&logoColor=black)
![Node.js](https://img.shields.io/badge/-Node.js-339933?style=flat-square&logo=nodedotjs&logoColor=white)

**IoTera** adalah ekosistem Internet of Things (IoT) *open-source* yang menyediakan solusi menyeluruh dari perangkat keras hingga aplikasi mobile. Proyek ini mempermudah koneksi perangkat ESP8266/ESP32 ke aplikasi seluler (Flutter) secara *real-time* menggunakan infrastruktur Firebase Realtime Database dan MQTT.

*(**IoTera** is a complete open-source Internet of Things (IoT) ecosystem providing an end-to-end solution from hardware to mobile app. It simplifies connecting ESP8266/ESP32 devices to a mobile app (Flutter) in real-time using Firebase Realtime Database and MQTT infrastructure.)*

---

## ✨ Fitur Utama / Key Features

- 📱 **Dashboard Dinamis (Dynamic Dashboard):** Tambahkan, edit, dan atur tata letak widget (Tombol, Slider, Teks Sensor, Color Picker) langsung dari HP tanpa perlu mengubah kode UI/UX (Hardcode).
- ⚡ **Real-time Control & Monitoring:** Kontrol perangkat dan pantau sensor dengan latensi rendah (Low Latency).
- 👥 **Sistem Role Master & Client:** Bagikan akses alat secara aman ke pengguna lain menggunakan fitur pemindai QR Code. Client dapat diberi batasan akses (Read-Only).
- ⏰ **Penjadwalan (Smart Scheduler):** Atur jadwal nyala/mati otomatis untuk hari dan jam tertentu (didukung oleh Node.js backend).
- 📈 **Grafik Riwayat & Ekspor (Chart & CSV Export):** Simpan dan lihat riwayat data sensor, lalu ekspor ke file `.csv`.
- 🔔 **Push Notifications:** Dapatkan peringatan (FCM) seketika jika nilai sensor melampaui batas yang Anda tentukan (*threshold*).

---

## 📁 Struktur Repositori / Repository Structure

| Direktori / Directory | Deskripsi / Description |
| --- | --- |
| `/Library/IoTera` | Library C++ Arduino untuk ESP8266/ESP32. |
| `/AppMobile` | Source Code Aplikasi Android & iOS berbasis Flutter. |
| `/Server` | Script Backend Node.js (untuk Cron Job penjadwalan & Notifikasi FCM). |

---

## 🚀 Cara Instalasi Library Arduino / Arduino Library Setup

### 1. Persiapan Dependensi (Dependencies)
Library IoTera menggunakan arsitektur *Async* terbaru dari Firebase. Pastikan Anda menginstal dependensi berikut melalui **Arduino IDE > Library Manager**:
- `FirebaseClient` (oleh mobizt) 👉 **PENTING: Gunakan versi `1.1.8`**.
- `FirebaseJson` (oleh mobizt).

*Catatan: Kami sangat merekomendasikan menggunakan **Core ESP32 versi 2.0.17** di Boards Manager untuk stabilitas kompilasi SSL/TLS.*

### 2. Instalasi IoTera
1. Unduh folder `/Library/IoTera` dan jadikan file `.zip`.
2. Di Arduino IDE, buka menu **Sketch** -> **Include Library** -> **Add .ZIP Library...** lalu pilih file yang baru saja diunduh.

---

## 💻 Penggunaan Dasar / Basic Usage

Berikut adalah struktur dasar untuk menghubungkan perangkat Anda ke platform IoTera.
*(Here is the basic structure to connect your device to the IoTera platform.)*

```cpp
#include <IoTeraDevice.h>

#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASSWORD"
#define DEVICE_KEY "SESSION_ID_FROM_MOBILE_APP"

IoTeraDevice iotDevice;
unsigned long previousMillis = 0;

// Fungsi penerima perintah dari Aplikasi Mobile
void onAppCommand(String topic, String payload) {
  Serial.println("Topic: " + topic + " | Payload: " + payload);
  
  // Contoh jika HP mengirim perintah ke pin_D1
  if (topic.indexOf("pin_D1") != -1) {
    if (payload == "TOGGLE") {
      Serial.println("Mengubah status Relay/LED!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Daftarkan fungsi callback
  iotDevice.setCommandCallback(onAppCommand);
  
  // Hubungkan ke WiFi dan Firebase
  iotDevice.begin(WIFI_SSID, WIFI_PASS, DEVICE_KEY);
}

void loop() {
  // Auto-reconnect WiFi jika terputus
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(2000);
    return;
  }

  // Wajib dipanggil untuk menjaga aliran data masuk (Stream)
  iotDevice.loop();
  
  // Contoh pengiriman data sensor setiap 10 detik
  if (millis() - previousMillis > 10000) {
    previousMillis = millis();
    float suhu = random(20, 35);
    iotDevice.sendSensorData("iotera/sensor/suhu", String(suhu));
  }
}
```

---

## 📱 Aplikasi Mobile & Server / Mobile App & Server Setup

### Flutter App
1. Masuk ke direktori `AppMobile`.
2. Jalankan perintah `flutter pub get`.
3. Ganti konfigurasi di `lib/firebase_options.dart` dengan project Firebase milik Anda sendiri.
4. Build ke perangkat dengan perintah `flutter run`.

### Node.js Backend (Opsional, untuk Fitur Jadwal & Notifikasi)
1. Masuk ke direktori `Server`.
2. Jalankan `npm install` untuk mengunduh modul `mqtt`, `firebase-admin`, dan `node-cron`.
3. Letakkan file `serviceAccountKey.json` Firebase Anda di direktori yang sama.
4. Jalankan server dengan `node server.js` (Bisa di-hosting di VPS, Render, atau Railway).

---

## 🛡️ Lisensi / License
Didistribusikan di bawah lisensi MIT. Lihat `LICENSE` untuk informasi lebih lanjut.

*(Distributed under the MIT License. See `LICENSE` for more information.)*

---

**IoTera Project** - *Control everything from anywhere.*