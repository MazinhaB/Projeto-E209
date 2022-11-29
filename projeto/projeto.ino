#include <Servo.h>
#include <stdio.h>
#include <stdlib.h>
#include "DHT.h"
 
#define DHTPIN A1 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
#define BIT0 0b00000001
#define BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

#define FOSC 16000000U
#define BAUD 9600
#define MYUBRR (FOSC / 16 / (BAUD - 1))
#define TAMANHO 6

Servo servo1;

char msg_tx[20];
char msg_rx[32];
int pos_msg_rx = 0;
int tamanho_msg_rx = TAMANHO;
int cont = 0;
int aux;
int del = 0;
void UART_config(unsigned int ubrr);
void UART_Transmit(char *dados);
int segundos = 0;
int minutos = 0;
int horas = 0;
int cont1 = 0;
int ligar = 0;

ISR(INT1_vect) { // interrupção do buzzer
  PORTB ^= BIT0; // ativa o buzzer
  EIFR = BIT1;
}
ISR(INT0_vect) { // botão liga/desliga

}
ISR (TIMER0_COMPA_vect) // interrupção ativa quando o timer 0 estoura o limite
{
  // pulsos de 100us
  cont++;
  if (cont == 10000) { // passou 1 segundo
    cont = 0;
    segundos++;
  }
  if (segundos == 60) { // passou 1 minuto
    segundos = 0;
    minutos++;
  }
  if (minutos == 60) { // passou 1 hora
    minutos = 0;
    horas++;
  }
  if (horas == 24 & minutos == 59 && segundos == 60) {
    segundos = 0;
    minutos = 0;
    horas = 0;
  }
  int valorH = 0;
  valorH = ((msg_rx[0] - 48) * 10) + ((msg_rx[1] - 48) * 1);
  int valorM = 0;
  valorM = ((msg_rx[3] - 48) * 10) + ((msg_rx[4] - 48) * 1);

  if ((valorH == horas) & (valorM == minutos)) // abre o servo
  {
    servo1.write(90);
  }
}
int main()
{
  // buzzer
  DDRB |= BIT0;
  PORTB |= BIT0;
  PORTB &= ~BIT0;

  servo1.attach(6);
  servo1.write(0);

  // entrada do sensor reflexivo em pd3 e botão de liga/desliga em pd2
  EICRA |= (BIT1 + + BIT2 + BIT3);
  EIMSK |= (BIT1 + BIT0);

  // habilita resistor de pull-up em pd2 e pd3
  PORTD |= (BIT2 + BIT3);

  // declaração dos timers
  TCCR0A = BIT1; // modo comparação
  TCCR0B = BIT1; // prescaler 8 -> frequência efetiva de 2MHz -> T = 500ns
  // delay de contagem -> 100us; limite de comparação -> 200

  // definição do comparador
  OCR0A = 200;

  // habilita interrupção de comparação
  TIMSK0 = BIT1;
  sei();

  //Inicialização da Serial
  UART_config(MYUBRR);

  aux = -1;
  while (1) {
    //Transmissão Serial de Números
    if (aux != cont1) {
      itoa(cont1, msg_tx, 10);
      UART_Transmit(msg_tx);
      UART_Transmit("\n");
      aux = cont1;
    }
  }
}
//Interrupção de Recebimento da Serial
ISR(USART_RX_vect) {

  // Escreve o valor recebido pela UART na posição pos_msg_rx do buffer msg_rx
  msg_rx[pos_msg_rx++] = UDR0;

  if (pos_msg_rx == tamanho_msg_rx)
    pos_msg_rx = 0;
}

//Transmissão de Dados Serial
void UART_Transmit(char *dados) {

  // Envia todos os caracteres do buffer dados ate chegar um final de linha
  while (*dados != 0) {
    while ((UCSR0A & BIT5) == 0);  // Aguarda a transmissão acabar

    // Escreve o caractere no registro de tranmissão
    UDR0 = *dados;
    // Passa para o próximo caractere do buffer dados
    dados++;
  }
}

//Configuração da Serial
void UART_config(unsigned int ubrr) {

  // Configura a  baud rate
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  // Habilita a recepcao, tranmissao e interrupcao na recepcao */
  UCSR0B = (BIT7 + BIT4 + BIT3);
  // Configura o formato da mensagem: 8 bits de dados e 1 bits de stop */
  UCSR0C = (BIT2 + BIT1);
}
