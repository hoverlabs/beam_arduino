/*
===========================================================================

  This is the library for Beam.

  Beam is a beautiful LED matrix — features 120 LEDs that displays scrolling text, animations, or custom lighting effects.
  Beam can be purchased here: http://www.hoverlabs.co

  Written by Emran Mahbub and Jonathan Li for Hover Labs.
  BSD license, all text above must be included in any redistribution

===========================================================================
*/

#include "Arduino.h"
#include "Wire.h"
#include "beam.h"
#include "charactermap.h"
#include "frames.h"

/*
=================
PUBLIC FUNCTIONS
=================
*/

/*
    This constructor used when multiple Beams behave like one long Beam
*/
Beam::Beam ( int rstpin, int irqpin, int numberOfBeams){
    _rst = rstpin;
    _irq = irqpin;
    _beamCount = numberOfBeams;
    activeBeams = numberOfBeams;
    _gblMode = 1;
}

/*
    This constructor used when multiple Beams behave like single Beam units
*/
Beam::Beam ( int rstpin, int irqpin, uint8_t syncMode, uint8_t beamAddress){
    _rst = rstpin;
    _irq = irqpin;
    _syncMode = 0;
    activeBeams = 1;
    _currBeam = beamAddress;
    _gblMode = 0;
}

bool Beam::begin(void){

    //resets beam - will clear all beams
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(200);
    digitalWrite(_rst, HIGH);
    delay(350);

    //reset cs[]
    int c = 0;
    for (c=0; c<12; c++){
        cs[c] = 0x00;
    }

    //reset segmentmask[]
    uint8_t val = 0x80;
    int s = 0;
    for (s=0; s<8; s++){
        segmentmask[s] = val;
        val = val/2;
    }

    return true;

}

void Beam::initBeam(){

    //initialize Beam
    if (_gblMode == 1 ){
        if (_beamCount == 1) {
            #if DEBUG
            Serial.println("clearing BEAMA");
            #endif
            initializeBeam(BEAMA);
        } else if (_beamCount == 2){
            #if DEBUG
            Serial.println("clearing BEAMA");
            #endif
            initializeBeam(BEAMA);
            #if DEBUG
            Serial.println("clearing BEAMB");
            #endif
            initializeBeam(BEAMB);
        } else if (_beamCount == 3){
            #if DEBUG
            Serial.println("clearing BEAMA");
            #endif
            initializeBeam(BEAMA);
            #if DEBUG
            Serial.println("clearing BEAMB");
            #endif
            initializeBeam(BEAMB);
            #if DEBUG
            Serial.println("clearing BEAMC");
            #endif
            initializeBeam(BEAMC);
        } else if (_beamCount == 4){
            #if DEBUG
            Serial.println("clearing BEAMA");
            #endif
            initializeBeam(BEAMA);
            #if DEBUG
            Serial.println("clearing BEAMB");
            #endif
            initializeBeam(BEAMB);
            #if DEBUG
            Serial.println("clearing BEAMC");
            #endif
            initializeBeam(BEAMC);
            #if DEBUG
            Serial.println("clearing BEAMD");
            #endif
            initializeBeam(BEAMD);
        } else {
            #if DEBUG
            Serial.println("beamCount should be between 1 and 4");
            #endif
        }
    } else {
        initializeBeam(_currBeam);
    }

}

void Beam::print(const char* text){

    //resets beam - will clear all beams
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(100);
    digitalWrite(_rst, HIGH);
    delay(250);

    #if DEBUG
    Serial.print("Text to print:");
    Serial.println(text);
    #endif

    initBeam();

    // Clear all frames
    for (int z=0; z<12; z++){
        cs[z] = 0x00;
    }
    if (_gblMode == 1 ){
        if (_beamCount == 1) {
            for (int i=0;i<36;i++){
                writeFrame(BEAMA, i);
            }
        } else if (_beamCount == 2){
            for (int i=0;i<36;i++){
                writeFrame(BEAMA, i);
                writeFrame(BEAMB, i);
            }
        } else if (_beamCount == 3){
            for (int i=0;i<36;i++){
                writeFrame(BEAMA, i);
                writeFrame(BEAMB, i);
                writeFrame(BEAMC, i);
            }
        } else if (_beamCount == 4){
            for (int i=0;i<36;i++){
                writeFrame(BEAMA, i);
                writeFrame(BEAMB, i);
                writeFrame(BEAMC, i);
                writeFrame(BEAMD, i);
            }
        }
    } else {
        #if DEBUG
        Serial.print("printing BEAM: ");
        Serial.println(_currBeam, HEX);
        #endif
        for (int i=0;i<36;i++){
            writeFrame(_currBeam, i);
        }
    }

    int i = 0;
    int j = 0;

    uint16_t fIndex=0;
    uint8_t fByte = 0;

    int frame = 0;

    int asciiVal;
    int cscount = 0;
    int stringLen = strlen(text);

    #if DEBUG
    Serial.println(strlen(text)-1);
    Serial.println(" ");
    #endif

    while ( (i<stringLen) && frame < 36 ){

      // pick a character to print to Beam
      asciiVal = toupper(text[i]) - 32;

      #if DEBUG
      Serial.print(text[i]);
      Serial.print(" = ");
      Serial.print(asciiVal);
      Serial.println("");
      Serial.print("cscolumn[] = ");
      #endif

      // loop through the Beam grid and place characters
      // from the character map
      fIndex=0;
      fByte = pgm_read_byte_near(&charactermap[asciiVal][fIndex]);

      while (cscount <24 && fByte != 0xFF){
        cscolumn[cscount] = fByte;
        #if DEBUG
        Serial.print(cscolumn[cscount], HEX);
        Serial.print(" ");
        #endif
        fByte = pgm_read_byte_near(&charactermap[asciiVal][++fIndex]);
        cscount++;
      }
      #if DEBUG
      Serial.println("");
      #endif
      i++;  // go to next character

      if (cscount>23) {
          // if end of grid is reached in current frame,
          // then start writing to the Beam registers
          #if DEBUG
          Serial.println("- end of frame reached");
          Serial.print("writing cs[] = ");
          #endif
          for (j=0; j<=11; j++){
            cs[j] = (cscolumn[j*2]) | (cscolumn[j*2+1] << 5);
            #if DEBUG
            Serial.print(cs[j], HEX);
            Serial.print(" ");
            #endif
          }
          #if DEBUG
          Serial.println("done cs");
          #endif

          if (_gblMode == 1){
              if(_beamCount == 1){
                writeFrame(BEAMA,frame+1);
                _lastFrameWrite = frame + 1;
              }
              else if (_beamCount == 2){
                writeFrame(BEAMA,frame+1+1);
                writeFrame(BEAMB,frame+1);
                _lastFrameWrite = frame + 1 + 1;
              }
              else if (_beamCount == 3){
                writeFrame(BEAMA,frame+1+1+1);
                writeFrame(BEAMB,frame+1+1);
                writeFrame(BEAMC,frame+1);
                _lastFrameWrite = frame + 1 + 1 + 1;
              }
              else if (_beamCount == 4){
                writeFrame(BEAMA,frame+1+1+1+1);
                writeFrame(BEAMB,frame+1+1+1);
                writeFrame(BEAMC,frame+1+1);
                writeFrame(BEAMD,frame+1);
                _lastFrameWrite = frame + 1 + 1 + 1 + 1;
              }
          } else {
            #if DEBUG
            Serial.print("printing last frame BEAM: ");
            Serial.println(_currBeam, HEX);
            #endif
            writeFrame(_currBeam,frame+1);
            _lastFrameWrite = frame + 1;

          }

          int x;
          for (x=0;x<12;x++){
            cs[x] = 0x00;
            cscolumn[x*2] = 0x00;
            cscolumn[x*2+1] = 0x00;
          }
          #if DEBUG
          Serial.println(" ");
          #endif
          frame = frame + 1;    // go to next frame
          cscount = 0;        // reset cscount
          //_lastFrameWrite = 8;

          // if a specific frame is specified, then return if that frame is done.

          //ADD THIS LATER
          //if (frameNum!=0 && frame > frameNum){
          //    return;
          //}

          if (fByte != 0xFF){
            //special case if current character needs to wrap to next frame
            #if DEBUG
            Serial.print("Continuing prev char ");
            Serial.print("cscolumn[] = ");
            #endif
            while (cscount <24 && fByte != 0xFF){
              cscolumn[cscount] = fByte;
              #if DEBUG
              Serial.print(cscolumn[cscount], HEX);
              Serial.print(" ");
              #endif

              fByte = pgm_read_byte_near(&charactermap[asciiVal][++fIndex]);
              cscount++;
            }
            #if DEBUG
            Serial.println("");
            #endif
          }
      }

      if (stringLen == i) {
          // if end of string is reached in current frame,
          // then start writing to Beam registers
          #if DEBUG
          Serial.println("- end of string reached");
          Serial.print("writing cs[] = ");
          #endif
          for (j=0; j<=11; j++){
            cs[j] = (cscolumn[j*2]) | (cscolumn[j*2+1] << 5);
            #if DEBUG
            Serial.print(cs[j], HEX);
            Serial.print(" ");
            #endif
          }
          #if DEBUG
          Serial.println("done cs print");
          #endif

          if (_gblMode == 1){

              if(_beamCount == 1){
                writeFrame(BEAMA,frame+1);
                _lastFrameWrite = frame + 1;
              }
              else if (_beamCount == 2){
                writeFrame(BEAMA,frame+1+1);
                writeFrame(BEAMB,frame+1);
                _lastFrameWrite = frame + 1 + 1;
              }
              else if (_beamCount == 3){
                writeFrame(BEAMA,frame+1+1+1);
                writeFrame(BEAMB,frame+1+1);
                writeFrame(BEAMC,frame+1);
                _lastFrameWrite = frame + 1 + 1 + 1;
              }
              else if (_beamCount == 4){
                writeFrame(BEAMA,frame+1+1+1+1);
                writeFrame(BEAMB,frame+1+1+1);
                writeFrame(BEAMC,frame+1+1);
                writeFrame(BEAMD,frame+1);
                _lastFrameWrite = frame + 1 + 1 + 1 + 1;
              }
          } else {
                #if DEBUG
                Serial.print("printing last last BEAM: ");
                Serial.println(_currBeam, HEX);
                #endif
              writeFrame(_currBeam,frame+1);
              _lastFrameWrite = frame + 1;

          }

          int x;
          for (x=0;x<12;x++){
            cs[x] = 0x00;
            cscolumn[x*2] = 0x00;
            cscolumn[x*2+1] = 0x00;
          }
          #if DEBUG
          Serial.println(" ");
          #endif
      }
    }

    //defaults Beam to basic settings
    setPrintDefaults(SCROLL, 0, 6, 7, 5, 1, 0);

}

void Beam::printFrame(uint8_t frameToPrint, const char * text){

    #if DEBUG
    Serial.print("Text to print:");
    Serial.println(text);
    #endif

    int i = 0;
    int j = 0;

    uint16_t fIndex=0;
    uint8_t fByte = 0;

    int frame = frameToPrint;

    int asciiVal;
    int cscount = 0;
    int stringLen = strlen(text);

    #if DEBUG
    Serial.println(strlen(text)-1);
    Serial.println(" ");
    #endif

    while ( (i<stringLen) && frame < 36 ){

      // pick a character to print to Beam
      asciiVal = toupper(text[i]) - 32;

      #if DEBUG
      Serial.print(text[i]);
      Serial.print(" = ");
      Serial.print(asciiVal);
      Serial.println("");
      #endif

      // loop through the Beam grid and place characters
      // from the character map
      #if DEBUG
      Serial.print("cscolumn[] = ");
      #endif

      fIndex = 0;
      fByte = pgm_read_byte_near(&charactermap[asciiVal][fIndex]);

      while (cscount <24 && fByte != 0xFF){
        cscolumn[cscount] = fByte;

        #if DEBUG
        Serial.print(cscolumn[cscount], HEX);
        Serial.print(" ");
        #endif

        fByte = pgm_read_byte_near(&charactermap[asciiVal][++fIndex]);
        cscount++;
      }

      #if DEBUG
      Serial.println("");
      #endif

      i++;  // go to next character

      if (cscount>23) {
          // if end of grid is reached in current frame,
          // then start writing to the Beam registers
          #if DEBUG
          Serial.println("- end of frame reached");
          Serial.print("writing cs[] = ");
          #endif

          for (j=0; j<=11; j++){
            cs[j] = (cscolumn[j*2]) | (cscolumn[j*2+1] << 5);

            #if DEBUG
            Serial.print(cs[j], HEX);
            Serial.print(" ");
            #endif
          }

          #if DEBUG
          Serial.println("done cs");
          #endif

          //write cs[0-11] to as1130 with current frame number.
          /*if(_beamCount >= 1){
            writeFrame(BEAMA,frame+1+1+1);
          }
          if (_beamCount >= 2){
            writeFrame(BEAMB,frame+1+1);
          }
          if (_beamCount >= 3){
            writeFrame(BEAMC,frame+1);
          }
          if (_beamCount >= 4){
            writeFrame(BEAMD,frame+1);
          }*/


          if(_gblMode == 0){
            writeFrame(_currBeam,frame);
            _lastFrameWrite = frame;
          }
          /*else if (_beamCount == 2){
            writeFrame(BEAMA,frame+1+1);
            writeFrame(BEAMB,frame+1);
            _lastFrameWrite = frame + 1 + 1;
          }
          else if (_beamCount == 3){
            writeFrame(BEAMA,frame+1+1+1);
            writeFrame(BEAMB,frame+1+1);
            writeFrame(BEAMC,frame+1);
            _lastFrameWrite = frame + 1 + 1 + 1;
          }
          else if (_beamCount == 4){
            writeFrame(BEAMA,frame+1+1+1+1);
            writeFrame(BEAMB,frame+1+1+1);
            writeFrame(BEAMC,frame+1+1);
            writeFrame(BEAMD,frame+1);
            _lastFrameWrite = frame + 1 + 1 + 1 + 1;
          }         */

          int x;
          for (x=0;x<12;x++){
            cs[x] = 0x00;
            cscolumn[x*2] = 0x00;
            cscolumn[x*2+1] = 0x00;
          }

          #if DEBUG
          Serial.println(" ");
          #endif

          frame = frame + 1;    // go to next frame
          cscount = 0;        // reset cscount
          _lastFrameWrite = frame;

          // if a specific frame is specified, then return if that frame is done.
          if (frameToPrint!=0 && frame > frameToPrint){
              //defaults Beam to basic settings
              setPrintDefaults(SCROLL, 0, _lastFrameWrite, 7, 15, 1, 1);
              return;
          }

      }

    }

}


void Beam::play(){

    #if DEBUG
    Serial.println("play() called");
    #endif

    if (_gblMode == 1){

        //start playing beams depending on scroll direction
        if (_scrollDir == LEFT){
            if (_beamCount == 1){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
            } else if (_beamCount == 2){
                sendWriteCmd(BEAMB, CTRL, SHDN, 0x03);
            } else if (_beamCount == 3){
                sendWriteCmd(BEAMC, CTRL, SHDN, 0x03);
            } else if (_beamCount == 4) {
                sendWriteCmd(BEAMD, CTRL, SHDN, 0x03);
            }
        } else if (_scrollDir == RIGHT) {
            if (_beamCount ==1){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
            } else if (_beamCount == 2){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
            } else if (_beamCount == 3){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
            } else if (_beamCount == 4) {
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
            }
        }

        if (_beamCount > 1) {
            while (checkStatus() != 1){
                delay(10);
            }
        }

    } else {
        //start playing current beam
        sendWriteCmd(_currBeam, CTRL, SHDN, 0x03);
    }

    #if DEBUG
    Serial.println("play() done");
    #endif

}

void Beam::startNextBeam(){

    Serial.println(_scrollDir);
    Serial.println(_beamCount);
    Serial.println(beamNumber);

    /*start playing beams depending on scroll direction*/
    if (_scrollDir == LEFT){
      if (_beamCount == 2){
        sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
      }
    }

}


void Beam::setScroll(uint8_t direction, uint8_t fade){

    if (!(direction == RIGHT || direction == LEFT)){
        #if DEBUG
        Serial.println("Select either LEFT or RIGHT for direction");
        #endif
        return;
    }

    _scrollDir = direction;
    _fadeMode = fade;
    _scrollMode = 1;

    uint8_t frameData = _fadeMode << 7 | _scrollDir << 6 | 0 << 5 | _scrollMode << 4 | _frameDelay;

    if (_gblMode == 1){
        if(_beamCount >= 1){
            sendWriteCmd(BEAMA, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 2){
            sendWriteCmd(BEAMB, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 3){
            sendWriteCmd(BEAMC, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 4){
            sendWriteCmd(BEAMD, CTRL, FRAMETIME, frameData);
        }

    } else {
        sendWriteCmd(_currBeam, CTRL, FRAMETIME, frameData);
    }

}

void Beam::setSpeed (uint8_t speed){

    if (!(speed >= 1 && speed <= 15)){
        #if DEBUG
        Serial.println("Enter a speed between 1 and 15");
        #endif
        return;
    }

    if (_beamMode == MOVIE){
        _scrollMode = 0;
    } else {
        _scrollMode = 1;
    }

    _frameDelay = speed;

    uint8_t frameData = _fadeMode << 7 | _scrollDir << 6 | 0 << 5 | _scrollMode << 4 | _frameDelay;

    if (_gblMode == 1){
        if(_beamCount >= 1){
          sendWriteCmd(BEAMA, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 2){
          sendWriteCmd(BEAMB, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 3){
          sendWriteCmd(BEAMC, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 4){
          sendWriteCmd(BEAMD, CTRL, FRAMETIME, frameData);
        }

    } else {
        sendWriteCmd(_currBeam, CTRL, FRAMETIME, frameData);
    }

}

void Beam::setLoops (uint8_t loops){

    if (!(loops >= 1 && loops <= 7)){
        #if DEBUG
        Serial.println("Enter a speed between 1 and 7");
        #endif
        return;
    }

    _numLoops = loops;
    uint8_t displayData = _numLoops << 5 | 0 << 4 | 0x0B;

    if (_gblMode == 1){

        if(_beamCount >= 1){
            sendWriteCmd(BEAMA, CTRL, DISPLAYO, displayData);
        }
        if (_beamCount >= 2){
            sendWriteCmd(BEAMB, CTRL, DISPLAYO, displayData);
        }
        if (_beamCount >= 3){
            sendWriteCmd(BEAMC, CTRL, DISPLAYO, displayData);
        }
        if (_beamCount >= 4){
            sendWriteCmd(BEAMD, CTRL, DISPLAYO, displayData);
        }

    } else {

        sendWriteCmd(_currBeam, CTRL, DISPLAYO, displayData);

    }


}


void Beam::setMode (uint8_t mode){

    if (!(mode == MOVIE || mode == SCROLL)){
        #if DEBUG
        Serial.println("Select either SCROLL or MOVIE for mode");
        #endif
        return;
    }

    _beamMode = mode;
    uint8_t frameData = 0;

    if (mode == MOVIE){
        frameData = 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | _frameDelay;
    } else if (mode == SCROLL){
        frameData = _fadeMode << 7 | _scrollDir << 6 | 0 << 5 | _scrollMode << 4 | _frameDelay;
    }

    if (_gblMode == 1){

        if(_beamCount >= 1){
            sendWriteCmd(BEAMA, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 2){
            sendWriteCmd(BEAMB, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 3){
            sendWriteCmd(BEAMC, CTRL, FRAMETIME, frameData);
        }
        if (_beamCount >= 4){
            sendWriteCmd(BEAMD, CTRL, FRAMETIME, frameData);
        }

    } else {

        sendWriteCmd(_currBeam, CTRL, FRAMETIME, frameData);

    }

}


/*
    Used by global mode to check when daisy chained Beams
    should be activated depending on the scroll direction.
*/
int Beam::checkStatus(){

    int frameDone = 0;

    if (_beamCount == 4){
        if (activeBeams == 4){
            frameDone = (sendReadCmd(BEAMD, CTRL, 0x0F)>>2);
            if (frameDone == 1){
                sendWriteCmd(BEAMC, CTRL, SHDN, 0x03);
                activeBeams--;
                return 0;
            }
        }

        if (activeBeams == 3){
            frameDone = (sendReadCmd(BEAMC, CTRL, 0x0F)>>2);
            if (frameDone == 2){
                sendWriteCmd(BEAMB, CTRL, SHDN, 0x03);
                activeBeams--;
                return 0;
            }
        }

        if (activeBeams == 2){
            frameDone = (sendReadCmd(BEAMB, CTRL, 0x0F)>>2);
            if (frameDone == 3){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
                activeBeams--;
                delay(10);
                activeBeams = _beamCount;
                return 1;
            }
        }

    } else if (_beamCount == 3){

        if (activeBeams == 3){
            frameDone = (sendReadCmd(BEAMC, CTRL, 0x0F)>>2);
            if (frameDone == 1){
                sendWriteCmd(BEAMB, CTRL, SHDN, 0x03);
                activeBeams--;
                return 0;
            }

        }

        if (activeBeams == 2){
            frameDone = (sendReadCmd(BEAMB, CTRL, 0x0F)>>2);
            if (frameDone == 2){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
                activeBeams--;
                delay(10);
                activeBeams = _beamCount;
                return 1;
            }

        }

    } else if (_beamCount == 2) {

        if (activeBeams == 2){
            frameDone = (sendReadCmd(BEAMB, CTRL, 0x0F)>>2);
            if (frameDone == 1){
                sendWriteCmd(BEAMA, CTRL, SHDN, 0x03);
                activeBeams--;
                activeBeams = _beamCount;
                return 1;
            }

        }

    }

    return 0;

}


void Beam::draw(){

    // resets beam - will clear all beams, see note on page 24
    // of AS1130 datasheet

    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(100);
    digitalWrite(_rst, HIGH);
    delay(250);
    initBeam();

    for (int i=0; i < MAXFRAME; ++i){

    // see convertFrameFromRAM for old RAM based frame storage
    convertFrame(i);

    if (_gblMode == 1){
        if(_beamCount == 1){
            writeFrame(BEAMA,i);
            _lastFrameWrite = i;
        }
        else if (_beamCount == 2){
            writeFrame(BEAMA,i+1+1);
            writeFrame(BEAMB,i+1);
            _lastFrameWrite = i + 1 + 1;
        }
        else if (_beamCount == 3){
            writeFrame(BEAMA,i+1+1+1);
            writeFrame(BEAMB,i+1+1);
            writeFrame(BEAMC,i+1);
            _lastFrameWrite = i + 1 + 1 + 1;
        }
        else if (_beamCount == 4){
            writeFrame(BEAMA,i+1+1+1+1);
            writeFrame(BEAMB,i+1+1+1);
            writeFrame(BEAMC,i+1+1);
            writeFrame(BEAMD,i+1);
            _lastFrameWrite = i + 1 + 1 + 1 + 1;
        }
    } else {

        writeFrame(_currBeam,i);
        _lastFrameWrite = i;

    }

    // reset cs[]
    for (int d=0; d<12; ++d){
      cs[d] = 0x00;
    }

    }

    setPrintDefaults(MOVIE, 1, MAXFRAME, 7, 2, 1, 0);
}

void Beam::display(int frameNum){

      uint8_t pictureData = 0 << 7 | 1 << 6 | frameNum;
      uint8_t displaydata = 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0x0B;

      sendWriteCmd(_currBeam, CTRL, PIC, pictureData);
      sendWriteCmd(_currBeam, CTRL, DISPLAYO, displaydata);
}

int Beam::status(){

    int frameDone = 0;

    if (_gblMode == 0){
        frameDone = (sendReadCmd(_currBeam, CTRL, 0x0F)>>2);

        //if (frameDone == (_beamCount + 1)){
            #if DEBUG
            Serial.println(frameDone);
            #endif
            return frameDone;
        //}
    }

}


/*
=================
PRIVATE FUNCTIONS
=================
*/

void Beam::initializeBeam(uint8_t baddr){

    //set basic config on each defined beam unit
    sendWriteCmd(baddr, CTRL, CFG, 0x01);

    //set each frame to off since cs[] is reset by default
    for (int i=0;i<36;i++){
        writeFrame(baddr, i);
    }

    //set basic blink + pwm registers for each defined beam
    for (int i=0x40; i<=0x45; i++)
    {
        for (int j=0x00; j<=0x17; j++)
        {
            sendWriteCmd(baddr, i, j, 0x00);
        }
        for (int k=0x18; k<=0x9b; k++)
        {
            sendWriteCmd(baddr, i, k, 0xFF);
        }
    }

}



void Beam::setPrintDefaults (uint8_t mode, uint8_t startFrame, uint8_t numFrames, uint8_t numLoops, uint8_t frameDelay, uint8_t scrollDir, uint8_t fadeMode){

  _scrollMode = 1;
  _scrollDir = scrollDir;
  _fadeMode = fadeMode;
  _frameDelay = frameDelay;
  _beamMode = mode;
  _numLoops = numLoops;

 if (mode == MOVIE || mode == SCROLL) {

    //make sure startFrame between 0 and 35
    //make sure numFrames between 2 and 36
    //make sure frameDelay between 0 and 1111
    //make sure numLoops between 000 and 111

    uint8_t movieData =  0 << 7 | 1 << 6 | startFrame;
    uint8_t moviemodeData = 0 << 7 | 0 << 6 | _lastFrameWrite;
    uint8_t frameData = 0;
    uint8_t syncData = 0;

    switch (mode) {
      case MOVIE:
        frameData = 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | frameDelay;
        break;
      case SCROLL:
        frameData = fadeMode << 7 | scrollDir << 6 | 0 << 5 | _scrollMode << 4 | frameDelay;
        break;
    }

    uint8_t displayData = _numLoops << 5 | 0 << 4 | 0x0B;
    uint8_t irqmaskData = 0xFF;
    uint8_t irqframedefData = 0x03;
    uint8_t currsrcData = 0;

    // change led current based on number of connected beams
    if (_beamCount == 4){
        currsrcData = 0x08;
    } else if (_beamCount == 3){
        currsrcData = 0x10;
    } else if (_beamCount == 2){
        currsrcData = 0x20;
    } else if (_beamCount == 1){
        currsrcData = 0x20;
    } else {
        currsrcData = 0x15;
    }


    if (_gblMode == 1){

        if (_scrollDir == LEFT){
            if(_beamCount >= 1){
              sendWriteCmd(BEAMA, CTRL, MOV, movieData );
              sendWriteCmd(BEAMA, CTRL, MOVMODE, moviemodeData);
              sendWriteCmd(BEAMA, CTRL, CURSRC, currsrcData);
              sendWriteCmd(BEAMA, CTRL, FRAMETIME, frameData);
              sendWriteCmd(BEAMA, CTRL, DISPLAYO, displayData);
              sendWriteCmd(BEAMA, CTRL, SHDN, 0x02);
            }

            if(_beamCount >= 2){
              sendWriteCmd(BEAMB, CTRL, MOV, movieData);
              sendWriteCmd(BEAMB, CTRL, MOVMODE, moviemodeData);
              sendWriteCmd(BEAMB, CTRL, CURSRC, currsrcData);
              sendWriteCmd(BEAMB, CTRL, FRAMETIME, frameData);
              sendWriteCmd(BEAMB, CTRL, DISPLAYO, displayData);
              sendWriteCmd(BEAMB, CTRL, SHDN, 0x02);
            }

            if(_beamCount >= 3){
              sendWriteCmd(BEAMC, CTRL, MOV, movieData);
              sendWriteCmd(BEAMC, CTRL, MOVMODE, moviemodeData);
              sendWriteCmd(BEAMC, CTRL, CURSRC, currsrcData);
              sendWriteCmd(BEAMC, CTRL, FRAMETIME, frameData);
              sendWriteCmd(BEAMC, CTRL, DISPLAYO, displayData);
              sendWriteCmd(BEAMC, CTRL, SHDN, 0x02);
            }

            if(_beamCount >= 4){
              sendWriteCmd(BEAMD, CTRL, MOV, movieData);
              sendWriteCmd(BEAMD, CTRL, MOVMODE, moviemodeData);
              sendWriteCmd(BEAMD, CTRL, CURSRC, currsrcData);
              sendWriteCmd(BEAMD, CTRL, FRAMETIME, frameData);
              sendWriteCmd(BEAMD, CTRL, DISPLAYO, displayData);
            }

        } else if (_scrollDir == RIGHT) {
            //NEED TO MODIFY  FOR RIGHT OR LEFT SCROLL//
        }

    } else {
        sendWriteCmd(_currBeam, CTRL, MOV, movieData );
        sendWriteCmd(_currBeam, CTRL, MOVMODE, moviemodeData);
        sendWriteCmd(_currBeam, CTRL, CURSRC, currsrcData);
        sendWriteCmd(_currBeam, CTRL, FRAMETIME, frameData);
        sendWriteCmd(_currBeam, CTRL, DISPLAYO, displayData);
        sendWriteCmd(_currBeam, CTRL, SHDN, 0x02);
    }



    if (_gblMode == 1){

        /* define clk sync in/out settings based on left/right scrolling direction */
        if (_scrollDir == LEFT){
          if (_beamCount == 2){
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x01);
          } else if (_beamCount == 3){
            sendWriteCmd(BEAMC, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x01);
          } else if (_beamCount == 4) {
            sendWriteCmd(BEAMD, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMC, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x01);
          }

        } else if (_scrollDir == RIGHT) {
          if (_beamCount == 2){
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x01);
          } else if (_beamCount == 3){
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMC, CTRL, CLKSYNC, 0x01);
          } else if (_beamCount == 4) {
            sendWriteCmd(BEAMA, CTRL, CLKSYNC, 0x02);
            sendWriteCmd(BEAMB, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMC, CTRL, CLKSYNC, 0x01);
            sendWriteCmd(BEAMD, CTRL, CLKSYNC, 0x01);
          }
        }
    } else {


    }
  }
}


unsigned int Beam::setSyncTimer(){

  unsigned int timeDelay = 0;

  switch (_frameDelay){
    case 1:
      timeDelay = 32.5;
    break;
    case 2:
      timeDelay = 65;
    break;
    case 3:
      timeDelay = 97.5;
    break;
    case 4:
      timeDelay = 130;
    break;
    case 5:
      timeDelay = 162.5;
    break;
    case 6:
      timeDelay = 195;
    break;
    case 7:
      timeDelay = 227.5;
    break;
    case 8:
      timeDelay = 260;
    break;
    case 9:
      timeDelay = 292.5;
    break;
    case 10:
      timeDelay = 325;
    break;
    case 11:
      timeDelay = 357.5;
    break;
    case 12:
      timeDelay = 390;
    break;
    case 13:
      timeDelay = 422.5;
    break;
    case 14:
      timeDelay = 455;
    break;
    case 15:
      timeDelay = 487.5;
    break;
    default:
      timeDelay = 10000;
    break;
    }
    return timeDelay;

}

void Beam::writeFrame(uint8_t addr, uint8_t f){

    uint8_t p = f;
    #if DEBUG
    Serial.print("writing frame ");
    Serial.print(p);
    Serial.print(" = ");
    #endif
    int data = 0;

    for (int j=0x00; j<=0x0B; j++)
    {
        sendWriteCmd(addr, p+1, 2*j,   cs[data]&0xFF);          // i = frame address, 2*j = frame register address (even numbers) then first data byte
        sendWriteCmd(addr, p+1, 2*j+1, (cs[data]&0x300)>>8);    // i = frame address, 2*j+1 = frame register address (odd numbers) then second data byte

        //Serial.print(cs[data]&0xFF, HEX);
        //Serial.print(" ");
        //Serial.print((cs[data]&0x300)>>8, HEX);
        //Serial.print(" ");

        data++;
    }
    #if DEBUG
    Serial.println("Done writing frame");
    #endif
}



//void Beam::convertFrame(uint8_t *currentFrame){
void Beam::convertFrame(uint16_t currentFrame){
    int i=0;

    //CS0 to CS3
    int n=0;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[0] = cs[0] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[1] = cs[1] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[2] = cs[2] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[3] = cs[3] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>12){
            n = 0;
        }

    }

    //CS4 to CS7
    n = 1;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[4] = cs[4] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[5] = cs[5] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[6] = cs[6] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[7] = cs[7] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>13){
            n = 1;
        }
    }

    //CS8 - CS11
    n = 2;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[8] = cs[8] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[9] = cs[9] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[10] = cs[10] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[11] = cs[11] | (((pgm_read_byte_near(&frameList[currentFrame][n]) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>14){
            n = 2;
        }
    }
}

void Beam::sendWriteCmd(uint8_t addr, uint8_t ramsection, uint8_t subreg, uint8_t subregdata){

    int stat;
    stat = i2cwrite(addr, REGSEL, ramsection);
    if (stat == 0) {
        i2cwrite(addr, subreg, subregdata);
    }
    else
    {
        #if DEBUG
        Serial.print("Beam not found: ");
        Serial.print(addr);
        Serial.println("");
        #endif
    }

}

uint8_t Beam::sendReadCmd(uint8_t addr, uint8_t ramsection, uint8_t subreg){

  uint8_t c;
  i2cwrite(addr, REGSEL, ramsection);

  Wire.beginTransmission(addr);
  Wire.write(subreg);
  Wire.endTransmission();

  Wire.requestFrom(addr, 1);
  while(Wire.available())
  {
    c = Wire.read();
    //Serial.println(c, HEX);
    return c;
  }

}

uint8_t Beam::i2cwrite(uint8_t address, uint8_t cmdbyte, uint8_t databyte) {

    Wire.beginTransmission(address);
    Wire.write(cmdbyte);
    Wire.write(databyte);
    return (Wire.endTransmission());

}

// convert a frame stored in RAM as a 15 (3x5) byte array
void Beam::convertFrameFromRAM(uint8_t *pFrameData){
    int i=0;

    //CS0 to CS3
    int n=0;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[0] = cs[0] | (((*(pFrameData + n) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[1] = cs[1] | (((*(pFrameData + n) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[2] = cs[2] | (((*(pFrameData + n) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[3] = cs[3] | (((*(pFrameData + n) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>12){
            n = 0;
        }

    }

    //CS4 to CS7
    n = 1;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[4] = cs[4] | (((*(pFrameData + n) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[5] = cs[5] | (((*(pFrameData + n) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[6] = cs[6] | (((*(pFrameData + n) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[7] = cs[7] | (((*(pFrameData + n) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>13){
            n = 1;
        }
    }

    //CS8 - CS11
    n = 2;
    for (int y=10; y>0; --y){

        if (y < 6){
            i=1;
        } else {
            i=0;
        }
        cs[8] = cs[8]   | (((*(pFrameData + n) & segmentmask[0+i]) <<(3+i)) >> y);
        cs[9] = cs[9]   | (((*(pFrameData + n) & segmentmask[2+i]) <<(5+i)) >> y);
        cs[10] = cs[10] | (((*(pFrameData + n) & segmentmask[4+i]) <<(7+i)) >> y);
        cs[11] = cs[11] | (((*(pFrameData + n) & segmentmask[6+i]) <<(9+i)) >> y);
        n=n+3;

        if (n>14){
            n = 2;
        }
    }
}

// load a frame stored in RAM to a given BEAM at a given frame
// number see note on see note on page 24 of AS1130 datasheet
void Beam::loadFrameFromRAM(int beam, uint8_t frameNum, uint8_t *pFrameData) {

  if (beam < 0) {
    beam = _currBeam;
  }

  convertFrameFromRAM(pFrameData);
  writeFrame(beam, frameNum);
}