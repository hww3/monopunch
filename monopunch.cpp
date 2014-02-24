// Do not remove the include below
#include "monopunch.h"
#include <SerialCommand.h>
#include <TimerOne.h>

//#define DEBUG

/*
 * PORTB
 * PORTC
 * PORTD
 * PORTF PF0..PF6 PUNCHES
 * PF7 = FORWARD FEED
 * PE0 = REVERSE FEED
 * PE1 = FEED DISABLE
 * PE5 = HARDWARE INHIBIT
 * PE6 = FAULT (INPUT)
 * PE7 = ONLINE (INPUT)
 * PE4 = TEST (INPUT)
 * PA4 = DATA LED (OUTPUT)
 * PA5 = FAULT LED (OUTPUT)
 */

#define READONLINE() (PINE & _BV(PE7))
#define READFAULT() (PINE & _BV(PE6))
#define READTEST() (!(PINE & _BV(PE4)))
#define READDATA() (PINA & _BV(PA4))

#define FAULT(X) (X?(PORTA |= (1<<PA5)):(PORTA &= ~(1<<PA5)))
#define DATA(X) (X?(PORTA |= (1<<PA4)):(PORTA &= ~(1<<PA4)))
#define ONLINE(X) (X?(PORTE |= (1<<PE5)):(PORTE &= ~(1<<PE5)))

//TimerOne * Timer;

SerialCommand cmd;
int inPunch = 0;
int serialUp = 0;
int online = 1	;
int paperfault = 0;
int inTest = 0;
unsigned short * buf;
unsigned short * testbuf; // the buffer for current test pins.
int bufpos = 0;
long timeDown = 0;
int lastBlink = 0;
//The setup function is called once at startup of the sketch
void setup()
{
	buf = (unsigned short *)malloc(6);
	memset(buf, 0, 6);
	testbuf = (unsigned short *)malloc(5);
	memset(testbuf, 1, 5);

	Serial.begin(9600); // USB is always 12 Mbit/sec

	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTE = 0;
	PORTF = 0;
	PORTE|=(1<<PE1);

	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	DDRF = 0xff;

	DDRE |= (1<<PE0); // OUTPUT
	DDRE |= (1<<PE1); // OUTPUT
	DDRE |= (1<<PE5); // OUTPUT

	PORTE |= (1<<PE4);
//	PORTE |= (1<<PE6);
//	PORTE &= ~(1<<PE7);

	DDRA |= (1<<PA5); // OUTPUT
	DDRA |= (1<<PA4); // OUTPUT
	DDRA |= (1<<PA0); // OUTPUT


	DDRE &= ~(1<<PE6); // INPUT
	DDRE &= ~(1<<PE7); // INPUT
	DDRE &= ~(1<<PE4); // INPUT

	attachInterrupt(INT4, goTest, CHANGE);
	attachInterrupt(INT6, goFault, CHANGE);
	attachInterrupt(INT7, goOnline, CHANGE);

	cmd.addCommand("AT", displayHello);
  cmd.addCommand("ATI", displayVersion);
  cmd.addCommand("ATS", displayStatus);
  cmd.addCommand("ATF", advanceLine);
  cmd.addCommand("ATR", reverseLine);
  cmd.addCommand("ATP", beginPunch);

//  sCmd.addCommand("P",     processCommand);  // Converts two arguments to integers and echos them back
  cmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
//  Serial.println("Ready");

  blinkLeds();


  goOnline();
 // goTest();
  goFault(); // Prime the fault handler.
}

void blinkLeds(void)
{
  FAULT(0);
  DATA(0);
  ONLINE(0);
  delay(400);
  FAULT(1);
  delay(400);
  FAULT(0);
  delay(80);
  DATA(1);
  delay(400);
  DATA(0);
  delay(80);
  ONLINE(1);
  delay(400);
  ONLINE(0);
  delay(200);
  FAULT(1);
  DATA(1);
  ONLINE(1);
  delay(400);
  FAULT(0);
  DATA(0);
  ONLINE(0);

}

void goTest()
{
  // TODO
  int test;
//	Serial.write("test!");
  noInterrupts();

  test = READTEST();
  if(test)
  {
    // we cannot initiate test while the punch is in "punch" mode.
    if(inPunch)
    {
//    	Serial.write("in punch, no test!");
    	interrupts();
    	return;
    }
//  	Serial.write("test down, starting timer!");

  	timeDown = millis();
  }
  else
  {
  //	Serial.write("stopping test!");
  	// but, we can stop the punch whenever.
  	stopTest();
  }

//  Serial.write((test>0)?"T":"t");
 // DATA(test);
interrupts();
}

void stopTest()
{
	noInterrupts();
	inTest = 0;
	DATA(0);
	timeDown = 0;
  Timer1.stop();
  interrupts();
}

void beginTest(void)
{
  noInterrupts();
	//Serial.write("test held, starting blink!");
	// we should always start with a fresh pattern.
	memset(testbuf, 1, 5);
	inTest = 1;
	lastBlink = 0;
	blinkData();
	interrupts();
}

void blinkData()
{
	noInterrupts();

	if((millis() - lastBlink) > 500)
	{
  	lastBlink = millis();
  //	Serial.write("blink!");

	  int d = READDATA();
  	DATA(!d);
	}
	interrupts();
}

void goOnline()
{
  online = READONLINE();
  //Serial.write((online>0)?"!":"?");
//  ONLINE(online);
}

void goFault()
{
	noInterrupts();
//	Serial.write("FAULT");
  paperfault = READFAULT();
 // Serial.write((paperfault>0)?"1":"0");
  FAULT(paperfault);
  interrupts();
}
// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char *command) {
  Serial.println("What?");
}

void displayHello()
{
	Serial.write("OK\n");
}

void displayVersion()
{
	Serial.write("OK Monotype 21 Channel Punch Interface ");
	Serial.write(FW_VERSION);
	Serial.write("\n");
}

void displayStatus()
{
	Serial.write("OK");
	if(online)
	{
  	Serial.write(" ONLINE");

  	if(paperfault)
  	{
    	Serial.write(" FAULT");
  	}
	}
	else
	{
		Serial.write(" OFFLINE");
	}
	Serial.write("\n");
}

void advanceLine()
{
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTF = 0b10000000; // leave the high pin on, for paper feed.

	delay(6);

	PORTF = 0;
}

void reverseLine()
{
	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTF = 0;
  PORTE |= (1<<PE0);
	delay(6);

  PORTE &= ~(1<<PE0);
}

void beginPunch()
{
	if(!inTest)
	{
   	inPunch = 1;
    DATA(0);
   	Serial.write("OK +++++ TO END\n");
	}
	else
	{
		// we're running a test.
		Serial.write("ERROR PUNCH BUSY");
	}
}

void endPunch()
{
	inPunch = 0;
	reset_buffer();
	DATA(0);
	Serial.write("OK PUNCH END\n");
}

int reading_punch;

void readPunch()
{
  while(Serial.available())
  {
  	char bte;
  	bte = (char)Serial.read();

  	if(!reading_punch && (bte == '\r' || bte == '\n')) // ignore
  	{
  		continue;
  	}
  	else if(!reading_punch)
  	{
  		reading_punch = 1;
  	}

  	buf[bufpos++] = bte;
  	if(bufpos == 5)
  	{
  		if(buf[4] == '\n' || buf[4] == '\r')
  		{
  			blinkData();
  			sendcode(buf);
	  		reset_buffer();
	  		return;
  		}
  		else if(buf[4] == '+')
  		{
  			if(buf[0] == '+' && buf[1] == '+' && buf[2] == '+' && buf[3] == '+')
  			{
  	  		reset_buffer();
  				endPunch();
  				return;
  			}
  			else
  			{
    			Serial.write("ERROR INVALID SEQUENCE\n");
      		reset_buffer();
  			}
  		}
  		else
  		{
  			Serial.write("ERROR INVALID SEQUENCE\n");
    		reset_buffer();
  		}
  	}
  }
}

void reset_buffer()
{
	reading_punch = 0;
	bufpos = 0;
	memset(buf, 0 , 5);
}


void sendcode(unsigned short * code)
{
	int pins = 0;
  int pos = 0;

  unsigned short int x = 0, y = 0;
  int i = 0, j = 0, k =0, m = 0, z = 0, q = 0, r = 0;
  unsigned short tw[5];

	if(!online)
	{
		Serial.write("ERROR OFFLINE PUNCH END\n");
		drain();
		inPunch = 0;
		return;
	}
	if(paperfault)
	{
		Serial.write("ERROR FAULT PUNCH END\n");
		drain();
		inPunch = 0;
		return;
	}


#ifdef DEBUG
	for(r = 0; r < 4; r++)
	{
		y = code[r];
    for(q = 0; q < 8; q++)
    {
  		if(y & 128)
        Serial.write("1");
   	  else Serial.write("0");

  	  y <<= 1;
    }
	}

  Serial.write("(rec'd)\n");
#endif /* DEBUG */

  i = 0;

	for(j = 3; j >=0; j--)
  {
  	// if i goes beyond 31, this will cause a failure.
    x = code[j];

    for(k = 0; k < 8; k++)
    {
	  if(x & 1)
	  {
	  	pins++;
	  }
  	x >>= 1;


  	if(pins > 7 || i == 30)
  	{
  		int overflow_pos = i;
#ifdef DEBUG
  		if(pins > 7)
    		Serial.write("\noverflow at pin ");
  		Serial.print(overflow_pos);
  		Serial.write(" starting at ");
  		Serial.print(m);
  		Serial.write("\n");
#endif /* DEBUG */

  	  q = 0, y = 0, z = 0;
  	  if(overflow_pos == 30) // if we hit the cap at 31, don't waste time sending an empty line.=
  	  	overflow_pos = 31;

  	  for(r = 3; r >=0; r--)
  		{
  			tw[r] = 0;

  	    for(q = 0; q < 8; q++)
  	    {
  	      tw[r] >>= 1;
  	    	if(z >= m && z <= overflow_pos)
  	    		tw[r] |= 128;
  	    	z++;
  	    }
  		}

#ifdef DEBUG
  		for(r = 0; r < 4; r++)
  		{
  			y = tw[r];
  	    for(q = 0; q < 8; q++)
  	    {
  	  		if(y & 128)
  	        Serial.write("1");
  	   	  else Serial.write("0");

  	  	  y <<= 1;
  	    }
  		}
  		Serial.write(" (mask)\n");
#endif /* DEBUG */

  	  tw[0] &= code[0];
  	  tw[1] &= code[1];
  	  tw[2] &= code[2];
  	  tw[3] &= code[3];
  	  pins = 0;
      m = overflow_pos + 1;

#ifdef DEBUG
  		for(r = 0; r < 4; r++)
  		{
  			y = tw[r];
  	    for(q = 0; q < 8; q++)
  	    {
  	  		if(y & 128)
  	        Serial.write("1");
  	   	  else Serial.write("0");

  	  	  y <<= 1;
  	    }
  		}

     // Serial.write("(calc)\n");
#endif /* DEBUG */

    low_sendcode(tw);
      if(i>=30) {
      	if(inPunch)
        	Serial.write("OKP\n");
      	return;
      }
  	}

  	i++;
    }
  }
}

void low_sendcode(unsigned short * code)
{

#ifdef DEBUG
	int r, y, q;
	Serial.write("> ");
	for(r = 0; r < 4; r++)
	{
		y = code[r];
    for(q = 0; q < 8; q++)
    {
  		if(y & 128)
        Serial.write("1");
   	  else Serial.write("0");

  	  y <<= 1;
    }
	}
	Serial.write("\n");
#endif /* DEBUG */
	PORTB = code[3];
	PORTC = code[2];
	PORTD = code[1];
	PORTF = (code[0]);

//	PORTF |= (1<<PF7); // leave the high pin on, for paper feed.

	delay(3);

	PORTB = 0;
	PORTC = 0;
	PORTD = 0;
	PORTF &= (1<<PF7); // leave the high pin on, for paper feed.

	delay(3);

	PORTF = 0;

	delay(55);
}

void drain()
{
	while(Serial.available())
		Serial.read();
}

void loop()
{
//	DATA(1);
//	delay(45);
//	DATA(0);
 // delay(1000);
//	return;
//	PORTE = 0;
//  PORTE |=(1<<PE1);
/*
	PORTF=0;
	PORTE|=(1<<PE1);
	//PORTF|=(1<<PF7);
	delay(6);
	//PORTF=0;
	PORTE&=~(1<<PE1);
	delay(1000);
	return;
	*/
	//Serial.write("loop\n");
	if(!inTest && timeDown)
	{
		if(millis() - timeDown > 2000)
			beginTest();
	}

	if(inTest)
	{
		unsigned short testbuf2[5];
		unsigned short * q;
    // run 1 set of pins.
		int a;
		blinkData();
		for(int b = 0; b < 4; b++)
		{
  		if(testbuf[b] > 128)
      	testbuf[b] = 1;
  		testbuf2[b] = testbuf[b];
  		testbuf[b] <<=1;
		}

		testbuf2[0]|=(1<<7);

		q = &(testbuf2[0])+0;

	sendcode(q);
	}

	if (Serial.dtr())
	{
		if(!serialUp)
		{
			serialUp = 1;  // wait for user to start the serial monitor
			reset_buffer();
		}
	}
	else
		inPunch = 0;

	if (serialUp)
	{
		ONLINE(1);
		if(!inPunch)
		{
			cmd.readSerial();
		}
		else
		{
			readPunch();
		}
	}
	else
	{
		DATA(0);
		ONLINE(0);
	}
}
