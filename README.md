# FIFE

This is a Framework for Investigating Functional Encryption (FIFE). This started as my M.Eng. Thesis project ((thesis here)[http://gauravsingh.com/files/meng-thesis.pdf]).

## Requirements:

Other than this project, you will need to install the libraries Crypto++, GMP, NTL, msgpack, and libgarble. The first three can be done via apt-get:
```
sudo apt-get install libcrypto++9v5 libcrypto++9v5-dbg libcrypto++-dev
sudo apt-get install libntl-dev
```
You will also need to install msgpack (http://msgpack.org/index.html). Some installation instructions are here (https://github.com/msgpack/msgpack-c/blob/master/QUICKSTART-C.md).

Lastly, you will need libgarble (https://github.com/amaloz/libgarble) to garble circuits. Clone the repo, and then checkout commit 131e963, which is the last commit I know to work with this library. Then, follow the installation instructions on the home page.

### Instructions for Mac:

These instructions may not be accurate, but might be helpful.
```
sudo ports install libcryptopp
sudo ports install gmp
```

Install NTL from source and compile according to http://www.shoup.net/ntl/doc/tour-gmp.html

Install msgpack and libgarble from source, as for linux.

## Instructions for using:

Run 'make', and then run 'a.out exampleConfig'. This will make and run the current src/main.cpp file, with the exampleConfig file. This config file can be modified to run different tests. You can run 'make tests' and './tests' to make and run the tests.
