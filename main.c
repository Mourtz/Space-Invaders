/*
 *
 */

// some unicode characters
#define BLACK_SQUARE "\u25A0"
#define BULLSEYE "\u25CE"
#define MIDMOON "\u2022"
#define MIDDOT "\u00B7"
#define RING_OPERATOR "\u2218"

#define DELAY 30000

#include <stdlib.h>
#include <locale.h>
#include <ncurses.h>  // ncurses library
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <termcap.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>  // POSIX theading

WINDOW *canvas;

int **pixel_matrix;

int max_y, max_x;

// draw call
void draw(){
  int x,y;

  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      switch (pixel_matrix[x][y]) {
        case 0:
          mvwprintw(canvas, y, x, " ");
          // mvprintw(y, x, " ");
          break;
        case 1:
          mvwprintw(canvas, y, x, "\u2801");
          // mvprintw(y, x, MIDDOT);
          break;
        case 2:
          mvwprintw(canvas, y, x, "\u28FF");
          // mvprintw(y, x, MIDDOT);
          break;
        default:
          break;
      }
      // usleep(300);
    }
  }
  return;
}

void clear_canvas(){
  int x,y;
  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      mvwprintw(canvas, y, x, " ");
    }
  }
}

/*
 * @param Beginning of Vector X
 * @param Beginning of Vector Y
 * @param Ending of Vector X
 * @param Ending of Vector Y
 */
void draw_line(int x0, int y0, int x1, int y1,int value){
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

  pixel_matrix[x][y]=value;
  while(loop++<limit){
    if(d>0){
      if (dir == 1){
        y+=ydir;
      }
      else{
        x+=xdir;
      }
      pixel_matrix[x][y]=value;
      d+=dir?dy+dy-dx-dx:dx+dx-dy-dy;
    }
    else{
      pixel_matrix[x][y]=value;
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

// @param rectangle position X (top left corner)
// @param rectangle position Y (top left corner)
// @param rectangle width
// @param rectangle height
void draw_rect(int _x0, int _y0, int _width, int _height, int value){
  int PosX=_x0;
  int PosY=_y0;
  int width=_width;
  int height=_height;
  int EndX=PosX+width;

  // Draw horizontal lines
  for (int i = 0 ; i < height ; i++){
    int row=PosY+i;
    draw_line(PosX, row, EndX, row, value);
  }
}

void draw_border(){
  // border
  draw_line(-1,0,max_x-1,0, 1);
  draw_line(-1,max_y-1,max_x-1,max_y-1, 1);
  draw_line(0,0,0,max_y-1, 1);
  draw_line(max_x-1,0,max_x-1,max_y-1, 1);
}

//////////////////////////////////////////////////////////////////////////
// OBSTACLE
//////////////////////////////////////////////////////////////////////////

struct _obstacle{
  unsigned int posX, posY;    // the position of the obstacle
  unsigned int width, height; // the actual scale of the obsacle
  unsigned int *health;       // health of each "pixel" consisting the obstacle
};
struct _obstacle obstacle1 = {0, 0, 20, 6};
struct _obstacle obstacle2 = {0, 0, 20, 6};
struct _obstacle obstacle3 = {0, 0, 20, 6};
struct _obstacle obstacle4 = {0, 0, 20, 6};
struct _obstacle *obst[] = {&obstacle1, &obstacle2, &obstacle3, &obstacle4};
void obstacle_draw(){
  for(int i=0; i<4; i++){
    obst[i]->width = abs(max_x/8);
    obst[i]->height = abs(obst[i]->width/3);
    obst[i]->posX = abs(obst[i]->width/2)+(i*abs(obst[i]->width*2.0));
    obst[i]->posY = abs(max_y)-3*obst[i]->height;

    // memory space of healths array
    obst[i]->health = malloc((obst[i]->width+1)*obst[i]->height*sizeof(int*));

    for(int p=0; p<(obst[i]->width+1)*obst[i]->height; p++){
      obst[i]->health[p] = 100;
    }

    draw_rect(obst[i]->posX ,obst[i]->posY, obst[i]->width, obst[i]->height, 2);
  }
}

void obstacle_damage(int x, int y, int n){
  for(int i=obst[n]->posY; i<=obst[n]->posY+obst[n]->height; i++){
    for(int l=obst[n]->posX; l<=obst[n]->posX+obst[n]->width; l++){
      if(x==l && y==i){
        int ii = i - obst[n]->posY;
        int ll = l - obst[n]->posX;

        obst[n]->health[obst[n]->width*ii + ll] -= 50;
        if(obst[n]->health[obst[n]->width*ii + ll] <= 0){
          pixel_matrix[x][y]=0;
        }
        return;
      }
    }
  }
}

int obstacle_inBounds(int x){
  for(int i=0; i<4; i++){
    if(x<=obst[i]->posX+obst[i]->width && x>obst[i]->posX )
      return i;
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////
// SPACECRAFT
//////////////////////////////////////////////////////////////////////////

struct _spacecraft{
  unsigned int posX, posY;
  unsigned int width, height;
  unsigned int health;  // [0,100]
  bool hasfired;
};
struct _spacecraft spacecraft = {0, 0, 7, 2, 100, false};
struct _spacecraft *spcr = &spacecraft;
// draw spacecraft on screen
void draw_spacecraft(){
  spcr->posX = abs(max_x/2)-abs(spcr->width/2);
  spcr->posY = abs(max_y)-2*spcr->height;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 1);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 1);
  return;
}
// erase spacecraft from canvas
void spacecraft_clear(){
  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 0);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 0);
  return;
}
// move the spacecraft to the right
void *spacecraft_moveR(){
  spacecraft_clear();
  spcr->posX ++;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 1);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 1);
  return;
}
// move the spacecraft to the left
void *spacecraft_moveL(){
  spacecraft_clear();
  spcr->posX --;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 1);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 1);
  return;
}
void *spacecraft_fire(int value){
  int pY=spcr->posY-1, pX=spcr->posX+abs(spcr->width/2)+1;
  while(pY>1){
    //
    if(pixel_matrix[pX][pY-1] == 2){
      int _p = obstacle_inBounds(pX);
      //decrease the health of obstacle by 5
      if(_p != -1){
        obstacle_damage(pX, pY-1, _p);
        wbkgdset(canvas, COLOR_PAIR(2));
      }

      return;
    }
    pixel_matrix[pX][--pY]=value;
  }
  return;
}

//////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  pthread_t tid;

  setlocale(LC_ALL,"");

  int ch, x, y;

  /* Curses Initialisations */
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(FALSE);
  nodelay(stdscr, TRUE);
  clear();
  // raw();
  // get max x and y
  getmaxyx(stdscr, max_y, max_x);
  use_default_colors();

  printw("Welcome - Press # to Exit\n");

  init_color(COLOR_CYAN, 0,255,255);
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);

  canvas = newwin(max_y,max_x,0,0);
  wbkgdset(canvas, COLOR_PAIR(1));
  // box(canvas, 0, 0);

  // allocating memory space for buffer and pixel_matrix array
  pixel_matrix =  (int**)malloc(max_x*(sizeof(int*)));
  for(x=0;x<max_x;x++){
    pixel_matrix[x] = malloc(sizeof(int)*max_y);
  }

  // fill pixel_matrix array with 0's
  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      pixel_matrix[x][y]=0;
    }
  }

  printw("Press anything to render...\n");
  getch();

  draw_border();
  draw_spacecraft();
  obstacle_draw();

  while(true){
    draw();
    wrefresh(canvas);

    ch = getch();
    if(ch != ERR){
      if(ch == '#'){
        break;
      }
      else if(ch == KEY_UP){
        if(spcr->hasfired == false){
          spacecraft_fire (1);
          spcr->hasfired = true;
        }
      }
      else if(ch == KEY_DOWN){

      }
      else if(ch == KEY_LEFT){
        pthread_create(&tid, NULL, spacecraft_moveL, NULL);
        if(spcr->hasfired == true){
          spacecraft_fire (0);
          spcr->hasfired = false;
        }
      }
      else if(ch == KEY_RIGHT){
        pthread_create(&tid, NULL, spacecraft_moveR, NULL);
        if(spcr->hasfired == true){
          spacecraft_fire (0);
          spcr->hasfired = false;
        }
      }
      else
      {

      }
    }
    else{
      if(spcr->hasfired == true){
        spacecraft_fire (0);
        spcr->hasfired = false;
      }
    }
    usleep(20000);
  }


printw("\n\nFarewell!\n");

refresh();
getch();
endwin();

pthread_exit(NULL);

return 0;
}
