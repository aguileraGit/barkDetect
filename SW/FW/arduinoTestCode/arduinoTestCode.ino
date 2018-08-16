#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

const int nips = 512;
const int nops = 2;

// Data object.
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

#define FILENAME "data"

const int chipSelect = BUILTIN_SDCARD;

void setup() {

  delay(1000);

  //Start SD Card
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
  Serial.println("initialization failed!");
  return;
  }
  
  Serial.println("initialization done.");
  
  data = build(FILENAME, nips, nops);

}

void loop() {
  // put your main code here, to run repeatedly:

  

}

static Data build(const char* path, const int nips, const int nops){

  //File open/close functions are done per function (lns and readln)
  const int rows = lns(path);

  Serial.println(rows, DEC);

  Data data = ndata(nips, nops, rows);

  for(int row = 0; row < rows; row++){
    Serial.println("New line");
      readln(path);
      //parse(data, line, row);
      //free(line);
  }
  Serial.println("Done Building");
  
  return data;
  
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

static void readln(const char* file){
    int ch = EOF;
    int reads = 0;
    int size = 128;
    //char* line = (char*) malloc((size) * sizeof(char));

    //Open file for reading
    File myFile;
    myFile = SD.open(file , FILE_READ);
    
    //char varChar = myFile.read()
    
    //while( myFile.available() ){

      String line = myFile.readStringUntil('\r');
      /*
      char varChar = myFile.read()
        line[reads++] = ch;
        if(reads + 1 == size)
            line = (char*) realloc((line), (size *= 2) * sizeof(char));
      */
    //}
    
    //line[reads] = '\0';

    Serial.println(line);

    myFile.close();
    
    //return line[0];
}

static int lns(const char* file){

    int lines = 0;

    Serial.print("FileName: ");
    Serial.println(file);
    
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
