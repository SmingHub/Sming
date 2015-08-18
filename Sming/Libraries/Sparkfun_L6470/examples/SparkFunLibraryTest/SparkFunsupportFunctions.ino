// Support functions.

#define NOTE_DIVISOR 2  // My cheesy way of reducing the note frequencies to a range
                        //  that doesn't cause the motor to slip. I *could* rewrite
                        //  the wantYouGone() function to change the notes, but that
                        //  would be a lot of work.

int stepDir = 1;        // Direction flipping bit. Rather than all going one way,
                        //  they change directions. It looks cooler.

// To play a note, we start the motor spinning at the note's frequency in steps/s.
//  The run() function automagically calculates the appropriate value to feed to the
//  dSPIN part based on the desired steps/s.
void playNote(int note, int duration)
{
  if (stepDir == 1)  boardA.run(FWD, note/NOTE_DIVISOR);
  else               boardA.run(REV, note/NOTE_DIVISOR);
  if (stepDir == 1)  boardB.run(REV, note/NOTE_DIVISOR);
  else               boardB.run(FWD, note/NOTE_DIVISOR);
  delay(duration);
  stepDir*=-1;
  boardA.softStop();
  boardB.softStop();
  while (boardA.busyCheck());
}

// This is the configuration function for the three dSPIN parts. Read the inline
//  comments for more info.
void dSPINConfig(void)
{
  boardA.configSyncPin(BUSY_PIN, 0);// BUSY pin low during operations;
                                    //  second paramter ignored.
  boardA.configStepMode(STEP_FS);   // 0 microsteps per step
  boardA.setMaxSpeed(10000);        // 10000 steps/s max
  boardA.setFullSpeed(10000);       // microstep below 10000 steps/s
  boardA.setAcc(10000);             // accelerate at 10000 steps/s/s
  boardA.setDec(10000);
  boardA.setSlewRate(SR_530V_us);   // Upping the edge speed increases torque.
  boardA.setOCThreshold(OC_750mA);  // OC threshold 750mA
  boardA.setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
  boardA.setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
  boardA.setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V
  boardA.setSwitchMode(SW_USER);    // Switch is not hard stop
  boardA.setOscMode(INT_16MHZ_OSCOUT_16MHZ); // for boardA, we want 16MHz
                                    //  internal osc, 16MHz out. boardB and
                                    //  boardC will be the same in all respects
                                    //  but this, as they will bring in and
                                    //  output the clock to keep them
                                    //  all in phase.
  boardA.setAccKVAL(255);           // We'll tinker with these later, if needed.
  boardA.setDecKVAL(255);
  boardA.setRunKVAL(255);
  boardA.setHoldKVAL(32);           // This controls the holding current; keep it low.
  
  
  boardB.configSyncPin(BUSY_PIN, 0);// BUSY pin low during operations;
                                    //  second paramter ignored.
  boardB.configStepMode(STEP_FS);   // 0 microsteps per step
  boardB.setMaxSpeed(10000);        // 10000 steps/s max
  boardB.setFullSpeed(10000);       // microstep below 10000 steps/s
  boardB.setAcc(10000);             // accelerate at 10000 steps/s/s
  boardB.setDec(10000);
  boardB.setSlewRate(SR_530V_us);   // Upping the edge speed increases torque.
  boardB.setOCThreshold(OC_750mA);  // OC threshold 750mA
  boardB.setPWMFreq(PWM_DIV_2, PWM_MUL_2); // 31.25kHz PWM freq
  boardB.setOCShutdown(OC_SD_DISABLE); // don't shutdown on OC
  boardB.setVoltageComp(VS_COMP_DISABLE); // don't compensate for motor V
  boardB.setSwitchMode(SW_USER);    // Switch is not hard stop
  boardB.setOscMode(EXT_16MHZ_OSCOUT_INVERT); // for boardA, we want 16MHz
                                    //  internal osc, 16MHz out. boardB and
                                    //  boardC will be the same in all respects
                                    //  but this, as they will bring in and
                                    //  output the clock to keep them
                                    //  all in phase.
  boardB.setAccKVAL(255);           // We'll tinker with these later, if needed.
  boardB.setDecKVAL(255);
  boardB.setRunKVAL(255);
  boardB.setHoldKVAL(32);           // This controls the holding current; keep it low.
}
  
