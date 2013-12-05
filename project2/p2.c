// CpE 213 Project 2
// //
// // Robert Higgins & Nick Eggleston
#include <reg932.h>

//#define PLAY_NN(delay) play_tone(delay, TH, TL)
#define PLAY_G5(delay) play_tone(delay, 0xF6, 0xCF)
#define PLAY_A5(delay) play_tone(delay, 0xF7, 0xD1)
#define PLAY_B5(delay) play_tone(delay, 0xF8, 0xB5)
#define PLAY_C5(delay) play_tone(delay, 0xF9, 0x1E)
#define PLAY_E5(delay) play_tone(delay, 0xFA, 0x8A)
#define PLAY_BREAK(delay) play_tone(delay, 0xFF, 0xF0)

#define MAX_LEVELS 10
#define DEFAULT_DELAY 5
#define LOSE_DELAY DEFAULT_DELAY * 2
#define PLAY_WIN(delay) PLAY_G5(delay); PLAY_C5(delay); PLAY_E5(delay); PLAY_BREAK(5);
#define PLAY_LOSE(delay) PLAY_G5(delay); PLAY_BREAK(5); PLAY_G5(delay); PLAY_BREAK(5); PLAY_G5(delay);

//#define readport() (~((P2 & 0x0F) << 4)) & (~(P0 | 0xF0))

// Switches 0-8
// Switch/LED layout:
// 0 1 2
// 3 4 5
// 6 7 8
sbit sw0 = P2 ^ 0;
sbit sw1 = P0 ^ 1;
sbit sw2 = P2 ^ 3;
sbit sw3 = P0 ^ 2;
sbit sw4 = P1 ^ 4;
sbit sw5 = P0 ^ 0;
sbit sw6 = P2 ^ 1;
sbit sw7 = P0 ^ 3;
sbit sw8 = P2 ^ 2;

// LEDs 0-8
sbit led0 = P2 ^ 4;
sbit led1 = P0 ^ 5;
bit led2 = P2 ^ 7;
sbit led3 = P0 ^ 6;
sbit led4 = P1 ^ 6;
sbit led5 = P0 ^ 4;
sbit led6 = P2 ^ 5;
sbit led7 = P0 ^ 7;
sbit led8 = P2 ^ 6;
sbit spkr = P1 ^ 7;

char SNDH, SNDL; // High and low bytes of pitch generated with Timer 0
char ISPLAYING = 0;
int DELAY_REPS; // 16 bit int for longer delays (measurable in seconds)
char SND = 1;	// Enable/Disable sound on delays
char MSG1[] = "Hello, World\0";
char MSGI = 0;
char pat[] = { 1 ,6,3,1,7,1,5,2,6,1};


// Jump to Main/ISR functions as needed
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// To that of certain notes in the playN# routines
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void tim0_isr() interrupt 1
{
   		spkr = ~spkr;

    TH0 = SNDH; // Since it is in non-resetting
    TL0 = SNDL;
    return;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Interrupt Service Routine for timer 1
// Set to delay ~0.017 seconds per CYCLE. Counts down, complementing
// the LEDS to create a flash effect every 0.017 * R6 seconds. This
// happens R7 times.
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void tim1_isr() interrupt 3
{
    if (DELAY_REPS == 0)
    {
        //ACTION! otherwise turn off speaker
        // Done with action, stop timer 1 and 0
        TR1 = 0;
		TR0 = 0;
		ISPLAYING = 0;
    }
    else
    {
        // ONE MORE TIME! Delay for.... A while
        TH1 = 0x00;
        TL1 = 0x01;
        DELAY_REPS += 1;
    }
    return;
}

void serial_isr() interrupt 4
{
	char dout;
    if (TI)
    {
        //transmit
		dout = MSG1[MSGI];
		if (dout == '\0')
		{
			ES=0;
		}
		TI = 0;
		SBUF=dout;
		MSGI++;
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
    IEN0 = 0x8A;
    TCON = 0x11;
	TMOD = 0x11;
    
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
    ES = 0; 
}

// Play a tone defined by the frequency relating to the delay
// for a predetermined duration
void play_tone(int duration, char delay_h, char delay_l)
{

	if (SND)
		TR0 = 1;
	else
		TR0 = 0;

	DELAY_REPS = -duration;
    SNDH = delay_h;
    SNDL = delay_l;
    TH0=delay_h;
    TL0=delay_l;
	TH1 = 0x00;
	TL1 = 0x01;
    TCON = 0x50;
	ISPLAYING = 1;
	while(ISPLAYING)
	{
	}
    return;
}


//;//Static routine to play La Cucaracha!
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// playTune:
// Play the notes from La Cucaracha in order
// Multiples of these can be extrapolated such as dotted notes
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void playtune(void)
{
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_C5(8);
	PLAY_BREAK(1);
	PLAY_E5(16);
	PLAY_BREAK(1);
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_G5(4);
	PLAY_BREAK(1);
	PLAY_C5(12);
	PLAY_BREAK(1);
	PLAY_E5(16);
	PLAY_BREAK(1);
	PLAY_BREAK(8);
	PLAY_C5(4);
	PLAY_BREAK(1);
	PLAY_C5(4);
	PLAY_BREAK(1);
	PLAY_B5(4);
	PLAY_BREAK(1);
	PLAY_B5(4);
	PLAY_BREAK(1);
	PLAY_A5(4);
	PLAY_BREAK(1);
	PLAY_A5(4);
	PLAY_BREAK(1);
	PLAY_G5(16);
	PLAY_BREAK(1);
	return;
}

// Cycle a single LED number on or off for delay time.
void blinkLED(char in, int delay)
{
	if (in == 1)
		led5 = 0;
	if (in == 2)
		led1 = 0;
	if (in == 3)
		led3 = 0;
	if (in == 4)
		led7 = 0;
	if (in == 5)
		led0 = 0;
	if (in == 6)
		led6 = 0;
	if (in == 7)
		led8 = 0;
	if (in == 8)
		led2 = 0;

    PLAY_BREAK(delay);
    P0 = P2 = 0xFF;
	TR0 = 1;
	return;
}

// Flash a pattern in hex on the outmost eight lights
void flashpat(char pattern, char delay)
{
	P2 = ~((pattern & 0xF0) >> 4);
	P0 = ~((pattern & 0x0F) | 0xF0);
	PLAY_BREAK(delay);
	P0 = P2 = 0xFF;
	return;
}

// Return integer value based on what button is pressed.
// determined by keypad layout.
char checkbutton()
{
	if (!sw5)
		return 1;
	else if (!sw1)
		return 2;
	else if (!sw3)
		return 3;
	else if (!sw7)
		return 4;
	else if (!sw0)
		return 5;
	else if (!sw6)
		return 6;
	else if (!sw8)
		return 7;
	else if (!sw2)
		return 8;
	else
		return 0;
}

// Plays a game of simon says
void simon()
{
  	char i, end, hold, pressed;
  	i = 0;
  	end = 1;
  	TR1 = 0;
	TR0 = 0;
	spkr = 0;
	SND = 0;
	EA = 1;
	do {
		TR1 = 1;
		for (i = 0; i < end; i++) {
			hold = pat[i];
    	    blinkLED(hold, 20);
			P0 = 0xFF;
		}

		for (i = 0; i < end; i++)
		{
			hold = pat[i];
			pressed = 0;
			while (pressed == 0)
			{
				pressed = checkbutton();
			}	

			if (pressed != hold)
			{
   			    end = 1; //This in effect ends the game and sets a flag for us to check whether we lost
				PLAY_LOSE(DEFAULT_DELAY);
  				flashpat(0x0F, 50);
				P2 = 0xFF;
				P0 = 0xFF;
				PLAY_BREAK(5);
     	 	}
			else if (checkbutton() == hold);
			{
				while (checkbutton() != 0)
				{}
			}
		}

    end++; //We've finished a level, now increment to include the next level        
  } while(end < MAX_LEVELS);
        
	PLAY_WIN(DEFAULT_DELAY);
    flashpat(0x0F, DEFAULT_DELAY);
	PLAY_BREAK(5);
    flashpat(0x0F, DEFAULT_DELAY);
	PLAY_BREAK(5);
    flashpat(0x0F, DEFAULT_DELAY);
	PLAY_BREAK(5);


    return;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
// Main loop. Plays a tune, simon says, and sends serial output upon winning
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
void main()
{
    P0M1 = 0;
    P1M1 = 0;
    P2M1 = 0;
	interrupt_init();
	ES = 1;
	EA = 1;

	P0 = 0xFF;
    P2 = 0xFF;
	spkr = 0x0;
	TR0 = TR1 = 1;
	playtune();
	PLAY_BREAK(50);
	while (1)
 	{
	    simon();
		ES = 1;
		TI = 1;
		PLAY_BREAK(5);
		ES = 0;

	}    
	return;
}
