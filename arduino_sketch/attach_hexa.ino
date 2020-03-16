//Attach Hexa - https://github.com/uzsito/attach-hexa
//Half Period (delay between switching states) calculation: 15000/RPM gives delay in millisecs.
//Channel logic is inverted: when a channel is LOW, the attached external TACH pin is HIGH

//Example serial commands:
//channel 1 2000
//channel 4 4500
//led link 2
//led multi 60

#include <EEPROMex.h>

int Channel_Rpms[6]; //RPMs for channels, integer size for easier EEPROM storage

unsigned long Channel_Delays[7]; //Calculated delays between switching states (half period of square wave) on channels

unsigned long When_State_Changed[7]; //Variables to store the last moment when channel states changed

const byte Channel_Pins[7] = {2, 3, 4, 6, 7, 8, LED_BUILTIN}; //Channel digital pin numbers: Channel 1-2-3-4-5-6-LED
unsigned long CurrentMicros; //Variable for time measurement

byte LED_Linked_Channel; //Link built-in indicator LED to desired channel
byte LED_Delay_Multiplier; //Multiplies LED delay to make visible results

char Command_First[8]; //Serial command handler variables
char Command_Second[6];
byte Command_Second_Num;
unsigned int Command_Third;

const byte Number_Of_Chars = 20; //Serial receiver variables
char Received_Chars[Number_Of_Chars];
boolean Serial_New_Data = false;
boolean Serial_Listening_Printed = false;

char Read_Char; //Variable to load chars from serial buffer (ASCII bytes, one-by-one)
static byte Char_Stepper = 0;
char * Char_Index; //Pointer to locate delimeter

char Command_Start_Char = '/'; //Markers to identify start & end of commands
char Command_End_Char = '\r'; //CR - Carriage Return
char Command_End_Char2 = '\n'; //LF - Line Feed

static boolean Receive_In_Progress = false; //Sets true if command start char catched

int address_LED_Link; //EEPROM addresses
int address_LED_Multi;
int address_Channel_Rpms;

void setup() {
  Serial.begin(115200);

  Read_EEPROM(); //Load settings from EEPROM to variables
  Update_Delays(); //Set delays to use in channel handling

  Serial.println("Attach Hexa running. More info: https://github.com/uzsito/attach-hexa");
  Serial.println("Completed loading settings from EEPROM.");

  Print_Settings();

  pinMode(Channel_Pins[0], OUTPUT); //Channel 1
  pinMode(Channel_Pins[1], OUTPUT); //Channel 2
  pinMode(Channel_Pins[2], OUTPUT); //Channel 3
  pinMode(Channel_Pins[3], OUTPUT); //Channel 4
  pinMode(Channel_Pins[4], OUTPUT); //Channel 5
  pinMode(Channel_Pins[5], OUTPUT); //Channel 6
  pinMode(Channel_Pins[6], OUTPUT); //Channel tracker LED
}

void loop() {
  CurrentMicros = micros();

  Channel_Handler();
  Serial_Receiver();
  Serial_Command_Handler();
}

void Channel_Handler () { //Handle pin states when appropriate delay passed

  int i = 0;

  do { //Check all channels

    if (CurrentMicros - When_State_Changed[i] >= Channel_Delays[i]) {

      //delay passed, read state of pin and change it

      switch (digitalRead(Channel_Pins[i])) { //Yes, we read an output pin! Returns binary state
        case 0:
          digitalWrite(Channel_Pins[i], HIGH);
          break;

        case 1:
          digitalWrite(Channel_Pins[i], LOW);
          break;
      }

      When_State_Changed[i] += Channel_Delays[i];  //save time when state change SHOULD be made, this provides more accuracy than saving current time
    }

    i++;

  } while (i <= 6);
}

void Serial_Receiver() { //Handle serial received data, listening for commands

  if (Serial_Listening_Printed == false) { //Inform user about serial listening only once.
    Serial.println("Listening to serial commands.");
    Serial_Listening_Printed = true;
  }

  while (Serial.available() > 0 && Serial_New_Data == false) { //Start to examine chars only if serial buffer is not empty, and previous segment already parsed
    Read_Char = Serial.read(); //Examine chars in serial buffer, one per iteration

    //Skips all the following if first character is not command start char (marker)

    if (Receive_In_Progress == true) { //Command start char found, start receiving procedure until command end char captured
      if (Read_Char != Command_End_Char && Read_Char != Command_End_Char2) {
        Received_Chars[Char_Stepper] = Read_Char;
        Char_Stepper++;
        if (Char_Stepper >= Number_Of_Chars) {
          Char_Stepper = Number_Of_Chars - 1;
        }
      }
      else {
        Received_Chars[Char_Stepper] = '\0'; //Command end char found, terminate receiving and insert null character
        Receive_In_Progress = false;
        Char_Stepper = 0;
        Serial_New_Data = true; //Received data is ready to parse, skip next iteration
      }
    }

    else if (Read_Char == Command_Start_Char) { //If command start char found, start receiving procedure
      Receive_In_Progress = true;
    }
  }
}

void Serial_Command_Handler() { //Parse serial data, execute received commands

  if (Serial_New_Data == true) {

    Char_Index = strtok(Received_Chars, " "); //Fill pointer with delimeter position
    strcpy(Command_First, Char_Index); //Get all chars to char array until pointer position

    Char_Index = strtok(NULL, " "); //Find next delimeter in previous source
    strcpy(Command_Second, Char_Index); //Get all chars to char array until pointer
    Command_Second_Num = byte(atoi(Char_Index)); //Also convert to number and store in a byte

    Char_Index = strtok(NULL, " "); //Find next delimeter in previous source
    Command_Third = atoi(Char_Index); //Get all chars and convert to int until pointer

    if (strcmp(Command_First, "channel") == 0 && Command_Second_Num >= 1 && Command_Second_Num <= 6 && Command_Third >= 1 && Command_Third <= 32767) { //valid channel rpm modification command
      Channel_Rpms[Command_Second_Num - 1] = Command_Third; //Update RPM data
      Update_Delays(); //Update all delays to use in channel handling, based on RPM data
      Serial.print("Channel "); Serial.print(Command_Second_Num); Serial.print(" RPM changed to "); Serial.println(Channel_Rpms[Command_Second_Num - 1]);
    }
    else if (strcmp(Command_First, "led") == 0 && strcmp(Command_Second, "link") == 0 && Command_Third >= 1 && Command_Third <= 6) { //valid led link command
      LED_Linked_Channel = Command_Third;
      Channel_Delays[6] = ((Channel_Delays[LED_Linked_Channel - 1]) * LED_Delay_Multiplier); //Update LED delay based on linked channel and multiplier
      Serial.print("LED linked to channel "); Serial.println(LED_Linked_Channel);
    }
    else if (strcmp(Command_First, "led") == 0 && strcmp(Command_Second, "multi") == 0 && Command_Third >= 1 && Command_Third <= 255) { //valid led delay multiplication command
      LED_Delay_Multiplier = Command_Third;
      Channel_Delays[6] = ((Channel_Delays[LED_Linked_Channel - 1]) * LED_Delay_Multiplier); //Update LED delay based on linked channel and multiplier
      Serial.print("LED delay multiplication changed to "); Serial.println(LED_Delay_Multiplier);
    }
    else if (strcmp(Command_First, "print") == 0 && Command_Second[0] == 0) { //valid print command
      Print_Settings();
    }

    else if (strcmp(Command_First, "save") == 0 && Command_Second[0] == 0) { //save all settings to EEPROM

      EEPROM.updateByte(address_LED_Link, LED_Linked_Channel); //Update saved data only if there is a difference
      EEPROM.updateByte(address_LED_Multi, LED_Delay_Multiplier);
      EEPROM.updateBlock<int>(address_Channel_Rpms, Channel_Rpms, 6);

      Serial.println("Current settings saved to EEPROM.");
    }

    else {
      Serial.println("Unknown request. Please check available commands at: https://github.com/uzsito/attach-hexa");
    }

    Serial_New_Data = false; //Received data parsed, allow Serial_Receiver function to capture chars again
  }
}

void Print_Settings() { //Print current settings

  Serial.print("LED Linked Channel: "); Serial.println(LED_Linked_Channel);
  Serial.print("LED Delay Multiplier: "); Serial.println(LED_Delay_Multiplier);

  for (int i = 0; i <= 5; i++) {
    Serial.print("Channel "); Serial.print(i + 1); Serial.print(" RPM: "); Serial.print(Channel_Rpms[i]); Serial.println(" rpm."); //Print channel RPMs and delays
    Serial.print("Channel "); Serial.print(i + 1); Serial.print(" Delay: "); Serial.print(Channel_Delays[i]); Serial.println(" usec.");
  }
  Serial.print("LED Delay: "); Serial.print(Channel_Delays[6]); Serial.println(" usec.");;
}

void Read_EEPROM() { //Load settings from EEPROM to variables

  EEPROM.setMemPool(0, EEPROMSizeATmega328); //EEPROM used space (starting & end address)

  address_LED_Link = EEPROM.getAddress(sizeof(byte)); //set available EEPROM addresses based on stored data size
  address_LED_Multi = EEPROM.getAddress(sizeof(byte));
  address_Channel_Rpms = EEPROM.getAddress(sizeof(int) * 6);

  LED_Linked_Channel = EEPROM.read(address_LED_Link); //read saved data from EEPROM
  LED_Delay_Multiplier = EEPROM.read(address_LED_Multi);
  EEPROM.readBlock<int>(address_Channel_Rpms, Channel_Rpms, 6);
}

void Update_Delays() { //Update all delays to use in channel handling, based on RPM data
  for (int i = 0; i <= 5; i++) {
    Channel_Delays[i] = (15000000 / Channel_Rpms[i]); //Update all calculated channel delays from RPMs
  }
  Channel_Delays[6] = ((Channel_Delays[LED_Linked_Channel - 1]) * LED_Delay_Multiplier); //Update calculated LED delay from linked channel data
}
