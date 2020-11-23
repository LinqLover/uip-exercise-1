#!/bin/bash
set -e

cd pscom
rm Makefile
qmake CONFIG+=debug
make clean all CONFIG+=debug

cd ../pscom-cli
rm Makefile
qmake CONFIG+=debug
make clean all CONFIG+=debug
