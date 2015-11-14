#Introduction
This project provides a Dual Tone Multi Frequency (DTMF) detection system developed on AVR Atmega128 board based on Goertzel algorithm. 


#Background
DTMF was originally developed to be used for dial tone representation for control signals to be transmitted over the telephone network. In addition to telephone calls set up, DTMF allows user input during a call which allows interactive applications such as telephone banking, electronic mail systems, conference calls or similar applications.

In DTMF, each digit is generated by two superimposed tones (or sinusoidal signals) selected from two frequency groups providing 16 unique keys as shown in the figure below:

![DTMF digits representation](https://github.com/OmaymaS/DTMF-Detection-Goertzel-Algorithm-/blob/master/Images/DTMF1.png)

This implementation provides unique characteristics for the system such as:

* All tones are in the audible frequency range.
* No frequency in both frequency groups is a multiple of another one.
* The sum or difference of any two frequencies does not equal another selected frequency.


Such features simplify DTMF decoding where the eight DTMF frequencies should be differentiated in order to detect valid DTMF tones and distinguish them from speech. A series of checks are always carried out at the receiver side in order to detect the tones and validate them based on the criteria of the used standard. The common algorithm used for DTMF detection is Goertzel algorithm which is similar to FFT but faster.

#System Analysis

##ADC Settings

The required system should be able to acquire data from an external source (which is a standalone program in our case) and use the ADC in order to get the samples to be used in calculations for DTMF detection. The given AVR board includes an ADC which can run either in a single-conversion mode or a free-running mode. In the free running mode the ADC is constantly sampling and updating the ADC Data Register. The samples are taken at the maximum speed and the interrupts can be used for reading results. The timing diagram shows the conversion cycles and the following settings should be used to use free-running mode at 125 KHz with interrupts:

  * ADEN = 1	Enable ADC
  * ADSC = 1	Start continuous conversion 
  * ADIF	
      * Set when an ADC conversion completes and the data registers are updated
      * cleared by hardware when executing the corresponding interrupt handling vector
  * ADFR = 1	Enable free running mode
  * ADIE = 1	Enable ADC interrupt 
  * ADPS2:0 = 111	ADC pre-scaler select bits (division factor 128)

![ADCSRA](https://github.com/OmaymaS/DTMF-Detection-Goertzel-Algorithm-/blob/master/Images/ADCSRA.png)

![ADMUX](https://github.com/OmaymaS/DTMF-Detection-Goertzel-Algorithm-/blob/master/Images/ADMUX.png)

![Timing Diagram](https://github.com/OmaymaS/DTMF-Detection-Goertzel-Algorithm-/blob/master/Images/Timing%20Diagram.png)

##Calculations and parameters selection

The telephone network operates within a bandwidth of approx. 300 to 3400 Hz which is suitable for voice communications. The sampling frequency of telephone systems is usually 8 KHz. However, in our system the internal ADC of the AVR board is used with different sampling rate according to the selected division factor. The block size N can be arbitrarily selected considering the balance between accuracy and detection time. The different standards suggest certain values to be used in the telephone systems. For our application we will select different values and test the results.

* According to the selected division factor (128), the ADC clock is at 125 KHz. And according to timing diagram, the conversion time is 13 cycles.	
* The sampling frequency equals (125 KHz)/(13 Cycles)=9615.38 sample/sec and the sampling time is 0.104 ms 
* For our system the selected value of N=96. Accordingly, the frequency resolution (or main lobe width) is determined
by fs/N=100. 
* The time required to receive N samples equals 96*0.104ms=9.98 ms.


#System Design

Using the previous settings for the ADC and considering the calculated values the software was developed to detect DTMF digits and display results. The following are some notes regarding the design:
* The ADC is used with division factor 128 in free running mode with interrupts.  
* The ADC readings are downscaled to avoid overflow or dealing with large values.
* Fixed-point arithmetic is used instead of floating point for faster response and better performance on the microcontroller. (Note: Floating point was also tried and provided the desired result; however the final version uses fixed-point for the previously mentioned reasons.)
* As per the given data, both the tone duration is and the inter-digit interval equal 90 ms. Due to the fast detection, the same digit might be decoded several times in a row within the 90 ms. Therefore, the system includes checks to avoid displaying the same digit more than once.
*	A detected tone is considered valid when only one row and one column frequency exceed a certain threshold. Another possible check is to make sure the percentage of the sum of powers of the dominant frequencies is sufficiently large. In the designed system, the threshold test is used. It was found during the debugging that in case of no signal or pause the samples read are zeros and consequently the power at the eight frequencies. On the other hand when there is a tone, the dominant row/column frequency power exceeds 1000 which can be taken as a threshold. These values may vary under different conditions.


The functionality of the system is summarized as follows:
* The system reads the ADC values continuously to fill the sample array (buffer) with size equals to block size N.
* Once the array has been filled with N samples a flag s raised by the ADC ISR to proceed with Goertzel calculations in the main function.
* Goertzel function is called to calculate the power at the eight DTMF frequencies.
* Post_test() function is called to find the row/column frequency corresponding to the highest power. In case there was a valid digit detected, the digit is displayed on the LCD and the 7-sgment display.
* When a pause/no valid tone is detected the new_dig flag is raised to indicate that the next detected digit should be displayed. Once the digit has been displayed, the new_dig flag is set to zero to stop displaying the same digit more than once.

#Flow Diagram
![Flow Diagram](https://github.com/OmaymaS/DTMF-Detection-Goertzel-Algorithm-/blob/master/Images/Flowdiagram.png)

