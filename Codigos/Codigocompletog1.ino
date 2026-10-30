#include <Arduino.h>

// ==========================================
// ASIGNACIÓN DE PINES - DISPLAY 7 SEGMENTOS
// ==========================================
#define DISP_A 12
#define DISP_B 13
#define DISP_C 14
#define DISP_D 15
#define DISP_E 16
#define DISP_F 17
#define DISP_G 18

// ==========================================
// ASIGNACIÓN DE PINES - TECLADO MATRICIAL
// ==========================================
#define ROW1 19
#define ROW2 21
#define ROW3 22
#define ROW4 23

#define COL1 25
#define COL2 26
#define COL3 27
#define COL4 33

// ==========================================
// ASIGNACIÓN DE PINES - MOTORES / PUENTE H (LEDS)
// ==========================================
#define M1_IN1 2   // Motor Izquierdo Adelante
#define M1_IN2 0   // Motor Izquierdo Reversa (Reasignado a GPIO 0)
#define M2_IN3 5   // Motor Derecho Adelante
#define M2_IN4 32  // Motor Derecho Reversa

// Arreglos para iteración
const uint8_t DISP_PINS[7] = {DISP_A, DISP_B, DISP_C, DISP_D, DISP_E, DISP_F, DISP_G};
const uint8_t ROW_PINS[4]  = {ROW1, ROW2, ROW3, ROW4};
const uint8_t COL_PINS[4]  = {COL1, COL2, COL3, COL4};

// Estructura y tabla de tipografía para Cátodo Común
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

// Tabla de búsqueda (LUT) del Teclado
const char KEY_MAP[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Prototipos de funciones
void printdisplay(char caracter);
char readKeyPad();
void controlMotors(char command);

void setup() {
  Serial.begin(115200);

  // Configuración de pines del Display como salidas
  for (int i = 0; i < 7; i++) {
    pinMode(DISP_PINS[i], OUTPUT);
    digitalWrite(DISP_PINS[i], LOW);
  }

  // Configuración de pines del Teclado
  for (int r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }
  for (int c = 0; c < 4; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }

  // Configuración de pines de Motores/Puente H como salidas
  pinMode(M1_IN1, OUTPUT); digitalWrite(M1_IN1, LOW);
  pinMode(M1_IN2, OUTPUT); digitalWrite(M1_IN2, LOW);
  pinMode(M2_IN3, OUTPUT); digitalWrite(M2_IN3, LOW);
  pinMode(M2_IN4, OUTPUT); digitalWrite(M2_IN4, LOW);

  Serial.println(">>> Sistema RobotLogo MVP Listo: Teclado + Display + Motores <<<");
}

void loop() {
  char key = readKeyPad();
  
  if (key != 0) {
    Serial.print("Comando Recibido: ");
    Serial.println(key);
    
    // Muestra el carácter en el display de 7 segmentos
    printdisplay(key);
    
    // Ejecuta el control de motores según la tecla seleccionada
    controlMotors(key);
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
        delay(200); // Filtro antirrebote por software
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
      Serial.println("Estado Motores: AVANZANDO");
      break;

    case '8': // RETROCEDER
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, HIGH);
      Serial.println("Estado Motores: RETROCEDIENDO");
      break;

    case '4': // GIRO IZQUIERDA
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, HIGH);
      digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);
      Serial.println("Estado Motores: GIRO IZQUIERDA");
      break;

    case '6': // GIRO DERECHA
      digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, HIGH);
      Serial.println("Estado Motores: GIRO DERECHA");
      break;

    default: // DETENER (Cualquier otra tecla)
      digitalWrite(M1_IN1, LOW);  digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN3, LOW);  digitalWrite(M2_IN4, LOW);
      Serial.println("Estado Motores: DETENIDOS");
      break;
  }
}
