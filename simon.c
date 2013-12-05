// Simon game
// Has 10 levels
// When you win, we play a happy tone(G5, C5, E5) and blink a diamond
// When you lose, we play a loser tone(G5, G5, G5) and solid 4 corners

#define MAX_LEVELS 10
#define DEFAULT_DELAY 5
#define LOSE_DELAY DEFAULT_DELAY * 2
#define PLAY_WIN(delay) PLAY_G5(delay); PLAY_C6(delay); PLAY_E6(delay);
#define PLAY_LOSE(delay) PLAY_G5(delay); PLAY_G5(delay); PLAY_G5(delay);

/*
#define PLAY_G5(delay) play_tone(delay, 0xB6, 0x87)
#define PLAY_A5(delay) play_tone(delay, 0xBE, 0x8B)
#define PLAY_B5(delay) play_tone(delay, 0xC5, 0xB3)
#define PLAY_C6(delay) play_tone(delay, 0xC9, 0x0A)
#define PLAY_E6(delay) play_tone(delay, 0xD4, 0x5D)
#define PLAY_BREAK(delay) play_tone(delay, 0x00, 0x00)
*/


void simon() {
  unsigned char i, end, hold;
  int pat[] = {0,6,3,1,7,0,5,2,6,1};
  do {
    for (i = 0; i < end; i++) {
        hold = 1 << pat[i];
	blinkLED(hold, DEFAULT_DELAY);
    }
  
    for (i = 0; i < end; i++)
      if (!checkbutton((hold = 1 << i))) {
        end = MAX_LEVELS + 2; //This in effect ends the game and sets a flag for us to check whether we lost 
        break;
      }
    end++; //We've finished a level, now increment to include the next level	  
  } while(end < MAX_LEVELS);
	
  if (end == MAX_LEVELS + 2) { //we lost, blink and music accordingly
    PLAY_WIN(DEFAULT_DELAY);
    blinkLED(0x07, DEFAULT_DELAY);
    blinkLED(0x07, DEFAULT_DELAY);
    blinkLED(0x07, DEFAULT_DELAY);
   }
  else { //we won, blink and music accordingly
    PLAY_LOSE(DEFAULT_DELAY);
    blinkLED(0x70, LOSE_DELAY);
  }	    
  return;
}

blinkLED(char in, int delay){
{
    P0 = ((~(in & 0x0F) <<4) | 0x0F);
    P2 = ((~(in & 0xF0) | 0x0F);
    PLAY_BREAK(delay);
    P0 = P2 = 0xFF;
  }
}


