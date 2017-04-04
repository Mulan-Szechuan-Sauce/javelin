#!/bin/sh

# Compiles to "a.out" dy default
OUTPUT=a.out

while getopts ":o:" opt; do
  case $opt in
    o)
      OUTPUT=$OPTARG
      shift 2
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

./bin/scopeParser < $1 | ./bin/javelinParser | g++ -o $OUTPUT -std=c++11 -O3 -xc++ -
