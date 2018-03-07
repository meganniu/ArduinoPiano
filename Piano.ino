int btns[] = {30, 32, 34, 36, 38, 40, 42, 44}; //keys of the piano
int btnsSize = 8; //amount of keys
int leds[] = {31, 33, 35, 37, 39, 41, 43, 45}; //one led per key to indicate when it is pressed
int freq[] = {261, 293, 329, 349, 392, 440, 493, 523}; //C4 to C5, one tone per key

int buzzerPin = 2; //PWM pin buzzer is located on

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long timeAtChange = 0; //time of state change
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int lastReading[8]; // last state of the button

unsigned long recordingStart; //start time of the recording
unsigned long *recordingTime; //pointer to the array for when a note was recorded

//pointer to the array for the notes/rests that were recorded
//the number stored corresponds to the button/key's position in the btns[] array
//-1 is stored for rests
int *recordingNotes;
int recordingSize = 0; //amount of notes/rests recorded
bool stillRecording; //boolean indicating whether or not the user still wishes to record their playing

//pin # for the recording button (pressed when user wishes to stop recording)
//Not pressed to start recording bc recording starts automatically when the program begins or when playback ends.
int recordingButton = 46;
int recLed = 47; //pin # for led that corresponds to the recording button
unsigned long recLastDebounceTime = 0; // the last time the recording button output pin was toggled
unsigned long recTimeAtChange = 0; //time of state chage of recording button
int recLastReading = HIGH; //last reading of the recording button

void setup() {
  Serial.begin(9600); //begin Serial communication with Arduino

  for (int i = 0; i < btnsSize; i++) {
    pinMode(btns[i], INPUT_PULLUP);//assign pins to all the buttons
    pinMode(leds[i], OUTPUT);//assign pins to all the leds

    lastReading[i] = HIGH;//initially, all the button readings are high (i.e. not being pressed)
  }

  pinMode(recordingButton, INPUT_PULLUP); //assign pin to the recording button

  recordingStart = millis(); //take note of the start time of the recording
  recordingTime = new unsigned long [0]; //pointer to an unsigned long array that stores the time notes/rests begin
  recordingNotes = new int [0]; //pointer to an int array that stores the note/rest that is played
  stillRecording = true;//by default, the program begins in recording mode
  delay(500);//delay for setup to complete
}

void loop() {
  int recReading = digitalRead(recordingButton);//get the current state of the recording button

  //if the current state does not equal the previous state, record the time at state change
  if (recReading != recLastReading) {
    recTimeAtChange = millis();
  }

  //if the time between the current state change and the last valid state change is greater than
  //expected for typical noise, the current state change is valid
  if ((recTimeAtChange - recLastDebounceTime) > debounceDelay) {
    if (recReading == LOW) { //if the current state is Low, the button is being pressed
      Serial.println("rec button pressed");
      digitalWrite(recLed, HIGH); //turn on led corresponding to the recording button
    }
    else {
      Serial.println("rec button released");
      digitalWrite(recLed, LOW); //turn off led corresponding to the recording button
      stillRecording = false; //register that the user no longer wants to record and wishes to enter the playback function
    }

    recLastReading = recReading; //store the current reading into last reading
    recLastDebounceTime = recTimeAtChange; //last debounce time become current time at change
  }

  //if still recording, allow user to play notes and record those notes and the time at which they are played
  if (stillRecording) {

    for (int i = 0; i < btnsSize; i++) { //check for the state of all the key buttons
      int reading = digitalRead(btns[i]); //get the state of the button

      //if the current state does not equal the previous state, record the time at state change
      if (reading != lastReading[i]) {
        timeAtChange = millis();
      }

      //if the time between the current state change and the last valid state change is greater than
      //expected for typical noise, the current state change is valid
      if ((timeAtChange - lastDebounceTime) > debounceDelay) {
        int buttonState;

        buttonState = reading;

        if (reading == LOW) { //if button is being pressed, store note that is being played and the time at which it began to play
          recordingNotes = resizeArray(recordingNotes, recordingSize, i);
          recordingTime = resizeArray(recordingTime, recordingSize, millis());
          recordingSize++;

          Serial.println("note Recorded");

          //turn on corresponding LED and play the tone desired
          digitalWrite(leds[i], HIGH);
          noTone(buzzerPin);
          tone(buzzerPin, freq[i]);
        }
        else {//if the button has been released, store a rest and the time at which the rest began
          Serial.println("Rest recorded");

          recordingNotes = resizeArray(recordingNotes, recordingSize, -1);
          recordingTime = resizeArray(recordingTime, recordingSize, millis());
          recordingSize++;

          //turn off the corresponding LED and stop the buzzer
          digitalWrite(leds[i], LOW);
          noTone(buzzerPin);
        }

        lastDebounceTime = timeAtChange; //the last change time becomes the time of the current change
      }
      lastReading[i] = reading; //store the current reading for future comparison
    }
  }
  else {
    Serial.println("Ended recording\nBeginning playback");

    //turn buzzer and all LEDs off
    noTone(buzzerPin);
    for (int i = 0; i < 8; i++) {
      digitalWrite(leds[i], LOW);
    }

    unsigned long playbackStartTime = millis();// get time at which the playback began
    int noteCounter = 0; //the note/rest that is current being played
    bool playbackDone = false; //end the playback when it is done

    while (!playbackDone) { //while the playback is not done, keep going

      //wait for the right time (within 30 milliseconds) to start playing a note/rest
      if (abs((millis() - playbackStartTime) - (*(recordingTime + noteCounter) - recordingStart)) < 30) {

        if (*(recordingNotes + noteCounter) != -1) { //if a note (not a rest) is recorded
          Serial.println("Playing a note");

          if (noteCounter != 0) { //turn off the last led that was played as long as we are not on the first note
            digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);
          }

          //play the note and turn on the corresponding LED
          Serial.println("Recorded Note: " + * (recordingNotes + noteCounter));
          digitalWrite(leds[*(recordingNotes + noteCounter)], HIGH);
          tone(buzzerPin, freq[*(recordingNotes + noteCounter)]);
        }
        else {//if a rest is being played
          Serial.println("Playing a REST");

          //turn off the buzzer and the led of the last note
          noTone(buzzerPin);
          digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);
        }

        noteCounter++; //after playing a note/rest, increment the note counter
      }

      // if the note counter equals the size of the recording, then the playback has ended
      if (noteCounter == recordingSize) {
        playbackDone = true;
        Serial.println("Done Playing");
      }
    }

    //turn the led of the final note off
    digitalWrite(leds[*(recordingNotes + noteCounter - 1)], LOW);

    //make the led for the recording button blink to indicate that the playback has ended
    digitalWrite(recLed, HIGH);
    delay(300);
    digitalWrite(recLed, LOW);

    //reset all the variables used in recording
    recordingStart = millis();
    delete[] recordingTime;
    delete[] recordingNotes;
    recordingTime = new unsigned long [0];
    recordingNotes = new int [0];
    stillRecording = true;
    recordingSize = 0;
  }

}

//return the pointer to a resized array with all old array elements with one specified element appended to the end
int* resizeArray(int *arr, int len, int num) {
  int *tempArr; //intialize pointer to new array

  tempArr = new int [len + 1]; //the new array is one size larger than the old array

  for (int i = 0; i < len; i++) { //copy the old array
    *(tempArr + i) = *(arr + i);
  }

  *(tempArr + len) = num; //append the specified element to the end of the new array

  delete [] arr; //delete the old array

  return tempArr; //return the new array
}

//return the pointer to a resized array with all old array elements with one specified element appended to the end
unsigned long* resizeArray(unsigned long *arr, int len, unsigned long num) {
  unsigned long *tempArr;//intialize pointer to new array

  tempArr = new unsigned long [len + 1]; //the new array is one size larger than the old array

  for (int i = 0; i < len; i++) { //copy the old array
    *(tempArr + i) = *(arr + i);
  }

  *(tempArr + len) = num;//append the specified element to the end of the new array

  delete [] arr; //delete the old array

  return tempArr; //return the new array
}

