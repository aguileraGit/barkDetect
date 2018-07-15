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

From input_adc
https://github.com/PaulStoffregen/Audio/blob/master/input_adc.cpp

uC Reference
https://www.nxp.com/docs/en/reference-manual/K66P144M180SF5RMV2.pdf

https://forum.pjrc.com/threads/24492-Using-the-PDB-on-Teensy-3 #8
20 MHz / 128 / 40 / 1 = 3.9k
10 MHz / 128 / 40 / 1 = 1.9k

#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
	| PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

// 48 MHz / 128 / 10 / 1 Hz = 37500
#define PDB_PERIOD (F_BUS / 128 / 10 / 1)



*/

//#define F_CPU 20000000

// GUItool: begin automatically generated code
AudioInputAnalog         adc1;           //xy=161,80
AudioOutputAnalog        dac1;           //xy=329,47
AudioOutputPWM           pwm1;           //xy=331,125
AudioConnection          patchCord1(adc1, dac1);
AudioConnection          patchCord2(adc1, pwm1);
// GUItool: end automatically generated code

uint8_t ledOn = 0;

#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
	| PDB_SC_CONT | PDB_SC_PRESCALER(7) | PDB_SC_MULT(1))

#define PDB_PERIOD (F_BUS / 128 / 10 / 1)

#define PDB_CH0C1_TOS 0x0100
#define PDB_CH0C1_EN 0x01

void pdbInit() {
	pinMode(13, OUTPUT);

	// Enable PDB clock
	SIM_SCGC6 |= SIM_SCGC6_PDB;
	// Timer period
	PDB0_MOD = PDB_PERIOD;
	// Interrupt delay
	PDB0_IDLY = 0;
	// Enable pre-trigger
	PDB0_CH0C1 = PDB_CH0C1_TOS | PDB_CH0C1_EN;
	// PDB0_CH0DLY0 = 0;
	PDB0_SC = PDB_CONFIG | PDB_SC_LDOK;


  // Modulus Register - from https://forum.pjrc.com/threads/24492-Using-the-PDB-on-Teensy-3?highlight=PDB+timer
  PDB0_MOD = 36; // 1/96 MHz * 9600 = 100 µs
  //If I understand this correctly, PDB_PERIOD is set to get the clk for PDB. Set this to get the period of the counter
  // 1/180Mhz/128/10 = 7.111us -> 7.111 / 36 = 256us -> 1/256us = 3.9kHz


	// Software trigger (reset and restart counter)
	PDB0_SC |= PDB_SC_SWTRIG;
	// Enable interrupt request
	NVIC_ENABLE_IRQ(IRQ_PDB);
}

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  Serial.begin(9600);
	while (!Serial);

  pdbInit();

  Serial.println(F_CPU);


}

void loop() {
  // Do nothing here.  The Audio flows automatically

  // When AudioInputAnalog is running, analogRead() must NOT be used.
}



void pdb_isr() {
	Serial.print("pdb isr: ");
	Serial.println(millis());
	digitalWrite(13, (ledOn = !ledOn));
	// Clear interrupt flag
	PDB0_SC &= ~PDB_SC_PDBIF;
}
