# UDP-socket-calculator
A simple calculator based on UDP socket and client-server communication in C.

First compile the server file:
gcc server.c -o server
./server

Second compile the client file:
gcc client.c -o client
./cient

Then type your expression to be calculated.
For example,
client sends the request expression 3*5
and the server answers back 15.
