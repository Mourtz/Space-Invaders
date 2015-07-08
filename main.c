/*
 *
 */
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
/*                               01       11(GEN)  10       11       10       01       11(ACTOR)11(ENEMIES)
 *                               00       11       10       00       01       10       11       11
 *                               00       11       10       00       00       00       11       11
 *                               00       11       10       00       00       00       11       11
 *                               00       11       10       00       00       00       11       11
 */
const char* charcode[9] ={" ","\u2801","\u28FF","\u2847","\u2809","\u2822","\u280A","\u28FF","\u28FF"};

pthread_t thread1, thread2;
int ch, x, y;

WINDOW *canvas;

int **pixel_matrix;

int *projectileCache_X, *projectileCache_Y, projectileCache_length = 0;
// max rows and columns of terminal.
int max_y, max_x;

// main draw call
void draw(){
  for(int x=0,y;x<max_x;x++,y=0)
    for(;y<max_y;y++)
      mvwprintw(canvas, y, x, charcode[pixel_matrix[x][y]]);
}

void clear_canvas(){
  for(int x=0,y;x<max_x;x++,y=0)
    for(;y<max_y;y++)
      mvwprintw(canvas, y, x, " ");
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

void clean_projectiles(){
  for(int x=0;x<projectileCache_length;x++){
    pixel_matrix[projectileCache_X[x]][projectileCache_Y[x]] = 0;
  }
  projectileCache_length = 0;
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
void obstacle_damage(int x, int y, int n, int damage){
  for(int i=obst[n]->posY; i<=obst[n]->posY+obst[n]->height; i++){
    for(int l=obst[n]->posX; l<=obst[n]->posX+obst[n]->width; l++){
      if(x==l && y==i){
        int ii = i - obst[n]->posY;
        int ll = l - obst[n]->posX;

        obst[n]->health[obst[n]->width*ii + ll] -= damage;
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
  int health;                 // spacecrafts health (0-100)
  bool hasfired;
};

struct _spacecraft spacecraft = {0, 0, 7, 2, 100, false};
struct _spacecraft *spcr = &spacecraft;

// draw spacecraft on screen
void draw_spacecraft(){
  spcr->posX = abs(max_x/2)-abs(spcr->width/2);
  spcr->posY = abs(max_y)-2*spcr->height;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 7);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 7);
  return;
}
// erase spacecraft from canvas
void spacecraft_clear(){
  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 0);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 0);
  return;
}
// move the spacecraft to the right
void spacecraft_moveR(){
  if((spcr->posX + spcr->width) == (max_x-2))
    return;

  spacecraft_clear();
  spcr->posX ++;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 7);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 7);
  return;
}
// move the spacecraft to the left
void spacecraft_moveL(){
  if(spcr->posX == 0)
    return;

  spacecraft_clear();
  spcr->posX --;

  draw_rect(spcr->posX ,spcr->posY, spcr->width, spcr->height, 7);
  draw_line(spcr->posX + spcr->width/3, spcr->posY-1, spcr->posX + 2*spcr->width/3+1, spcr->posY-1, 7);
  return;
}
void spacecraft_damage(int damage){
  spcr->health -= damage;
  if(spcr->health <= 0){
    endwin();
    printf("game over fella :( \n");
    exit(0);
  }
  return;
}
void *spacecraft_fire(int value){
  int pY=spcr->posY-2, pX=spcr->posX+abs(spcr->width/2)+1;
  while(pY>0){
    if(value != 0){
      if(pixel_matrix[pX][pY-1] == 2){
        int _p = obstacle_inBounds(pX);
        if(_p != -1){
          obstacle_damage(pX, pY-1, _p, 10);
          wbkgdset(canvas, COLOR_PAIR(2));
        }
        projectileCache_X[projectileCache_length] = pX;
        projectileCache_Y[projectileCache_length++] = pY;
        clean_projectiles();
        return;
      }
      else if(pixel_matrix[pX][pY-1] == 8){
        projectileCache_X[projectileCache_length] = pX;
        projectileCache_Y[projectileCache_length++] = pY;

        enemy_kill(enemy_inBounds_get(pX,pY-1));

        clean_projectiles();
        return;
      }
      pixel_matrix[pX][pY]=0;
      pixel_matrix[pX][--pY]=value;
    }
    usleep(DELAY);
  }
  projectileCache_X[projectileCache_length] = pX;
  projectileCache_Y[projectileCache_length++] = pY;
  clean_projectiles();
  return;
}
int spacecraft_inBounds(int x){
  if(x<=spcr->posX+spcr->width && x>spcr->posX )
    return 1;

  return -1;
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
struct _enemy enemy1 = {0, 0, 0, 0, 100, 'A', 'r'};struct _enemy enemy2 = {0, 0, 0, 0, 100, 'A', 'r'};
struct _enemy enemy3 = {0, 0, 0, 0, 100, 'A', 'r'};struct _enemy enemy4 = {0, 0, 0, 0, 100, 'A', 'r'};
struct _enemy enemy5 = {0, 0, 0, 0, 100, 'A', 'r'};struct _enemy enemy6 = {0, 0, 0, 0, 100, 'A', 'r'};
struct _enemy *enemA[] = {&enemy1, &enemy2, &enemy3, &enemy4, &enemy5, &enemy6};
//    B type enemies
struct _enemy enemy7 = {0, 0, 0, 0, 100, 'B', 'l'};struct _enemy enemy8 = {0, 0, 0, 0, 100, 'B', 'l'};
struct _enemy enemy9 = {0, 0, 0, 0, 100, 'B', 'l'};struct _enemy enemy10 = {0, 0, 0, 0, 100, 'B', 'l'};
struct _enemy enemy11 = {0, 0, 0, 0, 100, 'B', 'l'};struct _enemy enemy12 = {0, 0, 0, 0, 100, 'B', 'l'};
struct _enemy *enemB[] = {&enemy7, &enemy8, &enemy9, &enemy10, &enemy11, &enemy12};
//    C type enemies
struct _enemy enemy13 = {0, 0, 0, 0, 100, 'C', 'r'};struct _enemy enemy14 = {0, 0, 0, 0, 100, 'C', 'r'};
struct _enemy enemy15 = {0, 0, 0, 0, 100, 'C', 'r'};struct _enemy enemy16 = {0, 0, 0, 0, 100, 'C', 'r'};
struct _enemy enemy17 = {0, 0, 0, 0, 100, 'C', 'r'};struct _enemy enemy18 = {0, 0, 0, 0, 100, 'C', 'r'};
struct _enemy *enemC[] = {&enemy13, &enemy14, &enemy15, &enemy16, &enemy17, &enemy18};
void enemy_draw(){
  for(int i=0,z=0; i<18; i++){
    if(i<6){
      enemA[i]->width = abs(max_x/20);
      enemA[i]->height = abs(enemA[i]->width/2);
      enemA[i]->posX = abs(enemA[i]->width/2)+(i*abs(enemA[i]->width*2.0));
      enemA[i]->posY = abs(enemA[i]->height/2);

      draw_rect(enemA[i]->posX ,enemA[i]->posY, enemA[i]->width, enemA[i]->height, 8);
    }
    else if(i<12){
      z=i;
      i-=6;

      enemB[i]->width = abs(max_x/20);
      enemB[i]->height = abs(enemB[i]->width/2);
      enemB[i]->posX = abs(enemB[i]->width/2)+(i*abs(enemB[i]->width*2.0));
      enemB[i]->posY = abs(enemB[i]->height/2)+abs(enemB[i]->height*1.5);;
      draw_rect(enemB[i]->posX ,enemB[i]->posY, enemB[i]->width, enemB[i]->height, 8);

      i=z;
    }
    else{
      z=i;
      i-=12;

      enemC[i]->width = abs(max_x/20);
      enemC[i]->height = abs(enemC[i]->width/2);
      enemC[i]->posX = abs(enemC[i]->width/2)+(i*abs(enemC[i]->width*2.0));
      enemC[i]->posY = abs(enemC[i]->height/2)+abs(enemC[i]->height*3.0);;
      draw_rect(enemC[i]->posX ,enemC[i]->posY, enemC[i]->width, enemC[i]->height, 8);

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
struct enemy_fire_args{
  int _value,_pX, _pY;
};
void enemy_kill(int enemy){
    if(enemy<6){
      enemA[enemy]->health = 0;
      draw_rect(enemA[enemy]->posX ,enemA[enemy]->posY, enemA[enemy]->width, enemA[enemy]->height, 0);
      return;
    }
    else if(enemy<12){
      enemy-=6;

      enemB[enemy]->health = 0;
      draw_rect(enemB[enemy]->posX ,enemB[enemy]->posY, enemB[enemy]->width, enemB[enemy]->height, 0);
      return;
    }
    else{
      enemy-=12;

      enemC[enemy]->health = 0;
      draw_rect(enemC[enemy]->posX ,enemC[enemy]->posY, enemC[enemy]->width, enemC[enemy]->height, 0);
      return;
    }
  return;
}
bool enemy_inBounds(int x, char type){
  for(int i=0; i<6; i++){
    if(type == 'A'){
      if(x <= enemB[i]->posX + enemB[i]->width && x > enemB[i]->posX) return true;
      if(x <= enemC[i]->posX + enemC[i]->width && x > enemC[i]->posX) return true;
    }
    else if(type == 'B'){
      if(x <= enemC[i]->posX + enemC[i]->width && x > enemC[i]->posX) return true;
    }
  }
  return false;
}
int enemy_inBounds_get(int x, int y){
  for(int i=0, z=0; i<18 ;i++){
    if(i<6){
      if(x <= enemA[i]->posX + enemA[i]->width && x > enemA[i]->posX && y == enemA[i]->posY + enemA[i]->height - 1) return i;
    }
    else if(i<12){
      z=i;i-=6;
      if(x <= enemB[i]->posX + enemB[i]->width && x > enemB[i]->posX && y == enemB[i]->posY + enemB[i]->height - 1) return z;
      i=z;
    }
    else{
      z=i;i-=12;
      if(x <= enemC[i]->posX + enemC[i]->width && x > enemC[i]->posX && y == enemC[i]->posY + enemC[i]->height - 1) return z;
      i=z;
    }
  }
  return -1;
}
int enemy_fire_value, enemy_fire_pX, enemy_fire_pY;
void *enemy_fire( ){
  int cntr =0;
  while(enemy_fire_pY<max_y-1) {
    cntr++;
    if(enemy_fire_value != 0){
      if(pixel_matrix[enemy_fire_pX][enemy_fire_pY+1] == 2){
        int _p = obstacle_inBounds(enemy_fire_pX);
        if(_p != -1){
          obstacle_damage(enemy_fire_pX, enemy_fire_pY+1, _p, 50);
        }
        projectileCache_X[projectileCache_length] = enemy_fire_pX;
        projectileCache_Y[projectileCache_length++] = enemy_fire_pY;

        clean_projectiles();
        return;
      }
      else if(pixel_matrix[enemy_fire_pX][enemy_fire_pY+1] == 7){
        spacecraft_damage(50);

        projectileCache_X[projectileCache_length] = enemy_fire_pX;
        projectileCache_Y[projectileCache_length++] = enemy_fire_pY;

        clean_projectiles();
        return;
      }
      pixel_matrix[enemy_fire_pX][enemy_fire_pY]=0;
      pixel_matrix[enemy_fire_pX][++enemy_fire_pY]=enemy_fire_value;
    }
    usleep(60000);
  }
  projectileCache_X[projectileCache_length] = enemy_fire_pX;
  projectileCache_Y[projectileCache_length++] = enemy_fire_pY;
  clean_projectiles();
  return;
}
void *enemy_update() {
  bool stored_shot;
  while(true){
    stored_shot=false;
    enemy_clear();
    for(int i=0; i<6; i++){
        // A type enemies
        if(enemA[i]->health > 0){
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
          draw_rect(enemA[i]->posX ,enemA[i]->posY, enemA[i]->width, enemA[i]->height, 8);
          if(stored_shot == false && spacecraft_inBounds(enemA[i]->posX + abs(enemA[i]->width/2) + 1) == 1 && enemy_inBounds(enemA[i]->posX + abs(enemA[i]->width/2) + 1, enemA[i]->type) == false){
            enemy_fire_value = 2;
            enemy_fire_pX = enemA[i]->posX + abs(enemA[i]->width/2) + 1;
            enemy_fire_pY = enemA[i]->posY + enemA[i]->height + 1;
            stored_shot = true;
          }
        }

        // B type enemies

        if(enemB[i]->health > 0){
          if(enemB[i]->moveDir == 'r'){
            if((enemB[5]->posX + enemB[5]->width) == (max_x-3)){
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
          draw_rect(enemB[i]->posX ,enemB[i]->posY, enemB[i]->width, enemB[i]->height, 8);
          if(stored_shot == false && spacecraft_inBounds(enemB[i]->posX + abs(enemB[i]->width/2) + 1) == 1 && enemy_inBounds(enemB[i]->posX + abs(enemB[i]->width/2) + 1, enemB[i]->type) == false){
            enemy_fire_value = 2;
            enemy_fire_pX = enemB[i]->posX + abs(enemB[i]->width/2) + 1;
            enemy_fire_pY = enemB[i]->posY + enemB[i]->height + 1;
            stored_shot = true;
          }
        }


        // C type enemies

        if(enemC[i]->health > 0){
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
          draw_rect(enemC[i]->posX ,enemC[i]->posY, enemC[i]->width, enemC[i]->height, 8);
          if(stored_shot == false && spacecraft_inBounds(enemC[i]->posX + abs(enemC[i]->width/2) + 1) == 1){
            enemy_fire_value = 2;
            enemy_fire_pX = enemC[i]->posX + abs(enemC[i]->width/2) + 1;
            enemy_fire_pY = enemC[i]->posY + enemC[i]->height + 1;
            stored_shot = true;
          }
        }

    }
    if(stored_shot)
      enemy_fire();

    usleep(DELAY);
  }
  return;
}

//////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////

void loop(){
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
        if(spcr->hasfired == false)
          spcr->hasfired = true;
        pthread_create(&thread2, NULL, spacecraft_fire, 2);
      }
      // DOWN ARROW
      else if(ch == KEY_DOWN){
        clean_projectiles();
      }
      // LEFT ARROW
      else if(ch == KEY_LEFT){
        if(spcr->hasfired == true)
          spcr->hasfired = false;
        spacecraft_moveL();
      }
      // RIGHT ARROW
      else if(ch == KEY_RIGHT){
        if(spcr->hasfired == true)
          spcr->hasfired = false;
        spacecraft_moveR();
      }
      // SPACEBAR
      else if(ch == ' '){

      }
      else{

      }
    }
    else{
      if(spcr->hasfired == true)
        spcr->hasfired = false;
    }
    usleep(15000);
  }
  return;
}

int main(int argc, char *argv[])
{

  setlocale(LC_ALL,"");

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

  init_color(COLOR_CYAN, 0,255,255);
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);

  canvas = newwin(max_y,max_x,0,0);
  wbkgdset(canvas, COLOR_PAIR(1));
  // box(canvas, 0, 0);
  projectileCache_Y = malloc(max_y*sizeof(int*));
  projectileCache_X = malloc(max_x*sizeof(int*));


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

  draw_border();
  draw_spacecraft();
  obstacle_draw();
  enemy_draw();

  // game loop
  pthread_create(&thread1, NULL, enemy_update, NULL);
  loop();

  printw("\n\nFarewell!\n");
  endwin();
  pthread_exit(NULL);

  return 0;
}
