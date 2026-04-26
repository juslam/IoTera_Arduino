# 🌐 IoTera Arduino Library

![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=flat-square&logo=Arduino&logoColor=white)
![C++](https://img.shields.io/badge/-C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
![Firebase](https://img.shields.io/badge/-Firebase-FFCA28?style=flat-square&logo=firebase&logoColor=black)

**IoTera** adalah library Arduino (C++) untuk mempermudah koneksi perangkat ESP8266 dan ESP32 ke aplikasi seluler IoTera secara *real-time* menggunakan infrastruktur Firebase Realtime Database.

*(**IoTera** is an Arduino C++ library to simplify connecting ESP8266/ESP32 devices to the IoTera mobile app in real-time using Firebase Realtime Database infrastructure.)*

---

## ✨ Fitur Utama / Key Features

- 🔌 **Mudah Digunakan (Easy to Use):** API yang sangat sederhana untuk inisialisasi WiFi dan Firebase.
- ⚡ **Respon Cepat (Low Latency Stream):** Menerima perintah aksi (tombol/slider) dari aplikasi mobile IoTera secara instan.
- 📡 **Pengiriman Data (Send Data):** Kirim nilai sensor ke dashboard aplikasi secara periodik dengan mudah.
- 🔄 **Auto-Reconnect:** Terintegrasi dengan penanganan stream Firebase yang tangguh saat koneksi terputus.

---

## 🚀 Cara Instalasi / Installation

### 1. Persiapan Dependensi (Dependencies)
Library IoTera menggunakan arsitektur *Async* terbaru dari Firebase. Pastikan Anda menginstal dependensi berikut melalui **Arduino IDE > Library Manager**:
- `FirebaseClient` (oleh mobizt) 👉 **PENTING: Gunakan versi `1.1.8`**.
- `FirebaseJson` (oleh mobizt).

*Catatan: Kami sangat merekomendasikan menggunakan **Core ESP32 versi 2.0.17** di Boards Manager untuk stabilitas kompilasi SSL/TLS.*

### 2. Instalasi IoTera
1. Di Arduino IDE, buka menu **Sketch** -> **Include Library** -> **Manage Libraries...**
2. Cari **IoTera** dan klik Install. 
*(Atau unduh repositori ini sebagai `.zip` dan tambahkan melalui **Add .ZIP Library...**)*

---

## 💻 Penggunaan Dasar / Basic Usage

Berikut adalah struktur dasar untuk menghubungkan perangkat Anda ke platform IoTera.
*(Here is the basic structure to connect your device to the IoTera platform.)*

```cpp
#include <IoTeraDevice.h>

#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS "WIFI_PASSWORD"
#define DEVICE_KEY "API_KEY_FROM_MOBILE_APP" // Salin dari menu "Kelola Perangkat & API"

IoTeraDevice iotDevice;
unsigned long previousMillis = 0;

// Fungsi penerima perintah dari Aplikasi Mobile
void onAppCommand(String topic, String payload) {
  Serial.println("Topic: " + topic + " | Payload: " + payload);
  
  // Contoh jika HP mengirim perintah ke pin_D1
  if (topic.indexOf("pin_D1") != -1) {
    if (payload == "1") {
      Serial.println("Menyalakan Relay/LED!");
    } else if (payload == "0") {
      Serial.println("Mematikan Relay/LED!");
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

## ️ Lisensi / License
Didistribusikan di bawah lisensi MIT. Lihat `LICENSE` untuk informasi lebih lanjut.

*(Distributed under the MIT License. See `LICENSE` for more information.)*

---
