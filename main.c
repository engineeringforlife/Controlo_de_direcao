/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.77
        Device            :  PIC18F45K22
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include "xlcd.h"
#include <stdio.h>


/*
                         Main application
 */

        volatile unsigned char emerg_flag=0;
        volatile unsigned char start_flag=0;
        volatile unsigned char write_flag=0;    //ativada para escrita de start/ stop(apenas 1 vez))
        volatile unsigned char flag_CNT1=0;
        int valor=0;
        char adc_flag=0;
        uint16_t k=0;
        
        //volatile int valor ;

void emrgencia(void) {
    if (INTCON2bits.INTEDG0 == 0) //flanco descendente(carrega no botão)
    {
        emerg_flag = 1; //a emergencia foi ativada
        start_flag = 0; //desliga o sistema
        INTCON2bits.INTEDG0 = 1; //muda configuração de flanco
        TMR2_StartTimer();
        
    } else //flanco ascendente(larga o botão)
    {
        emerg_flag = 0; //a emergencia foi desativada
        INTCON2bits.INTEDG0 = 0; //muda configuração de flanco  
        TMR2_StopTimer();
    }
    write_flag = 1;

}

void start(void) {
 
        if (start_flag == 1) {
            start_flag = 0; //sistema off
        } else if (start_flag == 0) {
            start_flag = 1; //sistema on
        }
        write_flag = 1;

}

void convert_timer (void){

    ADC_StartConversion();

}

void CGRamAddr0(void)
{
     while (BusyXLCD());
    /*
     * Endereçamento da memória CGRAM:
     * Primeira linha do 1º caracter,
     * índice 0 = caracter 0 * 8 bytes por caracter
     */

    SetCGRamAddr(CARACTER_C);
    while (BusyXLCD());
    /*
     * Definição do conteúdo do caracter, linha a linha
     */
    WriteDataXLCD(0b00000111); // linha 0
    while (BusyXLCD());
    WriteDataXLCD(0b00001000); // linha 1
    while (BusyXLCD());
    WriteDataXLCD(0b00010000); // linha 2
    while (BusyXLCD());
    WriteDataXLCD(0b00010000); // linha 3
    while (BusyXLCD());
    WriteDataXLCD(0b00010000); // linha 4
    while (BusyXLCD());
    WriteDataXLCD(0b00001000); // linha 5
    while (BusyXLCD());
    WriteDataXLCD(0b00010111); // linha 6
    while (BusyXLCD());
    WriteDataXLCD(0b00000000); // linha 7
    while (BusyXLCD());
}

        

void main(void)
{
        // Initialize the device
        SYSTEM_Initialize();
        
        TMR2_StopTimer();

    
        char name1[21] = "DAVID DRUMOND";
        char name2[21] = "ADEOREL BANDEIRA";
       // char project_title1[25] = "SISTEMA DE CONTROLO DE";
       // char project_title2[40]= "UM PROCESSO DE INJECCAO";
        int i;
        int j;

        
               
    //Inicialização do LCD:

    
    OpenXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(DON & CURSOR_OFF & BLINK_OFF);
    while (BusyXLCD());
    WriteCmdXLCD(SHIFT_CUR_RIGHT);
    while (BusyXLCD());
    WriteCmdXLCD(CLEAR_LCD);
    while (BusyXLCD());
    CGRamAddr0();   //carrega o caracter especial Ç
    
    WriteCmdXLCD(LINE1CLUN1);
    while (BusyXLCD());
    putsXLCD(name1);
    while (BusyXLCD());
    WriteCmdXLCD(LINE2CLUN1);
    while (BusyXLCD()); 
    putsXLCD(name2);
    __delay_ms(500);  

    WriteCmdXLCD(CLEAR_LCD);
    while (BusyXLCD());

    WriteCmdXLCD(LINE1CLUN1);
    putsXLCD("SISTEMA DE CONTROLO DE");

    while (BusyXLCD());
    WriteCmdXLCD(LINE2CLUN1);
    putsXLCD("UM PROCESSO DE INJE");
    while (BusyXLCD());
     putcXLCD(CARACTER_C);
     while (BusyXLCD());
     putsXLCD("AO");

    for (i = 0; i < 10 ; i++ ) {

    WriteCmdXLCD(SHIFT_DISP_LEFT);
    while (BusyXLCD());
    for (j = 0; j < 100 ; j++ )
     __delay_ms(1);
    }       
            while (BusyXLCD());
            WriteCmdXLCD(CLEAR_LCD);//clear LCD
            
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable high priority global interrupts
    INTERRUPT_GlobalInterruptHighEnable();
    
    // Enable low priority global interrupts.
    INTERRUPT_GlobalInterruptLowEnable();

    INT1_SetInterruptHandler(start); 
    INT0_SetInterruptHandler(emrgencia);
    
    TMR1_SetInterruptHandler(convert_timer);
    
    //  ADC_SelectChannel(0);
    //  ADC_StartConversion();
            
    // Disable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighDisable();

    // Disable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowDisable();

    // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    while (1) {
        if (write_flag) {
            write_flag = 0;
            
            while (BusyXLCD());
            if (emerg_flag == 0) {
                if (start_flag == 0) {
                    WriteCmdXLCD(LINE1CLUN1);
                    putsXLCD("SISTEMA OFF             ");

                }
                else {
                    WriteCmdXLCD(0b10000000); //primeira linha/primeira coluna
                    putsXLCD("SISTEMA ON               ");
                }
            }
            else {
                WriteCmdXLCD(0b10000000); //primeira linha/primeira coluna
                while (BusyXLCD());
                putsXLCD("EMERGENCIA ATIVADA            ");
            }

        }

        if (adc_flag == 1) {
            float angulo= 0.1759530792*valor;
            char valor_analog[5];
            sprintf(valor_analog,"Deslocamento= %.2f   ", angulo);
            WriteCmdXLCD(LINE2CLUN1);
            while (BusyXLCD());
            
            putsXLCD(valor_analog);
            adc_flag=0;
        }
        

       
    }
}


