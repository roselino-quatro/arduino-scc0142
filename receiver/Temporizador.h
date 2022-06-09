//// ==== Digital PINs used ====
//#define PINO_RX 8
//#define PINO_TX 8
//#define PINO_RTS 2
//#define PINO_CTS 3
//#define PINO_CLCK 5
//// ==== Digital PINs used ====
//
//// ==== Transmission defines ====
//#define BAUD_RATE 1
//#define HALF_BAUD 1000/(2*BAUD_RATE)
//#define START_BIT -2
//// ==== Transmission defines ====
//
//// ==== Data variables ====
//// We use global variables so they're acessible
//// inside the ISR(TIMER1_COMPRA_vect) function
//char cur_letter = 0;
//char cur_bit = START_BIT;  // starts as -1 because the first bit is actually a marker (SB), not data
//bool is_valid = false, 
//  isOdd = false, 
//  receiving = false, 
//  read_bit,
//  clck = true;
//// ==== Data variables ====
//
//#include "Temporizador.h"
//
//// Calcule parity bit - odd or even
//// This file refers to an odd Receiver
//bool parity_bit(char lt){
//  char ones_count = 0;
//  for (int i = 0; i < 8; i++) {
//    ones_count += bitRead(lt, i);
//  }
//
//  return ones_count % 2 != 1;
//}
//
//void fimDeRecepcao() {
//  Serial.print("Letra recebida: ");
//  Serial.println(cur_letter);
//  digitalWrite(PINO_CTS, LOW);
//  Serial.println("PINO_CTS setado para LOW - fim de recebimento");
//  Serial.println("\nPronto para recepção");
//  receiving = false;
//  cur_letter = 0;
//  cur_bit = START_BIT;
//  is_valid = false;
//}
//
//// Timer1 interrupt routine
//// What to do every time 1s has passed?
//ISR(TIMER1_COMPA_vect){
//  read_bit = digitalRead(PINO_RX);
//  Serial.print("Bit recebido: ");
//  Serial.print(read_bit);
//
//  if (cur_bit == START_BIT) {
//    Serial.println("    [SB]");
//    if (!cur_bit) {
//      Serial.println("SB falso, parando transmissão!");
//      fimDeRecepcao();
//    }
//    cur_bit++;  // Increment to next bit position 
//  } else if (cur_bit == 8) { // Parity Bit
//    Serial.println("    [parity bit]");
//    fimDeRecepcao();
//  } else {
//    Serial.println("    [data bit]");
//    cur_letter |= (read_bit << cur_bit);
//    isOdd ^= read_bit;
//    cur_bit++;  // Increment to next bit position 
//  }
//}
//
//// Executada uma vez quando o Arduino reseta
//void setup(){
//  noInterrupts(); // Disable interrupts for startup
//  Serial.begin(9600); // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
//
//  // Initialize all used PINs
//  pinMode(PINO_RX, INPUT);
//  pinMode(PINO_RTS, INPUT);
//  pinMode(PINO_CTS, OUTPUT);
//  
//  // Configura timer
//  configuraTemporizador(BAUD_RATE);
//  // habilita interrupcoes
//  Serial.println("Pronto para recepção");
//  interrupts();
//}
//
//// O loop() eh executado continuamente (como um while(true))
//void loop ( ) {
//  if (digitalRead(PINO_RTS) == LOW || receiving) return;
//    
//  Serial.print("Iniciou a tranmissao - ");
//
//  // Initialize global variables
//  cur_letter = 0;
//  cur_bit = START_BIT;
//  is_valid = false;
//
//  digitalWrite(PINO_CTS, HIGH);
//  Serial.println("PINO_CTS setado para HIGH");
//  receiving = true;
//
//  iniciaTemporizador();
//}

// ==== Digital PINs used ====
#define PINO_RX 8
#define PINO_TX 8
#define PINO_RTS 2
#define PINO_CTS 3
#define PINO_CLCK 5
// ==== Digital PINs used ====

// ==== Transmission defines ====
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define START_BIT -1
// ==== Transmission defines ====

// ==== Data variables ====
// We use global variables so they're acessible
// inside the ISR(TIMER1_COMPRA_vect) function
char cur_letter = 0;
char cur_bit = START_BIT;  // starts as -1 because the first bit is actually a marker (SB), not data
bool is_valid = false, 
  isOdd = false, 
  receiving = false, 
  _receiving,
  read_bit,
  clck = true,
  curClck = true,
  first = true;
// ==== Data variables ====

#include "Temporizador.h"

// Calcule parity bit - odd or even
// This file refers to an odd Receiver
bool parity_bit(char lt){
  char ones_count = 0;
  for (int i = 0; i < 8; i++) {
    ones_count += bitRead(lt, i);
  }

  return ones_count % 2 != 1;
}

void setVariaveis() {
  cur_letter = 0;
  cur_bit = START_BIT;  // starts as -1 because the first bit is actually a marker (SB), not data
  is_valid = false;
  isOdd = false;
  receiving = false;
  clck = true;
  curClck = !clck;
  first = true;
}

void fimDeRecepcao() {
  if (read_bit == !isOdd) {
    Serial.print("Letra recebida: ");
    Serial.println(cur_letter);
  } else {
    Serial.println("Bit de paridade incorreto!");
  }
  digitalWrite(PINO_CTS, LOW);
  Serial.println("PINO_CTS setado para LOW - fim de recebimento");
  Serial.println("\nPronto para recepção");
  setVariaveis();
}

// Executada uma vez quando o Arduino reseta
void setup(){
//  noInterrupts();/ // Disable interrupts for startup
  Serial.begin(9600); // Configura porta serial (Serial Monitor - Ctrl + Shift + M)

  // Initialize all used PINs
  pinMode(PINO_RX, INPUT);
  pinMode(PINO_RTS, INPUT);
  pinMode(PINO_CLCK, INPUT);
  pinMode(PINO_CTS, OUTPUT);
  
  Serial.println("Pronto para recepção");

  // Initialize global variables
  setVariaveis();
}

// O loop() eh executado continuamente (como um while(true))
void loop () {
  receiving = receiving || digitalRead(PINO_RTS);
  if (!receiving) return;
  digitalWrite(PINO_CTS, HIGH);

//  if (first) {
//    clck = curClck = digitalRead(PINO_CLCK);
//    first = false;
//    return;
//  }
  
  curClck = digitalRead(PINO_CLCK);
//  if (receiving && curClck == clck) return;/
  if (curClck == clck) return;

  clck = curClck;

  read_bit = digitalRead(PINO_RX);
  Serial.print("Bit recebido: ");
  Serial.print(read_bit);

  if (cur_bit == START_BIT) {
    receiving = true;
    Serial.println("    [SB]");
    if (!cur_bit) {
      Serial.println("SB falso, parando transmissão!");
      fimDeRecepcao();
    }
    cur_bit++;  // Increment to next bit position 
  } else if (cur_bit == 8) { // Parity Bit
    Serial.println("    [parity bit]");
    fimDeRecepcao();
  } else {
    Serial.println("    [data bit]");
    cur_letter |= (read_bit << cur_bit);
    isOdd ^= read_bit;
    cur_bit++;  // Increment to next bit position 
  }
}
