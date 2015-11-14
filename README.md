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


