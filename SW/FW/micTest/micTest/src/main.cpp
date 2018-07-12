/*
 * A simple hardware test which receives audio on the A2 analog pin
 * and sends it to the PWM (pin 3) output and DAC (A14 pin) output.
 *
 * This example code is in the public domain.
 */

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

/*
AudioInputAnalog was found under Audio/input_adc.cpp.
https://github.com/PaulStoffregen/Audio/blob/master/input_adc.cpp

I believe line 66 sets the ADC sampling speed

Work on setting slower sampling rate

From input_adc
https://github.com/PaulStoffregen/Audio/blob/master/input_adc.cpp

uC Reference
https://www.nxp.com/docs/en/reference-manual/K66P144M180SF5RMV2.pdf

// set the programmable delay block to trigger the ADC at 44.1 kHz
#if defined(KINETISK)
if (!(SIM_SCGC6 & SIM_SCGC6_PDB)
  || (PDB0_SC & PDB_CONFIG) != PDB_CONFIG
  || PDB0_MOD != PDB_PERIOD
  || PDB0_IDLY != 1
  || PDB0_CH0C1 != 0x0101) {

  //Start config
  SIM_SCGC6 |= SIM_SCGC6_PDB;
    Pretty sure this enables PDB or ADC. Need to find it in other h/cpp files

  PDB0_IDLY = 1;
    PDB Interrupt Delay
    Specifies the delay value to schedule the PDB interrupt. It can be used to schedule an independent
    interrupt at some point in the PDB cycle. If enabled, a PDB interrupt is generated, when the counter is
    equal to the IDLY. Reading this field returns the value of internal register that is effective for the current
    cycle of the PDB.

  PDB0_MOD = PDB_PERIOD;
    Specifies the period of the counter. When the counter reaches this value, it will be reset back to zero. If the
    PDB is in Continuous mode, the count begins anew. Reading this field returns the value of the internal
    register that is effective for the current cycle of PDB.

    PDB_PERIOD defined in utilities/pdb.h
       #define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_CONT | PDB_SC_PDBIE | PDB_SC_DMAEN)
       #if F_BUS == 120000000
         #define PDB_PERIOD (2720-1)

  PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;
    Status and Control Register

  PDB0_SC = PDB_CONFIG | PDB_SC_SWTRIG;
  PDB0_CH0C1 = 0x0101;
}
#endif


*/

// GUItool: begin automatically generated code
AudioInputAnalog         adc1;           //xy=161,80
AudioOutputAnalog        dac1;           //xy=329,47
AudioOutputPWM           pwm1;           //xy=331,125
AudioConnection          patchCord1(adc1, dac1);
AudioConnection          patchCord2(adc1, pwm1);
// GUItool: end automatically generated code

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  adc1.
}

void loop() {
  // Do nothing here.  The Audio flows automatically

  // When AudioInputAnalog is running, analogRead() must NOT be used.
}
