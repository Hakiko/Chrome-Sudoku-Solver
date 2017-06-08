all: sudoku sudoku_standalone

sudoku: sudoku.cpp
	g++ -std=c++11 -o sudoku_chrome -DCHROME sudoku.cpp

sudoku_standalone: sudoku.cpp
	g++ -std=c++11 -o sudoku_standalone sudoku.cpp

clean:
	rm -f sudoku_standalone sudoku_chrome
