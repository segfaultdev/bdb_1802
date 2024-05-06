#!/bin/bash

gcc $(find . -name "*.c") -I ./include -o ./cosm -g -Og -fsanitize=undefined,address
