// CpE 213 Project 2
//
// Robert Higgins

#include <whatever>


//#define PLAY_NN(delay) play_tone(delay, TH, TL)
#define PLAY_G5(delay) play_tone(delay, 0xB6, 0x87)
#define PLAY_A5(delay) play_tone(delay, 0xBE, 0x8B)
#define PLAY_B5(delay) play_tone(delay, 0xC5, 0xB3)
#define PLAY_C6(delay) play_tone(delay, 0xC9, 0x0A)
#define PLAY_E6(delay) play_tone(delay, 0xD4, 0x5D)
#define PLAY_BREATH(delay) play_tone(1, 0x00, 0x00)


//playBreath:
//0x00  
//0x00
//playREST:
//0x00   
//0x00


// Switches 0-8
// Switch/LED layout: 
//  0   1   2  
//  3   4   5
//  6   7   8
sbit sw0 = P2 ^ 0;
sbit sw1 = P0 ^ 1;
sbit sw2 = P2 ^ 3;
sbit sw3 = P0 ^ 2;
sbit sw4 = P1 ^ 6;
sbit sw5 = P0 ^ 0;
sbit sw6 = P2 ^ 1;
sbit sw7 = P0 ^ 3;
sbit sw8 = P2 ^ 2;

// LEDs 0-8
sbit led0 = P2 ^ 4;
sbit led1 = P0 ^ 5;
sbit led2 = P2 ^ 7;
sbit led3 = P0 ^ 6;
sbit led4 = P1 ^ 5;
sbit led5 = P0 ^ 4;
sbit led6 = P2 ^ 5;
sbit led7 = P0 ^ 7;
sbit led8 = P2 ^ 6;

sbit spkr = P1 ^ 7;

char SNDH, SNDL;    // High and low bytes of pitch generated with Timer 0
int DELAY_REPS;     // 16 bit int for longer delays (measurable in seconds)

char MSG1[] = "Hello, World!\0";
char * MSG = MSG1;

// Jump to Main/ISR functions as needed
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//   To that of certain notes in the playN# routines
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void tim0_isr() interrupt 1
{
    spkr = ~spkr;
    TH0 = SNDH;     // Since it is in non-resetting
    TL0 = SNDL;
    TR0 = 1;
    return;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Interrupt Service Routine for timer 1
//   Set to delay ~0.017 seconds per CYCLE.  Counts down, complementing
//   the LEDS to create a flash effect every 0.017 * R6 seconds. This
//   happens R7 times.
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void tim1_isr() interrupt 2
{
    if (DELAY_REPS == 0)
    {
        //ACTION! otherwise turn off speaker
        // Done with action, stop timer 1 and 0
        TCON &= ~0x50;
    }
    else
    {
        // ONE MORE TIME! Delay for.... A while
        TH1 = 0x00;
        TL1 = 0x01;
        TCON &= 0x50;   // Ensure timers are on
        DELAY_REPS -= 1;
    }
    return;
}

void serial_isr() interrupt 4
{
    if (TI)
    {
        //transmit
        TI = 0;
        SBUF = *MSG;
        MSG++;
        if (*MSG == '\0')
        {
            MSG = MSG1;
            TI = 1;
        }
    }
    else if (RI)
    {
        //recieve
        RI = 0;
    }
    return;
}

  
void interrupt_init(void)
{
    // Configure timers and 
    IEN0 = 0x8A
    TCON = 0x03
    TMOD = 0x11
    
    // configure UART 
    PCON &= ~0x40; 
    SCON = 0x50; 
    // set or clear SMOD1 
    PCON &= 0x7F; 
    PCON |= (0 << 8);
    SSTAT = 0x00; 
    // enable break detect 
    AUXR1 |= 0x40; 
    // configure baud rate generator 
    BRGCON = 0x00; 
    BRGR0 = 0xF0; 
    BRGR1 = 0x02; 
    BRGCON = 0x03;
    ES = 1;
}


//;//Static routine to play La Cucaracha!
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// playTune:
//   Play the notes from La Cucaracha in order
//   16 => eigth
//   32 => quarter
//   Multiples of these can be extrapolated such as dotted notes
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov R0, #16
    acall playG5
    acall   playBreath
    mov R0, #16
    acall   playG5
    acall   playBreath
    mov R0, #16
    acall   playG5
    acall   playBreath
    mov R0, #48
    acall   playC6
    acall   playBreath
    mov R0, #32
    acall   playE6
    acall   playBreath
    mov R0, #16
    acall   playG5
    acall   playBreath
    mov R0, #16
    acall   playG5
    acall   playBreath
    mov R0, #16
    acall   playG5
    acall   playBreath
    mov R0, #48
    acall   playC6
    acall   playBreath
    mov R0, #64
    acall   playE6
    acall   playBreath
    mov R0, #32
    acall   playREST 
    mov R0, #16
    acall   playC6
    acall   playBreath
    mov R0, #16
    acall   playC6
    acall   playBreath
    mov R0, #16
    acall   playB5
    acall   playBreath
    mov R0, #16
    acall   playB5
    acall   playBreath
    mov R0, #16
    acall   playA5
    acall   playBreath
    mov R0, #16
    acall   playA5
    acall   playBreath
    mov R0, #64
    acall   playG5
    acall   playBreath
    mov R0, #16
    acall playREST
    clr TR0
    clr C	//Set sentinel bit
ret

// play_tone
// plays tone for duration * 0.0017 seconds...maybe?
void play_tone(char duration, char delay_h, char delay_l)
{
    SNDH = delay_h;
    SNDL = delay_l;
    TH0=delay_h;
    TL1=delay_l;
    

    return;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void main(void)
{
    //Set all ports used to bidirectional
    p0m1 = 0;
    p1m1 = 0;
    p2m1 = 0;

    //Initialize all LEDs/speaker off
    P0 = 0xFF;
    P2 = 0xFF;
    P1 ^ 6 = 0x1;
    P1 ^ 7 = 0x0;

    playtune();
    playgame();

    return;
}
