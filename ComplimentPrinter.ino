#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#include <EEPROM.h>
#include <math.h>
#include "compliments.h"
// Instantiate a Bounce object
#include <Bounce2.h>

Bounce debouncer = Bounce(); 
int buttonState;
unsigned long buttonPressTimeStamp;

#define BUTTON_PIN 2
#define LED_PIN 7

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor

boolean printing;
int last = 0;


void EEPROMWritelong(int address, long value)
{
//Decomposition from a long to 4 bytes by using bitshift.
//One = Most significant -> Four = Least significant byte
byte four = (value & 0xFF);
byte three = ((value >> 8) & 0xFF);
byte two = ((value >> 16) & 0xFF);
byte one = ((value >> 24) & 0xFF);

//Write the 4 bytes into the eeprom memory.
EEPROM.write(address, four);
EEPROM.write(address + 1, three);
EEPROM.write(address + 2, two);
EEPROM.write(address + 3, one);
}


long EEPROMReadlong(long address)
{
//Read the 4 bytes from the eeprom memory.
long four = EEPROM.read(address);
long three = EEPROM.read(address + 1);
long two = EEPROM.read(address + 2);
long one = EEPROM.read(address + 3);

//Return the recomposed long by using bitshift.
return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

long amountOfPrints = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("starting..");

  pinMode(BUTTON_PIN,INPUT_PULLUP);
  printing = false;

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5); // interval in ms


  pinMode(LED_PIN,OUTPUT);

  pinMode(7, OUTPUT); digitalWrite(7, LOW);
  randomSeed(analogRead(A0));
  
  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(19200);  // Initialize SoftwareSerial
  
  printer.begin(100);        // Init printer (same regardless of serial type)
  digitalWrite(LED_PIN, HIGH );
  long val = EEPROMReadlong(0);
  Serial.print("Starting; current EEPROM value is ");
  Serial.println(val);
  if(val == -1)
  {
    Serial.println("EEPROM NOT set, setting to 1");
    
    EEPROMWritelong(0,1);
    val = EEPROMReadlong(0);
    Serial.print("Current EEPROM value is ");
    Serial.println(val);
  }

  amountOfPrints = val;

  
  

}
char comp[150];

String padWithSpaces(String theString)
{
  int StrLen = 30;
  int textLen = theString.length();
  int leftPad = (StrLen-textLen)/2;
  
  int rightPad = leftPad;
  if((StrLen-textLen) % 2 != 0 || (StrLen-textLen) == 1)
  {
    leftPad++;
  }
  String fin = "";
  for(int x=0;x<leftPad;x++)
  {
    fin = fin + " ";
  }
  fin = fin + theString;
  for(int x=0;x<rightPad;x++)
  {
    fin = fin + " ";
  }
  return fin;
  
}

void showMem()
{
  Serial.print("Free Memory:");Serial.println(freeRam());
  
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void niceCompliment()
{
  printer.wake();       
  amountOfPrints = amountOfPrints +1;
  if(amountOfPrints % 10 == 0)
  {
    EEPROMWritelong(0,amountOfPrints);
  }
  
  
  int i = random(1,36);
  if(i == last)
  {
    i = random(1,36);
  }
  last = i;
    
  strcpy_P(comp, (char*)pgm_read_word(&(compliments_table[i])));
    
  
  printer.setLineHeight(24);
  /* Top Line */
  printer.write(201);
  for(int i=0;i<30;i++)
  {
    printer.write(205);
  }
  printer.write(187);
  printer.println("");

  

  /* Compliment lines */
  printer.write(186);
  
  String compHead = "Compliment #";
  compHead = compHead + String(amountOfPrints);
  printer.print(padWithSpaces(compHead));
  printer.write(186);
  printer.println("");
  
  printer.write(204);
  for(int i=0;i<30;i++)
  {
    printer.write(205);
  }
  printer.write(185);
  printer.println("");
  


  printer.write(186);
  for(int i=0;i<30;i++)
  {
    printer.print(" ");
  }
  printer.write(186);
  printer.println("");


/* compliment */


  String currentLine = "";
  int lineLength = 28;
  int lastSpace = 0;
  int currentPos = 0;
  for(int z=0;z<strlen(comp);z++)
  {
    currentLine = currentLine + String(comp[z]);
    currentPos++;
    if(comp[z] == ' ' || currentLine.length() == lineLength)
    {
      if(currentLine.length() >= lineLength)
      {
        if(comp[z] == ' ')
        {
          lastSpace = z;
        }
        
        String printableLine = currentLine.substring(0,lastSpace);
        printableLine.trim();
        printer.justify('L');
        printer.write(186);
        printer.print(padWithSpaces(printableLine));
        printer.write(186);
        printer.println();
        
        currentLine = currentLine.substring(lastSpace);
        currentLine.trim();
        lastSpace = 0;
        currentPos = currentLine.length();
      }
      else
      {
        lastSpace = currentPos;
      }
      
    }
    
    
  }

  if(currentLine.length() > 0)
  {
    currentLine.trim();
    printer.justify('L');
    printer.write(186);
    printer.print(padWithSpaces(currentLine));
    printer.write(186);
    printer.println();
  }


  /* Bottom Lines */

  printer.write(186);
  for(int i=0;i<30;i++)
  {
    printer.print(" ");
  }
  printer.write(186);
  printer.println("");
  
  printer.write(200);
  for(int i=0;i<30;i++)
  {
    printer.write(205);
  }
  printer.write(188);
  printer.println("");

  printer.justify('C');
  printer.println("www.andrewmohawk.com/compliment/");
  printer.feed(10);
  printer.justify('L');
  printer.sleep();
  
}

void loop() {

  // Update the Bounce instance :
  debouncer.update();

  if ( debouncer.fell()  )
  {
       digitalWrite(LED_PIN, LOW );
       printing = true;
       Serial.println("[+] Printing Compliment");
       niceCompliment();
       Serial.println("[+] ...Done!");
       digitalWrite(LED_PIN, HIGH );
       printing = false;
       delay(300);
       
  
  }
  
  delay(50);

}
