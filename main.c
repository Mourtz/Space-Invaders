#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <termcap.h>
#include <error.h>
#include <unistd.h>

#define DELAY 30000

int **chars;

// term buffer
static char termbuf[2048];

int max_y, max_x;

void draw(){
  int x=0, y=0;
  for(;x<max_x;x++){
    for(;y<max_y;y++){
      if(chars[x][y] == 1){
        mvprintw(y, x, ".");
      }
    }
  }
  refresh();
}

int main(int argc, char *argv[])
{
  int ch, i;

  /* Curses Initialisations */
  initscr();
  raw();
  // curs_set(FALSE);
  keypad(stdscr, TRUE);
  noecho();

  printw("Welcome - Press # to Exit\n");

  char *termtype = getenv("TERM");

  if (tgetent(termbuf, termtype) < 0) {
      error(EXIT_FAILURE, 0, "Could not access the termcap data base.\n");
  }

  int lines = tgetnum("li");
  int columns = tgetnum("co");
  printw("Terminal stats:\nlines = %d; columns = %d.\n", lines, columns);

  // get max x and y
  getmaxyx(stdscr, max_y, max_x);

  // allocating memory space for buffer and chars array
  chars = malloc(sizeof(int*)*max_x);
  for(i=0;i<max_x;i++)
    chars = malloc(sizeof(int)*2);

  chars[5][1]= 1;
  chars[5][2]= 1;
  chars[5][3]= 1;
  chars[5][5]= 1;

  draw();

  while(true){
    if((ch = getch()) != '#')
    {
      switch(ch)
      {
        case KEY_UP:
          // printw("\nUp Arrow");

          break;
        case KEY_DOWN:
          // printw("\nDown Arrow");

          break;
        case KEY_LEFT:
          // printw("\nLeft Arrow");

          break;
        case KEY_RIGHT:
          // printw("\nRight Arrow");

          break;
        case KEY_NPAGE:
          // printw("\nNext Page");

          break;
        default:
        {
          // printw("\nThe pressed key is ");
          attron(A_BOLD);
          // printw("%c", ch);
          attroff(A_BOLD);
        }
      }
    }
    else{
      break;
    }
  }


printw("\n\nFarewell!\n");

refresh();
getch();
endwin();

return 0;
}
