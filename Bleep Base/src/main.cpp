#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform3;      //xy=132.5047429402669,344.17140324910474
AudioSynthWaveform       waveform4;      //xy=134.1714096069336,394.17140674591064
AudioSynthWaveform       waveform1;      //xy=140.00001525878906,238.8333215713501
AudioSynthWaveform       waveform2;      //xy=143.50000381469727,301.33333587646484
AudioMixer4              mixer1;         //xy=326.4999237060547,313.16669273376465
AudioFilterStateVariable filter1;        //xy=484.8532943725586,285.8380069732666
AudioEffectDelay         delay1;         //xy=654.3333168029785,503.4998531341553
AudioMixer4              mixer2;         //xy=665.3715057373047,302.05106353759766
AudioOutputI2S           i2s1;           //xy=732.9999542236328,186.35896968841553
AudioConnection          patchCord1(waveform3, 0, mixer1, 2);
AudioConnection          patchCord2(waveform4, 0, mixer1, 3);
AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
AudioConnection          patchCord4(waveform2, 0, mixer1, 1);
AudioConnection          patchCord5(mixer1, 0, filter1, 0);
AudioConnection          patchCord6(filter1, 0, mixer2, 0);
AudioConnection          patchCord7(filter1, 1, mixer2, 1);
AudioConnection          patchCord8(filter1, 2, mixer2, 2);
AudioConnection          patchCord9(delay1, 0, mixer2, 3);
AudioConnection          patchCord10(mixer2, delay1);
AudioConnection          patchCord11(mixer2, 0, i2s1, 0);
AudioConnection          patchCord12(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=262.1666793823242,165.5000171661377
// GUItool: end automatically generated code



float potToHz(int knobVal, int low, int high) {
  float knobDec = knobVal / 1023.0; // 0.0 - 1.0
  float logged = pow(2.0, knobDec * 10.0); // linear to logrithmic, 1 - 1024 (mid is 32 not 512)
  float mapped = map(logged, 1, 1024, low, high); // map to correct range

  return mapped;
}


//then you can declare any variables you want.
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"
float freq1, spacing1;

void setup() {
  //there's a lot we need to do in setup now but most of it is just copy paste.
  // This first group should only be done in setup how much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 4.1 we can go up to almost 2000 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(100);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary is you're not using the ins
  sgtl5000_1.lineInLevel(10); //The volume of the input. 0-15 with 15 bing more amplifications
  //sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  //sgtl5000_1.micGain(13); //0 - 63bd of gain.

  //headphone jack output volume. Goes from 0.0 to 1.0 but a 100% signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level.
  // For now lets set this one once and leave it alone.
  sgtl5000_1.volume(0.25);

  //The line out has a separate level control but it's not meant to be adjusted like the volume function above.
  // If you're not using the line out don't worry about it.
  sgtl5000_1.lineOutLevel(21); //11-32, the smaller the louder. 21 is about 2 Volts peak to peak


  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.

  //Notice we start by writing the object we want, then a period, then the function
  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  waveform1.begin(1, 300.0, WAVEFORM_SAWTOOTH);
  waveform2.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  waveform3.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  waveform4.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  //See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  // but there is also these options which reduce digital aliasing : WAVEFORM_BANDLIMIT_SAWTOOTH, WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE, WAVEFORM_BANDLIMIT_SQUARE ,WAVEFORM_BANDLIMIT_PULSE

  //Sets the frequency of the filter. We're using lowpass so all frequencies under this can pass unchanged
  // while ones above will be attenuated, the higher up the quieter they will be
  filter1.frequency(15000); //15000 Hz is the top frequency it can do and means pretty much nothing is attenuated
  //resonance aka feedback aka q increases the amplitude at the filter frequency
  filter1.resonance(3.0); //.7 is no change, higher that that makes the classic filter sound


  //delay(output, milliseconds of delay)
  // whatever comes in will come out that output X millis later
  //You can't change this delay time without making lots of noise. We'll use my "tape delay" later
  delay1.delay(0, 250);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // -1.0 would mean the same volume but the signal is upside down, aka 180 degrees out of phase

  //Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // but the filter needs some headroom for the resonance so we'll take it down a bit more
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .2);
  mixer1.gain(1, .2);
  mixer1.gain(2, .2);
  mixer1.gain(3, .2);
  //the other channels of the mixer aren't used so don't need to be set

  //the second mixer is for delay feedback
  mixer2.gain(0, .5);//low pass in
  mixer2.gain(1, 0);//bandpass in
  mixer2.gain(2, 0);//highpass in
  mixer2.gain(3, 0);//signal coming back from the delay


} //setup is over

void loop() {
  current_time = millis();

  freq1 = analogRead(A10);
  spacing1 = (analogRead(A11) / 1023.0) * 3.0;
  waveform1.frequency(freq1);
  waveform2.frequency(freq1 * spacing1);
  waveform3.frequency(freq1 * 2.01);
  waveform4.frequency(freq1 * 0.501);

  float amp1 = 1.0 - ( analogRead(A14) / 1023.0);


  int temp1 = analogRead(A13);

  if (temp1 < 333) {
    mixer2.gain(0, amp1);//low pass in
    mixer2.gain(1, 0);//bandpass in
    mixer2.gain(2, 0);//highpass in
  }
  if (temp1 >= 333 && temp1 < 666) {
    mixer2.gain(0, 0);//low pass in
    mixer2.gain(1, amp1);//bandpass in
    mixer2.gain(2, 0);//highpass in
  }
  if (temp1 >= 666) {
    mixer2.gain(0, 0);//low pass in
    mixer2.gain(1, 0);//bandpass in
    mixer2.gain(2, amp1);//highpass in
  }


//  float filter_freq = map(analogRead(A12), 0, 1023, 100, 15000);
  float filter_freq = potToHz(analogRead(A12), 40, 12000);
  filter1.frequency(filter_freq);

  float amp2 = 1.0 - (analogRead(A15) / 1023.0);
  mixer2.gain(3, amp2); //feedback

  if (current_time - prev_time[0] > 16) {
    prev_time[0] = current_time;

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // If you're using too many block, jut increase the number up top until you're over it by a couple
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    Serial.print("freq: ");
    Serial.print(filter_freq);
    Serial.print("knob: ");
    Serial.print(analogRead(A15));
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();
  }
}