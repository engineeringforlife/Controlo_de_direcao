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

volatile unsigned char emerg_flag = 0;
volatile unsigned char start_flag = 0;
volatile unsigned char write_flag = 0; //ativada para escrita de start/ stop(apenas 1 vez))
volatile unsigned char flag_CNT1 = 0;
//variavel a ser alterada em fun��o de uma entrada PWM a ser analizada
volatile unsigned int angulo_dir = 0;
int valor = 0;
char adc_flag = 0;
int my_duty = 0, my_duty_aux = 0;


//volatile int valor ;
/*Fun��o chamda por Extern_ISR, coloca vari�veis auxiliares a on para escrita no display
Gere o bot�o de emerg�ncia. Se emerg�ncia ativada, ent�o buzzer ON(PWM no pino RC2)*/
void emrgencia(void) {
    if (INTCON2bits.INTEDG0 == 0) //flanco descendente(carrega no bot�o)
    {
        emerg_flag = 1; //a emergencia foi ativada
        start_flag = 0; //desliga o sistema
        INTCON2bits.INTEDG0 = 1; //muda configura��o de flanco
        TMR2_StartTimer();

    } else //flanco ascendente(larga o bot�o)
    {
        emerg_flag = 0; //a emergencia foi desativada
        INTCON2bits.INTEDG0 = 0; //muda configura��o de flanco  
        TMR2_StopTimer();
    }
    write_flag = 1;

}
//Fun��o chamda por Extern_ISR, coloca vari�veis auxiliares a on para escrita no display
void start(void) {
    if (start_flag == 1) {
        start_flag = 0; //sistema off
    } else if (start_flag == 0) {
        start_flag = 1; //sistema on
    }
    write_flag = 1;

}
//Fun��o utiliza o periodo do timer 1

void convert_timer(void) {
    ADC_StartConversion();
}

//Fun��o chamada a cada 21 ms. define o periodo do PWM. Utiliza o timer 3
//Para uma varia��o de 0 a 180� considere.se uma varia��o de 3000 a 6000 no registo TMR3
//Falta criar o algoritmo de deslocamento. Aletera apos conversa com prof Luis conde

void Tpwm(void) {


    LATCbits.LATC2 = 1;
    my_duty_aux++;
    if (my_duty_aux > 500) {
        my_duty_aux = 0;

        if (my_duty > 0)
            my_duty = 0;
        else
            my_duty = 3000;
        /*
        my_duty++;
        if (my_duty > 3000)
            my_duty = 0;
         */
         ECCP1_SetCompareCount(3000 + my_duty);
    }

}

void CGRamAddr0(void) {
    while (BusyXLCD());
    /*
     * Endere�amento da mem�ria CGRAM:
     * Primeira linha do 1� caracter,
     * �ndice 0 = caracter 0 * 8 bytes por caracter
     */

    SetCGRamAddr(CARACTER_C);
    while (BusyXLCD());
    /*
     * Defini��o do conte�do do caracter, linha a linha
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

void main(void) {
    // Initialize the device
    SYSTEM_Initialize();

    TMR2_StopTimer();


    char name1[21] = "DAVID DRUMOND";
    char name2[21] = "ADEOREL BANDEIRA";
    // char project_title1[25] = "SISTEMA DE CONTROLO DE";
    // char project_title2[40]= "UM PROCESSO DE INJECCAO";
    int i;
    int j;
    //
    uint8_t rxData;
    int temperatura = 20;
    int menu = 1;
    char mostra_menu = 1;
    char caracter_recebido = 0;

    //Fica a duvida de as configura��es devem estar antes ou depois da apresenta��o
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
    TMR3_SetInterruptHandler(Tpwm);
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
    
    
    
    
    
    
    
    
    
    //Inicializa��o do LCD:

    OpenXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(DON & CURSOR_OFF & BLINK_OFF);
    while (BusyXLCD());
    WriteCmdXLCD(SHIFT_CUR_RIGHT);
    while (BusyXLCD());
    WriteCmdXLCD(CLEAR_LCD);
    while (BusyXLCD());
    //carrega o caracter especial � 
    CGRamAddr0();
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
    putsXLCD("DIREC");
    while (BusyXLCD());
    putcXLCD(CARACTER_C);
    while (BusyXLCD());
    putsXLCD("AO");

    for (i = 0; i < 10; i++) {

        WriteCmdXLCD(SHIFT_DISP_LEFT);
        while (BusyXLCD());
        for (j = 0; j < 100; j++)
            __delay_ms(1);
    }
    while (BusyXLCD());
    WriteCmdXLCD(CLEAR_LCD); //clear LCD



    while (1) {
        if (write_flag) {
            write_flag = 0;

            while (BusyXLCD());
            if (emerg_flag == 0) {
                if (start_flag == 0) {
                    WriteCmdXLCD(LINE1CLUN1);
                    putsXLCD("SISTEMA OFF             ");

                } else {
                    WriteCmdXLCD(0b10000000); //primeira linha/primeira coluna
                    putsXLCD("SISTEMA ON               ");
                }
            } else {
                WriteCmdXLCD(0b10000000); //primeira linha/primeira coluna
                while (BusyXLCD());
                putsXLCD("EMERGENCIA ATIVADA            ");
            }

        }
        //Atualiza os valores do ADC
        if (adc_flag == 1) {
            float angulo = 0.1759530792 * valor;
            char valor_analog[50];
            sprintf(valor_analog, "Deslocamento= %.2f   ", angulo);
            WriteCmdXLCD(LINE2CLUN1);
            while (BusyXLCD());

            putsXLCD(valor_analog);
            adc_flag = 0;
        }
        
        
                // Ler caracter recebido (se tiver disponivel)
        if (EUSART1_is_rx_ready()) {
            rxData = EUSART1_Read();
            EUSART1_Write(rxData); // Mostra caracter recebido devolvendo-o para EUSART
            caracter_recebido = 1; // Indica caracter recebido disponivel em rxData
        }
        // Apresenta menu ou sub-menu
        switch (menu) {
            case 1:
                if (mostra_menu == 1) {
                    printf("\r\n1 - Visualizar o deslocamento atual");
                    printf("\r\n2 - Visualizar o tempo de estabelecimento das 15 ultimas opera��es");
                    printf("\r\n3 - Visualizar erros de offset das 15 ultimas opera��es");
                    printf("\r\n4 - Alterar os par�metros do controlador");
                    printf("\r\n5 - Ajustar os valores maximo e minimo de deslocamento dos servo");
                    
                    printf("\r\nOpcao: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido � v�lido
                    // Convers�o do c�digo ASCII para inteiro: rxData - 48
                    menu = 10 + (rxData - 48);
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 11:
                if (mostra_menu == 1) {
                    printf("\r\n1 - 10 graus");
                    printf("\r\n2 - 15 graus");
                    printf("\r\n3 - 20 graus");
                    printf("\r\n4 - 25 graus");
                    printf("\r\n5 - 30 graus");
                    printf("\r\nX - Recuar");
                    printf("\r\nOpcao: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido � v�lido
                    if (!(rxData == 'x') && !(rxData == 'X')) {
                        // Convers�o do c�digo ASCII para inteiro: rxData - 48
                        temperatura = 5 + (rxData - 48) * 5;
                    }
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 12:
                if (mostra_menu == 1) {
                    printf("\r\nTemperatura atual = %d", temperatura);
                    printf("\r\nPrima uma tecla... ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 13:
                 if (mostra_menu == 1) {
                    printf("\r\nTemperatura atual = %d", temperatura);
                    printf("\r\nPrima uma tecla... ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 14:
                 if (mostra_menu == 1) {
                    printf(" Introduza os valores de KP, KI, KD");
                    printf("\r\nPrima uma tecla... ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 15:
                 if (mostra_menu == 1) {
                    printf("\r\nTemperatura atual = %d", temperatura);
                    printf("\r\nPrima uma tecla... ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
                
                
            default:
                printf("\r\nOpcao errada\r\n");
                menu = 1;
                mostra_menu = 1;
                break;
        
        }




    }
}


/*
 void main(void) {
    uint8_t rxData;
    int temperatura = 20;
    int menu = 1;
    char mostra_menu = 1;
    char caracter_recebido = 0;

    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    while (1) {
        // Ler caracter recebido (se tiver disponivel)
        if (EUSART1_is_rx_ready()) {
            rxData = EUSART1_Read();
            EUSART1_Write(rxData); // Mostra caracter recebido devolvendo-o para EUSART
            caracter_recebido = 1; // Indica caracter recebido disponivel em rxData
        }
        // Apresenta menu ou sub-menu
        switch (menu) {
            case 1:
                if (mostra_menu == 1) {
                    printf("\r\n1 - Inserir temperatura desejada");
                    printf("\r\n2 - Mostrar temperatura atual");
                    printf("\r\nOpcao: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido � v�lido
                    // Convers�o do c�digo ASCII para inteiro: rxData - 48
                    menu = 10 + (rxData - 48);
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 11:
                if (mostra_menu == 1) {
                    printf("\r\n1 - 10 graus");
                    printf("\r\n2 - 15 graus");
                    printf("\r\n3 - 20 graus");
                    printf("\r\n4 - 25 graus");
                    printf("\r\n5 - 30 graus");
                    printf("\r\nX - Recuar");
                    printf("\r\nOpcao: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido � v�lido
                    if (!(rxData == 'x') && !(rxData == 'X')) {
                        // Convers�o do c�digo ASCII para inteiro: rxData - 48
                        temperatura = 5 + (rxData - 48) * 5;
                    }
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 12:
                if (mostra_menu == 1) {
                    printf("\r\nTemperatura atual = %d", temperatura);
                    printf("\r\nPrima uma tecla... ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            default:
                printf("\r\nOpcao errada\r\n");
                menu = 1;
                mostra_menu = 1;
                break;
        }
    }
}
/**
 End of File
 */
 


