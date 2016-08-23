#include <Adafruit_NeoPixel.h>
#include <ProTrinketKeyboard.h>  // Ensure the library is installed

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
    }
    // Update the pattern
    void Update()
    {
      TrinketKeyboard.poll();
      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        FadeUpdate();
        TrinketKeyboard.poll();
      }
    }
    // Increment the Index and reset at the end
    void Increment()
    {
      TrinketKeyboard.poll();
      if (Direction == FORWARD)
      {
        Index++;
        if (Index >= TotalSteps)
        {
          Index = 0;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
      else // Direction == REVERSE
      {
        --Index;
        if (Index <= 0)
        {
          Index = TotalSteps - 1;
          if (OnComplete != NULL)
          {
            OnComplete(); // call the comlpetion callback
          }
        }
      }
    }
    // Reverse pattern direction
    void Reverse()
    {
      TrinketKeyboard.poll();
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else
      {
        Direction = FORWARD;
        Index = 0;
      }
    }
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = FADE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      TrinketKeyboard.poll();
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
      uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
      uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

      ColorSet(Color(red, green, blue));
      show();
      Increment();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
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
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }
};


void StickComplete();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
NeoPatterns Stick(12, 6, NEO_RGBW + NEO_KHZ800, &StickComplete);

uint16_t scanIndex;

void setup()
{

  pinMode(12, INPUT_PULLUP);
  // Initialize all the pixelStrips

  Stick.begin();
  // Kick off a pattern
  Stick.Fade(Stick.Color(255, 0, 0), Stick.Color(50, 0, 0), 200, 5);
  TrinketKeyboard.begin();

}

// Main loop
void loop()
{
  if (digitalRead(12) == LOW) // Button #1 pressed
  {
    clearStrip();
    for (int k = 0; k < 5; k++) {
      TrinketKeyboard.poll();
      for (int j = 0; j < 13; j++) {
        TrinketKeyboard.poll();
        for (int i = 0; i < 12; i++)
        {
          TrinketKeyboard.poll();
          if (i == scanIndex)  // Scan Pixel to the right
          {
            Stick.setPixelColor(i, Stick.Color(0, 255, 0));
          }
          else // Fading tail
          {
            Stick.setPixelColor(i, Stick.DimColor(Stick.getPixelColor(i)));
          }
        }
        Stick.show();
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
    Stick.Update();

  }
}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Stick Completion Callback
void StickComplete()
{
  TrinketKeyboard.poll();
  Stick.Reverse();

}


void clearStrip() {
  TrinketKeyboard.poll();
  for ( int i = 0; i < 12; i++) {
    Stick.setPixelColor(i, 0x000000); Stick.show();
  }
}
