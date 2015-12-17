Compiling the code:
==================
g++ -std=c++0x -o driver  driver.cpp
g++ -std=c++0x -o processor processor.cpp

Running the code:
================
First run the driver as below

./driver MIN MAX
e.g. ./driver 0 1000   -> this will print all primes between 0 and 1000

Then run processor from a new terminal

./processor
