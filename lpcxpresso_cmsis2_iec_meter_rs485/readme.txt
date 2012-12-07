Code Red Technologies LPCXpresso176x - systick demo
===================================================

This project contains a LED flashing Systick example for the LPCXpresso
board mounted with an LPC1768/9 Cortex-M3 part.

When downloaded to the board and executed, LED2 will be illuminated.
The state of LED2 will toggle every 2 seconds, timed using the Cortex-M3's
built in Systick timer.

The project makes use of code from the following project:
- CMSISv2p00_LPC17xx : for CMSIS 2.00 files relevant to LPC17xx

This library project must exist in the same workspace in order
for the project to successfully build.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that this example project is intended for use with Red Suite /
LPCXpresso IDE v4.0 or later.

Use of this project with earlier versions of the Code Red IDE is 
not recommended or supported.
