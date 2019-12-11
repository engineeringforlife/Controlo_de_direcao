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
//variavel a ser alterada em função de uma entrada PWM a ser analizada
volatile unsigned int angulo_dir = 0;
int valor = 0;
char adc_flag = 0;
int my_duty = 0, my_duty_aux = 0;



//volatile int valor ;
/*Função chamda por Extern_ISR, coloca variàveis auxiliares a on para escrita no display
Gere o botão de emergência. Se emergência ativada, então buzzer ON(PWM no pino RC2)*/
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
//Função chamda por Extern_ISR, coloca variàveis auxiliares a on para escrita no display
void start(void) {
    if (start_flag == 1) {
        start_flag = 0; //sistema off
    } else if (start_flag == 0) {
        start_flag = 1; //sistema on
    }
    write_flag = 1;

}
//Função utiliza o periodo do timer 1

void convert_timer(void) {
    ADC_StartConversion();
}

//Função chamada a cada 21 ms. define o periodo do PWM. Utiliza o timer 3
//Para uma variação de 0 a 180º considere.se uma variação de 3000 a 6000 no registo TMR3
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
    char op;
    float KP=0.0, KI=0.0, KD=0.0;
    char V_KP[10] ={};
    char V_KI[10] ={};
    char V_KD[10] ={};
    int index=0;
    
    float angulo;
    char valor_analog[50];
    //Fica a duvida de as configurações devem estar antes ou depois da apresentação
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
    
    
    
    
    
    
    
    
    
    //Inicialização do LCD:

    OpenXLCD(FOUR_BIT & LINES_5X7);
    while (BusyXLCD());
    WriteCmdXLCD(DON & CURSOR_OFF & BLINK_OFF);
    while (BusyXLCD());
    WriteCmdXLCD(SHIFT_CUR_RIGHT);
    while (BusyXLCD());
    WriteCmdXLCD(CLEAR_LCD);
    while (BusyXLCD());
    //carrega o caracter especial Ç 
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
            angulo = (0.003515625 * valor * 50) - 90;
            valor_analog[50];
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
                    printf("\r\n2 - Visualizar o tempo de estabelecimento das 15 ultimas operações");
                    printf("\r\n3 - Visualizar erros de offset das 15 ultimas operações");
                    printf("\r\n4 - Alterar os parâmetros do controlador");
                    printf("\r\n5 - Ajustar os valores maximo e minimo de deslocamento dos servo");
                    
                    printf("\r\nOpcao: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido é válido
                    // Conversão do código ASCII para inteiro: rxData - 48
                    menu = 10 + (rxData - 48);
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                }
                break;
            case 11:
                if (mostra_menu == 1) {
                    printf("\r\nPara sair pressione ENTER");       
                    mostra_menu = 0;
                }
                printf("\r\nAngulo atual: %.2f \tSetpoint: %.2f", angulo, 0.06*my_duty-90);
                if (caracter_recebido == 1) {
                    // TODO: verificar que caracter recebido é válido
                    if (rxData == 13) {
                                            menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    }
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
                    printf(" Os valores atuais de KP, KI, KD sao os seguintes:");
                    printf("\r\n\t1-KP= %.2f", KP);
                    printf("\r\n\t2-KI= %.2f", KI);
                    printf("\r\n\t3-KD= %.2f", KD);
                    printf("\r\nQual a variavel que pretende alterar?(1,2ou 3?)");
                    printf("\r\n\tPara sair pressione qualquer techa");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    menu = 20 + (rxData - 48);
                }
                break;
            case 15:
                if (mostra_menu == 1) {
                    printf("Indique o valor máximo simétrico de deslocamento: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    V_KP[index] = rxData;
                    index++;

                    if (rxData == 13) { //enter pressionado
                        V_KP[index] = '\0';
                        KP = atof(V_KP);
                        menu = 14; //volta ao menu anterior
                        mostra_menu = 1;

                        index = 0;
                    }
                    caracter_recebido = 0;
                }
                break;
            case 21:
                if (mostra_menu == 1) {
                    printf("Introduza o novo valor de KP: ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    V_KP[index] = rxData;
                    index++;

                    if (rxData == 13) { //enter pressionado
                        V_KP[index] = '\0';
                        KP = atof(V_KP);
                        menu = 14; //volta ao menu anterior
                        mostra_menu = 1;
                        index = 0;
                    }
                    caracter_recebido = 0;
                }
                    break;
                    case 22:
                    if (mostra_menu == 1) {
                        printf("Introduza o novo valor de KI: ");
                        mostra_menu = 0;
                    }
                    if (caracter_recebido == 1) {
                        V_KI[index] = rxData;
                        index++;

                        if (rxData == 13) { //enter pressionado
                            V_KI[index] = '\0';
                            KI = atof(V_KI);
                            menu = 14; //volta ao menu anterior
                            mostra_menu = 1;

                            index = 0;
                        }
                        caracter_recebido = 0;
                    }
                        break;
                        case 23:
                            if (mostra_menu == 1) {
                        printf("Introduza o novo valor de KD: ");
                        mostra_menu = 0;
                    }
                    if (caracter_recebido == 1) {
                        V_KD[index] = rxData;
                        index++;

                        if (rxData == 13) { //enter pressionado
                            V_KD[index] = '\0';
                            KD = atof(V_KD);
                            menu = 14; //volta ao menu anterior
                            mostra_menu = 1;

                            index = 0;
                        }
                        caracter_recebido = 0;
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
                    // TODO: verificar que caracter recebido é válido
                    // Conversão do código ASCII para inteiro: rxData - 48
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
                    // TODO: verificar que caracter recebido é válido
                    if (!(rxData == 'x') && !(rxData == 'X')) {
                        // Conversão do código ASCII para inteiro: rxData - 48
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
 


