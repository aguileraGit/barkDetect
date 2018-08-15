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

#define FILENAME "mostlyWoofs.data"

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

  /*
  Serial.println(rows, DEC);

  Data data = ndata(nips, nops, rows);

  for(int row = 0; row < rows; row++){
      char* line = readln(path);
      parse(data, line, row);
      free(line);
  }
  Serial.println("Done Building");
  
  return data;
  */
}

static int lns(const char* file){

    int ch = EOF;
    int lines = 0;
    int pc = '\n';

    Serial.print("FileName: ");
    Serial.println(file);
    
    //Open file for reading
    File myFile;
    myFile = SD.open(file , FILE_READ);

    Serial.println("File open");

    /*
    while(ch = myFile.available()){
      Serial.write(myFile.read() );
        if(ch == '\r')
        Serial.println("");
            lines++;
        pc = ch;
    }
    */
    while(myFile.available()){
      Serial.write(myFile.read() );
    }

    Serial.println("Done");

    /*
    if(pc != '\n')
        lines++;
    //rewind(file);
    myFile.close();
    return lines;
    */
}
