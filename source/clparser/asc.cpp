/******************************************************************************
**
** asc.cpp
**
** Thu May 17 15:54:29 2001
** Linux 2.4.4 (#1 SMP Sam Apr 28 13:21:30 CEST 2001) i686
** martin@linux. (Martin Bickel)
**
** Definition of command line parser class
**
** Automatically created by genparse v0.5.2
**
** See http://genparse.sourceforge.net/ for details and updates
**
******************************************************************************/

#include <getopt.h>
#include <stdlib.h>
#include "asc.h"

/*----------------------------------------------------------------------------
**
** Cmdline::Cmdline()
**
** Constructor method.
**
**--------------------------------------------------------------------------*/

Cmdline::Cmdline(int argc, char *argv[])
{
  extern char *optarg;
  extern int optind;
  int option_index = 0;
  int c;

  static struct option long_options[] =
  {
    {"xresolution", 1, 0, 'x'},
    {"yresolution", 1, 0, 'y'},
    {"load", 1, 0, 'l'},
    {"mapfile", 1, 0, 'M'},
    {"configfile", 1, 0, 'c'},
    {"verbose", 1, 0, 'r'},
    {"window", 0, 0, 'w'},
    {"fullscreen", 0, 0, 'f'},
    {"nosound", 0, 0, 'q'},
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {"headless", 0, 0, 'H'},
    {"player1", 1, 0, 'P'},
    {"player2", 1, 0, 'Q'},
    {"turnlimit", 1, 0, 'T'},
    {0, 0, 0, 0}
  };

  _executable += argv[0];

  /* default values */
  _x = -1;
  _y = -1;
  _r = 0;
  _w = false;
  _f = false;
  _q = false;
  _h = false;
  _v = false;
  _headless = false;
  _player1 = "classic";
  _player2 = "classic";
  _turnLimit = 0;

  while ((c = getopt_long(argc, argv, "x:y:l:c:r:wfqhvHM:P:Q:T:", long_options, &option_index)) != EOF)
    {
      switch(c)
        {
        case 'x': 
          _x = atoi(optarg);
          if (_x < 800)
            {
              string s;
              s += "parameter range error: x must be >= 800";
              throw(s);
            }
          break;

        case 'y': 
          _y = atoi(optarg);
          if (_y < 600)
            {
              string s;
              s += "parameter range error: y must be >= 600";
              throw(s);
            }
          break;

        case 'l': 
          _l = optarg;
          break;

        case 'M': 
          _l = optarg;
          break;

        case 'c': 
          _c = optarg;
          break;

        case 'r': 
          _r = atoi(optarg);
          if (_r < 0)
            {
              string s;
              s += "parameter range error: r must be >= 0";
              throw(s);
            }
          if (_r > 10)
            {
              string s;
              s += "parameter range error: r must be <= 10";
              throw(s);
            }
          break;

        case 'w': 
          _w = true;
          break;

        case 'f': 
          _f = true;
          break;

        case 'q': 
          _q = true;
          break;

        case 'h': 
          _h = true;
          this->usage();
          break;

        case 'v': 
          _v = true;
          break;

        case 'H':
          _headless = true;
          break;

        case 'P':
          if (!optarg || !optarg[0])
            {
              string s;
              s += "player1 requires an argument";
              throw(s);
            }
          _player1 = optarg;
          break;

        case 'Q':
          if (!optarg || !optarg[0])
            {
              string s;
              s += "player2 requires an argument";
              throw(s);
            }
          _player2 = optarg;
          break;

        case 'T':
          if (!optarg || !optarg[0])
            {
              string s;
              s += "turnlimit requires an argument";
              throw(s);
            }
          _turnLimit = atoi(optarg);
          if (_turnLimit < 0)
            {
              string s;
              s += "parameter range error: turnlimit must be >= 0";
              throw(s);
            }
          break;

        default:
          this->usage();

        }
    } /* while */

  _optind = optind;
}

/*----------------------------------------------------------------------------
**
** Cmdline::usage()
**
** Usage function.
**
**--------------------------------------------------------------------------*/

void Cmdline::usage()
{
  cout << "Advanced Strategic Command: a turn based strategy game " << endl;
  cout << "usage: " << _executable << " [ -xylcrwfqhvHMPQT ] " << endl;
  cout << "  [ -x ] ";
  cout << "[ --xresolution ]  ";
  cout << "(";
  cout << "type=";
  cout << "INTEGER,";
  cout << " range=800...,";
  cout << " default=1024";
  cout << ")\n";
  cout << "         Set horizontal resolution to <X>\n";
  cout << "  [ -y ] ";
  cout << "[ --yresolution ]  ";
  cout << "(";
  cout << "type=";
  cout << "INTEGER,";
  cout << " range=600...,";
  cout << " default=800";
  cout << ")\n";
  cout << "         Set vertical resolution to <Y>\n";
  cout << "  [ -l ] ";
  cout << "[ --load ]  ";
  cout << "(";
  cout << "type=";
  cout << "STRING";
  cout << ")\n";
  cout << "         Load a map, save game, or email game on startup\n";
  cout << "  [ -c ] ";
  cout << "[ --configfile ]  ";
  cout << "(";
  cout << "type=";
  cout << "STRING";
  cout << ")\n";
  cout << "         Use given configuration file\n";
  cout << "  [ -r ] ";
  cout << "[ --verbose ]  ";
  cout << "(";
  cout << "type=";
  cout << "INTEGER,";
  cout << " range=0...10,";
  cout << " default=0";
  cout << ")\n";
  cout << "         Set verbosity level to x (0..10)\n";
  cout << "  [ -w ] ";
  cout << "[ --window ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Disable fullscreen mode\n";
  cout << "  [ -f ] ";
  cout << "[ --fullscreen ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Enable fullscreen mode (overriding config file)\n";
  cout << "  [ -q ] ";
  cout << "[ --nosound ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Disable sound\n";
  cout << "  [ -h ] ";
  cout << "[ --help ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Display help information.\n";
  cout << "  [ -v ] ";
  cout << "[ --version ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Output version.\n";
  cout << "  [ -H ] ";
  cout << "[ --headless ]  ";
  cout << "(";
  cout << "type=";
  cout << "FLAG";
  cout << ")\n";
  cout << "         Run the simulation without graphics.\n";
  cout << "  [ -M ] ";
  cout << "[ --mapfile ]  ";
  cout << "(";
  cout << "type=";
  cout << "STRING";
  cout << ")\n";
  cout << "         Load a map file (alias for --load).\n";
  cout << "  [ -P ] ";
  cout << "[ --player1 ]  ";
  cout << "(";
  cout << "type=";
  cout << "STRING,";
  cout << " default=classic";
  cout << ")\n";
  cout << "         Configure player 1 AI type for headless mode.\n";
  cout << "         Options: classic, mcts_balanced, mcts_aggressive, mcts_defensive, mcts_fast, mcts_deep\n";
  cout << "  [ -Q ] ";
  cout << "[ --player2 ]  ";
  cout << "(";
  cout << "type=";
  cout << "STRING,";
  cout << " default=classic";
  cout << ")\n";
  cout << "         Configure player 2 AI type for headless mode.\n";
  cout << "         Options: classic, mcts_balanced, mcts_aggressive, mcts_defensive, mcts_fast, mcts_deep\n";
  cout << "  [ -T ] ";
  cout << "[ --turnlimit ]  ";
  cout << "(";
  cout << "type=";
  cout << "INTEGER,";
  cout << " range=0...,";
  cout << " default=0";
  cout << ")\n";
  cout << "         Limit headless play to the given number of rounds (0 = unlimited).\n";
  exit(0);
}

