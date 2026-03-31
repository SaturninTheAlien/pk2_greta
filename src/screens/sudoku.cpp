#include "sudoku.hpp"
#include <sstream>

namespace trolled{

Sudoku::Sudoku(){

}

void Sudoku::loadDefault(){

    this->loadString(
"1 0 2   5 4 6   0 0 0\n"
"9 3 0   0 0 0   0 2 0\n"
"0 0 6   0 2 0   0 0 0\n"

"4 6 8   0 0 0   0 0 0\n"
"2 0 0   0 1 0   5 0 0\n"
"3 0 0   0 0 0   0 0 4\n"

"5 0 0   1 3 4   0 0 0\n"
"0 0 0   0 8 0   0 0 0\n"
"0 0 0   2 5 0   8 4 0\n"
    );
}


void Sudoku::loadString(const std::string& s){
    std::istringstream in(s);
    for(int y=0;y<9;++y){
        for(int x=0;x<9;++x){
            int i = 0;
            in >> i;

            Cell c = i==0 ? Cell(0, false) : Cell(i, true);
            this->set(x, y, c);
        }
    }

}

const Cell& Sudoku::get(int x, int y)const{

    static const Cell invalid(-1, false);
    if(x>=0 && x<9 && y>=0 && y<9){
        return this->mArray[9*y + x];
    }
    return invalid;
}

void Sudoku::set(int x, int y, const Cell & value){
    if(x>=0 && x<9 && y>=0 && y<9){
        this->mArray[9*y + x] = value;
    }
}

bool Sudoku::checkCorrectness(SudokuError & e)const{

    for(int y=0;y<9;++y){
        for(int x=0;x<9;++x){
            const Cell& c = get(x, y);
            if(c.value < 1 || c.value > 9){
                e.type = EMPTY;
                e.x = x;
                e.y = y;
                return false;
            }
        }
    }

    for(int y=0;y<9;++y){
        for(int v=1;v<=9;++v){
            int lastPosX = -1;

            for(int x=0;x<9;++x){
                const Cell& c = this->get(x, y);
                if(c.value==v){

                    if(lastPosX==-1){
                        lastPosX = x;

                    } else {

                        e.y = y;
                        e.type = ROW;
                        if(c.locked){
                            e.x = lastPosX;
                        } else {
                            e.x = x;
                        }
                        return false;
                    }

                }
            }
        }
    }

    for(int x=0;x<9;++x){
        for(int v=1;v<=9;++v){
            int lastPosY = -1;

            for(int y=0;y<9;++y){
                const Cell& c = this->get(x, y);
                if(c.value==v){

                    if(lastPosY==-1){
                        lastPosY = y;

                    } else {

                        e.x = x;
                        e.type = COLUMN;
                        if(c.locked){
                            e.y = lastPosY;
                        } else {
                            e.y = y;
                        }
                        return false;
                    }

                }
            }
        }
    }


    for(int sy=0;sy<9;sy+=3){
        for(int sx=0;sx<9;sx+=3){

            for(int v=1;v<=9;++v){
                int lastX = -1;
                int lastY = -1;

                for(int y=sy; y<sy+3;++y){
                    for(int x=sx; x<sx+3; ++x){
                        const Cell& c = this->get(x, y);
                        if(c.value==v){
                            if(lastX==-1){
                                lastX=x;
                                lastY=y;
                            } else {
                                e.type = SQUARE;
                                if(c.locked){
                                    e.x = lastX;
                                    e.y = lastY;
                                } else {
                                    e.x = x;
                                    e.y = y;
                                }
                                return false;

                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

}