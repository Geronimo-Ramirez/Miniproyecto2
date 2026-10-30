#include <Arduino.h>

// Directivas #define para asignación de etiquetas de hardware
#define ROW1 19
#define ROW2 21
#define ROW3 22
#define ROW4 23

#define COL1 25
#define COL2 26
#define COL3 27
#define COL4 33

void setup() {
  Serial.begin(115200);

  // Configuración de las filas como salidas digitales
  pinMode(ROW1, OUTPUT); digitalWrite(ROW1, HIGH);
  pinMode(ROW2, OUTPUT); digitalWrite(ROW2, HIGH);
  pinMode(ROW3, OUTPUT); digitalWrite(ROW3, HIGH);
  pinMode(ROW4, OUTPUT); digitalWrite(ROW4, HIGH);

  // Configuración de las columnas como entradas con resistencia Pull-Up
  pinMode(COL1, INPUT_PULLUP);
  pinMode(COL2, INPUT_PULLUP);
  pinMode(COL3, INPUT_PULLUP);
  pinMode(COL4, INPUT_PULLUP);

  Serial.println(">>> Prueba del Driver del Teclado Inicializada <<<");
}

void loop() {
  char tecla = 0;

  // Barrido Fila 1
  digitalWrite(ROW1, LOW); digitalWrite(ROW2, HIGH); digitalWrite(ROW3, HIGH); digitalWrite(ROW4, HIGH);
  if (digitalRead(COL1) == LOW)      tecla = '1';
  else if (digitalRead(COL2) == LOW) tecla = '2';
  else if (digitalRead(COL3) == LOW) tecla = '3';
  else if (digitalRead(COL4) == LOW) tecla = 'A';

  // Barrido Fila 2
  digitalWrite(ROW1, HIGH); digitalWrite(ROW2, LOW); digitalWrite(ROW3, HIGH); digitalWrite(ROW4, HIGH);
  if (digitalRead(COL1) == LOW)      tecla = '4';
  else if (digitalRead(COL2) == LOW) tecla = '5';
  else if (digitalRead(COL3) == LOW) tecla = '6';
  else if (digitalRead(COL4) == LOW) tecla = 'B';

  // Barrido Fila 3
  digitalWrite(ROW1, HIGH); digitalWrite(ROW2, HIGH); digitalWrite(ROW3, LOW); digitalWrite(ROW4, HIGH);
  if (digitalRead(COL1) == LOW)      tecla = '7';
  else if (digitalRead(COL2) == LOW) tecla = '8';
  else if (digitalRead(COL3) == LOW) tecla = '9';
  else if (digitalRead(COL4) == LOW) tecla = 'C';

  // Barrido Fila 4
  digitalWrite(ROW1, HIGH); digitalWrite(ROW2, HIGH); digitalWrite(ROW3, HIGH); digitalWrite(ROW4, LOW);
  if (digitalRead(COL1) == LOW)      tecla = '*';
  else if (digitalRead(COL2) == LOW) tecla = '0';
  else if (digitalRead(COL3) == LOW) tecla = '#';
  else if (digitalRead(COL4) == LOW) tecla = 'D';

  // Reporte en Monitor Serie al presionar una tecla
  if (tecla != 0) {
    Serial.print("Tecla detectada: ");
    Serial.println(tecla);
    delay(250); // Filtro antirrebote por software
  }
}
