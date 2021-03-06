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
uC Reference
https://www.nxp.com/docs/en/reference-manual/K66P144M180SF5RMV2.pdf

Change sampling speed reference
https://forum.pjrc.com/threads/24492-Using-the-PDB-on-Teensy-3 #8
*/

// GUItool: begin automatically generated code
AudioInputAnalog         adc1;           //xy=161,80
AudioOutputAnalog        dac1;           //xy=329,47
AudioOutputPWM           pwm1;           //xy=331,125
AudioConnection          patchCord1(adc1, dac1);
AudioConnection          patchCord2(adc1, pwm1);

uint8_t ledOn = 0;

#define PDB_CONFIG (PDB_SC_TRGSEL(15) | PDB_SC_PDBEN | PDB_SC_PDBIE \
	| PDB_SC_CONT | PDB_SC_PRESCALER(3) | PDB_SC_MULT(0))

#define PDB_CH0C1_TOS 0x0100
#define PDB_CH0C1_EN 0x01

//Define 4kHz
#define PDB_PERIOD (F_BUS / 8 / 4 / 2001)


void pdbInit() {
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
	//while (!Serial);

	//Wait for Serial
	delay(1000);

	//Change pdb speed to 4kHz
  pdbInit();

	//Serial.println(F_CPU);

}


void loop() {
  // Do nothing here.  The Audio flows automatically

  // When AudioInputAnalog is running, analogRead() must NOT be used.
}


/*
void pdb_isr() {
	Serial.print("pdb isr: ");
	Serial.println(millis());
	digitalWrite(13, (ledOn = !ledOn));
	// Clear interrupt flag
	PDB0_SC &= ~PDB_SC_PDBIF;
}
*/
