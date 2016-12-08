//
//	1.	ws2812b傳輸時間<1微秒(0.4us, 0.8us, etc.)
//		為了取得1微秒內時間差
//		會將SMCLK設成20MHz (原本預設是1MHz)
//		使用clock時(如UART)請注意
//	2.	控制燈條有時會不穩定
//		可以自己微調send_0()和send_1()內delay的時間試試看
//
//										by 王贊鈞
//
//	Function    :  control ws2812b led strip with msp430f5529lp
//	IDE         :  IAR
//	Connections :  LED <=> msp
//	 	           +5v     3V3
//	               GND     GND
//	               Din     P1.2 (default)
//	Instructions:  1. set RGB info with "set_RGB(int led, char R, char G, char B)"
//	               2. call "send_RGB()" to transmit data
//	               3. use "delay_m(ms)" between different mode
//	Reference   :  ws2812b document
//	               https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf

#include "msp430.h"

//delay function
#define CPU_CLOCK       20000000
#define delay_50ns(ns)  __delay_cycles(CPU_CLOCK/20000000*(ns))
#define delay_us(us)    __delay_cycles(CPU_CLOCK/1000000*(us))
#define delay_ms(ms)    __delay_cycles(CPU_CLOCK/1000*(ms))

//set Din
//e.g. connect to P1.2 
#define OUT_PORT_DIR P1DIR
#define OUT_PORT_OUT P1OUT
#define OUT_BIT BIT2

//total number of leds in the strip
#define LED_NUM 60

//RGB info
char RGB[LED_NUM * 3];

//timer and output setup
void Init();

//send code0 to Din
void send_0();

//send code1 to Din
void send_1();

//send reset to Din
void send_res();

//send latest RGB to strip
void send_RGB();

//clear RGB
void clear_RGB();

//set RGB with LED, R, G, B 
void set_RGB(int led, char R, char G, char B);

int main(){ 
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  //initialization
  Init();
  
  //start here
  
  //以下code是 紅 綠 藍 交錯閃
  while(1){
    clear_RGB();
    for(int i=0; i<LED_NUM; i+=3)
         set_RGB(i, 50, 0, 0);
    send_RGB(); 
    delay_ms(250);
    
    clear_RGB();
    for(int i=1; i<LED_NUM; i+=3)
         set_RGB(i, 0, 50, 0);
    send_RGB(); 
    delay_ms(250);
    
    clear_RGB();
    for(int i=2; i<LED_NUM; i+=3)
         set_RGB(i, 0, 0, 50);
    send_RGB(); 
    delay_ms(250);
  }
  
  
}

//set smclk : 20MHz
void Init(){
  //init RGB
  clear_RGB();
  
  //timer setup
  //ref: https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/393162
  P5SEL |= BIT2 + BIT3;           // enable XT2 ports: XT2OUT (P5.3) XT2IN (P5.2)
  __bis_SR_register(SCG0);        // disable FLL control loop

  // DCO frequency control
  UCSCTL0 = 0x0900;                                               // DCO = 9
  UCSCTL1 = 0x0050 | DISMOD;                                      // low freq DCO, modulation disabled

  // FLL frequency stabilization, fll loop divider and low freq multiplier
  UCSCTL2 = 0x0004;                                               // N = 4    fll to 20 MHz

  // set FLL reference clock to XT2 with div/1
  UCSCTL3 = SELREF__XT2CLK | FLLREFDIV_0;
  UCSCTL4 = SELA__REFOCLK | SELS__XT2CLK | SELM__DCOCLK;          // select clock sources
  UCSCTL5 = DIVPA__1 | DIVA__16 | DIVS__1 | DIVM__1;              // select clock source divisions 1/16/1/1
  UCSCTL6 = XT1OFF;                                               // No XT1 present
  UCSCTL7 = 0;                                                    // clear all clock faults
  UCSCTL8 = ACLKREQEN | MCLKREQEN | SMCLKREQEN | MODOSCREQEN;     // enable requests
  __bic_SR_register(SCG1);                                        // enable DCO
  __bic_SR_register(SCG0);                                        // enable FLL control loop
  __delay_cycles(100000);                                         // wait for clocks to stabilize

  // Loop until clocks stabilizes  (or timeout)
  do {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);                 // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                                          // Clear fault flags
  }while (SFRIFG1 & OFIFG);                                       // Test oscillator fault flag

  //output set up
  OUT_PORT_DIR = OUT_BIT;
  OUT_PORT_OUT = 0;
  
  //init led
  send_RGB();
}

//send code0 to Din
void send_0(){
  OUT_PORT_OUT = OUT_BIT;
  delay_50ns(5);
  OUT_PORT_OUT &= ~OUT_BIT;
  delay_50ns(18);
}

//send code1 to Din
void send_1(){
  OUT_PORT_OUT = OUT_BIT;
  delay_50ns(16);
  OUT_PORT_OUT &= ~OUT_BIT;
  delay_50ns(9);  
}

//send reset to Din
void send_res(){
  OUT_PORT_OUT &= ~OUT_BIT;
  delay_us(1000);
}

//send latest RGB to strip
void send_RGB(){
  char b;
  for(int i=0; i<LED_NUM*3; i++){
    b = 0x80;
    for(int j=0; j<8; j++){
      if(RGB[i] & b)
        send_1();
      else
        send_0();
      b >>= 1;
    }
  }
  send_res();
}

//clear RGB info
void clear_RGB(){
  for(int i=0; i<LED_NUM*3; i++)
    RGB[i] = 0;
}

//set RGB with LED, R, G, B
void set_RGB(int led, char R, char G, char B){
  if(led>=0){
    RGB[led * 3] = G;
    RGB[led * 3 + 1] = R;
    RGB[led * 3 + 2] = B;
  } 
}



