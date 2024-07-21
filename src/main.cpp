#include <FastLED.h>
#include <Arduino.h>

#define NUM_LEDS 8
#define DATA_PIN 2 // Utilise la broche D2

CRGB leds[NUM_LEDS];
CRGB initial;

void mesureSound();

void setup()
{
  /**
   * Initializes the LED strip connected to the specified data pin.
   * The LED strip uses the WS2812B LED type and the GRB color order.
   * The number of LEDs in the strip is specified by the NUM_LEDS macro.
   */
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(13, OUTPUT); // sets the digital pin 13 as output

  /**
   * Iterates through the LED strip and sets the color of each LED based on its position in the strip.
   * The color ranges from blue at the first LED to green at the last LED.
   */
  for (int i = 0; i < NUM_LEDS; i++)
  {
    int blue = map(i, 0, NUM_LEDS - 1, 255, 0);
    int green = map(i, 0, NUM_LEDS - 1, 0, 255);
    leds[i] = CRGB(150, blue, green);
  }

  /**
   * Stores the initial color of the LED at index 5 in the LED strip.
   * This value is used to restore the LED to its original color later.
   */
  initial = leds[5];

  /**
   * Sets the overall brightness of the LED strip to 80% of its maximum brightness.
   * This can be used to adjust the brightness of the entire LED strip without
   * changing the relative brightness of individual LEDs.
   */
  FastLED.setBrightness(80);

  /**
   * Displays the current LED strip configuration on the physical LED strip.
   * This function should be called after any changes have been made to the
   * `leds` array to update the physical LED strip to match the new configuration.
   */
  FastLED.show();

  /**
   * Initializes the serial communication interface at a baud rate of 115200 bps.
   * This allows the Arduino board to communicate with a computer or other serial devices.
   * The serial communication can be used for debugging, sending/receiving data, or other purposes.
   */
  Serial.begin(115200);
}

void mesureSound()
{
  int sampleWindow = 20;
  float sum = 0;
  for (int i = 0; i < sampleWindow; i++)
  {
    int sample = analogRead(A0);
    sum = sample * sample; // Somme des carrés
    Serial.println(sample);
    //    Serial.println(sum);
    delayMicroseconds(100); // Petit délai entre les échantillons
  }

  // Calcul de la moyenne RMS
  double rms = sqrt(sum / sampleWindow);

  // Serial.print("RMS: ");
  // Serial.println(rms);
}

void loop()
{

  if (leds[5] == initial)
  {
    leds[5] = CRGB(255, 0, 0);
  }
  else
  {
    leds[5] = initial;
  }
  mesureSound();
  FastLED.show();
  delay(100);
}
