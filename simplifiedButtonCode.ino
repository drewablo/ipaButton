#include <Adafruit_NeoPixel.h>
#include <ProTrinketKeyboard.h>

bool Forward = true; 

unsigned long Interval;   // milliseconds between updates
unsigned long lastUpdate; // last update of position

uint32_t Color1, Color2;  // What colors are in use for our fade
uint16_t TotalSteps;  // total number of steps in the pattern
uint16_t Index;  // current step within the pattern

Adafruit_NeoPixel ring (12, 6, NEO_RGBW + NEO_KHZ800);

uint16_t scanIndex;

void setup()
{

  pinMode(12, INPUT_PULLUP);

  ring.begin();
  Color1 = ring.Color(0, 255, 0, 0);
  Color2 = ring.Color(0, 0, 0, 0);
  
  TrinketKeyboard.begin();

}

// Main loop
void loop()
{
  TrinketKeyboard.poll();
  if (digitalRead(12) == LOW) // Button pressed
  {
    
    TrinketKeyboard.pressKey(KEYCODE_MOD_LEFT_CONTROL, KEYCODE_P); // Send print command
    TrinketKeyboard.pressKey(0, 0); // release keys
    //---------------------------
    //      CLEAR RING 
    //---------------------------
    for ( int i = 0; i < 12; i++) { 
      TrinketKeyboard.poll();
      ring.setPixelColor(i, 0x000000);
    }
    ring.show();
    //---------------------------
    //    Spring the ring 5 times
    //---------------------------
    for (int k = 0; k < 5; k++) {
      TrinketKeyboard.poll();
    //---------------------------
    //    Each time around the ring consists of 12 frames
    //---------------------------
      for (int j = 0; j < 13; j++) {
        TrinketKeyboard.poll();
    //---------------------------
    //    Set the individual pixes for each frame
    //---------------------------
        for (int i = 0; i < 12; i++)
        {
          TrinketKeyboard.poll();
          if (i == scanIndex)  // Scan Pixel to the right
          {
            ring.setPixelColor(i, ring.Color(0, 255, 0));
          }
          else // Fading tail
          {
            
            ring.setPixelColor(i, Red(ring.getPixelColor(i) >> 1), Green(ring.getPixelColor(i) >> 1), Blue(ring.getPixelColor(i) >> 1));
          }
        }
        ring.show();
    //-----------------------------------------------------
    //    Delay to prevent overlapping printing commands
    //-----------------------------------------------------
        for (int y = 0; y < 20; y++) {
          TrinketKeyboard.poll();
          delay(5);
        }
        scanIndex++;
        if (scanIndex >= 11)
        {
          scanIndex = 0;
        }
      }
    }
  } else {
    //-----------------------------------------------------
    //    This is the green breathing section
    //-----------------------------------------------------
  TrinketKeyboard.poll();
  if ((millis() - lastUpdate) > Interval) // time to update
      {
        TrinketKeyboard.poll();
        lastUpdate = millis();
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        for (int i = 0; i < 12; i++)
        {
          ring.setPixelColor(i, red, green, blue, 0);
        }
        ring.show();
        if (Forward == true)
        {
          Index++;
          if (Index > TotalSteps)
          {
            Forward = false;
            Index = TotalSteps - 1;
          }
        }
        else // Direction == REVERSE
        {
          --Index;
          if (Index < 0)
          {
            Forward = true;
            Index = 0;
          }
        }
      }
  }
}



// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
  return color & 0xFF;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}



