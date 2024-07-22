#include <FastLED.h>
#include <Arduino.h>

/**
 * Defines the digital pin used to output the LED data signal.
 */
#define DATA_PIN 2

/**
 * Defines the analog input pin used for reading the audio signal,
 *  and the output PIN to pilot the ARGB led strip (WS2812B)
 */
#define AUDIO_PIN A0
#define ARGB_PIN 13 // D10

/**
 * Conditionally enables or disables debug printing based on the value of the DEBUG macro.
 * If DEBUG is set to 1, the DEBUG_PRINT and DEBUG_PRINTLN macros will output the provided
 * arguments to the Serial port. If DEBUG is set to 0, the macros will do nothing.
 * This allows for easy toggling of debug output without having to remove the print statements.
 * 
 * Call DEBUG_SETUP() in the setup function, to enable (or not) the serial mode.
 * Do not call it, if you use Serial.begin in your code
 */
#define DEBUG 0 // Set to 0 for deactivate debug mode
#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_SETUP()   Serial.begin(115200)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_SETUP()
#endif

/**
 * Defines the size of the sample window in milliseconds for measuring sound amplitude.
 * The sample window determines the duration over which sound samples are collected to
 * calculate the average amplitude.
 */
const int SAMPLE_WINDOW = 50;

/**
 * Defines the sample rate in Hz for measuring sound amplitude.
 * The sample rate determines how many sound samples are collected per second.
 */
const int SAMPLE_RATE = 1000;

/**
 * Calculates the size of the sample buffer based on the sample window and sample rate.
 * The buffer size is the number of samples that can be stored in the sample window duration.
 */
const long BUFFER_SIZE = (long(SAMPLE_WINDOW) * SAMPLE_RATE) / 1000;

/**
 * Defines a buffer to store sound samples for amplitude measurement.
 * The buffer size is determined by the SAMPLE_WINDOW and SAMPLE_RATE constants.
 * The bufferIndex variable keeps track of the current position in the buffer.
 * The lastSampleTime variable stores the timestamp of the last sample taken.
 */
int16_t sampleBuffer[BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastSampleTime = 0;

/**
 * Stores the timestamp of the last time the LED strip was updated.
 * This is used to control the update interval and prevent the LEDs from
 * being updated too frequently, which could cause flickering.
 */
unsigned long lastUpdateTime = 0;

/**
 * Defines the interval in milliseconds between updates to the LED strip.
 * This value determines how often the LED strip is updated to reflect
 * changes in the sound amplitude.
 */
const unsigned long UPDATE_INTERVAL = 200;

/**
 * Defines the number of LEDs in the LED strip.
 */
#define NUM_LEDS 8

/**
 * Defines an array of CRGB (Color RGB) objects to represent the LEDs in the LED strip.
 * The size of this array is determined by the NUM_LEDS macro, which specifies the
 * total number of LEDs in the LED strip.
 */
CRGB leds[NUM_LEDS];

/**
 * An array of CRGB color values that correspond to the LED levels in the strip.
 * The colors are arranged in ascending order, with the first LED having the lowest
 * level and the last LED having the highest level.
 */
const CRGB LEVEL_COLORS[NUM_LEDS] = {
    CRGB::Green,
    CRGB::GreenYellow,
    CRGB::Yellow,
    CRGB::Gold,
    CRGB::Orange,
    CRGB::OrangeRed,
    CRGB::Red,
    CRGB::DarkRed};

/**
 * Stores the number of active LEDs in the previous update.
 * This is used to determine how many LEDs need to be updated in the current update.
 */
int lastActiveLeds = 0;

/**
 * An array of threshold values used to determine the amplitude level for each LED in the strip.
 * The threshold values are used to map the sound amplitude to the brightness of the corresponding LED.
 * The threshold values are arranged in ascending order, with the first LED having the lowest threshold
 * and the last LED having the highest threshold.
 */
const float THRESHOLDS[NUM_LEDS] = {350, 360, 365, 370, 380, 390, 400, 410}; // To be adjusted

float calculateRMS();
void collectSample();
void updateLEDs(float);

void setup()
{
  /**
   * Initializes the LED strip connected to the specified data pin.
   * The LED strip uses the WS2812B LED type and the GRB color order.
   * The number of LEDs in the strip is specified by the NUM_LEDS macro.
   */
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(ARGB_PIN, OUTPUT); // sets the digital pin 13 as output

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
   * Sets the overall brightness of the LED strip to a % of its maximum brightness.
   * This can be used to adjust the brightness of the entire LED strip without
   * changing the relative brightness of individual LEDs.
   */
  FastLED.setBrightness(30);

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
  DEBUG_SETUP();
}

/**
 * Calculates the root-mean-square (RMS) value of the audio samples stored in the sample buffer.
 * The RMS value is a measure of the average magnitude of the audio signal, and is commonly used
 * to analyze the overall volume or intensity of an audio signal.
 *
 * @return The RMS value of the audio samples in the sample buffer.
 */
float calculateRMS()
{
  long sum = 0;
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    sum += (long)sampleBuffer[i] * sampleBuffer[i];
  }
  return sqrt(sum / BUFFER_SIZE);
}

/**
 * Reads an audio sample from the specified analog pin and stores it in a circular buffer.
 * The sample is read at the specified sample rate, and the buffer index is updated accordingly.
 */
void collectSample()
{
  /**
   * Checks if the time since the last sample was taken is greater than or equal to the time required
   * between samples based on the specified sample rate. This is used to ensure that samples are
   * taken at the correct interval to maintain the desired sample rate.
   */
  if (micros() - lastSampleTime >= 1000000 / SAMPLE_RATE)
  {
    // Updates the last sample time to the current time in microseconds.
    lastSampleTime = micros();
    /**
     * Reads an audio sample from the specified analog pin and stores it in the sample buffer.
     * This function is used to capture audio data from an external source connected to the analog pin.
     * The captured sample is stored in the circular buffer for further processing.
     */
    int sample = analogRead(AUDIO_PIN);
    sampleBuffer[bufferIndex] = sample;
    /**
     * Updates the buffer index to the next position in the circular buffer.
     * This ensures that the buffer index wraps around to the beginning of the buffer
     * when it reaches the end, allowing the buffer to be used as a continuous loop.
     */
    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  }
}

/**
 * Updates the LED strip to display the current audio level based on the provided RMS (root-mean-square) value.
 * The number of active LEDs is determined by comparing the RMS value to a set of predefined thresholds.
 * Any LEDs beyond the active LED count are turned off, and the LED strip is updated to reflect the new level.
 *
 * @param rms The RMS value of the audio samples, used to determine the number of active LEDs.
 */
void updateLEDs(float rms)
{
  int activeLeds = 0;

  /**
   * Iterates through the LED strip and sets the number of active LEDs based on the current RMS value.
   * The number of active LEDs is determined by comparing the RMS value to a set of predefined thresholds.
   * Any LEDs beyond the active LED count are turned off.
   */
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (rms > THRESHOLDS[i])
    {
      activeLeds = i + 1;
    }
    else
    {
      break;
    }
  }

  /**
   * Updates the LED strip to display the current audio level.
   * If the number of active LEDs has changed since the last update, the LED strip is updated to reflect the new level.
   * Each LED is set to a color corresponding to its position in the LEVEL_COLORS array, and any remaining LEDs are turned off.
   * The updated LED strip is then displayed using FastLED.show().
   */
  if (activeLeds != lastActiveLeds)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      if (i < activeLeds)
      {
        leds[i] = LEVEL_COLORS[i];
      }
      else
      {
        leds[i] = CRGB::Black;
      }
    }
    FastLED.show();
    lastActiveLeds = activeLeds;
  }
}

/**
 * The main loop of the program, which collects audio samples, calculates the RMS value, and updates the LED strip to display the current audio level.
 *
 * The loop first calls the `collectSample()` function to collect a new audio sample.
 * It then checks if the current time minus the last update time is greater than or equal to the `UPDATE_INTERVAL` constant.
 * If so, it calculates the RMS value of the collected samples using the `calculateRMS()` function, and then calls the `updateLEDs()` function with the RMS value as a parameter to update the LED strip.
 * Finally, it updates the `lastUpdateTime` variable with the current time.
 */
void loop()
{
  collectSample();

  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL)
  {
    float rms = calculateRMS();

    DEBUG_PRINTLN(rms);

    updateLEDs(rms);
    lastUpdateTime = currentTime;
  }
}
