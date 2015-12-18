Intro to parallel Processing
Anhar Felimban 
Parallel Primes 


This folder contains the following files:
README.txt
driver.cpp
processor.cpp


Compile and run the project using terminal as below:

	g++ -std=c++0x -o driver  driver.cpp
	g++ -std=c++0x -o processor processor.cpp

The above lines will compile the necessary files and run the program.

Running the code:
First run the driver as below

	./driver MIN MAX
	e.g. ./driver 0 1000   -> this will print all primes between 0 and 1000

Then run processor from a new terminal

	./processor
