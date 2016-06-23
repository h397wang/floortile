#include <Adafruit_NeoPixel.h>

#define PIN 5
#define NUMPIXELS 16
#define NUMSWITCHES 4

int score = 0; 

int fsrPins[NUMSWITCHES] = {6,7,8,9};

// ranges from 0 to 4, for each tile
int lavaTile = 0;
int nextLavaTile = 0;


#define LAVA_TIME 5 // time in seconds that the tile is red for
#define WARNING_TIME 3 // time in seconds that flickers 

int lavaTime = 3; // in seconds

bool whiteOn = true; 

Adafruit_NeoPixel strip = Adafruit_NeoPixel(80, PIN, NEO_GRB + NEO_KHZ800);

int colorArray[][3] = { {255,20,0},    //red
                        {0,204 ,0}, //green
                        {128,0,255}, //purple
                        {255,255,0}, //yellow
                        {0,191,255}, //cyan
                        {255,128,0}, //orange
                        {0,0,255}, //blue
                        {255,0,255}, //magenta
                        {102,51,0}, //brown??????
                        {127,127,127}, //white
                        {127,127,200}, //white/blue
                        {255,129,129}, //white/red
                        {255,255,80}, //white/yellow
                        {80,255,80}, //white/green
                        {255,80,255}, //dummy color
                        {127,127,200}, //dummy
                        {255,129,129}, //dummy
                        {255,255,80}, //dummy
                        {80,255,80}, //dummy color
                        {80,255,80}}; //dummy color

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i<(sizeof(fsrPins)/sizeof(int)); i++){
    pinMode(fsrPins[i],INPUT_PULLUP);
  }
  Serial.begin(9600);

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  nextLavaTile = random(0, 4); // four tiles

  
    // INITIALIZE TIMER INTERRUPTS
  cli(); // disable global interrupts

  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s

  // Bit: 7,6,5,4,3,2,1,0
  // WGM12: 3, CS10: 1, CS12: 2
  TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match

  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  sei(); // enable global interrupts
  
}


void loop() {
  // put your main code here, to run repeatedly:
  int state = 0;

  int switchStates[NUMSWITCHES];
  switchRead(switchStates); // updates the flags

  flicker(switchStates); // sets the colours
  delay(50);
}



//returns array based on which inputs are set to high
void switchRead(int switchStates[]){
  for (int i = 0; i<(sizeof(fsrPins)/sizeof(int)); i++){
    if(digitalRead(fsrPins[i]) == LOW){
      switchStates[i] = 1;
    }
    else {
      switchStates[i] = 0; 
    }
  }
}

void updateLava(){

  
}


// every three seconds, update the current lavaTile and the next lavaTile

ISR(TIMER1_COMPA_vect) //is this every 1 second???
{
 
    // make the next laval tile flicker
   if (lavaTime == 2){
      // start flicking 
   }else if (lavaTime <= 0){
      // turn them white

      int copyNextLavaTile = nextLavaTile;
     
      nextLavaTile = random(0,4);      // change the location of the next lava
      while(nextLavaTile == lavaTile){
        nextLavaTile = random(0,4);
      }

      lavaTile = copyNextLavaTile;
      lavaTime = LAVA_TIME;

    }
    
    lavaTime--;
}


void flicker(int switchStates[]){
    int R = 0;
    int G = 0;
    int B = 0;
    int noise = 0 ;
    int updateNumber = 0;
    int tileIndex = 0;
    int colorNum = 0;
    
     
    for (uint16_t i = 0; i < NUMPIXELS; i = i +1){
      
      //Serial.println(i);
      noise = random (-20,20);
      updateNumber  = random(0,100);
      tileIndex = i/4;

      // assign colour according to switch state
      if (switchStates[tileIndex] == 0){
        colorNum = 9; // white when not stpped on
      }
      else if (switchStates[tileIndex] == 1)
      {

        // test if lava has been stepped on
        if (tileIndex == lavaTile){
          Serial.println("You stepped on lava");
        }
        colorNum = 1; // green when stepped on
      }
      
      //clamping values for RGB, if over 255 rolls over
      R = constrain(colorArray[colorNum][0]+noise,0,255);
      G = constrain(colorArray[colorNum][1]+noise,0,255);
      B = constrain(colorArray[colorNum][2]+noise,0,255);
  
      //random chance for updating,increases flicker
      if (updateNumber >= 8){
        strip.setPixelColor(i,strip.Color(R,G,B));

        
        // update the current lava pixels;
        if ( i >= 4*lavaTile && i < 4*lavaTile + 4){
          strip.setPixelColor(i, strip.Color(250,20,0));
        }
         // leave the pixels be in the nextLavaTiles
      }

        // make the pixels in the nextLavaTiles flicker, turn them all on or off
     if (updateNumber >= 95 && lavaTime <= WARNING_TIME){
        if (whiteOn){
          for (int j = 4*nextLavaTile; j < 4*nextLavaTile + 4; j++){
            strip.setPixelColor(j, strip.Color(255,20,0));
         }
          whiteOn = false;
        }else{
          for (int j = 4*nextLavaTile; j < 4*nextLavaTile + 4; j++){
            strip.setPixelColor(j, strip.Color(127,127,127));
          }
          whiteOn = true;
        }  
     }
     

   }
  strip.show();
}

