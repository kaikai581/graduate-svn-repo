#include <iomanip>
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <tclap/CmdLine.h>


using namespace std;



/// global variables
pair<int,int> move[8];
int           N; // size of chessboard



class ChessBoard
{
public:
  ChessBoard(int);
  ~ChessBoard(){};
  //int footprint[8][8];
  vector<vector<int> > footprint;
  bool Unvisited(int,int);
  bool Inbound(int,int);
  void PrintFootPrint();
private:
  int m_size;
};

ChessBoard::ChessBoard(int N) : m_size(N)
{
  for(int i = 0; i < m_size; i++)
  {
    vector<int> chessrow;
    for(int j = 0; j < m_size; j++)
      chessrow.push_back(-1);
    footprint.push_back(chessrow);
  }
}

bool ChessBoard::Inbound(int x, int y)
{
  return ((x >= 0 && x <= m_size-1) && (y >= 0 && y <= m_size-1));
}

bool ChessBoard::Unvisited(int x, int y)
{
  return (footprint[x][y] < 0);
}

void ChessBoard::PrintFootPrint()
{
  for(int j = m_size-1; j >= 0; j--)
  {
    for(int i = 0; i < m_size; i++)
      cout << setw(2) << footprint[i][j] << " ";
    cout << endl;
  }
  cout << endl;
}



class KnightChess
{
public:
  int x;
  int y;
  
  bool Move(int);
};

bool KnightChess::Move(int mode)
{
  int dx, dy;
  
  switch (mode) {
    case 1:
      dx = 2;
      dy = 1;
      break;
    case 2:
      dx = 2;
      dy = -1;
      break;
    case 3:
      dx = 1;
      dy = 2;
      break;
    case 4:
      dx = 1;
      dy = -2;
      break;
    case 5:
      dx = -2;
      dy = 1;
      break;
    case 6:
      dx = -2;
      dy = -1;
      break;
    case 7:
      dx = -1;
      dy = 2;
      break;
    case 8:
      dx = -1;
      dy = -2;
      break;
  }
  
  int newx = x+dx;
  int newy = y+dy;
  
  if((newx <= 7 && newx >= 0) && (newy <= 7 && newy >= 0))
  {
    x = newx;
    y = newy;
    return true;
  }
  
  return false;
}


KnightChess knight;

void advance_knight(ChessBoard&,int,int,int);


int main (int argc, char* argv[])
{
  /// set up command arguments
  TCLAP::CmdLine cmd("Command description message", ' ', "0.9");
  TCLAP::ValueArg<int> sizeArg("s", "size", "size of the chessborad", false, 4, "int");
  cmd.add(sizeArg);
  cmd.parse(argc, argv);
  N = sizeArg.getValue();
  cout << "size of the chessboard: " << N << endl;

  move[0] = pair<int,int>(2,1);
  move[1] = pair<int,int>(2,-1);
  move[2] = pair<int,int>(1,2);
  move[3] = pair<int,int>(1,-2);
  move[4] = pair<int,int>(-2,1);
  move[5] = pair<int,int>(-2,-1);
  move[6] = pair<int,int>(-1,2);
  move[7] = pair<int,int>(-1,-2);

  /* initialize random seed: */
  srand (time(NULL));

  /* generate secret number between 0 and 7: */
  int initx = rand() % N;
  int inity = rand() % N;

  /// initialize chessboard values
  //for(int i = 0; i < 8; i++)
    //for(int j = 0; j < 8; j++)
      //chessboard[i][j] = -1;
  
  ChessBoard chessboard(N);
  advance_knight(chessboard, initx,inity,1);
  //advance_knight(chessboard, 0,1,1);

  return 0;
}



void advance_knight(ChessBoard& chessboard, int x, int y, int stage)
{
  //cout << "current coordinate: (" << x << "," << y << ")";
  cout << " in the " << stage << " stage" << endl;
  //if(!(stage%10)) cout << "stage " << stage << endl;
  
  /// mark the stage on the chess board
  chessboard.footprint[x][y] = stage;
  chessboard.PrintFootPrint();
  
  if(stage == N*N)
  {
    cout << "problem solved!" << endl;
    chessboard.PrintFootPrint();
    exit(1);
  }
  else
  {
    for(int i = 0; i < 8; i++)
    {
      int nextx = x + move[i].first;
      int nexty = y + move[i].second;
      if(chessboard.Inbound(nextx, nexty) && chessboard.Unvisited(nextx, nexty))
        advance_knight(chessboard, nextx,nexty,stage+1);

    }
    cout << "stage " << stage << endl;
    //chessboard.PrintFootPrint();
    chessboard.footprint[x][y] = -1;
  }
}

