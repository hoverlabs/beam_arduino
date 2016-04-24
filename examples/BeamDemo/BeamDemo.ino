/*
===========================================================================    

  This is an example for Beam. 
  
  Beam is a beautiful LED matrix — features 120 LEDs that displays scrolling text, animations, or custom lighting effects. 
  Beam can be purchased here: http://www.hoverlabs.co
  
  Written by Emran Mahbub and Jonathan Li for Hover Labs.  
  BSD license, all text above must be included in any redistribution

  
#  HISTORY
    v1.0  -  Initial Release

#  INSTALLATION
    The 4 library files (beam.cpp, beam.h and charactermap.h and frames.h) are required to run Beam.
    Run the BeamDemo.ino file.
    
#  SUPPORT
    For questions and comments, email us at support@hoverlabs.co
===========================================================================    
*/ 
#include "Arduino.h"
#include "Wire.h"
#include "stdint.h"
#include "beam.h"

/* pin definitions for Beam */
#define RSTPIN 5        //use any digital pin
#define IRQPIN 9        //currently not used - leave unconnected
#define BEAMCOUNT 1     //number of beams daisy chained together

/* Iniitialize an instance of Beam */
Beam b = Beam(RSTPIN, IRQPIN, BEAMCOUNT);

/* Timer used by the demo loop */
unsigned long updateTimer = 0;
int demo = 0;

void setup() {
    
    Serial.begin(9600);
    Wire.begin();
    
    Serial.println("Starting Beam example");
    
    b.begin();
    b.print("Hello World. This is Beam!");
    b.play();

}



void loop() {

    /*
    The following cycles through two different demos every 20 seconds. 
    Note that it is not necessary to print to Beam on every loop. 
    */
    if (millis() - updateTimer > 20000){
        
        if (demo == 0){
            /*
            The print() command prints and scrolls text across Beam. 
            */
            b.print("This is an example of fast scrolling text on Beam. ");
            b.setSpeed(3);
            b.setLoops(7);
            b.play();
            
            demo = 1;
            updateTimer = millis();
            
        } else if (demo == 1){
            /*
            The draw() command draws the frames defined in the frames.h header file, 
            and animates them as if flipping through a book. 
            */
            b.draw();
            b.setSpeed(2);
            b.setLoops(7);
            b.play();     
            
            demo = 0;
            updateTimer = millis();
        }

    } 

    // do something else here

}
