# lispi
A basic lisp interpreter, written for fun, with no promises of accuracy or quality.
Very much a work in progress at this point. Lisp has about a billion dialetcs, I am probably
going to stick relatively closely to MIT Scheme, because it has good documentation and is what
I am familiar with.
(c) 2017 Sam Donow, software covered under the GPL license

Dependencies
 - A modern C++ compiler, meaning one of:
    - g++7.1 or newer (most supported/tested)
    - clang++5.0 or newer and equally new libc++ (may require some manual configuration)
 - GNU readline (sudo apt-get install libreadline-dev on Debian-based platforms) (GPL license)
