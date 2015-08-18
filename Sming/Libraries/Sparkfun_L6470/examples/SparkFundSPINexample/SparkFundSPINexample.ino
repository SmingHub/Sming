/*****************************************************************
Example code for the STMicro L6470 dSPIN stepper motor driver.
This code is public domain beerware/Sunny-D-ware. If you find it 
useful and run into me someday, I'd appreciate a cold one.

12/12/2011- Mike Hord, SparkFun Electronics

The breakout board for the dSPIN chip has 7 data lines:
BSYN- this line is LOW when the chip is busy; busy generally means
  things like executing a move command.
STBY- drag low to reset the device to default conditions. Also 
  should be performed after power up to ensure a known-good 
  initial state.
FLGN- when the dSPIN raises a flag it usually means an error has 
  occurred
STCK- used as a step clock input; the direction (and activation of
  this input) is done by setting registers on the chip.
SDI- SPI data FROM the uC TO the dSPIN
SDO- SPI data TO the uC FROM the dSPIN
CSN- active-low slave select for the SPI bus
CK- data clock for the SPI bus

A note about connecting motors:
It's unclear from the datasheet what gets connected to which 
  terminal. Bridge one (terminals 01A and 01B) gets one coil, and
  bridge two gets the other coil. For our mid-small stepper 
  (ROB-9238), that translates to
        01A -> RED
        01B -> GREEN
        02A -> BLUE
        02B -> YELLOW

ERRATA: IMPORTANT!!! READ THIS!!!
There are several errors in the datasheet for the L6470:
- the internal oscillator is specified as 16MHz +/- 3%. 
    Experimentally, it seems to be more like a 6% tolerance.
- when transitioning from one movement command to another, it may
    be necessary to include a dSPIN_SoftStop() between the two to
    ensure proper operation. For example, if dSPIN_Move(FWD, 800)
    is used to move 800 steps FWD, and immediately after that, 
    with no soft stop between them, a dSPIN_Run(FWD, 200) command
    is issued, the 'run' command will execute with a speed based
    on the value in the 'MAX_SPEED' register, the way the 'move' 
    command did, and NOT with the speed passed to it by the 
    function call.
   
v2.0- 5 Dec 2012- Purely cosmetic changes. I wanted to move the
  setup() and loop() functions into the first file, and make a
  real .h header file for the L6470 register definitions, to
  make sure the project works properly for everyone.
  
  

FILENAMES
dSPIN_example.ino - This file. Pin redefinition constants, setup()
  and loop().
dSPIN_commands.ino - High-level command implementations- movement
   and configuration commands, for example.
dSPIN_support.ino - Functions used to implement the high-level 
   commands, as well as utility functions for converting 
   real-world units (eg, steps/s) to values usable by the dSPIN 
   controller. Also contains the specialized configuration
   function for the dsPIN chip and the onboard peripherals needed 
   to use it.
L6470.h - Register and pin definitions and descriptions for use
   with the L6470.
*****************************************************************/

#include <SPI.h>    // include the SPI library:
#include "SparkFunL6470.h"  // include the register and bit definitions

#define SLAVE_SELECT_PIN 10  // Wire this to the CSN pin
#define MOSI             11  // Wire this to the SDI pin
#define MISO             12  // Wire this to the SDO pin
#define SCK              13  // Wire this to the CK pin
#define dSPIN_RESET      17  // Wire this to the STBY line
#define dSPIN_BUSYN      16  // Wire this to the BSYN line

#define STAT1            14  // Hooked to an LED on the test jig
#define STAT2            15  // Hooked to an LED on the test jig
#define SWITCH           8   // Hooked to the switch input and a 
                             //  pB on the jig

float testSpeed = 25;

void setup() 
{
  // Standard serial port initialization for debugging.
  Serial.begin(9600);
  
  // pin configurations for the test jig
  pinMode(STAT1, OUTPUT);
  pinMode(STAT2, OUTPUT);
  pinMode(SWITCH, INPUT);
  pinMode(5, INPUT);
  digitalWrite(STAT1, LOW);
  digitalWrite(STAT2, LOW);
  
  // dSPIN_init() is implemented in the dSPIN_support.ino file. 
  //  It includes all the necessary port setup and SPI setup to 
  //  allow the Arduino to control the dSPIN chip and relies 
  //  entirely upon the pin redefinitions
  //  in dSPIN_example.ino
  dSPIN_init();
  
  // First things first: let's check communications. The CONFIG 
  //  register should power up to 0x2E88, so we can use that to 
  //  check the communications. On the test jig, this causes an 
  //  LED to light up.
  if (dSPIN_GetParam(dSPIN_CONFIG) == 0x2E88) 
    digitalWrite(STAT1, HIGH);
  
  // The following function calls are for this demo application- 
  //  you will need to adjust them for your particular 
  //  application, and you may need to configure additional 
  //  registers.
  
  // First, let's set the step mode register:
  //   - dSPIN_SYNC_EN controls whether the BUSY/SYNC pin reflects
  //      the step frequency or the BUSY status of the chip. We 
  //      want it to be the BUSY status.
  //   - dSPIN_STEP_SEL_x is the microstepping rate- we'll go full 
  //      step.
  //   - dSPIN_SYNC_SEL_x is the ratio of (micro)steps to toggles
  //      on the BUSY/SYNC pin (when that pin is used for SYNC). 
  //      Make it 1:1, despite not using that pin.
  dSPIN_SetParam(dSPIN_STEP_MODE, 
                      !dSPIN_SYNC_EN | 
                      dSPIN_STEP_SEL_1_16 | 
                      dSPIN_SYNC_SEL_1);
                      
  // Configure the MAX_SPEED register- this is the maximum number
  //  of (micro)steps per second allowed. You'll want to mess 
  //  around with your desired application to see how far you can
  //  push it before the motor starts to slip. The ACTUAL 
  //  parameter passed to this function is in steps/tick; 
  //  MaxSpdCalc() will convert a number of steps/s into an 
  //  appropriate value for this function. Note that for any move 
  //  or goto type function where no speed is specified, this 
  //  value will be used.
  dSPIN_SetParam(dSPIN_MAX_SPEED, MaxSpdCalc(400));
  
  // Configure the FS_SPD register- this is the speed at which the
  //  driver ceases microstepping and goes to full stepping. 
  //  FSCalc() converts a value in steps/s to a value suitable for
  //  this register; to disable full-step switching, you can pass 
  //  0x3FF to this register.
  dSPIN_SetParam(dSPIN_FS_SPD, FSCalc(50));
  
  // Configure the acceleration rate, in steps/tick/tick. There is
  //  also a DEC register; both of them have a function (AccCalc()
  //  and DecCalc() respectively) that convert from steps/s/s into
  //  the appropriate value for the register. Writing ACC to 0xfff
  //  sets the acceleration and deceleration to 'infinite' (or as
  //  near as the driver can manage). If ACC is set to 0xfff, DEC 
  //  is ignored. To get infinite deceleration without infinite 
  //  acceleration, only hard stop will work.
  dSPIN_SetParam(dSPIN_ACC, 0xfff);
  
  // Configure the overcurrent detection threshold. The constants 
  //  for this are defined in the L6470.h file.
  dSPIN_SetParam(dSPIN_OCD_TH, dSPIN_OCD_TH_6000mA);
  
  // Set up the CONFIG register as follows:
  //  PWM frequency divisor = 1
  //  PWM frequency multiplier = 2 (62.5kHz PWM frequency)
  //  Slew rate is 290V/us
  //  Do NOT shut down bridges on overcurrent
  //  Disable motor voltage compensation
  //  Hard stop on switch low
  //  16MHz internal oscillator, nothing on output
  dSPIN_SetParam(dSPIN_CONFIG, 
                   dSPIN_CONFIG_PWM_DIV_1 | 
                   dSPIN_CONFIG_PWM_MUL_2 | 
                   dSPIN_CONFIG_SR_180V_us |
                   dSPIN_CONFIG_OC_SD_DISABLE | 
                   dSPIN_CONFIG_VS_COMP_DISABLE |
                   dSPIN_CONFIG_SW_HARD_STOP | 
                   dSPIN_CONFIG_INT_16MHZ);
                   
  // Configure the RUN KVAL. This defines the duty cycle of the 
  //  PWM of the bridges during running. 0xFF means that they are
  //  essentially NOT PWMed during run; this MAY result in more 
  //  power being dissipated than you actually need for the task.
  //  Setting this value too low may result in failure to turn.
  //  There are ACC, DEC, and HOLD KVAL registers as well; you may
  //  need to play with those values to get acceptable performance
  //  for a given application.
  dSPIN_SetParam(dSPIN_KVAL_RUN, 0xFF);
  
  // Calling GetStatus() clears the UVLO bit in the status 
  //  register, which is set by default on power-up. The driver 
  //  may not run without that bit cleared by this read operation.
  dSPIN_GetStatus();
  
  // Now we're going to set up a counter to track pulses from an
  //  encoder, to verify against the expected values.
  TCCR1A = 0;  // No waveform generation stuff.
  TCCR1B = B00000110; // Clock on falling edge, T1 pin.
  TCNT1 = 0;   // Clear the count.
}

void loop()
{
  dSPIN_Move(FWD, 25600);
  while (digitalRead(dSPIN_BUSYN) == LOW);  // Until the movement completes, the
                                            //  BUSYN pin will be low.
  dSPIN_SoftStop();                         // Inserting a soft stop between
                                            //  motions ensures that the driver
                                            //  will execute the next motion with
                                            //  the right speed.
  while (digitalRead(dSPIN_BUSYN) == LOW);  // Wait for the soft stop to complete.
  Serial.println(TCNT1, DEC);
  while(1);
}



