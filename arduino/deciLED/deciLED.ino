#include <FastLED.h>

#define NUM_LEDS 8
#define DATA_PIN 2  // Utilise la broche D2

CRGB leds[NUM_LEDS];
CRGB initial;

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
   pinMode(13, OUTPUT);    // sets the digital pin 13 as output

  for(int i = 0; i < NUM_LEDS; i++) {
    int bleu = map(i, 0, NUM_LEDS-1, 255, 0);
    int vert = map(i, 0, NUM_LEDS -1, 0, 255);
    leds[i] = CRGB(150, vert, bleu);
  }
  initial = leds[5];
  FastLED.setBrightness(80);
  FastLED.show();
  

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);  
}


void mesureSound() {
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

void loop() {

  if ( leds[5] == initial ) {
    leds[5] = CRGB( 255, 0, 0);
  } else {
     leds[5] = initial;
  }
  mesureSound();
 FastLED.show();
  delay(100);
}

