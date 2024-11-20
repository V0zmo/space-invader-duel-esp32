/* 
PASTIKAN INSTALL LIBRARY DIBAWAH INI!!!
Tools -> Manage Libraries... atau Ctrl +Shift+I

EspSoftwareSerial (Dirk Kaar, Peter Lerup) untuk SoftwareSerial.h
DFRobotDFPlayerMini (DFRobot) untuk DFRobotDFPlayerMini.h
Adafruit SSD1306 (Adafruit) untuk Adafruit_SSD1306.h
Adafruit GFX Library (Adafruit) untuk Adafruit_GFX.h
*/

// LIBRARY
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BAGIAN DISPLAY
#define OLED_RESET -1                                                      // Setel ulang pin # (atau -1 jika berbagi pin setel ulang Arduino)
#define OLED_ADDRESS 0x3C                                                  // Alamat OLED
#define SCREEN_WIDTH 128                                                   // Lebar tampilan OLED, dalam piksel
#define SCREEN_HEIGHT 64                                                   // Tinggi tampilan OLED, dalam piksel
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Deklarasi untuk layar SSD1306 yang tersambung ke I2C (SDA, pin SCL)

// BAGIAN AUDIO
static const uint8_t PIN_MP3_TX = 26;                   // Menghubungkan ke RX modul
static const uint8_t PIN_MP3_RX = 27;                   // Menghubungkan ke TX modul
DFRobotDFPlayerMini mpPlayer;                           // Membuat MP Player
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);  // Buat SoftwareSerial dengan pin TX/RX

// BAGIAN TOMBOL
#define SHOOT_BUTTON 12  // Pin tombol 1
#define LEFT_BUTTON 13   // Pin tombol 2
#define RIGHT_BUTTON 14  // Pin tombol 3

// BAGIAN GLOBAL GAME
#define ACTIVE 0     // Status konstanta aktif objek permainan
#define DESTROYED 2  // Status konstanta hancur objek permainan
// Struktur game global, Objek dasar yang akan disertakan oleh sebagian besar objek lain
struct GameObjectStruct {
  signed int X;          // Lokasi X
  signed int Y;          // Lokasi Y
  unsigned char Status;  // 0 Active | 1 Exploding | 2 Destroyed
};

// BAGIAN PLAYER
#define PLAYER_WIDTH 8          // Lebar player
#define PLAYER_HEIGHT 8         // Tinggi player
#define PLAYER_X_MOVE_AMOUNT 2  // Kecepatan jalan player dalam pixel
#define PLAYER_X_START 0        // Lokasi mulai player dalam koordinat X
#define PLAYER_Y_START 56       // Lokasi mulai player dalam koordinat Y
// Struktur untuk Player
struct PlayerStruct {
  GameObjectStruct Ord;  // Inisiasi class GameObjectStruct
};
PlayerStruct Player;  // Player global variable

// BAGIAN MISIL / PELURU
#define BULLET_WIDTH 8     // Lebar peluru
#define BULLET_HEIGHT 8    // Panjang peluru
#define BULLET_SPEED 4     // Kecepatana peluru (semakin besar semakin cepat)
bool BulletFrame = false;  // false = Diam | true = Jalan
GameObjectStruct Bullet;   // Objek peluru

// BAGIAN INVADER
#define NUM_INVADER_COLUMNS 7            // Jumlah invader lurus ke kanan
#define NUM_INVADER_ROWS 3               // Jumlah invader lurus ke bawah
#define SPACE_BETWEEN_INVADER_COLUMNS 5  // Jarak antara invader dari kanan
#define SPACE_BETWEEN_INVADER_ROWS 16    // Jarak antara invader dari bawah
#define INVADER_WIDTH 8                  // Ukuran lebar terbesar invader
#define INVADER_HEIGHT 8                 // Ukuran lebar terbesar invader
#define X_START_OFFSET 6                 // Offset X lokasi invader
#define INVADERS_DROP 4                  // Seberapa jauh invader jatuh dalam pixel
#define INVADERS_SPEED 12                // Kecepatan invader (semakin rendah semakin cepat)
signed char InvaderXMoveAmount = 2;      // Kecepatan jalan invaders dalam pixel
signed char InvadersMoveCounter;         // menghitung mundur, ketika 0 memindahkan invader, atur sesuai dengan berapa banyak alien di layar (Tersambung tidak langsung dengan INVADERS_SPEED)
bool InvaderFrame = false;               // false = Diam | true = Jalan
// Struktur untuk Invader
struct InvaderStruct {
  GameObjectStruct Ord;  // Inisiasi class GameObjectStruct
};
InvaderStruct Invader[NUM_INVADER_COLUMNS][NUM_INVADER_ROWS];  // Buat Invader dengan multidimension array (seperti tabel)


/*
Dikarenakan graphics musuh untuk ukuran 16x16 px terlalu besar jadi kita bakalan perkecil
jadi 8x8 px untuk mendapatkan suasanya 8bit game serta untuk menghemat tempat ruang dibagian
display dan juga bagian processor di ESP32. Untuk penulisan data diusahakan menggunakan
hexadecimal, untuk permudah bacaan dan ringkas kode. Apabila kesulitan bisa menggunakan
angka biner.
*/

// Graphics Player
static const unsigned char PROGMEM PLAYER_GFX[] = {
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
static const unsigned char PROGMEM BULLET1_GFX[] = {
  0x18,
  0x18,
  0x18,
  0x00,
  0x18,
  0x18,
  0x18,
  0x00
};

// Graphics Bullet 2
static const unsigned char PROGMEM BULLET2_GFX[] = {
  0x18,
  0x24,
  0x00,
  0x18,
  0x24,
  0x00,
  0x18,
  0x24
};

// Graphics Musuh 1 (Diam)
static const unsigned char PROGMEM INVADER_1_GFX_01[] = {
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
static const unsigned char PROGMEM INVADER_1_GFX_02[] = {
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
static const unsigned char PROGMEM INVADER_2_GFX_01[] = {
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
static const unsigned char PROGMEM INVADER_2_GFX_02[] = {
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
static const unsigned char PROGMEM INVADER_3_GFX_01[] = {
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
static const unsigned char PROGMEM INVADER_3_GFX_02[] = {
  0x66,
  0x7e,
  0xff,
  0xbd,
  0x99,
  0xff,
  0xa5,
  0x99
};

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
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  // Cek jika MP Player tidak dapat tersambung
  if (!mpPlayer.begin(softwareSerial)) {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }

  // Inisiasi untuk permainan

  InitInvaders(0);
  InitPlayer();
}

void loop() {
  Update();  // Perulangan logika game
  Draw();    // Perulangan pergambaran game
}

// Fungsi Update untuk semua yang berhubungan dengan logika atau perhitungan dalam kodingan
void Update() {
  InvaderControl();   // Fungsi logika Invader
  PlayerControl();    // Fungsi logika Player
  BulletControl();    // Fungsi logika Bullet
  CheckCollisions();  // Fungsi logika pengecekan kolisi
}

// Fungsi Draw untuk semua urusan yang berhubungan gambar
void Draw() {
  display.clearDisplay();                                       // Menghilangkan semua gambar display (Prosesor lambat jangan dicoba! ( •̀ ω •́ )✧)
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
      }
    }
  }
  display.drawBitmap(Player.Ord.X, Player.Ord.Y, PLAYER_GFX, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);  // Gambar pesawat Player
  if (Bullet.Status == ACTIVE) {
    if (!BulletFrame) {
      display.drawBitmap(Bullet.X, Bullet.Y, BULLET1_GFX, BULLET_WIDTH, BULLET_HEIGHT, WHITE);
    } else {
      display.drawBitmap(Bullet.X, Bullet.Y, BULLET2_GFX, BULLET_WIDTH, BULLET_HEIGHT, WHITE);
    }
  }
  display.display();  // Memunculkan semua gambar display
}

// Inisiasi untuk Player
void InitPlayer() {
  Player.Ord.X = PLAYER_X_START;  // Atur lokasi awal X player
  Player.Ord.Y = PLAYER_Y_START;  // Atur lokasi awal Y player
  Bullet.Status = DESTROYED;
}

// Fungsi untuk mengkontrol pemain
void PlayerControl() {
  if ((digitalRead(RIGHT_BUTTON) == false) && (Player.Ord.X + PLAYER_WIDTH < SCREEN_WIDTH))  // Jika tombol ke kanan ditekan dan badan pesawat lebih kecil dari lebar layar
  {
    Player.Ord.X += PLAYER_X_MOVE_AMOUNT;  // Majukan pemain ke kanan
  }
  if ((digitalRead(LEFT_BUTTON) == false) && (Player.Ord.X > 0))  // Jika tombol ke kiri ditekan dan badan pesawat lebih besar dari 0 (ujung kiri layar)
  {
    Player.Ord.X -= PLAYER_X_MOVE_AMOUNT;  // Majukan pemain ke kiri
  }
  if ((digitalRead(SHOOT_BUTTON) == false) && (Bullet.Status != ACTIVE))  // Jika tombol tembak ditekan dan status peluru tidak aktif
  {
    Bullet.X = Player.Ord.X + (PLAYER_WIDTH / 2) - 4;  // Atur posisi X peluru sesuai lokasi pemain
    Bullet.Y = PLAYER_Y_START;                         // Atur posisi Y peluru sesuai lokasi Y awal pemain
    Bullet.Status = ACTIVE;                            // Atur kondisi peluru menjadi aktif
  }
}

// Fungsi untuk mengatur peluru
void BulletControl() {
  if (Bullet.Status == ACTIVE)  // Jika status peluru aktif
  {
    Bullet.Y -= BULLET_SPEED;          // Jalankan peluru keatas
    if (Bullet.Y + BULLET_HEIGHT < 0)  // Jika peluru melewati layar atas
    {
      Bullet.Status = DESTROYED;  // Ubah status peluru menjadi "DESTROYED" (hancur)
    }
    BulletFrame = !BulletFrame;  // Ubah frame
  }
}

// Fungsi InitInvaders dengan argumen int untuk permulaan lokasi Y
void InitInvaders(int YSTART) {
  for (int across = 0; across < NUM_INVADER_COLUMNS; across++)  // Looping dan cek untuk baris ke kanan
  {
    for (int down = 0; down < NUM_INVADER_ROWS; down++)  // Looping dan cek untuk baris ke bawah
    {
      /* 
      CATATAN DARI XTronical (Tutor)
      
      kita tambahkan "down" untuk memusatkan alien, kebetulan nilai yang tepat yang kita butuhkan per baris!
      kita perlu menyesuaikan sedikit karena baris nol seharusnya 2, baris 1 seharusnya 1 dan baris paling bawah 0
      */

      // Melakukan kalkulasi letak koordinat masing-masing Invader (masing-masing beda) mulai dari X lanjut ke Y;
      Invader[across][down].Ord.X = X_START_OFFSET + (across * (INVADER_WIDTH + SPACE_BETWEEN_INVADER_COLUMNS)) - down;
      Invader[across][down].Ord.Y = YSTART + (down * SPACE_BETWEEN_INVADER_ROWS);
    }
  }
}

// Buat mengontrol jalan invaders
void InvaderControl() {
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
    InvadersMoveCounter = INVADERS_SPEED;  // Reset InvadersMoveCounter dengan INVADERS_SPEED (12)
    InvaderFrame = !InvaderFrame;          // Tukar frame dengan frame lain
  }
}

// Fungsi cek kolisi
void CheckCollisions() {
  BulletAndInvaderCollisions();  // Jalankan fungsi BulletAndInvaderCollisions
}

// Fungsi untuk mengecek kolosi antara Bullet dan Invader
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
            Invader[across][down].Ord.Status = DESTROYED;  // Ubah status Invader menjadi "DESTROYED"
            Bullet.Status = DESTROYED;                     // Ubah status peluru menjadi "DESTROYED"
          }
        }
      }
    }
  }
}

// Fungsi untuk mengecek kolosi dengan argumen yang dibawah
bool Collision(GameObjectStruct OBJECT1, unsigned char WIDTH1, unsigned char HEIGHT1, GameObjectStruct OBJECT2, unsigned char WIDTH2, unsigned char HEIGHT2) {
  // Mengembalikan nilai "true" jika 2 obek ter-kolosi
  return ((OBJECT1.X + WIDTH1 > OBJECT2.X) && (OBJECT1.X < OBJECT2.X + WIDTH2) && (OBJECT1.Y + HEIGHT1 > OBJECT2.Y) && (OBJECT1.Y < OBJECT2.Y + WIDTH2));
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