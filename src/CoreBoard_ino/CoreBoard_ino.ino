/* @file CoreBoard.ino
|| @author Louis Tovar
|| @contact gltovar85@gmail.com
||
|| @description
|| | 	This uses the arduino keypad library to detect key press events
|| |	Also uses the Teensy keyboard sending library to get the proper keys to the
|| |	host computer.  It will automatically pick the proper free set_key slots
|| |	and clear them when a key is not pressed any more.
|| |
|| |	in the future the automatically recognition of special characters will be added as well.
|| |
|| |	Also I wish that the 2d array of keys and keyCodes could be condensed into one entity.
|| #
*/

#include <Keypad.h>
#define USB_MAX_SEND 6

typedef void (usb_keyboard_class::*SetKeyFunction) (uint8_t c);

const byte ROWS = 5; //four rows
const byte COLS = 7; //three columns
char keys[ROWS][COLS] = {
	{'^','&','*','(',')','_','+'},
	{'6','7','8','9','0','-','='},
	{'t','y','u','i','o','p','['},
	{'g','h','j','k','l',';','"'},
	{'b','n','m',',','.','/','v'}
};

int keyCodes[ROWS*COLS] = {	KEY_F5,		KEY_F6,		KEY_F7,		KEY_F8,		KEY_F12,	KEY_F9,			KEY_F10,
							KEY_6, 		KEY_7,		KEY_8,		KEY_9,		KEY_0,		KEY_MINUS,		KEY_EQUAL,
							KEY_T,		KEY_Y,		KEY_U,		KEY_I,		KEY_O,		KEY_P,			KEY_LEFT_BRACE,
							KEY_G,		KEY_H,		KEY_J,		KEY_K,		KEY_L,		KEY_SEMICOLON, 	KEY_QUOTE,
							KEY_B,		KEY_N,		KEY_M,		KEY_COMMA,	KEY_PERIOD,	KEY_BACKSLASH,	KEY_V
						  };

byte rowPins[ROWS] = {1,2,3,4,5}; //connect to the column pinouts of the kpd
byte colPins[COLS] = {6,7,8,9,10,11,12}; //connect to the row pinouts of the kpd


Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char usbKeys[USB_MAX_SEND] = {'0','0','0','0','0','0'};

// this array keeps the various banks of keyssends in an easy to search through list
SetKeyFunction setKeys[USB_MAX_SEND] = {  &usb_keyboard_class::set_key1,
                              &usb_keyboard_class::set_key2,
                              &usb_keyboard_class::set_key3,
                              &usb_keyboard_class::set_key4,
                              &usb_keyboard_class::set_key5,
                              &usb_keyboard_class::set_key6 };
boolean shouldSendKeys = false;

void setup() {
  //Serial.begin(9600);
  kpd.setDebounceTime(1);
}

unsigned long loopCount = 0;
unsigned long startTime = millis();
String msg = "";



void loop() {

  loopCount++;
  if ( (millis()-startTime)>1000 ) {
      startTime = millis();
      loopCount = 0;
  }

  // Fills kpd.key[ ] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  if (kpd.getKeys())
  {
    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
                //msg = " PRESSED.";
                updateSetKey( kpd.key[i] );
                break;
            case HOLD:
                //msg = " HOLD.";
                break;
            case RELEASED:
                //msg = " RELEASED.";
                updateSetKey( kpd.key[i] );
                break;
            case IDLE:
                //msg = " IDLE.";
        }        
      }
    }
  }
  
  if( shouldSendKeys )
  {
    Keyboard.send_now();
    shouldSendKeys = false;
  }
}  // End loop

// update the tracked keys
void updateSetKey( Key key )
{
  // first check to see if we were tracking the key, as well as look for the first empty slot.
  int emptySlot = -1;
  int i;
  for(i=0; i <USB_MAX_SEND; i++)
  {
    
    if( usbKeys[i] == key.kchar )
    {
       if( key.kstate == RELEASED )
       {
         usbKeys[i] = '0';
         (Keyboard.*setKeys[i])(0);
         shouldSendKeys = true;
         return;
       }
       else // we are already tracking it. all we care about is released
       {
         return;
       }
    }
    else if( emptySlot == -1 && usbKeys[i] == '0' )
    {
      emptySlot = i;
    }
  }
  
  // if we have an available empty slot keep track of it
  if( emptySlot != -1 && key.kstate == PRESSED )
  {
    usbKeys[emptySlot] = key.kchar;
    (Keyboard.*setKeys[emptySlot])( keyCodes[ key.kcode ]);
    shouldSendKeys = true;
  }
}
