#define PINO_RX 8 // Pino do 'RECEPTOR'
#define PINO_TX 8 // Pino do 'EMISSOR' * estamos utilizando esse!
#define PINO_CLCK 5

#define PINO_RTS 12 // Sinal 'EMISSOR para RECEPTOR'
#define PINO_CTS 11 // Sinal 'RECEPTOR para EMISSOR'

#define BAUD_RATE 1 // Baud rate inicial = 1Hz
#define BYTE_SIZE 8
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define PREAMBULE_SIZE 1

#define CLCK_INIT true

#include "Temporizador.h"

/* Handshake: variáveis de controle para o controle de fluxo de dados entre 'EMISSOR' e 'RECEPTOR'
   Dessa forma, é possível evitar que haja sobrecarregamento do receptor, que pode não estar
   disponível para comunicação. */
enum TxState {
  WaitingForData, // Estado em aguardo de comunicação
  WaitingForCTS,  // Estado em aguardo do CTS por parte do receptor
  Transmitting,   // Estado em transmissão de dados
};

// O estado inicial da transmissão é a espera por bits da Serial
TxState txState = WaitingForData;

// O byte a ser transmitido
char byteToSend;

// O índice do bit a ser transmitido esse ciclo.
int bitIndex;

bool bit, isOdd = false, clck;
char mask;

void paraTransmissao() {
  paraTemporizador();
    digitalWrite(PINO_RTS, LOW);
    Serial.print("Char a enviar: ");
    // Retorna ao estado de espera por bits da Serial
    txState = WaitingForData;
    digitalWrite(PINO_CLCK, clck);
}

// Rotina de interrupção do timer, onde deverá ser feito o cálculo de bit de paridade
ISR(TIMER1_COMPA_vect){
  clck = !clck;
  // Contagem e verificação dos bits
  if (bitIndex < 0) {
    digitalWrite(PINO_TX, HIGH);
    Serial.print("\tSB ");
    Serial.println(HIGH);
    bitIndex++;
  } else if (bitIndex < 8) {
    // Cálculo do bit atual a partir do caractere enviado
    mask = 1 << bitIndex;
    bit = (byteToSend & mask) != 0;

    // Envio parcial de bits
    digitalWrite(PINO_TX, bit);
    Serial.print("\tBit (");
    Serial.print(bitIndex + 1);
    Serial.print("): " );
    Serial.println(bit);

    // Incremeta contador de bits 1, para paridade.
    isOdd ^= bit;
    bitIndex++;
  } else {
    // Verifica quantidade de 1's no caractere a ser transmitido
    // Envia o último bit de acordo com o resultado anterior
    digitalWrite(PINO_TX, !isOdd);
    Serial.print("\tParidade: ");
    Serial.println(!isOdd);

    // Para-se o temporizador e limpa-se o sinal RTS
    paraTransmissao();
  }
  digitalWrite(PINO_CLCK, clck);
}

void setup(){
  //Desabilita interruções
  noInterrupts();

  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);

  // Inicializa TX (não utilizaremos o RX, já que estamos programando o 'EMISSOR')
  pinMode(PINO_TX, OUTPUT);
  pinMode(PINO_RTS, OUTPUT);
  pinMode(PINO_CTS, INPUT);
  pinMode(PINO_CLCK, OUTPUT);
  
  digitalWrite(PINO_RTS, LOW);
  clck = CLCK_INIT;
  digitalWrite(PINO_RTS, clck);

  // Configura timer
  configuraTemporizador(BAUD_RATE);

  // Habilita interrupções
  interrupts();
  Serial.print("Char a enviar: ");
}

void loop ( ) {
  digitalWrite(PINO_CLCK, clck);
  switch(txState) {
    case WaitingForData:
      // Verifica a existência de dados (bits passados pela serial)
      if (Serial.available() > 0) {
        byteToSend = (char) Serial.read();
        if (byteToSend == '\n') break;
        
        // Leitura do bit enviado pela Serial
        Serial.println(byteToSend);
        Serial.println("Bits enviados [minor first]:");

        // Inicializa as variáveis globais
        bitIndex = -PREAMBULE_SIZE;
        isOdd = false;

        /* Handshake: Configura o RTS e entra em estado de espera
           para que o 'RECEPTOR' disponibilize o CTS*/
        digitalWrite(PINO_CLCK, clck);
        digitalWrite(PINO_RTS, HIGH);
        txState = WaitingForCTS;
      } else
        digitalWrite(PINO_RTS, LOW);
      break;

    case WaitingForCTS:
      /* Handshake: Espera que o 'RECEPTOR' inicialize o CTS */
      if (digitalRead(PINO_CTS) == HIGH) {
        /* Handshake: Reinicia o temporizador, voltando ao estado de transmissão */
        iniciaTemporizador();
        txState = Transmitting;
      }
      break;

    default:
      // Aguarda enquanto a rotina de interrupção (Handshake) controla o fluxo de dados
      break;
  }
}
