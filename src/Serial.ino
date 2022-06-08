#define PINO_RX 13 // Pino do 'RECEPTOR'
#define PINO_TX 13 // Pino do 'EMISSOR' * estamos utilizando esse!

#define PINO_RTS 12 // Sinal 'EMISSOR para RECEPTOR' 
#define PINO_CTS 11 // Sinal 'RECEPTOR para EMISSOR'

#define BAUD_RATE 1 // Baud rate inicial = 1Hz
#define HALF_BAUD 1000/(2*BAUD_RATE)

#include "Temporizador.h"

/* Handshake: variáveis de controle para o controle de fluxo de dados entre 'EMISSOR' e 'RECEPTOR' 
   Dessa forma, é possível evitar que haja sobrecarregamento do receptor, que pode não estar
   disponível para comunicação. */
enum TxState
{
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

// Contador de bits '1' transmitidos (para o cálculo do bit de paridade)
int bitOneCount;

// Rotina de interrupção do timer, onde deverá ser feito o cálculo de bit de paridade
ISR(TIMER1_COMPA_vect){
  // Contagem e verificação dos bits
  if (bitIndex <= 8) {
        // Cálculo do bit atual a partir do caractere enviado
        char mask = 1 << bitIndex;
        bool bit = (byteToSend & mask) != 0;
    
        // Envio parcial de bits
        digitalWrite(PINO_TX, bit);
        bitIndex++;
    
        // Incremeta contador de bits 1, para paridade.
        if (bit) {
          bitOneCount++;
        }
  } else if (bitIndex == 9) {
        // Verifica quantidade de 1's no caractere a ser transmitido
        bool isEven = bitOneCount % 2;
        
        // Envia o último bit de acordo com o resultado anterior
        digitalWrite(PINO_TX, isEven ? 1 : 0);
    
        // Para-se o temporizador e limpa-se o sinal RTS
        paraTemporizador();
        digitalWrite(PINO_RTS, LOW);

        // Retorna ao estado de espera por bits da Serial
        txState = WaitingForData;
  }
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
  
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  
  // Habilita interrupções
  interrupts();
}

void loop ( ) {
  
  switch(txState)
  {
    case WaitingForData:
      // Verifica a existência de dados (bits passados pela serial)
      if (Serial.available() > 0) {
        
        // Leitura do bit enviado pela Serial
        byteToSend = (char) Serial.read();

        // Inicializa as variáveis globais
        bitIndex = 0;
        bitOneCount = 0;

        /* Handshake: Configura o RTS e entra em estado de espera 
        para que o 'RECEPTOR' disponibilize o CTS*/
        digitalWrite(PINO_RTS, HIGH);
        txState = WaitingForCTS;
      }
      break;
    
    case WaitingForCTS:
      /* Handshake: Espera que o 'RECEPTOR' inicialize o CTS */
      if (digitalRead(PINO_CTS)) {
        
        /* Handshake: Reinicia o temporizador, voltando ao estado de transmissão */
        iniciaTemporizador();
        txState = Transmitting;
      }
      break;
      
    case Transmitting:
      // Aguarda enquanto a rotina de interrupção (Handshake) controla o fluxo de dados
      break;
  }
}
