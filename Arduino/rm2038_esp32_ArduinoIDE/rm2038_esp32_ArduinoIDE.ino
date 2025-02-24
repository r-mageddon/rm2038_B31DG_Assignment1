/* 
* Title:        rm2038_eps32_ArduinoIDE
* Name:         Rowan Maxwell
* H No.:        H00346635
* Date:         23/01/25
* Description:  Assignment 1 of B31DG (embedded software) - creating 2 data 
*               waveforms from letters of surname and synchronisation waveform on 
*               an esp32
*/


// Define pin Outputs
#define greenLED 19       // Green LED on GPIO19
#define redLED 21         // Red LED on GPIO21
#define OUTPUT_ENABLE 23  // OUTPUT_ENABLE (push button 1) on GPIO23
#define OUTPUT_SELECT 15  // OUTPUT_SELECT (push button 2) on GPIO15
#define TSyncON 50        // Time of TsyncON pulse

// Defining parameters
/*
* a -> Delay value of first pulse in TsyncON (microseconds)
* b -> Delay value of off-time between each pulse (microseconds)
* c -> Number of waveforms in data waveform cycle
* d -> Time delay at end of TsyncON before starting next TsyncON cycle
*/
const int param_a = 13 * 100;  // surname letter = m, value = 1300
const int param_b = 1 * 100;   // surname letter = a, value = 100
const int param_c = 3 + 4;     // surname letter = x, value = 7
const int param_d = 4 * 500;   // surname letter = w, value = 2000

// Button values
bool buttonSelectState; // Read value for button select state
bool buttonEnableState; // Read value for button enable state
const int debounceDelay = 500; // Delay to prevent debounce causing continuous incrementation 
                               // of count value

// OUTPUT_ENABLE counter
int count = 0;

// Boolean button toggle
bool toggle = false;

/////////////////////////////////////////////////////////////
//////////////////// Set Up Function ////////////////////////
/////////////////////////////////////////////////////////////

// Start setup function
void setup() 
{
  pinMode(greenLED, OUTPUT);      // Green LED = OUTPUT
  pinMode(redLED, OUTPUT);        // Red LED = OUTPUT
  pinMode(OUTPUT_ENABLE, INPUT);  // Output Enable = Input
  pinMode(OUTPUT_SELECT, INPUT);  // Output Select = Input
}
// End Setup function

/////////////////////////////////////////////////////////////
//////////////////// Loop Function //////////////////////////
/////////////////////////////////////////////////////////////

// Start Loop function
void loop() 
{
  buttonSelectState = digitalRead(OUTPUT_SELECT);  // Read pushbutton 2 as output select
  buttonEnableState = digitalRead(OUTPUT_ENABLE);  // Read pushbutton 1 as enable select

  // If statement to switch toggle value to true and false
  if (buttonEnableState == 1)
  {
    toggle = !toggle; // switch toggle value to true/false
    delay(debounceDelay); // Delay next action to prevent continuous toggle value switching
  }

    // If statement to add 1 to counter when OUTPUT_ENABLE button is pressed
  if (buttonSelectState == HIGH) 
  {
    count += 1; // Add 1 to count
    delay(debounceDelay); // Delay next action to prevent continuous count increase
  }

  // If statement with count value switch case to display original and alternative outputs 
  // using OUTPUT_ENABLE and OUTPUT_SELECT push buttons with button toggle
  if (toggle == true)
  {
    switch (count)
    {
      case 0:
        // Call original data output waveform
        dataOutputSignal();
        break;
      case 1:
        // Call alternative data waveform (option 2 from word document)
        altDataOutputSignal();
        break;
      case 2:
        count = 0; // Reset count value to 0 when count = 2
        break;
      default:
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, LOW);
    }
  }
  // End if statement
}
// End Loop function


/////////////////////////////////////////////////////////////
////////// Original Data Output Signal Function /////////////
/////////////////////////////////////////////////////////////

// Start original data waveform
void dataOutputSignal() 
{
  // Start T-sync-ON pulse
  digitalWrite(redLED, HIGH);
  delayMicroseconds(TSyncON);
  digitalWrite(redLED, LOW);
  // Start parameter A pulse
  digitalWrite(greenLED, HIGH);
  delayMicroseconds(param_a);
  digitalWrite(greenLED, LOW);
  delayMicroseconds(param_b);  // Delay next pulse by parameter B

  // Create continuous pulses with a greater duration depending on the product of parameter 
  // A with the index value
  for (int index = 1; index <= param_c; index++) 
  {
    digitalWrite(greenLED, HIGH);
    delayMicroseconds(param_a + (index * 50));  // delay = a + ((n-1)*50) (microsecondseconds)
    digitalWrite(greenLED, LOW);
    delayMicroseconds(param_b);  // Delay next pulse by parameter B
  }

  delayMicroseconds(param_d);  // Delay next T-sync-ON pulse by paramter D
}
// End original data waveform


/////////////////////////////////////////////////////////////
///////// Alternative Data Output Signal Function ///////////
/////////////////////////////////////////////////////////////

// Start alternative data waveform
void altDataOutputSignal() 
{
  // Start T-sync-ON pulse
  digitalWrite(redLED, HIGH);
  delayMicroseconds(TSyncON);
  digitalWrite(redLED, LOW);
  // Create continuous pulses with a lesser duration depending on the product of paramter 
  // A with the index value starting from parameter C
  for (int index = param_c; index >= 1; index--) 
  {
    digitalWrite(greenLED, HIGH);
    delayMicroseconds(param_a + (index * 50));  // delay = a + ((n+1)*50) (microseconds)
    digitalWrite(greenLED, LOW);
    delayMicroseconds(param_b);  // Delay next pulse by parameter B
  }

  // End with parameter A pulse
  digitalWrite(greenLED, HIGH);
  delayMicroseconds(param_a);
  digitalWrite(greenLED, LOW);
  delayMicroseconds(param_b); // Delay final waveform
  delayMicroseconds(param_d);  // Delay next T-sync-ON pulse by parameter D
}
// End alternative data waveform