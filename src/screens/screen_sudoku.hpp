#pragma once

#include "screen.hpp"
#include "sudoku.hpp"

class SudokuScreen: public Screen{
public:
    SudokuScreen() = default;
    void Init()override;
    void Loop()override;
    void onKeyPressed(const PInput::Key& key)override;
private:
    trolled::Sudoku sudoku;
    trolled::SudokuError sudokuError;
    bool showingError = false;
    bool finished = false;
    bool closing = false;

    void draw();
    void drawSudoku(int startX, int startY);

    void drawSudokuCell(int posX, int posY, int cellX, int cellY);

    void finish();

    void setCellValue(int i);
    void cycleCellValue(int dv=1);

    int selectedX = 0;
    int selectedY = 0;
    
};