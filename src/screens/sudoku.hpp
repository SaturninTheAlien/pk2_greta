#pragma once
#include <array>
#include <string>

namespace trolled{

class Cell{
public:
    Cell() = default;
    Cell(int value, bool locked):
    value(value), locked(locked){

    }
    int value = -1;
    bool locked = false;
};

enum ErrorType{
    INVALID = 0,
    EMPTY = 1,
    ROW = 2,
    COLUMN = 3,
    SQUARE = 4
};


class SudokuError{
public:
    int x = 0;
    int y = 0;
    ErrorType type = EMPTY;
};

class Sudoku{
public:
    Sudoku();
    
    void loadDefault();
    void loadString(const std::string& s);

    const Cell& get(int x, int y)const;
    void set(int x, int y, const Cell& value);
    bool checkCorrectness(SudokuError& e)const;
private:
    std::array<Cell, 81> mArray;
};

}