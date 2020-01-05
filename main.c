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

#define ISUM_MAX 10
#define ISUM_MIN -10


/*
                         Main application
 */

volatile unsigned char emerg_flag = 0;
volatile unsigned char start_flag = 0;
volatile unsigned char write_flag = 0; //ativada para escrita de start/ stop(apenas 1 vez))
volatile unsigned char flag_CNT1 = 0;
volatile unsigned char cnt_01 = 0;

//variavel a ser alterada em função de uma entrada PWM a ser analizada
volatile unsigned int angulo_dir = 0;
int valor = 0;
char adc_flag = 0;
int my_duty = 0, my_duty_aux = 0;
float erro_atual, erro_anterior;
float setpoint;
float pTerm, iTerm, dTerm, iSum;
float angulo;
float KP=0.0, KI=0.0, KD=0.0;
uint8_t V_KP[10] ={};
uint8_t V_KI[10] ={};
uint8_t V_KD[10] ={};
char V_Setpoint[10] = {};
uint8_t aux[2];
I2C1_MESSAGE_STATUS status;



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
    write_flag = 1;}

/** FUNCAO:  Convert Timer ***********************************************
* DESCRICAO: Função que utiliza o período do Timmer1 para iniciar conversão. Define-se assim a taxa de amostragem.
******************************************************************/
void convert_timer(void) {
    ADC_StartConversion();  
}
/** FUNCAO:  Aux Timer ***********************************************
* DESCRICAO: Serve de temporizador auxiliar para fazer temporizações de baixo rigor. O seu período é de 100ms.
******************************************************************/
void aux_Timer (void){
    cnt_01++;
}



/** FUNCAO:     Tpwm ***********************************************
* DESCRICAO:    Utiliza o timer 3 e define o período do PWM de controlo do Servo. Tem um período aprox 21 ms.
 *              Para uma variação de 0 a 180º considere.se uma variação de 3000 a 6000 no registo TMR3
******************************************************************/
void Tpwm(void) {
    LATCbits.LATC2 = 1;                         //coloca a saída a 1;
    ECCP1_SetCompareCount(3000 + my_duty);      //configura o dutty cycle
    }


/** FUNCAO:     CGRamAddr0 ***********************************************
* DESCRICAO:    Define o caratcter especial "Ç"
******************************************************************/
void CGRamAddr0(void) {
    while (BusyXLCD());
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

/** FUNCAO:     main ***********************************************
* DESCRICAO:    Contém do todo o código necessário à execução do programa
******************************************************************/
void main(void) {

    SYSTEM_Initialize();
    TMR2_StopTimer();

    char name1[21] = "DAVID DRUMOND";
    char name2[21] = "ADEOREL BANDEIRA";
    int i;
    int j;
    int l=0;  
    int c=0;
    
    uint8_t rxData;
    int temperatura = 20;
    int menu = 1;
    char mostra_menu = 1;
    char caracter_recebido = 0;
    char op;

    int index=0;

    char valor_analog[50];
    char valor_setpoint[50];
    float t_estabelecimento[5][3];
    float erro_offset [5];


    // Enable high priority global interrupts
    INTERRUPT_GlobalInterruptHighEnable();

    // Enable low priority global interrupts.
    INTERRUPT_GlobalInterruptLowEnable();

    INT1_SetInterruptHandler(start);
    INT0_SetInterruptHandler(emrgencia);

    TMR1_SetInterruptHandler(convert_timer);
    TMR3_SetInterruptHandler(Tpwm);
    TMR5_SetInterruptHandler(aux_Timer);     //temporizador auxiliar
    TMR5_StopTimer();

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
            __delay_ms(5);
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
        //Atualiza os valores do ADC e altera o dutyy-cycle
        if (adc_flag == 1) {
            angulo = (0.003515625 * valor * 50) - 90;
            valor_analog[50];
            valor_setpoint[50];
            sprintf(valor_setpoint, "Setpoint= %.2f   ", setpoint);
            sprintf(valor_analog, "Deslocamento= %.2f   ", angulo);
            WriteCmdXLCD(LINE1CLUN1);
            while (BusyXLCD());
            putsXLCD(valor_setpoint);           
            WriteCmdXLCD(LINE2CLUN1);
            while (BusyXLCD());
            putsXLCD(valor_analog);
            
         //Controlador PID
        erro_atual=setpoint-angulo;
        pTerm=KP*erro_atual;
        iSum = iSum + erro_atual;
        if (iSum > ISUM_MAX){
            iSum = ISUM_MAX;
        }else
            if(iSum < ISUM_MIN){
                iSum = ISUM_MIN;
            }   
        iTerm=KI*iSum;
        dTerm=KD*(erro_atual-erro_anterior);
        erro_anterior=erro_atual;
        my_duty += (pTerm+iTerm+dTerm); 
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
                    printf("\r\n6 - Deslocar o servo topo a topo");
                    printf("\r\n7 - Alterar o valor do setpoint");
                    printf("\r\n8 - Guardar Dados");
                    printf("\r\n9 - Carregar dados");
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
                printf("\r\nAngulo atual: %.2f \tSetpoint: %.2f", angulo, setpoint);
                if (caracter_recebido == 1) {
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
                    printf("\r\nTempo de estabalecimento das ultimas operacoes");
                    printf("\r\nP.Inicial   P.Final T.Estabelecimento");
                    for (l=0 ; l<5 ; l++){
                        printf("\r\n");
                        printf("%.2f        %.2f    %.2f",t_estabelecimento[l][1], t_estabelecimento[l][2] ,t_estabelecimento[l][3]);
                    }
                    mostra_menu = 0;
                    printf("\r\nPressione ENTER para sair");
                    
                }
                if (caracter_recebido == 1) {
                    if(rxData == 13){
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    l=0;
                    printf("\r\n"); 
                    }
                }
                break;
            case 13:
                 if (mostra_menu == 1) {
                    printf("\r\nErros de ofset das ultimas operacoes");
                    for(l=0 ; l<5 ; l++){
                        printf("%dº- %.2f", l, erro_offset);
                    }
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    l=0;
                }
                break;
            case 14:
                 if (mostra_menu == 1) {
                    printf(" Os valores atuais de KP, KI, KD sao os seguintes:");
                    printf("\r\n\t1-KP= %.5f", KP);
                    printf("\r\n\t2-KI= %.5f", KI);
                    printf("\r\n\t3-KD= %.5f", KD);
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
            case 16:
                if (mostra_menu == 1) {
                    printf("\r\nA Deslocar servo...");
                    TMR5_StartTimer();
                    setpoint =-90;
                    my_duty = 0;
                    mostra_menu = 0;
                }
                if (cnt_01 >= 40) { //espera 2 segundos
                    setpoint = 90;
                    if(cnt_01>=80){
                        setpoint = 0;
                        TMR5_StopTimer();
                        cnt_01=0;
                    }  
                }
                if (caracter_recebido == 1) {
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    menu = 20 + (rxData - 48);
                    TMR5_StopTimer();
                    cnt_01=0;
                }
                break;
            case 17:
                if (mostra_menu == 1) {
                    printf("Indique o novo valor de SetPoint ( -90: 90): ");
                    mostra_menu = 0;
                }
                if (caracter_recebido == 1) {
                    V_Setpoint[index] = rxData;
                    index++;
                    if (rxData == 13) { //enter pressionado
                        V_Setpoint[index] = '\0';
                        setpoint = atof(V_Setpoint);
                        menu = 1; //volta ao menu anterior
                        mostra_menu = 1;
                        index = 0;
                    }
                    caracter_recebido = 0;
                }  
                break;
            case 18:
                    if (caracter_recebido == 1) {
                        I2C1_MasterWrite(V_KP, 1, 0, &status);
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    menu = 1;
                }
                break;   
            case 19:
                    if (caracter_recebido == 1) {
                        I2C1_MasterRead(aux, 1, 0, &status);
                    menu = 1;
                    mostra_menu = 1;
                    caracter_recebido = 0;
                    printf("\r\n");
                    menu = 1;
                        printf("o valor de kv e: %d", aux[0]);
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
