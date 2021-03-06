/*
* ADC pin 16
* DAC pin DA0
* 
* Picoscope setup with Sine wave with 250mV amplitude and 600mV offset
* 
* format /FS:FAT32 X:
*/

#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Entropy.h>

extern "C"{
  #include "Tinn.h"
}

/*
uC Reference
https://www.nxp.com/docs/en/reference-manual/K66P144M180SF5RMV2.pdf

Change sampling speed reference
https://forum.pjrc.com/threads/24492-Using-the-PDB-on-Teensy-3 #8

https://forum.pjrc.com/threads/53212-Slowing-down-the-PDB?p=183860#post183860

https://github.com/glouw/tinn
*/

//Inputs
AudioInputAnalog         adc1;           //xy=161,80
AudioAnalyzeFFT1024      fft1;
AudioConnection          patchCord(adc1, fft1);

//Debug LED
uint8_t ledOn = 0;

//Menu items
volatile bool menuPrintFFT = false;
volatile bool menuLoadData = false;
volatile bool menuTrainData = false;
volatile bool menuPredictData = false;
volatile bool menuPrintMenu = false;
volatile bool menuBreakDown = false;
volatile bool menuSaveTinn = false;
volatile bool menuLoadTinn = false;

//Store the latest FFT for analysis, printing, etc
#define FFTLENGTH 512
float latestFFT[FFTLENGTH];

//Define data file 
// Note filenames have to keep kept short "data" is actually mostlyWoofs.data
#define FILENAME "data" //mostlyWoofs.data

//SD Card
const int chipSelect = BUILTIN_SDCARD;

//TINN variable
const int nips = 512;
const int nops = 2;
float rate = 0.1f;
const int nhid = 48;
const float anneal = 0.99f;
const int iterations = 1; //32

// Tinn data object.
typedef struct {
    // 2D floating point array of input.
    float** in;
    // 2D floating point array of target.
    float** tg;
    // Number of inputs to neural network.
    int nips;
    // Number of outputs to neural network.
    int nops;
    // Number of rows in file (number of sets for neural network).
    int rows;
} Data;

Data data;

void printMenu(void);
void printFFT(void);
void processIncomingSerial(int incomingByte);

uint32_t seed_value;

//Create NN
Tinn tinn = xtbuild(nips, nhid, nops);

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  //Define Windowing
  fft1.windowFunction(AudioWindowHanning1024);

  //Test with sinewave
  //sinewave.amplitude(0.8);
  //sinewave.frequency(100.0);

  //Setup LED
  pinMode(13, OUTPUT);
  
  //Serial.begin(9600);
  //while (!Serial);

  //Wait for Serial
  digitalWrite(13, HIGH);
  delay(1000);

  //Change sampling rate to 8kHz. Done in utility/pdb.h

  menuPrintFFT = false;
  menuLoadData = false;
  menuTrainData = false;
  menuPredictData = false;
  menuPrintMenu = true;
  menuBreakDown = false;
  menuSaveTinn = false;
  menuLoadTinn = false;

  //Start SD Card
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
  return;
  }
  
  Serial.println("initialization done.");

  Entropy.Initialize();
  seed_value = Entropy.random();
}


void loop() {

  //Handle incoming serial
  int incomingByte;
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    processIncomingSerial(incomingByte);
  }//End serial if

  //Get latest FFT
  if (fft1.available()) {
    
    //Debug LED
    digitalWrite(13, (ledOn = !ledOn));

    //Get FFT
    getFFT();

    //Print FFT
    if(menuPrintFFT){
      printFFT();
    }

    //Predict
    if(menuPredictData){
      predictFn();
      //Remove line below to make continuous predictions
      menuPredictData = false;
    }

  }//End if fft.available



  //Load training data from SD Card (data file)
  if(menuLoadData){
    menuLoadData = false;
    
    data = build(FILENAME, nips, nops);
  }

  //Train data - Data must be loaded first
  if(menuTrainData){
    menuTrainData = false;

    //Start training
    for(int i = 0; i < iterations; i++){
        shuffle(data);
        float error = 0.0f;
        for(int j = 0; j < data.rows; j++){
            const float* const in = data.in[j];
            const float* const tg = data.tg[j];
            error += xttrain(tinn, in, tg, rate);
        }

        Serial.print("Error ");
        Serial.print( (error / data.rows), 4 );
        Serial.print(" :: learning rate ");
        Serial.print(rate, 4);
        Serial.print(" :: iteration ");
        Serial.print(i);
        Serial.println(" ");
           
        rate *= anneal;
    }
    Serial.println("Done Building");
  }

  //Print Help Menu
  if(menuPrintMenu){
    menuPrintMenu = false;
    printMenu();
  }

  //BreakDownNN - Free memory
  if(menuBreakDown){
    menuBreakDown = false;
    //xtfree(loaded);
    dfree(data);
  }

  //Save Tinn NN to SD Card
  if(menuSaveTinn){
    menuSaveTinn = false;
    xxtsave(tinn, "someTinnNN");
  }

  //Load Tinn NN from SD Card
  if(menuLoadTinn){
    menuLoadTinn = false;

    tinn = xxtload("someTinnNN");

    //Need to load data. 
    data = build(FILENAME, nips, nops);
  }
  
}


//********** FFT Functions **********//

void getFFT(void){
  int i;
  for (i=0; i<512; i++) {
      latestFFT[i] = fft1.read(i);
  }
}

void printFFT(void){
  int i;
  for(i=0; i<FFTLENGTH; i++){
    Serial.print( latestFFT[i], 4 );
    Serial.print(" ");
  }
  Serial.println(" ");
}


//********** Menu and Serial Functions **********//

void printMenu(void){
  Serial.println("========= Menu =========");
  Serial.println("f - Print FFT values");
  Serial.println("h - Prints this menu");
  Serial.println("l - Load training data");
  Serial.println("t - Train data");
  Serial.println("p - Predict data");
  Serial.println("s - Save TINN NN");
  Serial.println("k - Load TINN NN");
  Serial.println("b - Breakdown NN");
}

void processIncomingSerial(int incomingByte){

    switch(incomingByte){
      case 'f': 
        menuPrintFFT = !menuPrintFFT;
        if(menuPrintFFT){ 
          Serial.println("Print FFT ON");
        } else {
          Serial.println("Print FFT OFF");
        }
        break;

      case 'l':
        Serial.println("Loading Data");
        menuLoadData = true;
        break;

      case 't':
        menuTrainData = true;
        Serial.println("Training Data");
        break;

      case 'p':
        menuPredictData = !menuPredictData;
        if(menuPredictData){
          Serial.println("Predicting Data");
        } else {
          Serial.println("Done Predicting Data");
        }
        break;

      case 'b':
        Serial.print("Break down NN - Free Mem");
        menuBreakDown = true;
        break;

      case 'h':
        menuPrintMenu = true;
        break;

      case 's':
        Serial.println("Save Tinn NN");
        menuSaveTinn = true;
        break;

      case 'k':
        Serial.println("Load Tinn NN");
        menuLoadTinn = true;
        break;
        
      default:
        Serial.println("Unknown Option");
        break;
    }
}

//********** TINN related Functions **********//
// This doesn't touch the core Tinn Functions. These are "helper" functions found in
// test.c in the Tinn repo. 

//Returns the number of lines in a file. Modified from Original.
static int lns(const char* file){
    int lines = 0;

    //Serial.print("FileName: ");
    //Serial.println(file);
    
    //Open file for reading
    File myFile;
    myFile = SD.open(file , FILE_READ);

    while (myFile.available()) {
      char varChar = myFile.read();
      //Serial.write(varChar);

      if(varChar == '\r'){
        lines++;
      }
    }
    return lines;
}


//Returns the number of lines in a file. Modified from Original.
static char* readln(const char* file, int lineNum){
  int charCount = 0;
  int currLineNum = 0;  
  String line;

  //Open file for reading
  File myFile;
  myFile = SD.open(file , FILE_READ);

  if(!myFile){
    Serial.println("Error reading file");
  }
  
  //Serial.print("Reading line: ");
  //Serial.println(lineNum);

  while( myFile.available() ){

    //Get char and update counter
    char varChar = myFile.read(); 
    charCount++;

    //Found a line
    if(varChar == '\r'){
      line[ charCount ] = '\0';

      if(currLineNum == lineNum){
         break;
      } else {
        line = "";
        charCount = 0;
        currLineNum++;
        myFile.read(); 
      }

    } else {
      line += varChar;
    }
  }

  myFile.close();

  //Convert to char*
  charCount++;
  char* copyArray = (char*) malloc((charCount) * sizeof(char));
  line.toCharArray(copyArray, charCount);

  //Serial.print("Array: ");
  //Serial.println(copyArray);

  copyArray[charCount] = '\0';
  
  return copyArray;
}

// New 2D array of floats.
static float** new2d(const int rows, const int cols){
    float** row = (float**) malloc((rows) * sizeof(float*));
    for(int r = 0; r < rows; r++)
        row[r] = (float*) malloc((cols) * sizeof(float));
    return row;
}

// New data object.
static Data ndata(const int nips, const int nops, const int rows){
    const Data data = {
        new2d(rows, nips), new2d(rows, nops), nips, nops, rows
    };
    return data;
}

// Gets one row of inputs and outputs from a string.
static void parse(const Data data, char* line, const int row)
{
    const int cols = data.nips + data.nops;
    for(int col = 0; col < cols; col++)
    {
        const float val = atof(strtok(col == 0 ? line : NULL, " "));
        //Serial.println(val);
        if(col < data.nips)
            data.in[row][col] = val;
        else
            data.tg[row][col - data.nips] = val;
    }
}

// Frees a data object from the heap.
static void dfree(const Data d)
{
    for(int row = 0; row < d.rows; row++)
    {
        free(d.in[row]);
        free(d.tg[row]);
    }
    free(d.in);
    free(d.tg);
}

// Randomly shuffles a data object.
static void shuffle(const Data d)
{
    for(int a = 0; a < d.rows; a++)
    {
        //const int b = rand() % d.rows;
        const int b = Entropy.random() % d.rows;
        float* ot = d.tg[a];
        float* it = d.in[a];
        // Swap output.
        d.tg[a] = d.tg[b];
        d.tg[b] = ot;
        // Swap input.
        d.in[a] = d.in[b];
        d.in[b] = it;
    }
}

//Builds data to be used with Training. Modified from Original.
static Data build(const char* path, const int nips, const int nops){
  
  //File open/close functions are done per function (lns and readln)
  const int rows = lns(path);

  Data data = ndata(nips, nops, rows);

  for(int row = 0; row < rows; row++){
      //Modified
      char* line = readln(path, row);
      Serial.print("...");
      parse(data, line, row);
      free(line);
  }
  Serial.println(".");
  Serial.println("Done Loading");
  
  return data;
}

//Predict ONLY exsisting data
void predictFn(){
  Serial.println("Predict Function");

    int c = 0;
    for(c=0; c < 51; c++){

        Serial.print("Sample Number: ");
        Serial.println(c);
        const float* const in = data.in[c];
        const float* const tg = data.tg[c];
        const float* const pd = xtpredict(tinn, in);
    
        // Prints target.
        Serial.print("Target: ");
        xxtprint(tg, data.nops);
        
        // Prints prediction.
        Serial.print("Prediction: ");
        xxtprint(pd, data.nops);
    }
}

//Prints an array of floats. Useful for printing predictions.
// This would normally be xprint under the "core" of the Tinn repo. But,
// this calls printf which isn't possible. 
// Also this function will only print the two outputs. It's ugly.
void xxtprint(const float* arr, const int sizeNum){
  //Only have 2 outputs
  Serial.print(arr[0]);
  Serial.print(" - ");
  Serial.print(arr[1]);
  Serial.println(" ");  
}

// Saves a tinn to disk.
void xxtsave(const Tinn t, const char* const path){

    unsigned long lineNum = 0;
    
    File myFile;
    myFile = SD.open("SomeNN" , FILE_WRITE);

    if(myFile){
    
      // Save header.
      myFile.print(t.nips);
      myFile.print(" ");
      myFile.print(t.nhid);
      myFile.print(" ");
      myFile.print(t.nops);
      myFile.print("\r");

  
      //Serial.print(t.nips);
      //Serial.print(" ");
      //Serial.print(t.nhid);
      //Serial.print(" ");
      //Serial.println(t.nops);
  
      //Arduino SD Lib can't handle floats - https://stackoverflow.com/questions/2988791/converting-float-to-char
      //4 is mininum width, 3 is precision; float value is copied onto buff
      //dtostrf(123.234, 4, 3, charVal);
      char charVal[10];
      
      //Save biases
      Serial.println("Saving biases");
      for(int i = 0; i < t.nb; i++){
        dtostrf( ((double)t.b[i]), 4, 3, charVal );
        myFile.println(charVal);
        //Serial.println(charVal);
        lineNum++;
      }
      Serial.print("Lines saved: ");
      Serial.println(lineNum);
      
      //Save weights
      Serial.println("Saving weights");
      lineNum = 0;
      for(int i = 0; i < t.nw; i++){
        dtostrf( ((double)t.w[i]), 4, 3, charVal );
        myFile.println(charVal);
        //Serial.println(charVal);
        lineNum++;
      }
      Serial.print("Lines saved: ");
      Serial.println(lineNum);
    
    
      Serial.println("Done Saving");
      myFile.close();
    }
}

// Loads a tinn from disk.
Tinn xxtload(const char* const path){

    unsigned long lineNum = 0;
    
    int nips = 512;
    int nhid = 48;
    int nops = 2;
      
    const Tinn tt = xtbuild(nips, nhid, nops);
    
    File myFile;
    myFile = SD.open("SomeNN");

    if(myFile){
        Serial.println("File opened!");
        
        // Load header.
        for(int i = 0; i < 3; i++){
          String header = myFile.readStringUntil(' ');
          if(i == 0){
            nips = header.toFloat();
            Serial.println(nips);
          }
          if(i == 1){
            nhid = header.toFloat();
            Serial.println(nhid);
          }
          if(i == 2){
            nops = header.toFloat();
            Serial.println(nops);
          }
        }
        
        //Read rest of line
        myFile.readStringUntil('\r');
        
        // Load biases
        Serial.println("Loading biases");
        //for(int i = 0; i < t.nb; i++) fscanf(file, "%f\n", &t.b[i]);
        for(int i = 0; i < tt.nb; i++){
          String biasVar = myFile.readStringUntil('\r');
          tt.b[i] = (float)biasVar.toFloat();
          //Serial.println(biasVar.toFloat(), 3);
          lineNum++;
        }
        
        Serial.print("Lines read: ");
        Serial.println(lineNum);
        
        //Load weights
        lineNum = 0;
        Serial.println("Loading weights");
        //for(int i = 0; i < t.nw; i++) fscanf(file, "%f\n", &t.w[i]);
        for(int i = 0; i < tt.nw; i++){
          String weightsVar = myFile.readStringUntil('\r');
          tt.w[i] = weightsVar.toFloat();
          //Serial.println(weightsVar.toFloat(), 3);
          lineNum++;
        }
        
        Serial.print("Lines read: ");
        Serial.println(lineNum);
    
      Serial.println("Done Loading");
    }// end if myFile

    myFile.close();
    return tt;
}
