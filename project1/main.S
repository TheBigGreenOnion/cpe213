;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CpE 213 Project 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Robert Higgins
; Nicholas Eggleston
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This program uses the 8051 and various programming principles we have
; learned over the semester to preform the following roles:
;   - On startup, the microprocessor is initialized and flashes the LEDs
;       five times over about 2 seconds.
;   - Plays La Cucaracha for a little while, halting processing while running
;   - Enters a sort of adder mode
;       Button 1:   increment A
;       Button 4:   Store A
;       Button 7:   Decrement A
;       Button 5:   Multiply A and stored value
;       Button 8:   Add A and stored value
;       Button 2:   Subtract (A - stored)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#include <reg932.inc>
; Jump to Main/ISR functions as needed
org 0x00
    ljmp start

org 0x0b ;tim0
    ljmp tim0_isr

org 0x1b ;tim1 
    ljmp tim1_isr 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interrupt service routine for timer0
;   Sets timer to the delay given by R3 & R4.  This is precalibrated
;   To that of certain notes in the playN# routines
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
tim0_isr:
    cpl p1.7
    clr c
    mov R5, A   ; Save A. tim0 is higher in priority
    mov A, R3   ; than tim1, so this exchange is safe
    mov TH0, A
    mov A, R4 
    mov TL0, A
    mov A, R5   ; restore A
reti 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interrupt Service Routine for timer 1
;   Set to delay ~0.017 seconds per CYCLE.  Counts down, complementing
;   the LEDS to create a flash effect every 0.017 * R6 seconds. This
;   happens R7 times.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
tim1_isr:
    djnz R6, AGAIN 
    ACTION:             ; Decrement when something actually happens
    	djnz R7, CYCLE  ; otherwise just refresh R6
	clr TR1             ; Turn timer off when done flashing
    reti

    CYCLE:
        mov R6, #100    ; About 200 ms per flash on/off
        xrl A, #0xFF
        mov P0, A
        mov P2, A
        cpl P1.6      
    AGAIN:      
        mov TH1, #0x0b	;0.0017s
        mov TL1, #0x32
	setb TR1
reti

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
start:
    ; Set all ports used to bidirectional
    mov p0m1, #0
    mov p1m1, #0
    mov p2m1, #0

    ; Initialize all LEDs/speaker off
    mov A, #0x0FF
    mov P0, #0xFF
    mov P2, #0xFF
    setb P1.6
    clr P1.7

    ;;; Configure timers their interrupts
    ; enable both timers and set to 16bit non-resetting mode (1)      
    mov IEN0, #0x8A
    mov TCON, #0x11
    setb TR1
    setb TR0

    ;;; Flash LEDS 5 times in ~2 s
    mov R7, #11     ; 5 * 2 + 1 for turn off
    mov R6, #100
    acall flash

    ;;; Tune stuff
    setb c          ; Set C as sentinel to indicate music is playing
    acall playTune
    
    ;;; Stall to finish playing music using carry bit as sentinel
    MUSIC:
    	jc MUSIC

    ;;; counter/adder functionality
    ; 4 output LEDs are LED1 (MSB), LED4, LED7, LED8 (LSB)
    mov P1, #0x50
    mov R1, #0x80   	; Indirect addressing upper RAM for storage
    mov P2, #0xFF   	; ALL LEDs off, switches input
    mov P0, #0xFF

    mov R6, #235
    mov R7, #3
    ; Polling loop that will consume the rest of the program's
    ; life.  Waits for buttonpress

    clr A
    mov R1, #0x80	; Lowest RAM address to start storage
    mov @R1, A 		; Start with 0x00 in 0x80 RAM

    WATCHINPUT:
    	mov R0, #13	; Hopefully, this will prevent false positives?
	acall stall     ; Delay for a short period
    	clr TR1
	    clr TR0
        ; Poll continuiously for button presses
        acall updateleds
        jnb P2.0, INCREMENT
        jnb P0.1, STORE
        jnb P2.3, DECREMENT
	    jnb P0.2, SUB
	    jnb P1.4, MULT
	    jnb P0.0, SUM
	    jnb P0.3, PLAYLEDS

    sjmp WATCHINPUT

    INCREMENT:
        inc A
	    anl A, #00001111b   ; Lower four bits
        jz ROLLUP           ; if OF on the AC bit occurs
        sjmp WATCHINPUT
    	ROLLUP:             ; A > 16
    	acall beep
	    clr A
	sjmp WATCHINPUT

    DECREMENT:
    	anl A, #00001111b   ; Lower four bits
    	jz ROLLDOWN         ; A = 0 upon dec, roll
        dec A
        sjmp WATCHINPUT
    	ROLLDOWN: 
        acall beep 
    	mov A, #0xFF
        sjmp WATCHINPUT

    STORE:
    	mov R1, #0x80       ; Put A in 0x80 RAM
        mov @R1, A
        clr A
        sjmp WATCHINPUT

    SUM:
        mov R1, #0x80       ; Retrieve 0x80 and add to A
        add A, @R1
        jb AC, OVERFLOW
        sjmp WATCHINPUT

    SUB:
    	mov R1, #0x80       ; A-@0x80 = A
	    clr C
	    subb A, @R1
	    jc OVERFLOW
	    sjmp WATCHINPUT
    
    MULT:
    	clr OV
    	mov R1, #0x80
    	mov B, A
    	mov A, @R1
    	mul AB	            ; Check upper 12 bits for OF
    	jb OV, OVERFLOW     ; Any OF in b causes jump
    	cjne A, #0x10, COMP ; so does A >= 0x10
    	COMP: jnc OVERFLOW
   	sjmp WATCHINPUT

    PLAYLEDS:
    	acall ledShow
	sjmp WATCHINPUT

    OVERFLOW:
    acall beep
	mov R6, #100
	mov R7, #2
	mov A, #0x0FF

    acall flash
	mov R0, #20
	acall stall
	mov P0, #0xFF
	mov P2, #0xFF
	setb P1.6

	clr A
    ljmp WATCHINPUT

;--- End of polling routines

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Beep function
;   Set timer and play quarternote beep to indicate error
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
beep:	    
    setb TR0
    mov R0, #32
    acall playB5 
    acall playBreath
    clr TR0
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Flash:
;   R6/R7 T=0.0017*R6   R7=iterations
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
flash:       
    mov TH1, #0x0b
    mov TL1, #0x32
    setb TR1
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; update LEDS
;   Flip the bits of A for active low and use rotate to pick off the
;   bits shifted down to LSB
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;   
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LEDLIGHTSHOWAWEYEAH
;   Show each byte of codespace inverted on the LEDS around the edge of
;   the array.  Middle LED changes on every byte
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ledShow:    
    mov DPTR, #0
    mov R1, #40
    READLOOP:
    	djnz R1, SEQ
    	sjmp ESCAPE
     
    SEQ:
    	clr A
    	movc A, @A+DPTR
	inc DPTR
	mov P2, A     
	swap A
	mov P0, A
	cpl p1.6
	mov R0,#5
        acall stall 
    sjmp READLOOP

    ESCAPE:
    	mov P0, #0xFF
    	mov P2, #0xFF
    	setb P1.6
ret


;;; Static routine to play La Cucaracha!
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; playTune:
;   Play the notes from La Cucaracha in order
;   16 => eigth
;   32 => quarter
;   Multiples of these can be extrapolated such as dotted notes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
    clr C	; Set sentinel bit
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; All note definitions from here down.  Set timer to rollover f times
; per seconds and play a note for as long as R0 specifies.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
playG5: 
        mov R3, #0xB6
        mov R4, #0x87
        acall stall
    ret

playA5: 
        mov R3, #0xBE
        mov R4, #0x8B
        acall stall
    ret

playB5:
        mov R3, #0xC5
        mov R4, #0xB3
        acall stall
    ret

playC6:           
        mov R3, #0xC9
        mov R4, #0x0A
        acall stall 
    ret
 
playE6: 
        mov R3, #0xD4
        mov R4, #0x5D
        acall stall
    ret

playBreath:
        mov R0, #1
        mov R3, #0x00  
        mov R4, #0x00  
        acall stall
    ret


playREST:
        mov R3, #0x00   
        mov R4, #0x00   
        clr TR0     
        acall stall
        setb TR0    
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Stall for a few microseconds. Used for music and tones to hold notes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stall:
    loop0:     
	mov R1, #75
    loop1:            
        mov R2, #255  
    loop2:           
        nop
        djnz R2, loop2
        djnz R1, loop1
        djnz R0, loop0
        ret

end
