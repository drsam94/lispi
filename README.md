# lispi
A basic lisp interpreter, written for fun, with no promises of accuracy or usability.
Very much a work in progress at this point. Lisp has about a billion dialetcs, I am basing my implementation on MIT Scheme (https://www.gnu.org/software/mit-scheme/documentation/mit-scheme-ref.pdf), because of the quality of documentation and my relatively familiarity compared with other dialects.
(c) 2017 Sam Donow, software covered under the GPL license

Dependencies
 - A modern C++ compiler, meaning one of:
    - g++7.1 or newer
    - clang++5.0 or newer and equally new libc++ (may require some manual configuration, see
        Makefile)
 - GNU Make (Tested only with v3.81)
 - GNU readline (sudo apt-get install libreadline-dev on Debian-based platforms) (GPL license); only required to build the REPL, not needed to build and run the tests.
