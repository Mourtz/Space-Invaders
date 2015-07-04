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

// https://en.wikipedia.org/wiki/Braille_Patterns#Chart
const char* charcode[5] = {" ", "\u2801", "\u28FF" , "\u2847", "\u2809"};

WINDOW *canvas;

int **pixel_matrix;

int max_y, max_x;

// draw call
void draw(){
  int x,y;

  for(x=0;x<max_x;x++){
    for(y=0;y<max_y;y++){
      mvwprintw(canvas, y, x, charcode[pixel_matrix[x][y]]);
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
  draw_line(-1,0,max_x-1,0, 4);
  draw_line(-1,max_y-1,max_x-1,max_y-1, 4);
  draw_line(0,-1,0,max_y-2, 3);
  draw_line(max_x-1,-1,max_x-1,max_y-2, 3);
}

//////////////////////////////////////////////////////////////////////////
// ENEMY
//////////////////////////////////////////////////////////////////////////

struct _enemy{
  unsigned int posX, posY;    // enemy's postion
  unsigned int width, height; // the actual scale of enemy
  unsigned int health;        // enemy's health (0-100)
  char type;                  // enemy type
  char moveDir;
};
//    A type Enemies
struct _enemy enemy1 = {0, 0, 0, 0, 100, 'a', 'r'};struct _enemy enemy2 = {0, 0, 0, 0, 100, 'a', 'r'};
struct _enemy enemy3 = {0, 0, 0, 0, 100, 'a', 'r'};struct _enemy enemy4 = {0, 0, 0, 0, 100, 'a', 'r'};
struct _enemy enemy5 = {0, 0, 0, 0, 100, 'a', 'r'};struct _enemy enemy6 = {0, 0, 0, 0, 100, 'a', 'r'};
struct _enemy *enemA[] = {&enemy1, &enemy2, &enemy3, &enemy4, &enemy5, &enemy6};
//    B type enemies
struct _enemy enemy7 = {0, 0, 0, 0, 100, 'b', 'l'};struct _enemy enemy8 = {0, 0, 0, 0, 100, 'b', 'l'};
struct _enemy enemy9 = {0, 0, 0, 0, 100, 'b', 'l'};struct _enemy enemy10 = {0, 0, 0, 0, 100, 'b', 'l'};
struct _enemy enemy11 = {0, 0, 0, 0, 100, 'b', 'l'};struct _enemy enemy12 = {0, 0, 0, 0, 100, 'b', 'l'};
struct _enemy *enemB[] = {&enemy7, &enemy8, &enemy9, &enemy10, &enemy11, &enemy12};
//    C type enemies
struct _enemy enemy13 = {0, 0, 0, 0, 100, 'c', 'r'};struct _enemy enemy14 = {0, 0, 0, 0, 100, 'c', 'r'};
struct _enemy enemy15 = {0, 0, 0, 0, 100, 'c', 'r'};struct _enemy enemy16 = {0, 0, 0, 0, 100, 'c', 'r'};
struct _enemy enemy17 = {0, 0, 0, 0, 100, 'c', 'r'};struct _enemy enemy18 = {0, 0, 0, 0, 100, 'c', 'r'};
struct _enemy *enemC[] = {&enemy13, &enemy14, &enemy15, &enemy16, &enemy17, &enemy18};
void enemy_draw(){
  for(int i=0,z=0; i<18; i++){
    if(i<6){
      enemA[i]->width = abs(max_x/18);
      enemA[i]->height = abs(enemA[i]->width/2);
      enemA[i]->posX = abs(enemA[i]->width/2)+(i*abs(enemA[i]->width*2.0));
      enemA[i]->posY = abs(enemA[i]->height/2);
      draw_rect(enemA[i]->posX ,enemA[i]->posY, enemA[i]->width, enemA[i]->height, 2);
    }
    else if(i<12){
      z=i;
      i-=6;

      enemB[i]->width = abs(max_x/18);
      enemB[i]->height = abs(enemB[i]->width/2);
      enemB[i]->posX = abs(enemB[i]->width/2)+(i*abs(enemB[i]->width*2.0));
      enemB[i]->posY = abs(enemB[i]->height/2)+abs(enemB[i]->height*1.5);;
      draw_rect(enemB[i]->posX ,enemB[i]->posY, enemB[i]->width, enemB[i]->height, 2);

      i=z;
    }
    else{
      z=i;
      i-=12;

      enemC[i]->width = abs(max_x/18);
      enemC[i]->height = abs(enemC[i]->width/2);
      enemC[i]->posX = abs(enemC[i]->width/2)+(i*abs(enemC[i]->width*2.0));
      enemC[i]->posY = abs(enemC[i]->height/2)+abs(enemC[i]->height*3.0);;
      draw_rect(enemC[i]->posX ,enemC[i]->posY, enemC[i]->width, enemC[i]->height, 2);

      i=z;
    }
  }
}
void enemy_clear(){
  for(int i=0,z=0; i<18; i++){
    if(i<6){
      draw_rect(enemA[i]->posX ,enemA[i]->posY, enemA[i]->width, enemA[i]->height, 0);
    }
    else if(i<12){
      z=i;i-=6;
      draw_rect(enemB[i]->posX ,enemB[i]->posY, enemB[i]->width, enemB[i]->height, 0);
      i=z;
    }
    else{
      z=i;i-=12;
      draw_rect(enemC[i]->posX ,enemC[i]->posY, enemC[i]->width, enemC[i]->height, 0);
      i=z;
    }
  }
}
void enemy_update() {
  enemy_clear();
  for(int i=0,z=0; i<18; i++){
    if(i<6){
      if(enemA[i]->moveDir == 'r'){
        if((enemA[5]->posX + enemA[5]->width) == (max_x-2)){
          enemA[i]->moveDir = 'l';
        }else{
          enemA[i]->posX ++;
        }
      }
      else{
        if(enemA[0]->posX == 0){
          enemA[i]->moveDir = 'r';
        }else{
          enemA[i]->posX --;
        }
      }
      draw_rect(enemA[i]->posX ,enemA[i]->posY, enemA[i]->width, enemA[i]->height, 2);
    }
    else if(i<12){
      z=i;i-=6;
      if(enemB[i]->moveDir == 'r'){
        if((enemB[5]->posX + enemB[5]->width) == (max_x-2)){
          enemB[i]->moveDir = 'l';
        }else{
          enemB[i]->posX ++;
        }
      }
      else{
        if(enemB[0]->posX == 0){
          enemB[i]->moveDir = 'r';
        }else{
          enemB[i]->posX --;
        }
      }
      draw_rect(enemB[i]->posX ,enemB[i]->posY, enemB[i]->width, enemB[i]->height, 2);
      i=z;
    }
    else{
      z=i;i-=12;
      if(enemC[i]->moveDir == 'r'){
        if((enemC[5]->posX + enemC[5]->width) == (max_x-2)){
          enemC[i]->moveDir = 'l';
        }else{
          enemC[i]->posX ++;
        }
      }
      else{
        if(enemC[0]->posX == 0){
          enemC[i]->moveDir = 'r';
        }else{
          enemC[i]->posX --;
        }
      }
      draw_rect(enemC[i]->posX ,enemC[i]->posY, enemC[i]->width, enemC[i]->height, 2);
      i=z;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// OBSTACLE
//////////////////////////////////////////////////////////////////////////

struct _obstacle{
  unsigned int posX, posY;    // obstacles position
  unsigned int width, height; // the actual scale of obsacle
  int *health;       // health of each "pixel" consisting the obstacle
};

struct _obstacle obstacle1 = {0, 0, 0, 0};
struct _obstacle obstacle2 = {0, 0, 0, 0};
struct _obstacle obstacle3 = {0, 0, 0, 0};
struct _obstacle obstacle4 = {0, 0, 0, 0};
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

        obst[n]->health[obst[n]->width*ii + ll] -= 10;
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
  unsigned int posX, posY;    // spacecrafts position
  unsigned int width, height; //  the actual scale of spacecraft
  unsigned int health;        // spacecrafts health (0-100)
  bool hasfired;
};

struct _spacecraft spacecraft = {0, 0, 7, 2, 100, false};
struct _spacecraft *spcr = &spacecraft;

// draw spacecraft on screen
void draw_spacecraft(){
  spcr->posX = abs(max_x/2)-abs(spcr->width/2);
  spcr->posY = abs(max_y)-2*spcr->height;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 2);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 2);
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

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 2);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 2);
  return;
}
// move the spacecraft to the left
void *spacecraft_moveL(){
  spacecraft_clear();
  spcr->posX --;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 2);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 2);
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
  enemy_draw();

  while(true){
    draw();
    wrefresh(canvas);

    ch = getch();
    if(ch != ERR){
      if(ch == '#'){
        break;
      }
      // UP ARROW
      else if(ch == KEY_UP){
        if(spcr->hasfired == false){
          spacecraft_fire (4);
          spcr->hasfired = true;
        }
      }
      // DOWN ARROW
      else if(ch == KEY_DOWN){

      }
      // LEFT ARROW
      else if(ch == KEY_LEFT){
        spacecraft_moveL();
        if(spcr->hasfired == true){
          spacecraft_fire (0);
          spcr->hasfired = false;
        }
      }
      // RIGHT ARROW
      else if(ch == KEY_RIGHT){
        spacecraft_moveR();
        if(spcr->hasfired == true){
          spacecraft_fire (0);
          spcr->hasfired = false;
        }
      }
      // SPACEBAR
      else if(ch == ' '){
        //pthread_create(&tid, NULL, enemy_update, NULL);
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
    pthread_create(&tid, NULL, enemy_update, NULL);
    usleep(20000);
  }


  printw("\n\nFarewell!\n");

  refresh();
  getch();
  endwin();

  pthread_exit(NULL);

  return 0;
}
