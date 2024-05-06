#!/bin/bash

gcc $(find . -name "*.c") -Iinclude -g -Og -fsanitize=address,undefined -o ./emul
