#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SSD1306.h"
#include "i2c.h"
#include "dht11.h"

#define OLED_PWR    BIT0                        // OLED power connect/disconnect on pin P1.0

#define LONG_DELAY  __delay_cycles(3000000)
#define SHORT_DELAY __delay_cycles(50000)

#define Pump_water BIT4     //Gan chan may bom
#define light_sensor BIT4 // Gan chan cam bien anh sang
#define soil_sensor BIT5 // Gan chan cam bien do am dat
#define LED BIT5 //Chan den LED
#define FAN BIT3 // Fan

unsigned int ADC_Value; // Variable to store the analog value from the light sensor
unsigned int ADC_soilValue; // Variable to store the analog value from the light sensor
unsigned int isAuto = 1;


void initADC(void);
void initsoilADC(void);
void itoa_float(float value, char *str);
void itoa(int value, char *str, int base);
void initUART(void);
void ser_output(char *str);
void itoa_float(float value, char *str);
void uartSendString(char* str);

//void init_LED(void) {
//    P2DIR |= LED;                 // Set P2 to output direction
//    P2OUT &= ~LED;                // LED off

int main(void)

 {
    WDTCTL = WDTPW | WDTHOLD;                   // stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1SEL |= light_sensor; // Select light sensor input
    P1SEL |= soil_sensor;  // Select soil sensor input

    __delay_cycles(50000);

    i2c_init();                                 // initialize I2C to use with OLED
    P1SEL |= BIT1 + BIT2;    // Set P1.1 to RXD and P1.2 to TXD
    P1SEL2 |= BIT1 + BIT2;   // Set P1.1 to RXD and P1.2 to TXD
    initUART();
    P2DIR |= Pump_water;                       // Set Pump water outside as output
    P2DIR |= LED;                       // Set LED outside as output
    P2DIR |= FAN;                       // Set Fan as output
    P1DIR = OLED_PWR;                           // P1.0 output, else input
    P1OUT ^= OLED_PWR;                          // turn on power to display



    ssd1306_init();                             // Initialize SSD1306 OLED

    ssd1306_clearDisplay();                     // Clear OLED display

    initADC();

    // Cau hinh ADC
    ADC10CTL0 = REFON + ADC10SHT_2 + ADC10ON; // Bat ADC10 và cau hinh thoi gian lay mau

    ssd1306_printText(30, 1, "HE THONG");
    ssd1306_printText(10, 2, "VUON THONG MINH");
    ssd1306_printText(20, 4, "SMART GARDEN");
    LONG_DELAY;
    ssd1306_clearDisplay();

    ssd1306_printText(30, 1, "TRUONG DHBK");
    ssd1306_printText(30, 2, " KHOA DTVT");
    ssd1306_printText(20, 4, "MON KT VI XU LY");
    ssd1306_printText(0, 6,  "GVHD:");
    ssd1306_printText(30, 7, " HO VIET VIET");
    LONG_DELAY;
    ssd1306_clearDisplay();
    ssd1306_printText(10, 1, "THANH VIEN NHOM");
    ssd1306_printText(20, 3, "1. ANH QUOC");
    ssd1306_printText(20, 4, "2. THANH DAT");
    ssd1306_printText(20, 5, "3. CHI KIEN");
    ssd1306_printText(20, 6, "4. THANH NGA");
    ssd1306_printText(20, 7, "5. THINH ANH");

    LONG_DELAY;
    ssd1306_clearDisplay();

//    init_LED();
    __bis_SR_register(GIE);

    while(1) {

//Read and process light sensor data
        ADC10CTL0 &= ~ENC; // Xoa bit ENC de tat chuyen doi ADC
        ADC10CTL1 = INCH_4; // Chon kenh P1.0 de doc do am dat
        ADC10AE0 |= BIT4;
        ADC10CTL0 |= ENC + ADC10SC; // Enable and start conversion
        while (ADC10CTL0 & BUSY); // Wait for conversion complete
        ADC_Value = ADC10MEM; // Read the result
        //Gui light data

        char light_val[20];
        float adc_val = (float) ADC_Value;//  ADC_value sang float
        adc_val = (1024 - adc_val); //calculate brightness
        itoa_float(adc_val, light_val);// chuyển float sang string


                    // Control an LED based on ADC value
        if (isAuto == 1) {
                    if (ADC_Value > 0x2BC){ //dec  value > 700 turn on led
                        P2OUT &= ~LED; // Set P2.5 high
                        __delay_cycles(200000);
                    } else {
                        P2OUT |= LED; // Set P2.5 low
                        __delay_cycles(200000);
                    }
        }
                    __delay_cycles(500000);
                    //WDTCTL = WDT_MRST_0_064; // reset

//Read and process soil sensor data
        ADC10CTL0 &= ~ENC; // Xoa bit ENC de tat chuyen doi ADC
        ADC10CTL1 = INCH_5; // Chon kenh P1.5 de doc do am dat
        ADC10AE0 |= BIT5;
        ADC10CTL0 |= ENC + ADC10SC; // Enable and start conversion
        while (ADC10CTL0 & BUSY); // Wait for conversion complete
        ADC_soilValue = ADC10MEM; // Read the result

//Gui soil data
        char soil_val[20];
        float adc_soilval = (float) ADC_soilValue;//  ADC_value sang float
        adc_soilval = (105 - (adc_soilval / 1023.0)*100);
        itoa_float(adc_soilval, soil_val);// chuyển float sang string
        // Kiem tra và dieu khien dong co may bom
                                        if (isAuto == 1) {
                                                if (ADC_soilValue >= 0x2BC) //dec  value > 50 Turn on pump water
                                                  {
                                                   P2OUT &= ~BIT4;
                                                   __delay_cycles(2000000);
                                                  }
                                                else
                                                  {
                                                    P2OUT |= BIT4;
                                                    __delay_cycles(2000000);
                                                  }
                                                __delay_cycles(500000);

                                        }

//Gui do am kk va nhiet do
        char mois_val[20];
        char temp_val[20];
        itoa_float(doAm, mois_val);// chuyển float sang string

        itoa_float(nhietDo, temp_val);// chuyển float sang string

        // Dieu khien quat dua vao gia tri nhiet do
                       if (isAuto == 1) {
                               if (nhietDo >= 0x020) //dec  value > 32 C Turn on fan
                                 {
                                  P2OUT &=~ BIT3;
                                  __delay_cycles(2000);
                                 }
                               else
                                 {
                                   P2OUT |= BIT3;
                                   __delay_cycles(2000);
                                 }
       //                        __delay_cycles(20000);

                       }

                // Gửi chuỗi kết hợp qua UART
        char combined_str[100];
        sprintf(combined_str, "%s,%s,%s,%s,", light_val, soil_val, temp_val, mois_val);
        uartSendString(combined_str);
        __delay_cycles(50000);


        // Hien thi Nhiet do, Do am kk
        readDHT11();//get du lieu dht11

        ssd1306_printText(0, 5, "NHIET DO:");
        ssd1306_printUI32(80, 5, nhietDo, HCENTERUL_OFF);
        ssd1306_printText(95, 5, "oC");

        ssd1306_printText(0, 7, "DO AM KHONG KHI:");
        ssd1306_printUI32(100, 7, doAm, HCENTERUL_OFF);
        ssd1306_printText(115, 7, "%");

        ssd1306_printText(0, 1, "DO SANG:");
        ssd1306_printUI32(80, 1, adc_val, HCENTERUL_OFF);
        ssd1306_printText(100, 1, "lux");

        ssd1306_printText(0, 3, "DO AM DAT:");
        ssd1306_printUI32(80, 3, adc_soilval, HCENTERUL_OFF);
        ssd1306_printText(95, 3, "%");


    }
}
// USCI_A0 RX ISR
//#pragma vector=USCIAB0RX_VECTOR
//__interrupt void USCI0RX_ISR(void) {
//    char rx_char = UCA0RXBUF;      // Read received character
//    if (rx_char == '1') {
//        P2OUT |= LED;             // Turn on LED
//    } else if (rx_char == '0') {
//        P2OUT &= ~LED;            // Turn off LED
//    }
//}

// USCI_A0 RX ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
    char rx_char = UCA0RXBUF;      // Read received character
    if (isAuto == 0) {
    if (rx_char == '1') {
        P2OUT |= LED;             // Turn on LED
    } else if (rx_char == '0') {
        P2OUT &= ~LED;            // Turn off LED
    } else if (rx_char == '3') {
               P2OUT |= Pump_water;            // Turn on Motor
               __delay_cycles(2000);
    } else if (rx_char == '2') {
                 P2OUT &= ~Pump_water;            // Turn off Motor
               __delay_cycles(2000);
    } else if (rx_char == '5') {
               P2OUT |= FAN;            // Turn on Fan
    } else if (rx_char == '4') {
                 P2OUT &= ~FAN;            // Turn off Fan
    }
}
    if (rx_char == '6') {
    isAuto = 1;
    } else if (rx_char == '7') {
    isAuto = 0;
    }
}


//int to char
void itoa(int value, char *str, int base)
{
    sprintf(str, "%d", value);
}

//float to char
void itoa_float(float value, char *str) {
    int int_part = (int)value;
        float decimal_part = value - (float)int_part;

        char int_str[10];
        itoa(int_part, int_str, 10);

        int decimal_int = (int)(decimal_part * 1000);

        char decimal_str[10];
        itoa(decimal_int, decimal_str, 10);

        strcpy(str, int_str);
        strcat(str, ".");
        strcat(str, decimal_str);
}


//function send data over UART
void ser_output(char *str)
{
    while (*str != 0)
    {
       while (!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = *str++;
    }
}

void uartSendString(char* str) {
    while(*str) {
        while (!(IFG2 & UCA0TXIFG)); // Wait for TX buffer to be ready
        UCA0TXBUF = *str;            // Send character
        str++;                       // Move to next character
    }
}

//setup ADC
void initADC(void){
    ADC10AE0 = light_sensor; // Enable analog signal to A2
    ADC10CTL0 |= ADC10SHT_2; // 16 ADC10CLK cycles
    ADC10CTL0 |= ADC10ON; // Turn ADC10 on
    ADC10CTL1 |= ADC10SSEL_2; // ADC10 clock source = SMCLK
    ADC10CTL1 |= INCH_4; // ADC10 input channel = A2
}
//setup UART
void initUART(void){
    // Configure the USCI_A0 module for UART mode
    UCA0CTL1 |= UCSSEL_2;   // Use SMCLK as the clock source
    UCA0BR0 = 104;          // 1MHz 9600 (lower byte)
    UCA0BR1 = 0;            // 1MHz 9600 (upper byte)
    UCA0MCTL = UCBRS0;      // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;   // Initialize USCI state machine

    // Enable USCI_A0 RX interrupt
    IE2 |= UCA0RXIE;
}

void initsoilADC(void){
    ADC10AE0 = soil_sensor; // Enable analog signal to A2
    ADC10CTL0 |= ADC10SHT_2; // 16 ADC10CLK cycles
    ADC10CTL0 |= ADC10ON; // Turn ADC10 on
    ADC10CTL1 |= ADC10SSEL_2; // ADC10 clock source = SMCLK
    ADC10CTL1 |= INCH_5; // ADC10 input channel = A0
}
