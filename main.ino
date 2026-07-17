#include <Wire.h>
#include <BleMouse.h>
#include <MPU6500_WE.h>

#define MPU6500_ADDR 0x68  // Alamat I2C default MPU6500 (bisa juga 0x69)

// Inisialisasi Objek
MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);
BleMouse bleMouse("ESP32 Air Mouse", "Proyek IoT", 100);
const int PIN_LEFT_CLICK = 32;
const int PIN_RIGHT_CLICK = 33;
// Variabel untuk melacak status tahanan tombol
bool isLeftPressed = false;

void setup() {
  Serial.begin(115200);

  // Mengaktifkan resistor pull-up internal pada pin tombol
  pinMode(PIN_LEFT_CLICK, INPUT_PULLUP);
  pinMode(PIN_RIGHT_CLICK, INPUT_PULLUP);

  Wire.begin();

  Serial.println("Mencari MPU6500...");
  if (!myMPU6500.init()) {
    Serial.println("MPU6500 tidak terdeteksi! Cek kabel.");
    while (1);  // Berhenti di sini jika sensor tidak ada
  }
  Serial.println("MPU6500 ditemukan!");

  // Kalibrasi awal (Pastikan sensor diam saat ESP32 dinyalakan)
  Serial.println("Melakukan kalibrasi... JANGAN GERAKKAN SENSOR!");
  delay(2000);
  myMPU6500.autoOffsets();

  // Mengatur Digital Low Pass Filter untuk mengurangi getaran/noise
  myMPU6500.enableGyrDLPF();
  myMPU6500.setGyrDLPF(MPU6500_DLPF_6);

  // Memulai BLE Mouse (Cukup dipanggil 1 kali)
  Serial.println("Memulai Bluetooth Mouse...");
  bleMouse.begin();
  Serial.println("Bluetooth siap! Silakan pairing di Laptop Anda.");
}

void loop() {
  // HANYA mengeksekusi perintah jika terhubung ke laptop
  if (bleMouse.isConnected()) {
    xyzFloat gyr = myMPU6500.getGyrValues();

    // Konversi nilai gyroscope menjadi pergerakan kursor (vx, vy)
    int vx = -gyr.z / 7;  // Menggunakan rotasi sumbu Z untuk geser Kanan-Kiri
    int vy = gyr.x / 7;   // Menggunakan rotasi sumbu X untuk geser Atas-Bawah

    // Deadzone: Mencegah kursor bergerak/bergetar sendiri saat tangan diam
    if (abs(vx) < 5) vx = 0;
    if (abs(vy) < 5) vy = 0;

    // Pindahkan kursor jika ada gerakan
    if (vx != 0 || vy != 0) {
      bleMouse.move(vx, vy);
    }

    // Delay agar pergerakan kursor lebih mulus dan tidak terlalu cepat
    delay(15);
    
   // ----- LOGIKA KLIK KIRI (DRAG & DROP) -----
    if (digitalRead(PIN_LEFT_CLICK) == LOW) { 
      // Jika tombol ditekan SECARA FISIK
      
      if (!isLeftPressed) { 
        // Hanya eksekusi jika status SEBELUMNYA adalah belum ditekan (Edge Detection)
        bleMouse.press(MOUSE_LEFT);  // Kirim sinyal TAHAN KLIK ke laptop
        isLeftPressed = true;        // Perbarui memori status
        delay(50);                   // Debouncing mekanis
      }
    } 
    else { 
      // Jika tombol dilepas SECARA FISIK
      
      if (isLeftPressed) { 
        // Hanya eksekusi jika status SEBELUMNYA sedang ditahan
        bleMouse.release(MOUSE_LEFT); // Kirim sinyal LEPAS KLIK ke laptop
        isLeftPressed = false;        // Perbarui memori status
        delay(50);                    // Debouncing mekanis
      }
    }

    // ----- LOGIKA KLIK KANAN -----
    if (digitalRead(PIN_RIGHT_CLICK) == LOW) {
      bleMouse.click(MOUSE_RIGHT);
      delay(200); // Sederhana debouncing
    }
    
  } // Kurung tutup untuk isConnected dipindah ke akhir
}
