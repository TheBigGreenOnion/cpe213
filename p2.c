// CpE 213 Project 2
//
// Robert Higgins

#include <whatever>
//#define PLAY_NN(delay) play_tone(delay, TH, TL)
// Global variables for timers
char SNDH, SNDL;

#define PLAY_G5(delay) play_tone(delay, 0xB6, 0x87)
#define PLAY_A5(delay) play_tone(delay, 0xBE, 0x8B)
#define PLAY_B5(delay) play_tone(delay, 0xC5, 0xB3)
#define PLAY_C6(delay) play_tone(delay, 0xC9, 0x0A)
#define PLAY_E6(delay) play_tone(delay, 0xD4, 0x5D)

//playBreath:
//0x00  
//0x00
//playREST:
//0x00   
//0x00


clr TR0     
acall stall
setb TR0    

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

// Jump to Main/ISR functions as needed
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//   To that of certain notes in the playN# routines
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void tim0_isr() interrupt 1
{
    spkr = ~spkr;
    TH0 = SNDH;
    TL0 = SNDL;
    TR1 = 1;
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
    djnz R6, AGAIN 
    ACTION:             //Decrement when something actually happens
    	djnz R7, CYCLE  //otherwise just refresh R6
	clr TR1             //Turn timer off when done flashing
    reti

    CYCLE:
        mov R6, #100    //About 200 ms per flash on/off
        xrl A, #0xFF
        mov P0, A
        mov P2, A
        cpl P1.6      
    AGAIN:      
        mov TH1, #0x0b	//0.0017s
        mov TL1, #0x32
        
	setb TR1
    reti
}

void serial_isr() interrupt 4
{
    if (TI)
    {
        //transmit
        TI = 0;
    }
    else if (RI)
    {
        //recieve
        RI = 0;
    }
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

//  Phillips 8051 has special onboard timer for scon, so t1 and t0 are free
//  t0 controls frequency for tones
//  t1 controls duration

    //Initialize all LEDs/speaker off
    P0 = 0xFF;
    P2 = 0xFF;
    P1 ^ 6 = 0x1;
    P1 ^ 7 = 0x0;

    mov IEN0, #0x8A
    mov TCON, #0x11
    
    setb TR1
    setb TR0

    ;;//Flash LEDS 5 times in ~2 s
    mov R7, #11     //5 * 2 + 1 for turn off
    mov R6, #100
    acall flash

    ;;//Tune stuff
    setb c          //Set C as sentinel to indicate music is playing
    acall playTune
    
void interrupt_init(void)
{
    //serial setup
    PCON &= derp;
    SCON = derp;
    SSTAT = derp;

    // configure UART 
    // clear SMOD0 
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

}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Beep function
//   Set timer and play quarternote beep to indicate error
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
beep:	    
    setb TR0
    mov R0, #32
    acall playB5 
    acall playBreath
    clr TR0
ret

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Flash:
//   R6/R7 T=0.0017*R6   R7=iterations
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
flash:       
    mov TH1, #0x0b
    mov TL1, #0x32
    setb TR1
ret

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// update LEDS
//   Flip the bits of A for active low and use rotate to pick off the
//   bits shifted down to LSB
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;//  
updateleds:
    mov R0, A
    cpl A
    rrc A
    mov P0.4, C
    rrc A
    mov P2.7, C
    rrc A
    mov P0.5, C
    rrc A
    mov P2.4, C
    mov A, R0
    clr C
ret


//;//Static routine to play La Cucaracha!
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// playTune:
//   Play the notes from La Cucaracha in order
//   16 => eigth
//   32 => quarter
//   Multiples of these can be extrapolated such as dotted notes
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
playTune:	    
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


