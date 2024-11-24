/*
TODO LIST
1. Fix Audio BGM_01 not looping in MainMenu()
2. Fix Audio INVADER_MOVE_SFX overriding other sfx
3. Fix Audio BULLET_ATTACK_COLLISION_SFX not playing audio (Check SD Card)
4. Add ESP-NOW to both ESP32 [DONED!!! ( •̀ ω •́ )✧]
5. Sharing score to duel and compare to both ESP32
6. Each player advancing to their own stage and level
7. If 1 player is die. Lose -100 point
8. The winner based on the high score
9. For multiplayer add timer between 1-10 minutes
10. Calculation of the winner is HighScore + (Stage * 10)
*/

/* 
PASTIKAN INSTALL LIBRARY DIBAWAH INI!!!
Tools -> Manage Libraries... atau Ctrl +Shift+I

EspSoftwareSerial (Dirk Kaar, Peter Lerup) untuk SoftwareSerial.h
DFRobotDFPlayerMini (DFRobot) untuk DFRobotDFPlayerMini.h
Adafruit SSD1306 (Adafruit) untuk Adafruit_SSD1306.h
Adafruit GFX Library (Adafruit) untuk Adafruit_GFX.h

Untuk library yang tidak disebut diatas, adalah library bawaan ESP32.
*/

// LIBRARY
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// KONSTANT VARIABLE

// BAGIAN DISPLAY
#define OLED_RESET -1      // Setel ulang pin # (atau -1 jika berbagi pin setel ulang Arduino)
#define OLED_ADDRESS 0x3C  // Alamat OLED
#define SCREEN_WIDTH 128   // Lebar tampilan OLED, dalam piksel
#define SCREEN_HEIGHT 64   // Tinggi tampilan OLED, dalam piksel

/*

UNTUK DIRIKU DIMASA DEPAN HARAP MAINKAN GAMENYA LAGI TERUS PERHATIKAN BAGIAN
AUDIO. KAMU DITUGASKAN UNTUK MEMPERBAIKI AUDIO! KERJAKAN PADA HARI SENIN
KARENA MINGGU ADALAH WAKTU ISTIRAHATMU!

JANGAN DIPERBUDAK!!!

*/


// BAGIAN AUDIO
#define AUDIO_VOLUME 25                // Besar suara audio
#define BGM_01 1                       // BGM 1
#define PLAYER_SHOOT_SFX 2             // Audio pemain menembak
#define PLAYER_DIE_SFX 3               // Audio pemain mati
#define INVADER_MOVE_SFX 4             // Audio Invader jalan
#define INVADER_SHOOT_SFX 5            // Audio Invader menembak
#define INVADER_DIE_SFX 6              // Audio Invader mati
#define MOTHERSHIP_SFX 7               // Audio Mothership
#define MOTHERSHIP_DIE_SFX 8           // Audio Mothership meledak
#define BULLET_ATTACK_COLLISION_SFX 9  // Peluru dan serangan bentrok

// BAGIAN TOMBOL
#define SHOOT_BUTTON 12  // Pin tombol 1
#define LEFT_BUTTON 13   // Pin tombol 2
#define RIGHT_BUTTON 14  // Pin tombol 3

// BAGIAN GLOBAL GAME
#define ACTIVE 0              // Status konstanta aktif objek permainan
#define EXPLODING 1           // Status konstanta meledak objek permainan
#define DESTROYED 2           // Status konstanta hancur objek permainan
#define EXPLOSION_GFX_TIME 7  // Durasi berapa lama EXPLOSION_GFX berada dalam layar

// BAGIAN PLAYER
#define PLAYER_WIDTH 8          // Lebar player
#define PLAYER_HEIGHT 8         // Tinggi player
#define PLAYER_X_MOVE_AMOUNT 2  // Kecepatan jalan player dalam pixel
#define PLAYER_X_START 0        // Lokasi mulai player dalam koordinat X
#define PLAYER_Y_START 56       // Lokasi mulai player dalam koordinat Y
#define PLAYER_LIVES 3          // Nyawa pemain
#define BULLET_WIDTH 2          // Lebar peluru
#define BULLET_HEIGHT 7         // Panjang peluru
#define BULLET_SPEED 4          // Kecepatana peluru (semakin besar semakin cepat)
#define SHOOT_COOLDOWN 15       // Cooldown serangan pemain

// BAGIAN MOTHERSHIP
#define MOTHERSHIP_WIDTH 8                // Panjang Mothership
#define MOTHERSHIP_HEIGHT 8               // Tinggi Mothership
#define MOTHERSHIP_SPEED 2                // Kecepatan Mothership dalam pixel
#define MOTHERSHIP_SPAWN_CHANCE 250       // Nilai lebih tinggi, kemungkinan muncul lebih kecil
#define DISPLAY_MOTHERSHIP_BONUS_TIME 25  // Berapa lama bonus tetap berada di layar untuk menampilkan Mothership

// BAGIAN INVADER
#define NUM_INVADER_COLUMNS 7            // Jumlah invader lurus ke kanan
#define NUM_INVADER_ROWS 3               // Jumlah invader lurus ke bawah
#define SPACE_BETWEEN_INVADER_COLUMNS 5  // Jarak antara invader dari kanan
#define SPACE_BETWEEN_INVADER_ROWS 16    // Jarak antara invader dari bawah
#define INVADER_WIDTH 8                  // Ukuran lebar terbesar invader
#define INVADER_HEIGHT 8                 // Ukuran lebar terbesar invader
#define X_START_OFFSET 6                 // Offset X lokasi invader
#define AMOUNT_TO_DROP_PER_LEVEL 4       // Seberapa jauh Invader turun tiap level baru
#define INVADERS_DROP 4                  // Seberapa jauh invader jatuh dalam pixel
#define INVADERS_SPEED 12                // Kecepatan invader (semakin rendah semakin cepat)
#define LEVEL_RESET_TO_START_HEIGHT 4    // Setiap kelipatan dari tingkat ini, posisi awal y akan diatur ulang ke atas
#define INVADER_X_MOVE_AMOUNT 1          // Jumlah pixel tiap mulai ronde baru
#define CHANCE_ATTACK 20                 // Semakin besar semakin kecil persentase Invader menyerang pemain
#define ATTACK_WIDTH 4                   // Lebar serangan Invader
#define ATTACK_HEIGHT 8                  // Tinggi serangan Invader
#define MAX_ATTACK 3                     // Jumlah maksimum bom yang diizinkan untuk dijatuhkan dalam satu waktu

// STRUKTUR GAME

// Struktur game global, Objek dasar yang akan disertakan oleh sebagian besar objek lain
struct GameObjectStruct {
  signed int X;          // Lokasi X
  signed int Y;          // Lokasi Y
  unsigned char Status;  // 0 Active | 1 Exploding | 2 Destroyed
};

// Struktur untuk Player
struct PlayerStruct {
  GameObjectStruct Ord;               // Inisiasi class GameObjectStruct
  unsigned int Score;                 // Skor untuk masing-masing player (Saat Multiplayer)
  unsigned int Lives;                 // Nyawa player
  unsigned char Level;                // Level stage pemain yang diraih
  unsigned char KillCount;            // Invader yang dibunuh
  unsigned char InvaderSpeed;         // Semakin tinggi semakin lambat, di kalkulasi saat Invader dibunuh
  unsigned char ExplosionGfxCounter;  // Variable untuk menentukan berapa lama efek ledakan berlangsung
  unsigned char ShootCooldown;        // Cooldown serang
};

// Struktur untuk Invader
struct InvaderStruct {
  GameObjectStruct Ord;               // Inisiasi class GameObjectStruct
  unsigned char ExplosionGfxCounter;  // Variable untuk menentukan berapa lama efek ledakan berlangsung
};

// GLOBAL VARIABLE

// Global audio
static const uint8_t PIN_MP3_RX = 27;  // Menghubungkan ke TX modul
static const uint8_t PIN_MP3_TX = 26;  // Menghubungkan ke RX modul
bool ImportantAudioPlayed = false;     // Cek apabila audio penting sedang jalan (BGM dan Mothership)

// Global Game
unsigned int HighScore;      // Skor tertinggi dalam game secara global
bool GameInPlay = false;     // Apakah game sedang dimainkan
bool MenuSelection = false;  // Apakah dalam mode menu pilihan

// Global Mothership
signed char MothershipSpeed;           // Kecepatan Mothership dalam pixel yang dapat diubah
unsigned int MothershipBonus;          // Bonus pada Mothership
signed int MothershipBonusXPos;        // Lokasi koordinat X pada Mothership bonus
unsigned char MothershipBonusCounter;  // Berapa banyak ketemu Mothership bonus

// Global Invader
static const int TOTAL_INVADER = NUM_INVADER_COLUMNS * NUM_INVADER_ROWS;  // Total semua Invader
signed char InvaderXMoveAmount = 2;                                       // Kecepatan jalan invaders dalam pixel
signed char InvadersMoveCounter;                                          // menghitung mundur, ketika 0 memindahkan invader, atur sesuai dengan berapa banyak alien di layar (Tersambung tidak langsung dengan INVADERS_SPEED)
bool InvaderFrame = false;                                                // false = Diam | true = Jalan

// Inisiasi Library dan Struktur Game

// System
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Deklarasi untuk layar SSD1306 yang tersambung ke I2C (SDA, pin SCL)
DFRobotDFPlayerMini mpPlayer;                                              // Membuat MP Player
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);                     // Buat SoftwareSerial dengan pin TX/RX
Preferences preferences;                                                   // Inisiasi class preferences dari library <Preferences.h>
// Game global variable
GameObjectStruct Bullet;                                       // Inisiasi class Bullet
PlayerStruct Player;                                           // Player global variable
InvaderStruct Invader[NUM_INVADER_COLUMNS][NUM_INVADER_ROWS];  // Buat Invader dengan multidimension array (seperti tabel)
InvaderStruct Mothership;                                      // Buat Mothership
GameObjectStruct InvaderAttack[MAX_ATTACK];                    // Buat objek serangan musuh



// MODE MULTIPLAYER

// UBAH SESUAI DENGAN MAC ADDRESS ESP32 MASING-MASING!!!
// PILIH SATU!

// uint8_t PlayerMACAddress[] = { 0x88, 0x13, 0xBF, 0x0B, 0x09, 0x40 };  // MAC Address Player 1 (Kabel Speaker Merah / Hitam)
uint8_t PlayerMACAddress[] = { 0xCC, 0x7B, 0x5C, 0xF0, 0xC4, 0xA4 };  // MAC Address Player 2 (Kabel Speaker Abu-Abu / Cokelat)
bool Multiplayer = false;                                             // Variabel untuk menyimpan mode permainan
unsigned long StartTime;                                              // Waktu mulai permainan
const int GameDuration = 5 * 60 * 1000;                               // Durasi permainan: 5 menit (dalam milidetik)

PlayerStruct Opponent;  // Player musuh global variable

// Graphics Player
static const unsigned char PLAYER_GFX[] = {
  0x18,
  0x18,
  0x18,
  0x3c,
  0xbd,
  0xbd,
  0xff,
  0xdb
};

// Graphics Bullet 1
static const unsigned char PLAYER_BULLET[] = {
  0xc0,
  0xc0,
  0xc0,
  0x00,
  0xc0,
  0xc0,
  0xc0
};

// Graphics Bullet 2
static const unsigned char INVADER_BULLET[] = {
  0x90,
  0x60,
  0x00,
  0x90,
  0x60,
  0x00,
  0x90,
  0x60,
};

// Graphics Musuh 1 (Diam)
static const unsigned char INVADER_1_GFX_01[] = {
  0x24,
  0xff,
  0x5a,
  0xff,
  0xdb,
  0xc3,
  0x66,
  0x24
};

// Graphics Musuh 1 (Jalan)
static const unsigned char INVADER_1_GFX_02[] = {
  0x42,
  0xff,
  0x5a,
  0xff,
  0xdb,
  0xc3,
  0xc3,
  0x42
};

// Graphics Musuh 2 (Diam)
static const unsigned char INVADER_2_GFX_01[] = {
  0x3c,
  0x7e,
  0xe7,
  0xc3,
  0xe7,
  0xbd,
  0x7e,
  0x5a
};

// Graphics Musuh 2 (Jalan)
static const unsigned char INVADER_2_GFX_02[] = {
  0xbd,
  0xff,
  0xe7,
  0xc3,
  0x66,
  0x3c,
  0x7e,
  0x99
};

// Graphics Musuh 3 (Diam)
static const unsigned char INVADER_3_GFX_01[] = {
  0xc3,
  0x7e,
  0xff,
  0x99,
  0xff,
  0xff,
  0xa5,
  0xbd
};

// Graphics Musuh 3 (Jalan)
static const unsigned char INVADER_3_GFX_02[] = {
  0x66,
  0x7e,
  0xff,
  0xbd,
  0x99,
  0xff,
  0xa5,
  0x99
};

// Graphics Mothership
static const unsigned char MOTHERSHIP_GFX[] = {
  0x18,
  0x3c,
  0x3c,
  0xff,
  0xff,
  0xff,
  0x7e,
  0x3c
};

// Graphics ledakan
static const unsigned char EXPLOSION_GFX[] = {
  0x85,
  0x6a,
  0x9a,
  0x7d,
  0xbe,
  0x7c,
  0x52,
  0x8a
};

// Fungsi untuk menerima data dari ESP32 lain
void onDataReceive(const uint8_t *macAddr, const uint8_t *data, int dataLen) {
  memcpy(&Opponent, data, sizeof(Opponent));                                                                  // Menyalin data yang diterima ke 'Opponent'
  Serial.println("Data Diterima:");                                                                           // Menampilkan pesan bahwa data diterima
  Serial.printf("Skor Musuh: %d | Level: %d | Nyawa: %d\n", Opponent.Score, Opponent.Level, Opponent.Lives);  // Menampilkan data lawan
}

// Fungsi untuk mengonfirmasi pengiriman data
void onDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Data dikirim!" : "Data gagal dikirim!");  // Mengecek apakah data berhasil dikirim atau gagal
}

// Fungsi inisialisasi utama program
void setup() {

  // Inisiasi untuk perangkat keras

  // JANGAN UBAH BAUD 9600 DI "softwareSerial.begin()" NTAR MP3 NGAMBEK ＞﹏＜
  Serial.begin(115200);        // Init port serial USB untuk debugging
  softwareSerial.begin(9600);  // Init port serial untuk DFPlayer Mini

  pinMode(SHOOT_BUTTON, INPUT_PULLUP);  // Atur mode pin tombol 1 jadi INPUT_PULLUP
  pinMode(LEFT_BUTTON, INPUT_PULLUP);   // Atur mode pin tombol 2 jadi INPUT_PULLUP
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);  // Atur mode pin tombol 3 jadi INPUT_PULLUP

  // Cek jika Display monitor tidak dapat tersambung
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));  // Tampilkan error dalam Serial Monitor
    for (;;)                                         // Agar tidak looping
      ;
  }

  // Cek jika MP Player tidak dapat tersambung
  if (!mpPlayer.begin(softwareSerial)) {
    Serial.println("Connecting to DFPlayer Mini failed!");  // Tampilkan error dalam Serial Monitor
  }

  preferences.begin("storage", false);             // Buat penyimpanan atau buka bernama "storage" dengan mode Read & Write
  HighScore = preferences.getInt("HighScore", 0);  // Baca data "HighScore"
  preferences.end();                               // Tutup penyimpanan saat tidak diperlukan

  // Inisiasi untuk permainan

  display.setTextSize(1);       // Atur ukuran teks
  display.setTextColor(WHITE);  // Atur warna teks

  mpPlayer.volume(AUDIO_VOLUME);  // Atur besar suara audio
  mpPlayer.play(BGM_01);          // Mainkan BGM pas awal main

  InitInvaders(0);  // Manggil fungsi penciptaan Invader
  InitPlayer();     // Manggil fungsi penciptaan Player
}

// Fungsi perulangan utama program
void loop() {
  if (GameInPlay && !Multiplayer)  // Jika game sedang berlangsung dan offline
  {
    Update();                            // Perulangan logika game
    Draw();                              // Perulangan pergambaran game
  } else if (GameInPlay && Multiplayer)  // Jika game sedang berlangsung dan offline
  {
    MultiplayerMode();       // Mengaktifkan fitur online
  } else if (MenuSelection)  // Jika dalam mode seleksi
  {
    ModeMenu();  // Fungsi untuk pilihan mode menu
  } else         // Jika game tidak berlangsung
  {
    MenuScreen();  // Fungsi layar menu
  }
}

// Fungsi Update untuk semua yang berhubungan dengan logika atau perhitungan dalam kodingan
void Update() {
  if (Player.Ord.Status == ACTIVE)  // Jika pemain masih hidup
  {
    InvaderControlUpdate();     // Fungsi logika Invader
    MothershipControlUpdate();  //Fungsi logika Mothership
    PlayerControlUpdate();      // Fungsi logika Player
    BulletControlUpdate();      // Fungsi logika Bullet
    CheckCollisionsUpdate();    // Fungsi logika pengecekan kolisi
  }
}

// Fungsi Draw untuk semua urusan yang berhubungan gambar
void Draw() {
  int i;  // Index pengulangan

  display.clearDisplay();  // Menghilangkan semua gambar display (Prosesor lambat jangan dicoba! ( •̀ ω •́ )✧)

  // GAMBAR PLAYER
  if (Player.Ord.Status == ACTIVE)  // Jika status pemain "ACTIVE"
  {
    display.drawBitmap(Player.Ord.X, Player.Ord.Y, PLAYER_GFX, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);  // Gambar pesawat Player
  } else if (Player.Ord.Status == EXPLODING)                                                         // Jika status pemain "EXPLODING"
  {
    for (i = 0; i < PLAYER_WIDTH; i += 2)  // Lopping index jika dibawah lebar pemain
    {
      display.drawBitmap(Player.Ord.X + i, Player.Ord.Y, EXPLOSION_GFX, random(4) + 2, 8, WHITE);  // Gambar ledakan
    }
    Player.ExplosionGfxCounter--;         // Kurangi counter "ExplosionGfxCounter"
    if (Player.ExplosionGfxCounter == 0)  // Jika counter telah habis
    {
      Player.Ord.Status = DESTROYED;  // Ubah status pemain menjadi "DESTROYED"
      LoseLife();                     // Panggil fungsi "LoseLife()"
    }
  }

  // GAMBAR INVADER
  for (int across = 0; across < NUM_INVADER_COLUMNS; across++)  // Looping dan cek untuk baris ke kanan
  {
    for (int down = 0; down < NUM_INVADER_ROWS; down++)  // Looping dan cek untuk baris ke bawah
    {
      if (Invader[across][down].Ord.Status == ACTIVE)  // Jika Invader memiliki status "ACTIVE"
      {
        switch (down)  // Check nomor dari variable "down" dan gambar sesuai baris kebawah musuh
        {
          case 0:               // Jika 0 (atas)
            if (!InvaderFrame)  // Jika InvaderFrame = false
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_1_GFX_01, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 1 (Diam)
            } else                                                                                                                                   // Jika InvaderFrame = true
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_1_GFX_02, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 1 (Jalan)
            }
            break;              // Lepas dari switch statement
          case 1:               // Jika 1 (tengah)
            if (!InvaderFrame)  // Jika InvaderFrame = false
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_2_GFX_01, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 2 (Diam)
            } else                                                                                                                                   // Jika InvaderFrame = true
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_2_GFX_02, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 2 (Jalan)
            }
            break;              // Lepas dari switch statement
          default:              // Jika bukan keduanya (2 (Bawah))
            if (!InvaderFrame)  // Jika InvaderFrame = false
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_3_GFX_01, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 3 (Diam)
            } else                                                                                                                                   // Jika InvaderFrame = true
            {
              display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, INVADER_3_GFX_02, INVADER_WIDTH, INVADER_HEIGHT, WHITE);  // Gambar Invader 3 (Jalan)
            }
        }
      } else if (Invader[across][down].Ord.Status == EXPLODING)  // Jika status "Invader" adalah "EXPLODING"
      {
        Invader[across][down].ExplosionGfxCounter--;        // Kurangi timer "ExplosionGfxCounter"
        if (Invader[across][down].ExplosionGfxCounter > 0)  // Jika waktu timer belum habis
        {
          display.drawBitmap(Invader[across][down].Ord.X, Invader[across][down].Ord.Y, EXPLOSION_GFX, 8, 8, WHITE);  // Gambar EXPLOSION_GFX
        } else                                                                                                       // Jika waktu habis
        {
          Invader[across][down].Ord.Status = DESTROYED;  // Ubah status menjadi "DESTROYED"
        }
      }
    }
  }

  // GAMBAR PELURU
  if (Bullet.Status == ACTIVE)  // Jika status "Bullet" aktif
  {
    display.drawBitmap(Bullet.X, Bullet.Y, PLAYER_BULLET, BULLET_WIDTH, BULLET_HEIGHT, WHITE);  // Gambar PLAYER_BULLET ke display
  }

  // GAMBAR SERANGAN MUSUH
  for (i = 0; i < MAX_ATTACK; i++)  // Setiap serangan dibawah maksimal serangan
  {
    if (InvaderAttack[i].Status == ACTIVE)  // Jika status serangan "ACTIVE"
    {
      display.drawBitmap(InvaderAttack[i].X, InvaderAttack[i].Y, INVADER_BULLET, ATTACK_WIDTH, ATTACK_HEIGHT, WHITE);  // Gambar "INVADER_BULLET"
    } else                                                                                                             // Jika bukan status "ACTIVE"
    {
      if (InvaderAttack[i].Status == EXPLODING)  // Jika status "EXPLODING"
      {
        display.drawBitmap(InvaderAttack[i].X, InvaderAttack[i].Y, EXPLOSION_GFX, 8, 8, WHITE);  // Gambar "EXPLOSION_GFX"
      }
      InvaderAttack[i].Status = DESTROYED;  // Atur status serangan menjadi "DESTROYED"
    }
  }

  // GAMBAR MOTHERSHIP
  if (Mothership.Ord.Status == ACTIVE)  // Jika Mothership statusnya "ACTIVE"
  {
    display.drawBitmap(Mothership.Ord.X, Mothership.Ord.Y, MOTHERSHIP_GFX, MOTHERSHIP_WIDTH, MOTHERSHIP_HEIGHT, WHITE);  // Gambar MOTHERSHIP_GFX ke display
  } else if (Mothership.Ord.Status == EXPLODING)                                                                         // Jika Mothership statusnya "EXPLODING"
  {
    for (i = 0; i < MOTHERSHIP_WIDTH; i += 2)  // Randomisasi ledakan untuk MOTHERSHIP
    {
      display.drawBitmap(Mothership.Ord.X + i, Mothership.Ord.Y, EXPLOSION_GFX, random(4) + 2, MOTHERSHIP_HEIGHT, WHITE);  // Gambar EXPLOSION_GFX dan disesuaikan dengan variable "i"
    }
    Mothership.ExplosionGfxCounter--;         // Kurangi timer "ExplosionGfxCounter"
    if (Mothership.ExplosionGfxCounter == 0)  // Jika waktu timer habis
    {
      Mothership.Ord.Status = DESTROYED;  // Atur status Mothership menjadi "DESTROYED"
    }
  }

  // GAMBAR BONUS ANGKA MOTHERSHIP
  if (MothershipBonusCounter > 0)  // Cek jika "MothershipBonusCounter" lebih dari 0
  {
    display.setCursor(MothershipBonusXPos, 0);  // Atur lokasi tulis teks pada lokasi X "MothershipBonusXPos"
    display.print(MothershipBonus);             // Tampilkan jumlah bonus point yang didapatkan
    MothershipBonusCounter--;                   // Hitung mundur nilai agar tidak permanen di layar
  } else {
    display.setCursor(0, 0);                 // Atur letak penulisan teks dipojok kiri
    display.print(Player.Score);             // Tulis skor pemain
    display.setCursor(SCREEN_WIDTH - 7, 0);  // Atur letak penulisan teks dipojok kanan
    display.print(Player.Lives);             // Tulis nyawa pemain
  }

  display.display();  // Memunculkan semua gambar display
}

// Inisiasi untuk Player
void InitPlayer() {
  Player.Ord.X = PLAYER_X_START;  // Atur lokasi awal X player
  Player.Ord.Y = PLAYER_Y_START;  // Atur lokasi awal Y player
  Player.Ord.Status = ACTIVE;     // Atur status pemain jadi aktif
  Player.Lives = PLAYER_LIVES;    // Atur nyawa pemain
  Player.Level = 0;               // Atur level yang diraih pemain jadi 0
  Player.Score = 0;               // Atur skor pemain menjadi 0
  Player.ShootCooldown = 0;       // Atur cooldown pemain jadi 0
  Bullet.Status = DESTROYED;      // Atur agar status "Bullet" menjadi "DESTROYED"
}

// Fungsi untuk mengkontrol pemain
void PlayerControlUpdate() {
  if ((digitalRead(RIGHT_BUTTON) == false) && (Player.Ord.X + PLAYER_WIDTH < SCREEN_WIDTH))  // Jika tombol ke kanan ditekan dan badan pesawat lebih kecil dari lebar layar
  {
    Player.Ord.X += PLAYER_X_MOVE_AMOUNT;  // Majukan pemain ke kanan
  }
  if ((digitalRead(LEFT_BUTTON) == false) && (Player.Ord.X > 0))  // Jika tombol ke kiri ditekan dan badan pesawat lebih besar dari 0 (ujung kiri layar)
  {
    Player.Ord.X -= PLAYER_X_MOVE_AMOUNT;  // Majukan pemain ke kiri
  }
  if (Player.ShootCooldown > 0)  // Jika cooldown belum habis
  {
    Player.ShootCooldown--;  // Kurangi cooldown
  }
  if ((digitalRead(SHOOT_BUTTON) == false) && (Bullet.Status != ACTIVE) && (Player.ShootCooldown == 0))  // Jika tombol tembak ditekan dan status peluru tidak aktif
  {
    Player.ShootCooldown = SHOOT_COOLDOWN;         // Terapkan serangan cooldown
    Bullet.X = Player.Ord.X + (PLAYER_WIDTH / 2);  // Atur posisi X peluru sesuai lokasi pemain
    Bullet.Y = PLAYER_Y_START;                     // Atur posisi Y peluru sesuai lokasi Y awal pemain
    Bullet.Status = ACTIVE;                        // Atur kondisi peluru menjadi aktif
    if (!ImportantAudioPlayed)                     // Jika audio penting tidak jalan
    {
      mpPlayer.play(PLAYER_SHOOT_SFX);  // Mainkan audio player menembak
    }
  }
}

// Fungsi untuk mengatur peluru
void BulletControlUpdate() {
  if (Bullet.Status == ACTIVE)  // Jika status peluru aktif
  {
    Bullet.Y -= BULLET_SPEED;          // Jalankan peluru keatas
    if (Bullet.Y + BULLET_HEIGHT < 0)  // Jika peluru melewati layar atas
    {
      Bullet.Status = DESTROYED;  // Ubah status peluru menjadi "DESTROYED" (hancur)
    }
  }
}

// Fungsi InitInvaders dengan argumen int untuk permulaan lokasi Y
void InitInvaders(int YSTART) {
  //BAGIAN INVADER
  for (int across = 0; across < NUM_INVADER_COLUMNS; across++)  // Looping dan cek untuk baris ke kanan
  {
    for (int down = 0; down < NUM_INVADER_ROWS; down++)  // Looping dan cek untuk baris ke bawah
    {
      /* 
      CATATAN DARI XTronical (Tutor)
      
      kita tambahkan "down" untuk memusatkan alien, kebetulan nilai yang tepat yang kita butuhkan per baris!
      kita perlu menyesuaikan sedikit karena baris nol seharusnya 2, baris 1 seharusnya 1 dan baris paling bawah 0
      */

      Invader[across][down].Ord.X = X_START_OFFSET + (across * (INVADER_WIDTH + SPACE_BETWEEN_INVADER_COLUMNS)) - down;  // Melakukan kalkulasi letak koordinat X Invader
      Invader[across][down].Ord.Y = YSTART + (down * SPACE_BETWEEN_INVADER_ROWS);                                        // Melakukan kalkulasi letak koordinat Y Invader
      Invader[across][down].Ord.Status = ACTIVE;                                                                         // Mengaktifkan status Invader
      Invader[across][down].ExplosionGfxCounter = EXPLOSION_GFX_TIME;                                                    // Mengatur waktu Invader apabila ditembak
    }
  }

  // BAGIAN MOTHERSHIP
  Mothership.Ord.X = -MOTHERSHIP_WIDTH;  // Atur lokasi X Mothership sesuai dengan nilai negatif lebar gambar
  Mothership.Ord.Y = 0;                  // Atur lokasi Y Mothership diatas layar
  Mothership.Ord.Status = DESTROYED;     // Atur status menjadi "DESTROYED"
}

// Buat mengontrol jalan invaders
void InvaderControlUpdate() {
  if ((InvadersMoveCounter--) < 0)  // Cek jika jalan invaders (12) dibawah 0
  {
    bool Dropped = false;                                                                                   // buat variable "Dropped" untuk mengatur invaders turun kebawah
    if ((RightMostPos() + InvaderXMoveAmount >= SCREEN_WIDTH) || (LeftMostPos() + InvaderXMoveAmount < 0))  // Cek apakah kiri/kanan invaders menyentuh layar
    {
      InvaderXMoveAmount = -InvaderXMoveAmount;  // Membalikkan arah
      Dropped = true;                            // Variable "Dropped" hidup
    }
    for (int across = 0; across < NUM_INVADER_COLUMNS; across++)  // Looping dan cek untuk baris ke kanan
    {
      for (int down = 0; down < NUM_INVADER_ROWS; down++)  // Looping dan cek untuk baris ke bawah
      {
        if (Invader[across][down].Ord.Status == ACTIVE)  // Cek apakah invader statusnya "ACTIVE"
        {
          if (!Dropped)  // Jika variable "Dropped" = "false"
          {
            Invader[across][down].Ord.X += InvaderXMoveAmount;  // Jalankan invader ke kanan/kiri
          } else                                                // Jika variable "Dropped" = "true"
          {
            Invader[across][down].Ord.Y += INVADERS_DROP;  // Turunkan invader ke bawah
          }
        }
      }
    }
    InvadersMoveCounter = INVADERS_SPEED;                                                                                                  // Reset InvadersMoveCounter dengan INVADERS_SPEED (12)
    InvaderFrame = !InvaderFrame;                                                                                                          // Tukar frame dengan frame lain
    if ((!ImportantAudioPlayed) && (Bullet.Status == DESTROYED) && (Player.Ord.Status == ACTIVE) && (Mothership.Ord.Status == DESTROYED))  // Jika audio Mothership tidak jalan
    {
      mpPlayer.play(INVADER_MOVE_SFX);  // Mainkan audio Invader jalan
    }

    if (random(CHANCE_ATTACK) == 1)  // Cek kemungkinan musuh menyerang
    {
      InvaderAttacking();  // Jalankan fungsi untuk membuat Invader menyerang
    }
    AttackMovement();  // Jalankan fungsi untuk membuat serangan Invader berjalan
  }
}

// Fungsi untuk serangan Invader
void InvaderAttacking() {
  bool FreeSlot = false;                            // Buat variable "FreeSlot" apabila ada slot serangan kosong
  unsigned char ActiveColumn[NUM_INVADER_COLUMNS];  // Kolom yang aktif
  unsigned char AttackIdx = 0;                      // Index serangan Invader

  while ((FreeSlot == false) && (AttackIdx < MAX_ATTACK))  // Jika slot penuh dan serangan yang ada lebih kecil dari maksimal serangan
  {
    if (InvaderAttack[AttackIdx].Status == DESTROYED)  // Jika status serangan yang ditembak Invader "DESTROYED"
    {
      FreeSlot = true;  // Ada slot kosong
    } else              // Jika serangan musuh bukan "DESTOYED"
    {
      AttackIdx++;  // Tambahkan jumlah serangan musuh
    }
  }

  if (FreeSlot)  // Jika ada slot kosong
  {
    unsigned char Columns = 0;        // Buat variable kolom
    unsigned ActiveColumnsCount = 0;  // Buat variable kolom yang aktif
    signed char Row;                  // Variable baris kanan
    unsigned char ChosenColumn;       // Variable kolom yang dipilih

    while (Columns < NUM_INVADER_COLUMNS)  // Jika kolom lebih kecil dari "NUM_INVADER_COLUMNS"
    {
      Row = 2;          // Atur variable "Row" atau baris menjadi 2
      while (Row >= 0)  // Jika variable "Row" lebih besar atau sama dari 0
      {
        if (Invader[Columns][Row].Ord.Status == ACTIVE)  // Jika Invader yang ingin nembak statusnya "ACTIVE"
        {
          ActiveColumn[ActiveColumnsCount] = Columns;  // Buat variable "ActiveColumn[ActiveColumnsCount]" sesuai "Columns"
          ActiveColumnsCount++;                        // Nambahkan satu pada variable "ActiveColumnsCount"
          break;                                       // Keluar dari "while" loop
        }
        Row--;  // Kurangi "Row"
      }
      Columns++;  // Tambahi "Columns"
    }
    ChosenColumn = random(ActiveColumnsCount);  // Kolom yang dipilih adalah randomisasi dari kolom yang aktif
    Row = 2;                                    // Atur "Row" menjadi 2

    while (Row >= 0)  // Loop apabila "Row" lebih besar atau sama dengan 0
    {
      if (Invader[ActiveColumn[ChosenColumn]][Row].Ord.Status == ACTIVE)  // Jika Invader dari kolom yang hidup statusnya "ACTIVE"
      {
        InvaderAttack[AttackIdx].Status = ACTIVE;                                                              // Atur serangan musuh menjadi "ACTIVE"
        InvaderAttack[AttackIdx].X = Invader[ActiveColumn[ChosenColumn]][Row].Ord.X + int(INVADER_WIDTH / 2);  // Atur serangan pada lokasi Invader
        InvaderAttack[AttackIdx].X = (InvaderAttack[AttackIdx].X - 2) + random(0, 4);                          // Menambahkan randomisasi dari lokasi X Invader
        InvaderAttack[AttackIdx].Y = Invader[ActiveColumn[ChosenColumn]][Row].Ord.Y + 4;                       // Majukan serangan musuh kebawah
        if (!ImportantAudioPlayed)                                                                             // Jika audio Mothership tidak jalan
        {
          mpPlayer.play(INVADER_SHOOT_SFX);  // Mainkan audio Invader menembak
        }
        break;  // Keluar dari "while" loop
      }
      Row--;  // Kurangi variable "Row"
    }
  }
}

// Fungsi untuk memajukan serangan Invader
void AttackMovement() {
  for (int i = 0; i < MAX_ATTACK; i++)  // Buat perulangan tiap maksimal serangan yang ada
  {
    if (InvaderAttack[i].Status == ACTIVE)  // Cek jika status serangan "ACTIVE"
    {
      InvaderAttack[i].Y += 2;  // Jalankan serangan kebawah
    }
  }
}

// Fungsi untuk logika update Mothership
void MothershipControlUpdate() {
  if (Mothership.Ord.Status == ACTIVE)  // Jika status Mothership "ACTIVE"
  {
    Mothership.Ord.X += MothershipSpeed;  // Gerak koordinat X Mothership sesuai dengan "MothershipSpeed"
    if (!ImportantAudioPlayed)            // Cek Jika audio mothership tidak dijalankan
    {
      mpPlayer.play(MOTHERSHIP_SFX);  // Mainkan audio Mothership
      ImportantAudioPlayed = true;    // Beri tanda audio sedang berlangsung
    }
    if (MothershipSpeed > 0)  // Jika "MothershipSpeed" lebih besar dari 0 (Ke kanan)
    {
      if (Mothership.Ord.X >= SCREEN_WIDTH)  // Jika Mothership keluar dari layar kanan
      {
        Mothership.Ord.Status = DESTROYED;  // Ubah status Mothership jadi "DESTROYED"
        ImportantAudioPlayed = false;       // Beri tanda audio sudah selesai
      }
    } else  // Jika "MothershipSpeed" lebih kecil dari 0 (Ke kiri)
    {
      if (Mothership.Ord.X + MOTHERSHIP_WIDTH < 0)  // Jika Mothership keluar dari layar kiri
      {
        Mothership.Ord.Status = DESTROYED;  // Ubah status Mothership jadi "DESTROYED"
        ImportantAudioPlayed = false;       // Beri tanda audio sudah selesai
      }
    }
  } else  // Jika status Mothership bukan "ACTIVE"
  {
    if (random(MOTHERSHIP_SPAWN_CHANCE) == 1)  // Jika kemungkinan "MOTHERSHIP_SPAWN_CHANCE" (0-249 / 1-250(????????)) dan dapat hasil 1
    {
      Mothership.Ord.Status = ACTIVE;  // Aktifkan status Mothership
      if (random(2) == 1)              // Jika kemungkinan angka dari 0-1 dan hasilnya 1
      {
        Mothership.Ord.X = SCREEN_WIDTH;      // Atur lokasi X Mothership dibagian pojok kanan layar
        MothershipSpeed = -MOTHERSHIP_SPEED;  // Dan atur kecepatan Mothership menjadi negatif
      } else                                  // Jika kemungkinan angka dari 0-1 dan hasilnya bukan 1 (0)
      {
        Mothership.Ord.X = -MOTHERSHIP_WIDTH;  // Atur lokasi X Mothership dibagian pojok kiri layar
        MothershipSpeed = MOTHERSHIP_SPEED;    // Dan atur kecepatan Mothership menjadi positif
      }
    }
  }
}

// Fungsi cek kolisi
void CheckCollisionsUpdate() {
  BulletAndInvaderCollisions();  // Jalankan fungsi BulletAndInvaderCollisions untuk cek tabarakan
  MothershipCollision();         // Jalankan fungsi MothershipCollision untuk cek tabarakan
  InvaderAttackCollisions();     // Jalankan fungsi InvaderAttackCollisions untuk cek tabrakan
}

// Fungsi untuk mengecek tabarakan antara Bullet dan Invader
void BulletAndInvaderCollisions() {
  for (int across = 0; across < NUM_INVADER_COLUMNS; across++)  // Jika variablle "across" lebih kecil dari jumlah kolom Invader
  {
    for (int down = 0; down < NUM_INVADER_ROWS; down++)  // Jika variable "across" lebih kecil dari jumlah baris bawah Invader
    {
      if (Invader[across][down].Ord.Status == ACTIVE)  // Jika status Invader "ACTIVE"
      {
        if (Bullet.Status == ACTIVE)  // Jika status "Bullet" "ACTIVE"
        {
          if (Collision(Bullet, BULLET_WIDTH, BULLET_HEIGHT, Invader[across][down].Ord, INVADER_WIDTH, INVADER_HEIGHT))  // Cek jika funsi Collision() memberikan statement true
          {
            Invader[across][down].Ord.Status = EXPLODING;                                              // Ubah status Invader menjadi "EXPLODING"
            Bullet.Status = DESTROYED;                                                                 // Ubah status peluru menjadi "DESTROYED"
            Player.Score += InvaderScore(down);                                                        // Menambah nilai skor pemain berdasarkan baris bawah mana yang ditembak
            Player.KillCount++;                                                                        // Tambahkan jumlah Invader yang dibunuh
            Player.InvaderSpeed = ((1 - (Player.KillCount / (float)TOTAL_INVADER)) * INVADERS_SPEED);  // Atur kecepatan Invader berdasarkan kalkulasi disamping

            if (!ImportantAudioPlayed)  // Jika audio Mothership tidak jalan
            {
              mpPlayer.play(INVADER_DIE_SFX);  // Mainkan audio Invader meledak
            }
            if (Player.KillCount == TOTAL_INVADER - 2)  // Jika pemain membunuh musuh dan sisa 2
            {
              if (InvaderXMoveAmount > 0)  // Jika jalan ke kanan
              {
                InvaderXMoveAmount = INVADER_X_MOVE_AMOUNT * 2;  // Percepat dengan dikali 2
              } else                                             // Jika jalan ke kiri
              {
                InvaderXMoveAmount = -(INVADER_X_MOVE_AMOUNT * 2);  // Percepat dengan dikali 2 dan di invert nilainya
              }
            }
            if (Player.KillCount == TOTAL_INVADER - 1)  // Jika pemain membunuh musuh dan sisa 1
            {
              if (InvaderXMoveAmount > 0)  // Jika jalan ke kanan
              {
                InvaderXMoveAmount = INVADER_X_MOVE_AMOUNT * 4;  // Percepat dengan dikali 4
              } else                                             // Jika jalan ke kiri
              {
                InvaderXMoveAmount = -(INVADER_X_MOVE_AMOUNT * 4);  // Percepat dengan dikali 2 dan di invert nilainya
              }
            }
            if (Player.KillCount == TOTAL_INVADER)  // Jika semua musuh dibunuh
            {
              NextLevel(&Player);  // Lanjut ke level berikutnya
            }
          }
        }
        if (Invader[across][down].Ord.Status == ACTIVE)  // Cek jika status Invader aktif
        {
          if (Collision(Player.Ord, PLAYER_WIDTH, PLAYER_HEIGHT, Invader[across][down].Ord, INVADER_WIDTH, INVADER_HEIGHT))  // Terjadi bentrokan antara pemain dan Invader
          {
            PlayerHit();                                               // Pemain terkena serangan Invader
          } else if (Invader[across][down].Ord.Y + 8 > SCREEN_HEIGHT)  // Jika Invader melewati batas layar dibawah
          {
            PlayerHit();  // Pemain terkena serangan
          }
        }
      }
    }
  }
}

//Fungsi untuk mengecek tabrakan kepada serangan Invader
void InvaderAttackCollisions() {
  for (int i = 0; i < MAX_ATTACK; i++)  // Setiap index lebih kecil dari maksimal serangan
  {
    if (InvaderAttack[i].Status == ACTIVE)  // Cek apabila serangan aktif
    {
      if (InvaderAttack[i].Y > 64)  // Cek apabila serangan keluar dari layar bawah
      {
        InvaderAttack[i].Status = DESTROYED;                                                                     // Ubah status menjadi "DESTROYED"
      } else if (Collision(InvaderAttack[i], ATTACK_WIDTH, ATTACK_HEIGHT, Bullet, BULLET_WIDTH, BULLET_HEIGHT))  // Cek jika serangan bentrok dengan peluru pemain
      {
        InvaderAttack[i].Status = EXPLODING;  // Ledakan serangan musuh
        Bullet.Status = DESTROYED;            // Hancurkan peluru pemain
        if (!ImportantAudioPlayed) {
          mpPlayer.play(BULLET_ATTACK_COLLISION_SFX);
        }
      } else if (Collision(InvaderAttack[i], ATTACK_WIDTH, ATTACK_HEIGHT, Player.Ord, PLAYER_WIDTH, PLAYER_HEIGHT))  // Cek jika serangan bentrok dengan pemain
      {
        PlayerHit();                          // Jalankan fungsi saat pemain tertembak
        InvaderAttack[i].Status = DESTROYED;  // Hancurkan serangan musuh
      }
    }
  }
}


// Fungsi untuk mengecek tabarakan antara Bullet dan Mothership
void MothershipCollision() {
  if ((Bullet.Status == ACTIVE) && (Mothership.Ord.Status == ACTIVE))  // Jika status Bullet dan Mothership "ACTIVE"
  {
    if (Collision(Bullet, BULLET_WIDTH, BULLET_HEIGHT, Mothership.Ord, MOTHERSHIP_WIDTH, MOTHERSHIP_HEIGHT))  // Jika tabrakan terjadi
    {
      Mothership.Ord.Status = EXPLODING;                    // Atur status Mothership menjadi "EXPLODING"
      mpPlayer.play(MOTHERSHIP_DIE_SFX);                    // Mainkan audio Mothership mati
      ImportantAudioPlayed = false;                         // Matikan audio yang sedang berjalan
      Mothership.ExplosionGfxCounter = EXPLOSION_GFX_TIME;  // Mengatur waktu Invader apabila ditembak
      Bullet.Status = DESTROYED;                            // Mengubah status peluru menjadi "DESTROYED"
      MothershipBonus = random(4);                          // GACHA! (Pilih angka random dari (0-3))
      switch (MothershipBonus)                              // Cek kondisi dari gacha (angka random) "MothershipBonus"
      {
        case 0:                   // Jika nilainya 0
          MothershipBonus = 50;   // Atur "MothershipBonus" mendapatkan nilai bonus 50
          break;                  // Keluar dari switch statement
        case 1:                   // Jika nilainya 1
          MothershipBonus = 100;  // Atur "MothershipBonus" mendapatkan nilai bonus 100
          break;                  // Keluar dari switch statement
        case 2:                   // Jika nilainya 2
          MothershipBonus = 150;  // Atur "MothershipBonus" mendapatkan nilai bonus 150
          break;                  // Keluar dari switch statement
        case 3:                   // Jika nilainya 3
          MothershipBonus = 200;  // Atur "MothershipBonus" mendapatkan nilai bonus 200
          break;                  // Keluar dari switch statement
      }
      Player.Score += MothershipBonus;
      MothershipBonusXPos = Mothership.Ord.X;  // Atur posisi Mothership bonus koordinat X dengan lokasi Mothership koordinat X
      if (MothershipBonusXPos > 100)           // Jika lokasi Mothership bonus melebihi nilai 100 (melewati layar kanan)
      {
        MothershipBonusXPos = 100;         // Atur letak koordinat X untuk memastikan tidak setengah dari sisi kanan layar
      } else if (MothershipBonusXPos < 0)  // Jika lokasi Mothership bonus lebih kecil dari 0 (melewati layar kiri)
      {
        MothershipBonusXPos = 0;  // Atur letak koordinat X untuk memastikan tidak setengah dari sisi kiri layar
      }
      MothershipBonusCounter = DISPLAY_MOTHERSHIP_BONUS_TIME;  // Mengatur "MothershipBonusCounter" untuk durasi yang ditampilkan di layar
    }
  }
}

// Fungsi jika pemain tertembak
void PlayerHit() {
  Player.Ord.Status = EXPLODING;                    // Ubah status pemain jadi meledak
  Player.ExplosionGfxCounter = EXPLOSION_GFX_TIME;  // Reset dan atur timer seperti semula
  Bullet.Status = DESTROYED;                        // Ubah status peluru jadi hancur
  mpPlayer.play(PLAYER_DIE_SFX);                    // Mainkan audio player meledak
}

// Fungsi untuk menghilangkan nyawa pemain
void LoseLife() {
  Player.Lives--;  // Kurangi nyawa pemain

  if (Player.Lives > 0)  // Jika nyawa pemain masih ada
  {
    DisplayPlayerStatus(&Player);  // Jalankan fungsi "DisplayPlayerStatus" dengan argumen "&Player"

    for (int i = 0; i < MAX_ATTACK; i++)  // Looping index jika dibawah maksimal serangan musuh
    {
      InvaderAttack[i].Status = DESTROYED;  // Ubah status serangan musuh menjadi "DESTROYED"
      InvaderAttack[i].Y = 0;               // Atur ulang posisi serangan Y 0
    }
    Player.Ord.Status = ACTIVE;  // Ubah kembali status pemain menjadi "ACTIVE"
    Player.Ord.X = 0;            // Atur ulang posisi pemain X ke 0
  } else                         // Jika nyawa sudah habis
  {
    GameOver();  // Jalankan fungsi "GameOver()"
  }
}

// Fungsi layar main menu
void MenuScreen() {
  display.clearDisplay();    // Membersihkan semua tampilan display
  if (mpPlayer.available())  // Cek apakah ada pesan dari DFPlayer. Untuk meng-looping BGM
  {
    uint8_t type = mpPlayer.readType();  // Dapatkan jenis pesan
    if (type == DFPlayerPlayFinished)    // Jika playback selesai
    {
      mpPlayer.play(BGM_01);  // Mainkan kembali track 1
    }
  }
  CentreText("Mulai", 0);                 // Menulis teks ditengah, dengan argumen koordinat Y
  CentreText("Loli Invaders", 12);        // Menulis teks ditengah, dengan argumen koordinat Y
  CentreText("Tekan tombol tengah", 24);  // Menulis teks ditengah, dengan argumen koordinat Y
  CentreText("High Score     ", 36);      // Menulis teks ditengah, dengan argumen koordinat Y
  display.setCursor(88, 36);              // Atur letak teks untuk ditulis
  display.print(HighScore);               // Tulis skor tertinggi
  display.display();                      // Tampilkan dalam layar

  if (digitalRead(SHOOT_BUTTON) == false)  // Jika tombol tembak ditekan
  {
    MenuSelection = true;  // Aktifkan mode seleksi menu
  }

  if ((digitalRead(LEFT_BUTTON) == false) && (digitalRead(RIGHT_BUTTON) == false))  // Jika tombol kiri dan kanan ditekan bersamaan
  {
    //TOLONG DITAMBAHKAN FITUR DEBOUNCE ATAU WAKTU LAMA PENEKANAN AGAR TIDAK TERJADI KETIDAK SENGAJAAN RESET SKOR
    HighScore = 0;                               // Reset skor tertinggi menjadi 0
    preferences.begin("storage", false);         // Buat penyimpanan atau buka bernama "storage" dengan mode Read & Write
    preferences.putInt("HighScore", HighScore);  // Ganti data "HighScore" jadi nilai 0
    preferences.end();                           // Tutup penyimpanan saat tidak diperlukan
  }
}

// Fungsi layar game over
void GameOver() {
  GameInPlay = false;              // Atur kalo pemain sedang tidak bermain
  display.clearDisplay();          // Bersihkan gambar layar
  CentreText("Pemain 1", 0);       // Tulis teks berdasar argumen, serta lokasi Y
  CentreText("Skill Issues", 12);  // Tulis teks berdasar argumen, serta lokasi Y
  CentreText("Skor", 24);          // Tulis teks berdasar argumen, serta lokasi Y
  display.print(Player.Score);     // Tampilkan skor pemain
  if (Player.Score > HighScore)    // Jika skor pemain lebih tinggi dari skor tertinggi
  {
    CentreText("SKOR BARU!!!", 36);              // Tulis teks berdasar argumen, serta lokasi Y
    CentreText("**SELAMAT**", 48);               // Tulis teks berdasar argumen, serta lokasi Y
    HighScore = Player.Score;                    // Atur skor tertinggi menjadi skor tertinggi yang pemain raih
    preferences.begin("storage", false);         // Buat penyimpanan atau buka bernama "storage" dengan mode Read & Write
    preferences.putInt("HighScore", HighScore);  // Baca data "HighScore"
    preferences.end();                           // Tutup penyimpanan saat tidak diperlukan
  }
  display.display();  // Tampilkan semua teks diatas
  delay(2500);        // Memberi jeda
}

// Fungsi untuk menampilkan status pemain
void DisplayPlayerStatus(PlayerStruct *PLAYER) {
  display.clearDisplay();          // Bersihkan semua tampilan dalam layar
  CentreText("Pemain 1", 0);       // Tuliskan teks pada layar sesuai argumen teks dan koordinat Y
  CentreText("Skor ", 12);         // Tuliskan teks pada layar sesuai argumen teks dan koordinat Y
  display.print(PLAYER->Score);    // Tampilkan skor pemain
  CentreText("Nyawa ", 24);        // Tuliskan teks pada layar sesuai argumen teks dan koordinat Y
  display.print(PLAYER->Lives);    // Tampilkan nyawa pemain
  CentreText("Level ", 36);        // Tuliskan teks pada layar sesuai argumen teks dan koordinat Y
  display.print(PLAYER->Level);    // Tampilkan level yang pemain capai
  display.display();               // Tampilkan semua diatas
  delay(2000);                     // Kasih jeda waktu untuk kodingan berikutnya
  PLAYER->Ord.X = PLAYER_X_START;  // Atur lokasi pemain ketempat semula
}

// Fungsi untuk ke level selanjutnya
void NextLevel(PlayerStruct *PLAYER) {
  int YStart;  // Buat variable untuk menyimpan lokasi awal Y

  for (int i = 0; i < MAX_ATTACK; i++)  // Jika index lebih kecil dari serangan Invader
  {
    InvaderAttack[i].Status = DESTROYED;  // Hancurkan semua serangan musuh
  }
  ImportantAudioPlayed = false;                                                             // Cek apabila audio sedang jalan (memastikan audio sekali putar di loop
  InvaderFrame = false;                                                                     // Atur frame Invader menjadi "false" atau Diam
  PLAYER->Level++;                                                                          // Naik ke tahap level berikutnya
  YStart = ((PLAYER->Level - 1) % LEVEL_RESET_TO_START_HEIGHT) * AMOUNT_TO_DROP_PER_LEVEL;  // Atur awal lokasi Y sesuai kalkulasi
  InitInvaders(YStart);                                                                     // Buat Invader baru berdasarkan variable "YStart"
  InvaderXMoveAmount = INVADER_X_MOVE_AMOUNT;                                               // Reset variable "InvaderXMoveAmount"
  PLAYER->InvaderSpeed = INVADERS_SPEED;                                                    // Reset kecepatan InvaderSpeed
  PLAYER->KillCount = 0;                                                                    // Reset jumlah Invader yang dibunuh
  PLAYER->ShootCooldown = 0;
  Mothership.Ord.X = -MOTHERSHIP_WIDTH;  // Reset lokasi Mothership
  Mothership.Ord.Status = DESTROYED;     // Atur status Mothership jadi "DESTROYED"
  Bullet.Status = DESTROYED;             // Atur status peluru pemain menjadi "DESTROYED"
  randomSeed(100);                       // Buat seed secara acak
  DisplayPlayerStatus(&Player);          // Jalankan fungsi "DisplayPlayerStatus"
}

// Fungsi untuk memilih mode permainan
void ModeMenu() {
  display.clearDisplay();    // Bersihkan semua tampilan dalam layar
  if (mpPlayer.available())  // Cek apakah ada pesan dari DFPlayer. Untuk meng-looping BGM
  {
    uint8_t type = mpPlayer.readType();  // Dapatkan jenis pesan
    if (type == DFPlayerPlayFinished)    // Jika playback selesai
    {
      mpPlayer.play(BGM_01);  // Mainkan kembali track 1
    }
  }
  CentreText("Pilih Mode", 0);     // Menulis teks ditengah, dengan argumen koordinat Y
  CentreText("Singleplayer", 12);  // Menulis teks ditengah, dengan argumen koordinat Y
  CentreText("Multiplayer", 24);   // Menulis teks ditengah, dengan argumen koordinat Y
  display.display();               // Tampilkan dalam layar

  if (digitalRead(LEFT_BUTTON) == false)  // Jika tombol kiri ditekan
  {
    GameInPlay = true;      // status game berlangsung aktif
    Multiplayer = false;    // Status game singleplayer
    MenuSelection = false;  // Status mode seleksi matikan
    NewGame();              // Buat game ronde baru
  } else if (digitalRead(RIGHT_BUTTON) == false) {
    GameInPlay = true;      // status game berlangsung aktif
    Multiplayer = true;     // Status game multiplayer
    MenuSelection = false;  // Status mode seleksi matikan
    MultiplayerMode();      // Buat game ronde baru secara multiplayer
  }
}

// Fungsi untuk memulai permainan baru
void NewGame() {
  InitPlayer();
  NextLevel(&Player);
}

// Fungsi untuk memainkan mode Multiplayer
void MultiplayerMode() {
  WiFi.mode(WIFI_STA);  // Mengatur Wi-Fi ESP32 ke mode Station (tidak perlu internet)

  if (esp_now_init() != ESP_OK)  // Mengecek apakah ESP-NOW berhasil diinisialisasi
  {
    Serial.println("ESP-NOW gagal diinisialisasi!");  // Menulis teks error
    return;                                           // Menghentikan eksekusi jika gagal
  }

  esp_now_register_send_cb(onDataSent);  // Mendaftarkan fungsi untuk mengirim data

  esp_now_peer_info_t peerInfo = {};                // Menambahkan ESP32 lawan sebagai peer
  memcpy(peerInfo.peer_addr, PlayerMACAddress, 6);  // Menyalin alamat MAC lawan
  peerInfo.channel = 0;                             // Channel default
  peerInfo.encrypt = false;                         // Tanpa enkripsi

  if (esp_now_add_peer(&peerInfo) != ESP_OK)  // Mengecek apakah peer berhasil ditambahkan
  {
    Serial.println("Gagal menemukan MAC Address lawan!");  // Menulis teks error
    return;                                                // Menghentikan eksekusi jika gagal
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataReceive));  // Mendaftarkan fungsi untuk menerima data

  // TODO
  // PINDAHKAN PAS BAGIAN AKHIR KALKULASI SKOR
  esp_now_send(PlayerMACAddress, (uint8_t *)&Player, sizeof(Player));  // Kirimkan data pemain kita ke ESP32 lawan
  // PINDAHKAN PAS BAGIAN AWAL KALKULASI SKOR
  StartTime = millis();  // Menyimpan waktu mulai permainan
}

// Fungsi untuk mengecek tabarakan dengan argumen yang dibawah
bool Collision(GameObjectStruct OBJECT1, unsigned char WIDTH1, unsigned char HEIGHT1, GameObjectStruct OBJECT2, unsigned char WIDTH2, unsigned char HEIGHT2) {
  // Mengembalikan nilai "true" jika 2 obek ter-tabarakan
  return ((OBJECT1.X + WIDTH1 > OBJECT2.X) && (OBJECT1.X < OBJECT2.X + WIDTH2) && (OBJECT1.Y + HEIGHT1 > OBJECT2.Y) && (OBJECT1.Y < OBJECT2.Y + WIDTH2));
}

// Fungsi untuk menengahkan teks
void CentreText(const char *TEXT, unsigned char Y) {
  display.setCursor(int((float)(SCREEN_WIDTH) / 2 - ((strlen(TEXT) * 6) / 2)), Y);
  display.print(TEXT);
}

// Fungsi untuk skor ketika menembak Invader
unsigned char InvaderScore(int ROW_NUMBER) {
  switch (ROW_NUMBER)  // Cek Invader baris bawah mana yang ditembak
  {
    case 0: return 30;  // Jika di atas dapat skor 30
    case 1: return 20;  // Jika di tengah dapat skor 20
    case 2: return 10;  // Jika di bawah dapat skor 10
  }
}

// Cek invader mana yang paling kanan
int RightMostPos() {
  int across = NUM_INVADER_COLUMNS - 1;  // Buat variable "across" sesuai dengan nilai
  int down;                              // Buat variable "down"
  int largest = 0;                       // Atur terbesar menjadi 0
  int rightPos;                          // Buat variable posisi kanan

  while (across >= 0)  // Looping jika across lebih besar atau sama dengan 0
  {
    down = 0;                        // Atur variable down menjadi nilai 0
    while (down < NUM_INVADER_ROWS)  // Looping jika nilai 0 lebih besar dari kolom invader
    {
      if (Invader[across][down].Ord.Status == ACTIVE)  // Cek kondisi invader apakah statusnya "ACTIVE"
      {
        rightPos = Invader[across][down].Ord.X + INVADER_WIDTH;  // Atur posisi paling kanan dengan invadernya
        if (rightPos > largest)                                  // Jika posisi kanan lebih besar dari variable "largest" (0)
        {
          largest = rightPos;  // Atur variable largest dengan angka posisi paling kanan
        }
      }
      down++;  // Nambah nilai variable down
    }
    if (largest > 0)  // Jika nilai tertinggi lebih dari 0
    {
      return largest;  // Kasih nilai tertinggi tersebut
    }
    across--;  // Kurangi nilai variable "across"
  }
  return 0;  // Seharusnya tidak pernah sampai sejauh ini
}

// Cek invader mana yang paling kiri
int LeftMostPos() {
  int across = 0;                       // Atur variable "across" menjadi 0
  int down;                             // Buat variable "down"
  int smallest = SCREEN_WIDTH * 2;      // Buat variable smallest dengan nilai yang dimasukkan
  while (across < NUM_INVADER_COLUMNS)  // Jika across lebih kecil dari jumlah kolom invader
  {
    down = 0;                        // Atur variable "down" jadi 0
    while (down < NUM_INVADER_ROWS)  // Looping jika nilai 0 lebih kecil dari kolom invader
    {
      if (Invader[across][down].Ord.Status == ACTIVE)  // Cek kondisi invader apakah statusnya "ACTIVE"
      {
        if (Invader[across][down].Ord.X < smallest)  // Cek jika invader "ACTIVE" lebih kecil dari variable "smallest"
        {
          smallest = Invader[across][down].Ord.X;  // Atur variable terkecil sesuai dengan invader "ACTIVE" posisi X
        }
      }
      down++;  // Menambah nilai variable "down"
    }
    if (smallest < SCREEN_WIDTH * 2)  // Jika variable "smallest" lebih kecil dari layar
    {
      return smallest;  // Kasih nilai tertinggi tersebut
    }
    across++;  // Tambahi nilai variable "across"
  }
  return 0;  // Seharusnya tidak pernah sampai sejauh ini
}