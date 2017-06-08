#!/bin/sh
killall -9 chrome
cd /home/hayachi/Tools/sudoku
./clear_len <&0 | python unwrap.py | ./sudoku | ./wrap
