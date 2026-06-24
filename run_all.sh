#!/bin/bash
case $1 in
  11) cd Lektion11/build && cmake .. && make && ./Lektion11 ;;
  12) cd Lektion12/build && cmake .. && make && ./Lektion12 ;;
  13) cd Lektion13/build && cmake .. && make && ./Lektion13 ;;
  14) cd Lektion14/build && cmake .. && make && ./Lektion14 ;;
  15) cd Lektion15/build && cmake .. && make && ./Lektion15 ;;
  16) cd Lektion16/build && cmake .. && make && ./Lektion16 ;;
  17) cd Lektion17/build && cmake .. && make && ./Lektion17 ;;
  18) cd Lektion18/build && cmake .. && make && ./Lektion18 ;;
  19) cd Lektion19/build && cmake .. && make && ./Lektion19 ;;
  *) echo "Nutze: ./run_all.sh [11|12|13|14|15|16|17|18|19]" ;;
esac