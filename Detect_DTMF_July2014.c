//This code realizes DTMF detection using Goertzel algorithm on AVR Atmega128 board
//Version 1.0 - 04 July 2014 by Omayma Said, FH-SWF

#include <kamavr.h>
#include <math.h>
#include <stdbool.h>

long int goertzel (int sample[], long int coeff, int N);
void post_test (void);

//-------Global variables--------//

int N = 96;		        	// block size
volatile int samples[96];	// buffer to store N samples
volatile int count;	    	// samples count
volatile bool flag;	    	// flag set when the samples buffer is full with N samples
volatile bool new_dig;		// flag set when inter-digit interval (pause) is detected

int power_all[8];		// array to store calculated power of 8 frequencies

int coeff[8];			// array to store the calculated coefficients                                                                   
int f_tone[8] = { 697, 770, 852, 941, 1209, 1336, 1477, 1633 };	// frequencies of rows & columns 


//-------Start Main--------//

void
main (void)
{
  int i;

  init_7seg ();			// initialize 7 segment
  init_lcd ();			// initialize LCD

  ADCSRA = 0x8F;		// 10001111   - ADEN-ADSC-ADFR-ADIF-ADIE-ADPS2-ADPS1-ADPS0
  // ADEN Enable
  // ADIE Interrupt Enable
  // Division factor 128 - ADC clock 125 KHz      

  ADCSRA |= 1 << ADFR;		// Free Running Mode
  ADCSRA |= 1 << ADSC;		// ADSC     Start Conversion
  ADMUX = 0;			// Select ADMUX channel

  sei ();			// Enable Global Interrupts

  for (i = 0; i < 8; i++)
    {
      coeff[i] = (2 * cos (2 * M_PI * (f_tone[i] / 9615.0))) * (1 << 14);
    }				// calculate coeff at each frquency - Q15 format

  while (1)
    {
      count = 0;		//rest count
      flag = 0;			//reset flag    

      while (flag == 0);	// wait till N samples are read in the buffer and the flag set by the ADC ISR

      {
	for (i = 0; i < 8; i++)
	  power_all[i] = goertzel (samples, coeff[i], N);	// call goertzel to calculate the power at each frequency and store it in the power_all array

	post_test ();		// call post test function to validate the data and display the pressed digit if applicable
      }

    }

}

//-------End of Main--------//


//---------------------------------------------------------------//     

//-------ADC ISR--------//
ISR (ADC_vect)
{
  if (count < N)
    samples[count++] = ADC >> 2;	// scale down ADC reading, store the value in the sampls buffer & increment the count
  else if (count == N)		        // if the buffer is full with N samples
    flag = 1;			            // set flag to 1 to start decoding
}



//---------------------------------------------------------------//     

//-------Goertzel function---------------------------------------//     
long int
goertzel (int sample[], long int coeff, int N)
//---------------------------------------------------------------//     
{
//initialize variables to be used in the function
  int Q, Q_prev, Q_prev2, i;
  long prod1, prod2, prod3, power;

  Q_prev = 0;			//set delay element1 Q_prev as zero
  Q_prev2 = 0;			//set delay element2 Q_prev2 as zero
  power = 0;			//set power as zero

  for (i = 0; i < N; i++)	// loop N times and calculate Q, Q_prev, Q_prev2 at each iteration
    {
      Q = (sample[i]) + ((coeff * Q_prev) >> 14) - (Q_prev2);	// >>14 used as the coeff was used in Q15 format
      Q_prev2 = Q_prev;		// shuffle delay elements
      Q_prev = Q;
    }

  //calculate the three products used to calculate power
  prod1 = ((long) Q_prev * Q_prev);
  prod2 = ((long) Q_prev2 * Q_prev2);
  prod3 = ((long) Q_prev * coeff) >> 14;
  prod3 = (prod3 * Q_prev2);

  power = ((prod1 + prod2 - prod3)) >> 8;	//calculate power using the three products and scale the result down

  return power;
}


//---------------------------------------------------------------//     

//-------Post-test function---------------------------------------//    
void
post_test (void)
//---------------------------------------------------------------//     
{
//initialize variables to be used in the function
  int i, row, col, max_power;

  char row_col[4][4] =		// array with the order of the digits in the DTMF system
  {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
  };

// find the maximum power in the row frequencies and the row number

  max_power = 0;		        //initialize max_power=0

  for (i = 0; i < 4; i++)	    //loop 4 times from 0>3 (the indecies of the rows)
    {
      if (power_all[i] > max_power)	//if power of the current row frequency > max_power
	{
	  max_power = power_all[i];	//set max_power as the current row frequency
	  row = i;		//update row number
	}
    }


// find the maximum power in the column frequencies and the column number

  max_power = 0;		//initialize max_power=0

  for (i = 4; i < 8; i++)	//loop 4 times from 4>7 (the indecies of the columns)
    {
      if (power_all[i] > max_power)	//if power of the current column frequency > max_power
	{
	  max_power = power_all[i];	//set max_power as the current column frequency
	  col = i;		//update column number
	}
    }


  if (power_all[col] == 0 && power_all[row] == 0)	//if the maximum powers equal zero > this means no signal or inter-digit pause
    new_dig = 1;		//set new_dig to 1 to display the next decoded digit


  if ((power_all[col] > 1000 && power_all[row] > 1000) && (new_dig == 1))	// check if maximum powers of row & column exceed certain threshold AND new_dig flag is set to 1
    {
      write_lcd (1, row_col[row][col - 4]);	// display the digit on the LCD
      dis_7seg (8, row_col[row][col - 4]);	// display the digit on 7-seg
      new_dig = 0;		// set new_dig to 0 to avoid displaying the same digit again.
    }
}
