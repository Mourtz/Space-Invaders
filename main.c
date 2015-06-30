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

// draw call
void draw(){
  int x,y;
  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      //chars[x][y]=1;
      if(chars[x][y] == 1){
        mvprintw(y, x, ".");
        refresh();
        // usleep(300);
      }
    }
  }
}

/*
 * @param Beginning of Vector X
 * @param Beginning of Vector Y
 * @param Ending of Vector X
 * @param Ending of Vector Y
 */
void draw_line(int x0, int y0, int x1, int y1){
  int xdir=(x1-x0)>=0?1:-1;
  int ydir=(y1-y0)>=0?1:-1;
  int dx=xdir>0?x1-x0:x0-x1;
  int dy=ydir>0?y1-y0:y0-y1;
  int dir=dx>dy?1:0;
  int d=dir?dy+dy-dx:dx+dx-dy;
  int y=dir?y0:y0+ydir;
  int x=dir?x0+xdir:x0;
  int loop=0;
  int limit=dir?dx:dy;

  chars[x][y]=1;
  while(loop++<limit){
    if(d>0){
      if (dir == 1){
        y+=ydir;
      }
      else{
        x+=xdir;
      }
      chars[x][y]=1;
      d+=dir?dy+dy-dx-dx:dx+dx-dy-dy;
    }
    else{
      chars[x][y]=1;
      d+=dir?dy+dy:dx+dx;
    }
    if (dir==1){
      x+=xdir;
    }
    else{
      y+=ydir;
    }
  }
}

void draw_spacecraft(){

}

int main(int argc, char *argv[])
{
  int ch, x, y;
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
  printw("Terminal stats: lines = %d, columns = %d.\n", lines, columns);

  // get max x and y
  getmaxyx(stdscr, max_y, max_x);

  // allocating memory space for buffer and chars array
  chars =  (int**)malloc(max_x*(sizeof(int*)));
  for(x=0;x<max_x;x++){
    chars[x] = malloc(sizeof(int)*max_y);
  }

  // fill chars array with 0's
  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      chars[x][y]=0;
    }
  }

  printw("Press anything to render...\n");
  getch();

  // border
  draw_line(-1,0,max_x-1,0);
  draw_line(-1,max_y-1,max_x-1,max_y-1);
  draw_line(0,0,0,max_y-1);
  draw_line(max_x-1,0,max_x-1,max_y-1);


  while(true){
    draw();
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
    usleep(100);
  }


printw("\n\nFarewell!\n");

refresh();
getch();
endwin();

return 0;
}
