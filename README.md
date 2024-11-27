# ESP32 Space Invader Duel 🚀🎮

## 🇬🇧 English 🇬🇧

**ESP32 Space Invader** is a 2-player arcade game project based on the **ESP32** microcontroller, developed for a school assignment. The game adopts the classic Space Invader mechanic with a competitive twist where two players can compete for the highest score.

---

### 📜 Project Description

This game is designed using:

- **ESP32 microcontroller**: The brain of this project, controls all game logic.
- **DFPlayerMini**: For sound effects that bring the game experience to life.
- **OLED Display I2C**: Displays the game in a simple yet attractive graphical format.
- **3 Button Input**: Left direction, right direction, and a button to shoot.

---

### 🎯 Main Objective

1. Implement basic Space Invader game mechanics (left-right movement, shooting, enemy falling).
2. Provide competitive score-based multiplayer mode.
3. Integrates simple audio to enhance the gaming experience.

---

### 🚧 Project Status

The project is still in the early stages of development and **contains some bugs** that need to be fixed. Nevertheless, the game is already playable to demonstrate its main features.

---

### 📦 Hardware Specifications

A quick reminder that **YOU NEED 2** each of this for each player. Here are the main components used:

- **ESP32 Microcontroller**
- **DFPlayerMini** (audio module)
- **OLED Display** (128x64 resolution)
- **Push Button** (3 pieces for input)
- **Speaker Rubber Mini** (audio output)
- **Breadboard and Jumper Cable**

_A **Resistor 1K Ohm** could be used for RX/TX in DFPlayerMini. But for material efficiency, we doesn't use it._

---

### 🔧 How to Set Up and Run

1. Clone this repository to local:
   ```bash
   git clone https://github.com/V0zmo/space-invader-duel-esp32.git
   cd esp32-space-invader
   ```
   or you can install it trough **ZIP** file.
2. Make sure to change the ESP32 MAC address inside the code to the one you have. Please see the [Resources Material](#-resources-material) for more information.

   ```c
   // uint8_t PlayerMACAddress[] = { 0x88, 0x13, 0xBF, 0x0B, 0x09, 0x40 };  // MAC Address Player 1 (Kabel Speaker Merah / Hitam)
   uint8_t PlayerMACAddress[] = { 0xCC, 0x7B, 0x5C, 0xF0, 0xC4, 0xA4 };  // MAC Address Player 2 (Kabel Speaker Abu-Abu / Cokelat)
   ```

3. Upload the code to the ESP32 using the **Arduino IDE** or another platform.
4. Make sure all hardware is assembled according to the wiring schematics in the `schematics/` directory. **[SOON TO BE ADDED!!!]**
5. Power up the ESP32, and the game is ready to play!

_Since for now we don't have a wiring schematics, please look trough the code or a link to [Resources Material](#-resources-material) reference for the wiring schematics._

---

### 📜 License

This project uses the MIT license. It is free to use, change, and develop further.

---

### 📕 Resources Material

Here are the resources material from the internet, provide useful information of each component does and great learning resources for this project.

- [ESP32 Pinout, Datasheet, Features & Applications](https://www.theengineeringprojects.com/2020/12/esp32-pinout-datasheet-features-applications.html)
- [Installing the ESP32 Board in Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
- [ESP32 OLED Display with Arduino IDE](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)
- [DFPlayer Mini Connection and Diagram](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299#Connection_Diagram)
- [Tutorial Video DFPlayer with ESP32](https://www.youtube.com/watch?v=9w_AaIwlsE4)
- [Tutorial Video Playlist Space Invader with Arduino UNO](https://www.youtube.com/watch?v=Dz9BtsmyHgo&list=PLpyo4J4M9YqL8TxN9orfPCZxZ3JDuNlnG&index=1)
- [ESP32 Save Data Permanently using Preferences Library](https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/)
- [ESP-NOW Two-Way Communication Between ESP32 Boards](https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/)
- [SSD1306 OLED Display: draw images, splash and animations – 2](https://mischianti.org/ssd1306-oled-display-draw-images-splash-and-animations-2/)

---

### 🚀 Future Development

Features planned for development:

- Bug fixes and code optimization.
- Implementation of bosses when reach certain level.
- Addition of more varied sound effects if needed.
- Add team mode if possible.

---

Developed with ❤️ for a school assignment. All contributions and feedback are greatly appreciated!

---

<br>

## 🇮🇩 Indonesia 🇮🇩

**ESP32 Space Invader** adalah proyek arcade game 2 pemain berbasis microcontroller **ESP32**, dikembangkan untuk tugas sekolah. Game ini mengadopsi mekanik klasik Space Invader dengan sentuhan kompetitif di mana dua pemain dapat bersaing untuk mendapatkan skor tertinggi.

---

### 📜 Deskripsi Proyek

Game ini dirancang menggunakan:

- **Microcontroller ESP32**: Otak dari proyek ini, mengontrol semua logika permainan.
- **DFPlayerMini**: Untuk efek suara yang menghidupkan pengalaman bermain.
- **OLED Display**: Menampilkan permainan dalam format grafis sederhana namun menarik.
- **Input 3 Tombol**: Arah kiri, kanan, dan tombol untuk menembak.

---

### 🎯 Tujuan Utama

1. Mengimplementasikan mekanik dasar game Space Invader (gerak kiri-kanan, menembak, musuh jatuh).
2. Menyediakan mode multiplayer berbasis skor kompetitif.
3. Mengintegrasikan audio sederhana untuk meningkatkan pengalaman bermain.

---

### 🚧 Status Proyek

Proyek ini masih dalam tahap pengembangan awal dan **mengandung beberapa bug** yang perlu diperbaiki. Meski demikian, game ini sudah dapat dimainkan untuk mendemonstrasikan fitur utamanya.

---

### 📦 Spesifikasi Hardware

Berikut adalah komponen utama yang digunakan:

- **ESP32 Microcontroller**
- **DFPlayerMini** (modul audio)
- **OLED Display** (128x64 resolusi)
- **Push Button** (3 buah untuk input)
- **Speaker Rubber Mini** (output audio)
- **Breadboard dan Kabel Jumper**

_**Resistor 1K Ohm** dapat digunakan untuk RX/TX di DFPlayerMini. Tetapi untuk efisiensi bahan, kami tidak menggunakannya._

---

### 🔧 Cara Memasang dan Menjalankan

1. Clone repository ini ke lokal:
   ```bash
   git clone https://github.com/V0zmo/space-invader-duel-esp32.git
   cd esp32-space-invader
   ```
   atau anda dapat menginstal langsung melalui file **ZIP**.
2. Pastikan untuk mengubah alamat MAC ESP32 di dalam kode ke alamat yang anda miliki. Silakan lihat halaman [Materi Sumber Daya](#-materi-sumber-daya) untuk informasi lebih lanjut.

   ```c
   // uint8_t PlayerMACAddress[] = { 0x88, 0x13, 0xBF, 0x0B, 0x09, 0x40 };  // MAC Address Player 1 (Kabel Speaker Merah / Hitam)
   uint8_t PlayerMACAddress[] = { 0xCC, 0x7B, 0x5C, 0xF0, 0xC4, 0xA4 };  // MAC Address Player 2 (Kabel Speaker Abu-Abu / Cokelat)
   ```

3. Unggah kode ke ESP32 menggunakan **Arduino IDE** atau platform lain..
4. Pastikan semua hardware sudah dirakit sesuai skema wiring di direktori `schematics/`. **[AKAN SEGERA DITAMBAHKAN!!!]**
5. Nyalakan ESP32, dan game siap dimainkan!
   _Karena untuk saat ini kami tidak memiliki skema wiring, silakan lihat melalui kode atau tautan di [Materi Sumber Daya](#-materi-sumber-daya) untuk referensi skema wiring._

---

### 📜 Lisensi

Proyek ini menggunakan lisensi MIT. Bebas untuk digunakan, diubah, dan dikembangkan lebih lanjut.

---

### 📕 Materi Sumber Daya

Berikut ini adalah materi sumber daya dari internet, memberikan informasi yang berguna dari setiap komponen yang ada dan sumber belajar yang bagus untuk proyek ini.

- [ESP32 Pinout, Datasheet, Features & Applications](https://www.theengineeringprojects.com/2020/12/esp32-pinout-datasheet-features-applications.html)
- [Installing the ESP32 Board in Arduino IDE](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
- [ESP32 OLED Display with Arduino IDE](https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/)
- [DFPlayer Mini Connection and Diagram](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299#Connection_Diagram)
- [Tutorial Video DFPlayer with ESP32](https://www.youtube.com/watch?v=9w_AaIwlsE4)
- [Tutorial Video Playlist Space Invader with Arduino UNO](https://www.youtube.com/watch?v=Dz9BtsmyHgo&list=PLpyo4J4M9YqL8TxN9orfPCZxZ3JDuNlnG&index=1)
- [ESP32 Save Data Permanently using Preferences Library](https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/)
- [ESP-NOW Two-Way Communication Between ESP32 Boards](https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/)
- [SSD1306 OLED Display: draw images, splash and animations – 2](https://mischianti.org/ssd1306-oled-display-draw-images-splash-and-animations-2/)

---

### 🚀 Pengembangan di Masa Depan

Fitur yang direncanakan untuk dikembangkan:

- Perbaikan bug dan pengoptimalan kode.
- Implementasi bos ketika mencapai level tertentu.
- Penambahan efek suara yang lebih bervariasi jika diperlukan.
- Menambahkan mode tim jika memungkinkan.

---

Dikembangkan dengan ❤️ untuk tugas sekolah. Semua kontribusi dan masukan sangat dihargai!

---
