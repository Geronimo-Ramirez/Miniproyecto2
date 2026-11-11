#include <Arduino.h>
#include <ESP32Servo.h>

// ==========================================
// ESTADOS Y VARIABLES GLOBALES
// ==========================================
enum { 
  QUIETO, 
  ADELANTE, 
  ATRAS, 
  DERECHA, 
  IZQUIERDA, 
  LEVANTAPLUMA, 
  BAJAPLUMA 
};

int comando = QUIETO;  
int valor = 0;         
bool moviendo = false; 

Servo servoPluma;
#define SERVO_PIN 33   

// ==========================================
// DISPLAY 7 SEGMENTOS
// ==========================================
#define DISP_A 12
#define DISP_B 13
#define DISP_C 14
#define DISP_D 15
#define DISP_E 16
#define DISP_F 17
#define DISP_G 18

const uint8_t DISP_PINS[7] = {DISP_A, DISP_B, DISP_C, DISP_D, DISP_E, DISP_F, DISP_G};

struct FontEntry {
  char key;
  uint8_t mask;
};

const FontEntry TYPOGRAPHY[] = {
  {'0', 0x3F}, {'1', 0x06}, {'2', 0x5B}, {'3', 0x4F},
  {'4', 0x66}, {'5', 0x6D}, {'6', 0x7D}, {'7', 0x07},
  {'8', 0x7F}, {'9', 0x6F}, {'A', 0x77}, {'B', 0x7C},
  {'C', 0x39}, {'D', 0x5E}, {'*', 0x63}, {'#', 0x76}
};
const size_t TYPOGRAPHY_SIZE = sizeof(TYPOGRAPHY) / sizeof(TYPOGRAPHY[0]);

// ==========================================
// TECLADO MATRICIAL 4x4 
// ==========================================
#define ROW1 19
#define ROW2 21
#define ROW3 22
#define ROW4 23

#define COL1 25
#define COL2 26
#define COL3 27
#define COL4 4    

const uint8_t ROW_PINS[4] = {ROW1, ROW2, ROW3, ROW4};
const uint8_t COL_PINS[4] = {COL1, COL2, COL3, COL4};

const char KEY_MAP[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// ==========================================
// MOTORES / PUENTE H (LEDS)
// ==========================================
#define M1_IN1 2   // Motor Izq. Adelante (LED Verde 1)
#define M1_IN2 0   // Motor Izq. Reversa  (LED Rojo 1)
#define M2_IN3 5   // Motor Der. Adelante (LED Verde 2)
#define M2_IN4 32  // Motor Der. Reversa  (LED Rojo 2)

// Prototipos
void printdisplay(char caracter);
char readKeyPad();
void controlMotors(char command);
int distancia_tiempo(int valor);
int angulo_tiempo(int valor);
void interpreta_comando(char* cadena);
void leer_comando_teclado();
bool delayConInterrupcion(int tiempo_ms);
char getKeyBlocking();

void setup() {
  Serial.begin(115200);

  // Display
  for (int i = 0; i < 7; i++) {
    pinMode(DISP_PINS[i], OUTPUT);
    digitalWrite(DISP_PINS[i], LOW);
  }

  // Teclado
  for (int r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }

  // Motores
  pinMode(M1_IN1, OUTPUT); digitalWrite(M1_IN1, LOW);
  pinMode(M1_IN2, OUTPUT); digitalWrite(M1_IN2, LOW);
  pinMode(M2_IN3, OUTPUT); digitalWrite(M2_IN3, LOW);
  pinMode(M2_IN4, OUTPUT); digitalWrite(M2_IN4, LOW);

  // Servomotor en GPIO 33
  servoPluma.attach(SERVO_PIN);
  servoPluma.write(0); 

  Serial.println("\n==================================================");
  Serial.println("  ROBOTLOGO E2   ");
  Serial.println("==================================================");
}

void loop() {
  leer_comando_teclado();

  if (comando == ADELANTE) {
    Serial.print("[EJECUCIÓN] ADELANTE: "); Serial.print(valor); Serial.println(" cm");
    controlMotors('2');
    moviendo = true;
    if (!delayConInterrupcion(distancia_tiempo(valor))) {
      controlMotors('0');
    }
    moviendo = false;
  }
  else if (comando == ATRAS) {
    Serial.print("[EJECUCIÓN] ATRÁS: "); Serial.print(valor); Serial.println(" cm");
    controlMotors('8');
    moviendo = true;
    if (!delayConInterrupcion(distancia_tiempo(valor))) {
      controlMotors('0');
    }
    moviendo = false;
  }
  else if (comando == DERECHA) {
    Serial.print("[EJECUCIÓN] GIRO DERECHA: "); Serial.print(valor); Serial.println(" deg");
    controlMotors('6');
    moviendo = true;
    if (!delayConInterrupcion(angulo_tiempo(valor))) {
      controlMotors('0');
    }
    moviendo = false;
  }
  else if (comando == IZQUIERDA) {
    Serial.print("[EJECUCIÓN] GIRO IZQUIERDA: "); Serial.print(valor); Serial.println(" deg");
    controlMotors('4');
    moviendo = true;
    if (!delayConInterrupcion(angulo_tiempo(valor))) {
      controlMotors('0');
    }
    moviendo = false;
  }
  else if (comando == BAJAPLUMA) {
    Serial.println("[EJECUCIÓN] BAJAR PLUMA");
    servoPluma.write(90);
    delay(300);
  }
  else if (comando == LEVANTAPLUMA) {
    Serial.println("[EJECUCIÓN] LEVANTAR PLUMA");
    servoPluma.write(0);
    delay(300);
  }

  comando = QUIETO;
}

// ==========================================
// FUNCIONES DE SOPORTE
// ==========================================

int distancia_tiempo(int valor) {
  return valor * 50; 
}

int angulo_tiempo(int valor) {
  return valor * 12; 
}

bool delayConInterrupcion(int tiempo_ms) {
  unsigned long inicio = millis();
  while (millis() - inicio < (unsigned long)tiempo_ms) {
    char key = readKeyPad();
    if (key == '*') {
      controlMotors('0'); 
      comando = QUIETO;
      moviendo = false;
      Serial.println("\n!!! PARADA DE EMERGENCIA ACTIVA ('*') !!!");
      return true;
    }
    delay(10);
  }
  return false;
}

void interpreta_comando(char* cadena) {
  comando = QUIETO;
  valor = 0;

  if (cadena[0] == '\0') return;

  if (cadena[0] == 'A' && cadena[1] == 'A') {
    comando = ATRAS;
    valor = atoi(&cadena[2]);
  } 
  else if (cadena[0] == 'A') {
    comando = ADELANTE;
    valor = atoi(&cadena[1]);
  } 
  else if (cadena[0] == 'B') {
    comando = DERECHA;
    valor = atoi(&cadena[1]);
  } 
  // Acepta la letra 'C' del teclado o la 'I' para Giro Izquierda
  else if (cadena[0] == 'C' || cadena[0] == 'I') {
    comando = IZQUIERDA;
    valor = atoi(&cadena[1]);
  } 
  else if (cadena[0] == 'D' && cadena[1] == '\0') {
    static bool plumaAbajo = false;
    plumaAbajo = !plumaAbajo;
    comando = plumaAbajo ? BAJAPLUMA : LEVANTAPLUMA;
  }
}

char getKeyBlocking() {
  char key = 0;
  while (key == 0) {
    key = readKeyPad();
    delay(20);
  }

  printdisplay(key);

  while (readKeyPad() != 0) {
    delay(20);
  }

  return key;
}

void leer_comando_teclado() {
  int pos = 0;
  char cad_comando[20];
  cad_comando[0] = '\0';

  while (pos < (sizeof(cad_comando) - 1)) {
    char key = getKeyBlocking();

    if (key == '*') {
      comando = QUIETO;
      moviendo = false;
      controlMotors('0');
      Serial.println("\n-> Tecla '*' presionada: Comando cancelado.");
      return;
    }

    if (key == '#') {
      cad_comando[pos] = '\0';
      Serial.println();
      break;
    }

    cad_comando[pos] = key;
    pos++;
    Serial.print(key);
  }

  if (pos > 0) {
    Serial.print("Comando Interpretado: ");
    Serial.println(cad_comando);
    interpreta_comando(cad_comando);
  }
}

void printdisplay(char caracter) {
  uint8_t mask = 0x00;

  for (size_t i = 0; i < TYPOGRAPHY_SIZE; i++) {
    if (TYPOGRAPHY[i].key == caracter) {
      mask = TYPOGRAPHY[i].mask;
      break;
    }
  }

  for (int bitIndex = 0; bitIndex < 7; bitIndex++) {
    bool estadoSegmento = (mask >> bitIndex) & 0x01;
    digitalWrite(DISP_PINS[bitIndex], estadoSegmento ? HIGH : LOW);
  }
}

char readKeyPad() {
  for (int r = 0; r < 4; r++) {
    digitalWrite(ROW_PINS[r], LOW);

    for (int c = 0; c < 4; c++) {
      if (digitalRead(COL_PINS[c]) == LOW) {
        char tecla = KEY_MAP[r][c];
        digitalWrite(ROW_PINS[r], HIGH);
        return tecla;
      }
    }

    digitalWrite(ROW_PINS[r], HIGH);
  }
  return 0;
}

void controlMotors(char command) {
  switch (command) {
    case '2': // AVANZAR
      digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);
      break;

    case '8': // RETROCEDER
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, HIGH);
      break;

    case '4': // GIRO IZQUIERDA
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
      digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);
      break;

    case '6': // GIRO DERECHA
      digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, HIGH);
      break;

    default: // DETENER
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, LOW);
      break;
  }
}
