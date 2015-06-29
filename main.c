#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>			/* ncurses.h includes stdio.h */
#include <string.h>
#include <signal.h>
#include <termcap.h>
#include <error.h>

// term buffer
static char termbuf[2048];

int main(int argc, char *argv[])
{
  int ch;

  /* Curses Initialisations */
  initscr();
  raw();
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

  while((ch = getch()) != '#')
  {
    switch(ch)
    {
      case KEY_UP:
        printw("\nUp Arrow");

        break;
      case KEY_DOWN:
        printw("\nDown Arrow");

        break;
      case KEY_LEFT:
        printw("\nLeft Arrow");

        break;
      case KEY_RIGHT:
        printw("\nRight Arrow");

        break;
      case KEY_NPAGE:
        printw("\nNext Page");

        break;
      default:
      {
        printw("\nThe pressed key is ");
        attron(A_BOLD);
        printw("%c", ch);
        attroff(A_BOLD);
      }

    }
  }

printw("\n\nFarewell!\n");

refresh();
getch();
endwin();

return 0;
}
