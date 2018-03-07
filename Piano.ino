int btns[] = {30, 32, 34, 36, 38, 40, 42, 44}; //C4 to C5
int btnsSize = 8;
int leds[] = {31, 33, 35, 37, 39, 41, 43, 45};
int freq[] = {261, 293, 329, 349, 392, 440, 493, 523};

int buzzerPin = 2;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long timeAtChange = 0;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int lastReading[8];

unsigned long recordingStart;
unsigned long *recordingTime;
int *recordingNotes;
int recordingSize = 0;
bool stillRecording;
int recordingButton = 46;
int recLed = 47;
unsigned long recLastDebounceTime = 0;
unsigned long recTimeAtChange = 0;
int recLastReading = HIGH;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < btnsSize; i++) {
    pinMode(btns[i], INPUT_PULLUP);
    pinMode(leds[i], OUTPUT);

    lastReading[i] = HIGH;
  }

  pinMode(recordingButton, INPUT_PULLUP);

  recordingStart = millis();
  recordingTime = new unsigned long [0];
  recordingNotes = new int [0];
  stillRecording = true;
  delay(500);
}

void loop() {
  int recReading = digitalRead(recordingButton);

  if (recReading != recLastReading) {
    recTimeAtChange = millis();
  }

  if ((recTimeAtChange - recLastDebounceTime) > debounceDelay) {
    if (recReading == LOW) {
      Serial.println("rec button pressed");
      digitalWrite(recLed, HIGH);
    }
    else {
      Serial.println("rec button released");
      digitalWrite(recLed, LOW);
      stillRecording = false;
    }

    recLastReading = recReading;
    recLastDebounceTime = recTimeAtChange;
  }

  if (stillRecording) {

    for (int i = 0; i < btnsSize; i++) {
      int reading = digitalRead(btns[i]);

      if (reading != lastReading[i]) {
        timeAtChange = millis();
      }

      if ((timeAtChange - lastDebounceTime) > debounceDelay && reading != lastReading[i]) {
        int buttonState;

        if (reading != lastReading[i]) {
          buttonState = reading;
        }
        else {
          buttonState = lastReading[i];
        }

        if (reading == LOW) {
          recordingNotes = resizeArray(recordingNotes, recordingSize, i);
          recordingTime = resizeArray(recordingTime, recordingSize, millis());
          recordingSize++;

          Serial.println("note Recorded");

          digitalWrite(leds[i], HIGH);
          noTone(buzzerPin);
          tone(buzzerPin, freq[i]);
        }
        else if (reading == HIGH && lastReading[i] == LOW) {
          Serial.println("Rest recorded");

          recordingNotes = resizeArray(recordingNotes, recordingSize, -1);
          recordingTime = resizeArray(recordingTime, recordingSize, millis());
          recordingSize++;

          digitalWrite(leds[i], LOW);
          noTone(buzzerPin);
        }

        lastDebounceTime = timeAtChange;
      }
      lastReading[i] = reading;
    }

    //delay(150); //delay so that the sound is held long enough so that it can be heard
  }
  else {
    Serial.println("Ended recording\nBeginning playback");

    noTone(buzzerPin);
    for (int i = 0; i < 8; i++) {
      digitalWrite(leds[i], LOW);
    }

    for(int i = 0; i < recordingSize; i++){
      Serial.print("*(recordingTime + i) - recordingStart)");
      Serial.println(*(recordingTime + i) - recordingStart);
    }

    unsigned long playbackStartTime = millis();
    int noteCounter = 0;
    bool playbackDone = false;

    while (!playbackDone) {
      //Serial.println("trying to playback");
      
      if (abs((millis() - playbackStartTime) - (*(recordingTime + noteCounter) - recordingStart)) < 30) {

        if (*(recordingNotes + noteCounter) != -1) {
          Serial.println("Playing a note");

          if (noteCounter != 0) {
            digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);
          }

          Serial.println("Recorded Note: " + * (recordingNotes + noteCounter));
          digitalWrite(leds[*(recordingNotes + noteCounter)], HIGH);
          tone(buzzerPin, freq[*(recordingNotes + noteCounter)]);
        }
        else {
          Serial.println("Playing a REST");

          noTone(buzzerPin);
          digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);
        }

        noteCounter++;
      }

      if (noteCounter == recordingSize) {
        playbackDone = true;
        Serial.println("Done Playing");
      }
    }

    digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);
    
    digitalWrite(recLed, HIGH);
    delay(300);
    digitalWrite(recLed, LOW);

    recordingStart = millis();
    recordingTime = new unsigned long [0];
    recordingNotes = new int [0];
    stillRecording = true;
    recordingSize = 0;
  }

}

int* resizeArray(int *arr, int len, int num) {
  int *tempArr;

  tempArr = new int [len + 1];

  for (int i = 0; i < len; i++) {
    *(tempArr + i) = *(arr + i);
  }

  *(tempArr + len) = num;

  return tempArr;
}

unsigned long* resizeArray(unsigned long *arr, int len, unsigned long num) {
  unsigned long *tempArr;

  tempArr = new unsigned long [len + 1];

  for (int i = 0; i < len; i++) {
    *(tempArr + i) = *(arr + i);
  }

  *(tempArr + len) = num;

  return tempArr;
}

