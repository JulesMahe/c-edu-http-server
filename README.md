# edu-http-server

## For those stumbling here:
This is a presonal project in which I program a minimal HTTP server. It is only intended to be educational.

## Tasklist:

### A - Answering a single get request

The program must be able to listen on a given port, receive a get request (lets say for an index.html page), and respond correctly.
That means:

1 - Having a simple TCP connection handler that can receive and send data

* Setup a basic socket and test it ==> example straight from beej’s network programming guide :white_check_mark:
* Finish over-commenting the code and fully understand what it does
* In depth exercice on pointers because some of it is non trivial
* Find a way to store what curl sends to the server and parse it ?
* How to articulate the code between the network coding and the applicative part ?


## References:

[Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/html/)
[Beej’s Guide to C Programming](https://beej.us/guide/bgc/)
[HTTP 1.1 RFC ](https://datatracker.ietf.org/doc/html/rfc2616)
