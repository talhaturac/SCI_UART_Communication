//#############################################################################
//                                                                            #
//   FILE:   uart_haberlesme  / Talha Turaç Türk                              #
//                                                                            #
//   CODES OF THE STUDY                                                       #
//                                                                            #
//   STUDY NAME : UART Communication                                          #
//   MAIN GOAL  : Transmitting data by asynchronous serial communication      #
//   CONTACT    : tturac.turk@gazi.edu.tr  &  linkedin.com/in/talhaturacturk  #
//                                                                            #
//   This work is an empty project setup for Driverlib development.           #
//   (And F28002X Drivers)                                                    #
//                                                                            #
//   Watch Expression                                                         #
//   LoopCount  => Counts how many times it is transmitted                    #
//                                                                            #
//   Configure desired communication pins                                     #
//   Receiver for GPIO28 (SCIRXDA)                                            #
//   Transmitter for GPIO29 (SCITXDA)                                         #
//                                                                            #
//#############################################################################

/*
 * Libraries
 */
#include "driverlib.h"
#include "device.h"
#include "f28002x_device.h"
#include "f28002x_sci.h"
#include "f28002x_gpio_defines.h"
#include "f28002x_piectrl.h"

/*
 * Functions
 */
void scia_pin_config_init(void);            //  Configure SCI-A pins using GPIO regs.
void scia_echoback_init(void);              //  SCI-A Settings [DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity]
void scia_fifo_init(void);                  //  Initalize the SCI FIFO
void UART_transmit_message(int a);          //  Transmit a character from the SCI
void UART_message(unsigned char *msg);      //  To split character strings one by one and transmit them as characters from SCI
void stop_func(void);                       //  To stop where there is a error in the software.

/*
 * Interrupts
 */
__interrupt void sciaRXFIFOISR(void);       //  SCI-A Receiver Interrupt

/*
 * Defines
 */
#define BUFFER_SIZE 100                     //  Buffer Size-Length

/*
 * Globals
 */
unsigned int LoopCount;                     //  Counts how many times it is transmitted
unsigned char ReceivedChar[BUFFER_SIZE];    //  Received Message Array-Buffer
unsigned char *msg;                         //  Message pointer
int SCIFLG = 0;                             //  Manuel Interrupt Flag
int index = 0;                              //  To control the index of the buffer


/*
 * Main Loop
 */
void main(void)
{

    Device_init();                  //  Step 1. Initialize System Control: Initialize device clock and enable all peripherals [Settings of PLL, WatchDog, enable Peripheral Clocks]
    Device_initGPIO();              //  Step 2. Initalize GPIO: Disable pin locks and enable internal pull-ups.

    scia_pin_config_init();         //  Step 3. Configure SCI-A pins using GPIO registers

    DINT;                           //  Step 4. Clear all interrupts and initialize PIE vector table: Disable CPU interrupts

    Interrupt_initModule();         //  Step 5. Initialize PIE control registers to their default state. The default state is all PIE interrupts disabled and flags are cleared.

    IER = 0x0000;                   //  Step 6. Disable CPU interrupts and clear all CPU interrupt flags:
    IFR = 0x0000;                   //  Step 6. Disable CPU interrupts and clear all CPU interrupt flags:

    Interrupt_initVectorTable();    //  Step 7. Initialize the PIE vector table with pointers to the shell Interrupt. Service Routines (ISR).

    Interrupt_register(INT_SCIA_RX, sciaRXFIFOISR);     //  Step 8. Interrupts that are used in this example are re-mapped to sciaRXFIFOISR

    IER |= 0x0100;                          //  Step 9.  Turn INT9 ON
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;      //  Step 10. PIE Group 9, INT1 [SCIA_RX]
    PieCtrlRegs.PIEIER9.bit.INTx2 = 0;      //  Step 11. PIE Group 9, INT2 [SCIA_TX]

    EnableInterrupts();                     //  Step 12. Enable PIE and global interrupts

    LoopCount = 0;

    scia_fifo_init();      // Initialize the SCI FIFO
    scia_echoback_init();  // Initalize SCI for echoback

    msg = "\r\n\nWelcome UART/SCI Communication!\0";
    UART_message(msg);

    msg = "\r\nYou will enter (send) a character, and the MCU will echo it back!\n\0";
    UART_message(msg);

    msg = "\r\nEnter a array\t\t: \0";
    UART_message(msg);

    for(;;)
    {
        if(SCIFLG==1)
        {
            msg = "\r\nYou sent\t\t: \0";
            UART_message(msg);
            UART_message(ReceivedChar);

            msg = "\r\n\n\n\0";
            UART_message(msg);

            LoopCount++;

            msg = "\r\nEnter a array\t\t: \0";
            UART_message(msg);

            SCIFLG = 0;
        }

        else
        {
            index = 0;
            int k;
            for(k=0 ; k < BUFFER_SIZE ; k++)
            {
                ReceivedChar[k] = '\0';
            }
        }
    }
}


/*
 * SCI-A GPIO Pin Settings
 */
void scia_pin_config_init(void)
{
    EALLOW;

    GPIO_setPinConfig(GPIO_28_SCIA_RX);                             //  GPIO28 is the SCI Rx pin.
    GPIO_setDirectionMode(GPIO_28_SCIA_RX, GPIO_DIR_MODE_IN);       //  Pin is a GPIO input.
    GPIO_setPadConfig(GPIO_28_SCIA_RX, GPIO_PIN_TYPE_STD);          //  Push-pull output or floating input.
    GPIO_setQualificationMode(GPIO_28_SCIA_RX, GPIO_QUAL_ASYNC);    //  This will select asynch (no qualification) for the selected pins.

    GPIO_setPinConfig(GPIO_29_SCIA_TX);                             //  GPIO29 is the SCI Tx pin.
    GPIO_setDirectionMode(GPIO_29_SCIA_TX, GPIO_DIR_MODE_OUT);      //  Pin is a GPIO output.
    GPIO_setPadConfig(GPIO_29_SCIA_TX, GPIO_PIN_TYPE_STD);          //  Push-pull output or floating input.
    GPIO_setQualificationMode(GPIO_29_SCIA_TX, GPIO_QUAL_ASYNC);    //  This will select asynch (no qualification) for the selected pins.

    EDIS;
}


/*
 * SCI-A Receiver Interrupt
 */
__interrupt void sciaRXFIFOISR()
{
    EALLOW;

    ReceivedChar[index] = SciaRegs.SCIRXBUF.all;
    index++;

    //SciaRegs.SCIFFRX.bit.RXFFOVRCLR= 1;                   //  Clear Overflow flag
    //SciaRegs.SCIFFRX.bit.RXFFINTCLR= 1;                   //  Clear Interrupt flag
    //PieCtrlRegs.PIEACK.all |= 0x0100;                     //  Issue PIE ack

    SCIFLG = 1;

    SCI_clearOverflowStatus(SCIA_BASE);                     //  Clear Overflow flag
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);      //  Clear the requested interrupt sources.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);          //  Issue PIE ack

    EDIS;
}


/*
 * SCI-A Settings [DLB, 8-bit word, baud rate 0x000F, default, 1 STOP bit, no parity]
 */
void scia_echoback_init()
{
    EALLOW;

    // Communications control register - SCI Individual Register Bit Definitions
    //
    SciaRegs.SCICCR.bit.SCICHAR         = 0x7;              //  These bits select the SCI character length from one to eight bits. (7h = 8 bits)
    SciaRegs.SCICCR.bit.ADDRIDLE_MODE   = 0x0;              //  SCI multiprocessor mode control bit.
    SciaRegs.SCICCR.bit.LOOPBKENA       = 0x0;              //  Loop Back test mode disable.
    SciaRegs.SCICCR.bit.PARITYENA       = 0x0;              //  SCI parity disable.
    SciaRegs.SCICCR.bit.STOPBITS        = 0x0;              //  SCI number of stop bits. (sets 1 bit)

    // Control register 1
    //
    SciaRegs.SCICTL1.bit.RXENA          = 0x1;              //  SCI receiver enable.
    SciaRegs.SCICTL1.bit.TXENA          = 0x1;              //  SCI transmitter enable.
    SciaRegs.SCICTL1.bit.SLEEP          = 0x0;              //  SCI sleep disable.
    SciaRegs.SCICTL1.bit.TXWAKE         = 0x0;              //  SCI transmitter wake-up method select.
    SciaRegs.SCICTL1.bit.SWRESET        = 0x0;              //  SCI software reset (active low).
    SciaRegs.SCICTL1.bit.RXERRINTENA    = 0x0;              //  SCI receive error interrupt enable.

    // Control register 2
    //
    SciaRegs.SCICTL2.all                = 0x0003;           //  0000 0000 0000 0011
    SciaRegs.SCICTL2.bit.TXINTENA       = 0x1;              //  SCITXBUF-register interrupt enable.
    SciaRegs.SCICTL2.bit.RXBKINTENA     = 0x1;              //  Receiver-buffer/break interrupt enable.

    // 9600 baud @LSPCLK = 100MHz(25 MHz SYSCLK)
    // Formulation : [ SCI BRR = LSPCLK / (SCI BAUD x 8) - 1 ]  via: Technical Guide Manual

    // Baud rate (high) register
    //
    //SciaRegs.SCIHBAUD.bit.BAUD          = 0x0;            //  SCI 16-bit baud selection Registers SCIHBAUD (MSbyte).
    SciaRegs.SCIHBAUD.all = 0x0001;
    SciaRegs.SCILBAUD.all = 0x0044;

    // Baud rate (low) register                             //  LSPCLK=25MHz, Baud=9600, BRR=324,52
    //
    // SciaRegs.SCILBAUD.bit.BAUD         = 0x0144;         //  See SCIHBAUD Detailed Description (Datasheet).

    // Control register 1 (again) - "Relinquish SCI from Reset"
    //
    SciaRegs.SCICTL1.bit.RXENA          = 0x1;              //  SCI receiver enable.
    SciaRegs.SCICTL1.bit.TXENA          = 0x1;              //  SCI transmitter enable.
    SciaRegs.SCICTL1.bit.SLEEP          = 0x0;              //  SCI sleep disable.
    SciaRegs.SCICTL1.bit.TXWAKE         = 0x0;              //  SCI transmitter wake-up method select.
    SciaRegs.SCICTL1.bit.SWRESET        = 0x1;              //  SCI software reset (active low).
    SciaRegs.SCICTL1.bit.RXERRINTENA    = 0x0;              //  SCI receive error interrupt enable.

    EDIS;
}


/*
 * Transmit a character from the SCI
 */
void UART_transmit_message(int a)
{
    EALLOW;

    while (SciaRegs.SCIFFTX.bit.TXFFST != 0)
    {
       // waiting
    }

    SciaRegs.SCITXBUF.all = a;

    EDIS;
}


/*
 * To split character strings one by one and transmit them as characters from SCI
 */
void UART_message(unsigned char *msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        UART_transmit_message(msg[i]);
        i++;
    }
}


/*
 * Initalize the SCI FIFO
 */
void scia_fifo_init()
{
    EALLOW;

    SciaRegs.SCIFFTX.all  = 0xE060;
    SciaRegs.SCIFFRX.all  = 0x2061;
    SciaRegs.SCIFFCT.all  = 0x0;

    SciaRegs.SCIFFTX.bit.TXFIFORESET= 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET= 1;

    EDIS;
}


/*
 * Stop Function
 */
void stop_func(void)
{
    __asm("     ESTOP0");   // Test failed! Stop!
    for (;;);
}

// End of File
