#### Platformio
- autocomplete (intellicomplete) doesn't work. No better than using Arduino. Also, the Teensy Audio libraries aren't well documented.

- Digital pins aren't defined. Or are defined, but not being picked up by Platformio.

#### PCB
- Negative/Positive values don't match (3.3V and -4.2V)
- Mic has 0.75V offset

#### Code
- Need to figure out if I can lower the ADC sampling rate so I can use the audio libraries. Or setup timer to sample and push to buffer (DMA preferred).
- Look at: https://forum.pjrc.com/threads/45610-fft1024-with-smaller-delta-F-bins-(decimate-run-ADC-slower-)
