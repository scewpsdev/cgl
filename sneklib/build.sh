#!/bin/bash

code="$PWD"
opts=-g
cd bin > /dev/null
g++ $opts $code/windows -o sneklib.exe
cd $code > /dev/null
