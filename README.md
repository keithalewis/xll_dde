# DDEML

Simple [`DDE`](https://learn.microsoft.com/en-us/windows/win32/api/_dataxchg/)
server/client wrappers for the 
[`ddeml.h`](https://learn.microsoft.com/en-us/windows/win32/api/ddeml/) header file.

## History

DDE is an ancient technology developed
during the late 1980's for Windows 2.0/3.x (a thing on a thing) era.
This was prior to general OS capabilities like threads, overlapped I/O,
COM, RPC, windows message isolation, and UNICODE. 
The DDEML library was introduce in 1992 for Windows 3.1 to make the
underlying DDE simpler and safer to use. Microsoft even provides
a means of requiring users to provide an API key to prevent
execution of arbitrary code.

The prefered method for getting realtime data in Excel is `RTD`. 

The good news is that it has not been actively developed for decades.
One day it may be phased out, but that would require Microsoft
to actively mess with their Office applications. They
no longer have employees who understand the code so it
would be risky for a PM to suggest that.
Its limitations are well known and the workarounds are well understood.

## Overview

The most important thing to understand is DDEML uses windows
message passing. It runs an event loop/message pump and assumes cooperative 
multitasking.
If any DDE application has a slow or modal callback deadlock ensues.
It permits a type of asynchronous behaviour similar to Javascript
but always runs in a single thread.

A DDE program can behave as a server or client or both.
Microsoft Excel™ is a prime example of the latter.
It is also possible to write a DDE monitoring program that does not 
participate message exchange. 
The monitor callback is called whenever any other DDE program callback is run.  
We will not be considering such programs.

### Common

Every DDEML programs calls `DdeInitialize` to specify a callback function
and get an integer id for each instance. Many DDEML functions require this,
but there is no API for providing this to the callback function. The simplest
solution is to provide a global variable with this value. To allow for
more than one service we provide a global map from service name to id.


The function
```
UINT DdeInitializeA(
  [in, out] LPDWORD     pidInst,
  [in]      PFNCALLBACK pfnCallback,
  [in]      DWORD       afCmd,
            DWORD       ulRes
);
```
should be called with `DWORD idInst = 0` and `pidInst = &idIst` to register 
the `pfnCallback` function. The `afCmd` flag is usually `APPCLASS_STANDARD`
and `ulRes` must be `0`. 

This call will fill in `idInst` with a value that needs to be used in
many `DDEML` functions. One DDEML API infelicity is that the `pfnCallback`
needs to know this value. In the old days programs would declare
a global variable to hold this value. 