#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <DFPlayer.h>

MPU6050 mpu;

// -------------------- DFPlayer --------------------
#define MP3_RX_PIN 10
#define MP3_TX_PIN 11
#define MP3_SERIAL_SPEED 9600
#define MP3_SERIAL_TIMEOUT 350
#define VOL_AUDIO 30

SoftwareSerial mp3Serial(MP3_RX_PIN, MP3_TX_PIN, false);
DFPlayer mp3;
// --------------------------------------------------

int valor = 0;

// --------- Variables de control ----------
bool movimientoDetectado = false;
bool numeroYaDicho = false;
bool audioMovimientoSonado = false;

unsigned long tiempoInicioEstable = 0;
int ultimaCaraLeida = 0;

// Idioma
int idiomaActual = 0;
int idiomaAnterior = 0;

// Umbrales
const int UMBRAL_MOVIMIENTO = 6000;
const unsigned long TIEMPO_ESTABLE = 1200;
// ----------------------------------------

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();

  mp3Serial.begin(MP3_SERIAL_SPEED);
  mp3.begin(mp3Serial, MP3_SERIAL_TIMEOUT, DFPLAYER_MINI, false);

  mp3.stop();
  mp3.reset();
  delay(1000);

  mp3.setSource(2);
  mp3.setEQ(0);
  mp3.setVolume(VOL_AUDIO);

  if (mpu.testConnection()) {
    Serial.println("MPU6050 conectado");
  } else {
    Serial.println("Error MPU6050");
  }

  pinMode(2, INPUT_PULLUP);   // Idioma 1
  pinMode(3, INPUT_PULLUP);   // Idioma 2

  pinMode(4, OUTPUT); // 6
  pinMode(5, OUTPUT); // 1
  pinMode(6, OUTPUT); // 4
  pinMode(7, OUTPUT); // 2
  pinMode(8, OUTPUT); // 3
  pinMode(9, OUTPUT); // 5

  resetear();

  // Anunciar idioma al inicio
  idiomaActual = detectarIdioma();
  idiomaAnterior = idiomaActual;
  anunciarIdioma(idiomaActual);
  delay(2000);
}

void loop() {
  delay(100);

  digitalWrite(11, HIGH);

  // -------- detectar cambio de idioma --------
  idiomaActual = detectarIdioma();

  if (idiomaActual != idiomaAnterior) {
    idiomaAnterior = idiomaActual;

    anunciarIdioma(idiomaActual);
    delay(2000);
    return;
  }
  // ------------------------------------------

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  int numero = detectarCara(ax, ay, az);
  valor = numero;

  long magnitudMovimiento = abs(ax) + abs(ay) + abs(az - 16384);

  Serial.print("Cara: ");
  Serial.print(numero);
  Serial.print(" | Movimiento: ");
  Serial.println(magnitudMovimiento);

  // 1. Detectar movimiento
  if (magnitudMovimiento > UMBRAL_MOVIMIENTO) {

    if (!movimientoDetectado) {
      mp3.playTrack(19);
      delay(3000);
      audioMovimientoSonado = true;
    }

    movimientoDetectado = true;
    numeroYaDicho = false;
    tiempoInicioEstable = 0;
    ultimaCaraLeida = numero;
    resetear();

    return;
  }

  // 2. Si no hay movimiento → no hacer nada
  if (!movimientoDetectado) return;

  // 3. Esperar estabilización
  if (numero != ultimaCaraLeida) {
    ultimaCaraLeida = numero;
    tiempoInicioEstable = millis();
    return;
  }

  if (tiempoInicioEstable == 0) {
    tiempoInicioEstable = millis();
    return;
  }

  // 4. Confirmar número estable
  if (!numeroYaDicho && (millis() - tiempoInicioEstable >= TIEMPO_ESTABLE)) {

    Serial.print("Número confirmado: ");
    Serial.println(numero);

    encenderLed(numero);

    for (int i = 0; i < 3; i++) {
      reproducirNumero(numero);
      delay(2000);
    }

    numeroYaDicho = true;
    movimientoDetectado = false;
    audioMovimientoSonado = false;

    digitalWrite(11, LOW);
  }
}

// ----------- FUNCIONES -----------

int detectarIdioma() {
  if (digitalRead(2) == LOW) return 1;
  else if (digitalRead(3) == LOW) return 2;
  else return 3;
}

void anunciarIdioma(int idioma) {
  if (idioma == 1) mp3.playTrack(20); // español
  else if (idioma == 2) mp3.playTrack(21); // quichua
  else mp3.playTrack(22); // inglés
}

int detectarCara(int ax, int ay, int az) {
  int absX = abs(ax);
  int absY = abs(ay);
  int absZ = abs(az);

  if (absZ > absX && absZ > absY) {
    if (az > 0) return 1;
    else return 6;
  }

  if (absX > absY) {
    if (ax > 0) return 2;
    else return 5;
  }

  if (ay > 0) return 3;
  else return 4;
}

void reproducirNumero(int numero) {

  // Español
  if (detectarIdioma() == 1) {
    if (numero == 1) mp3.playTrack(1);
    if (numero == 2) mp3.playTrack(2);
    if (numero == 3) mp3.playTrack(3);
    if (numero == 4) mp3.playTrack(4);
    if (numero == 5) mp3.playTrack(5);
    if (numero == 6) mp3.playTrack(6);
  }

  // Quichua
  else if (detectarIdioma() == 2) {
    if (numero == 1) mp3.playTrack(7);
    if (numero == 2) mp3.playTrack(8);
    if (numero == 3) mp3.playTrack(9);
    if (numero == 4) mp3.playTrack(10);
    if (numero == 5) mp3.playTrack(11);
    if (numero == 6) mp3.playTrack(12);
  }

  // Inglés
  else {
    if (numero == 1) mp3.playTrack(13);
    if (numero == 2) mp3.playTrack(14);
    if (numero == 3) mp3.playTrack(15);
    if (numero == 4) mp3.playTrack(16);
    if (numero == 5) mp3.playTrack(17);
    if (numero == 6) mp3.playTrack(18);
  }
}

void encenderLed(int numero) {
  resetear();

  if (numero == 1) digitalWrite(5, HIGH);
  if (numero == 2) digitalWrite(7, HIGH);
  if (numero == 3) digitalWrite(8, HIGH);
  if (numero == 4) digitalWrite(6, HIGH);
  if (numero == 5) digitalWrite(9, HIGH);
  if (numero == 6) digitalWrite(4, HIGH);
}

void resetear() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
}