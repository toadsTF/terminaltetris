#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>

#define NRM   "\x1B[0m"

#define BRED  "\x1B[41m"
#define BGRN  "\x1B[42m"
#define BYEL  "\x1B[43m"
#define BBLU  "\x1B[44m"
#define BMAG  "\x1B[45m"
#define BCYN  "\x1B[46m"
#define BWHT  "\x1B[47m"

#define POS   "\x1B[%d;%dH"

#define H  26
#define W  80
#define MH 24
#define MW 12

#define BLACK   0
#define WHITE   1
#define RED     2
#define BLUE    3
#define YELLOW  4
#define MAGENTA 5
#define GREEN   6

#define clear() printf("\033[H\033[J\n")


short int plus[4][4] = {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}};
short int line[4][4] = {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}};
short int box [4][4] = {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}};
short int zigz[4][4] = {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}};
short int zigs[4][4] = {{0,0,0,0},{0,0,1,1},{0,1,1,0},{0,0,0,0}};
short int js  [4][4] = {{0,0,1,0},{0,0,1,0},{0,1,1,0},{0,0,0,0}};
short int ls  [4][4] = {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}};

short int shapes[7][4][4];// = {plus,line,box,zigz,zigs,js,ls};

short int display[80][26];
short int map[12][24];

int score = 0;
int next = 0;
typedef struct shape_t
{
    short int parts[4][4]; //1 if piece, 0 else
    short int y;
    short int x;
    short int color;
}shape;

bool isFull(short int *row)
{
    for(int i = 0; i < 12; i++)
    {
        if(row[i] == 0)return false;
    }
    return true;
}
bool isEmpty(short int *row)
{
    for(int i = 0; i < 12; i++)
    {
        if(row[i] != 0)return false;
    }
    return true;
}


int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void drawScreen(void)
{
    for(int x = 0; x < W; x++)
    {
        for(int y = 0; y < H; y++)
        {
            printf(POS,26-y,x+1); //set cursor location
            if(!(x > MW+1 && x < MW+9 && y > 18 && y < 25))
            switch(display[x][y])
            {
                case 0:  //black
                    printf(NRM " " NRM);
                    break;
                case 1:  //white
                    printf(BWHT " " NRM);
                    break;
                case 2:  //red
                    printf(BRED " " NRM);
                    break;
                case 3:  //blue
                    printf(BBLU " " NRM);
                    break;
                case 4:  //yellow
                    printf(BYEL " " NRM);
                    break;
                case 5:  //magenta
                    printf(BMAG " " NRM);
                    break;
                case 6:  //green
                    printf(BGRN " " NRM);
                    break;
                default:
                    break;
            }
        }
    } 
    printf(NRM);
    char space[10] = "          ";
    char strScore[8];
    sprintf(strScore,"%d",score);
    strncat(strScore,space,8-strlen(strScore));
    
    printf(POS,2,15);
    printf(" SCORE: ");
    printf(POS,3,15);
    printf("%s",strScore);
    printf(POS,4,15);
    printf("        ");
    printf(POS,5,15);
    printf("        ");
    printf(POS,6,15);
    printf("        ");
    printf(POS,7,15);
    printf("        ");
    
    for(int x = 0; x < 4; x++)
        for(int y = 0; y < 4; y++)
        {
            printf(POS,4+y,17+x);
            if(shapes[next][x][y])printf(BRED " "); 
        }
    
    printf(POS,0,0);
}

int main(int argc, char **argv)
{
    //check if terminal is properly sized
    struct winsize w;
    srandom(time(NULL));
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if(w.ws_row < 26 || w.ws_col < 24)
    {
        printf("\nMake your terminal at least 24 by 26!\n");
        sleep(1);
        exit(1);
    }
    //fill display buffer with nothing
    for(int i = 0; i < W; i++)
        for(int j = 0; j < H; j++)
            display[i][j] = 0;
    for(int i = 0; i < MW; i++)
        for(int j = 0; j < MH; j++)
            map[i][j] = 0;
    
    //clear screen
    for(int x = 0; x < W; x++)
    {
        for(int y = 0; y < H; y++)
        {
            printf(POS,26-y,x+1);
            printf(NRM " ");
        }
    }    
    
    //places shape into array
    memcpy(shapes,plus,sizeof(plus));
    memcpy(shapes+1,line,sizeof(plus));
    memcpy(shapes+2,box ,sizeof(plus));
    memcpy(shapes+3,zigz,sizeof(plus));
    memcpy(shapes+4,zigs,sizeof(plus));
    memcpy(shapes+5,js  ,sizeof(plus));
    memcpy(shapes+6,ls  ,sizeof(plus));
    
    
    //put unmoving frame into draw buffer
    for(int i = 0; i < MW+11; i++)display[i][0]    = 1;
    for(int i = 0; i < MW+11; i++)display[i][MH+1] = 1; 
    for(int i = 0; i < MH+2; i++)display[0][i]    = 1; 
    for(int i = 0; i < MH+2; i++)display[MW+1][i] = 1; 
    for(int i = 0; i < MH+2; i++)display[MW+10][i] = 1;
    for(int i = MW+1; i < MW+10;i++)display[i][18] = 1; 
    
    //define first object for testing
    shape s;
    s.x     = 4;
    s.y     = 23;
    s.color = 2+(rand() % 7);
    
    next = (int)(rand() % 7);
    usleep(500000);
    int index =  (int)(rand() % 7);
    printf(POS, 26,0);
    printf(NRM"%d", index);
    memcpy(s.parts,shapes[index],sizeof(short int)*16);
    
    int timer = 0;
    
    //main loop
    for(;;)
    {
        usleep(100000);
        
        //move down code (will be in if statement)
        if(timer % 10 == 0)
        {
        bool canMoveDown = true;
        for(int x = 0; x < 4; x++)
            for(int y = 0; y < 4; y++)
            {
                if(((s.y-(y+1)) == -1 && s.parts[x][y]) || (map[s.x+x][s.y-1-y] != 0 && s.parts[x][y]))
                {
                    canMoveDown = false;
                }
            }
        if(!canMoveDown)
        {
            //place in map
            for(int x = 0; x < 4; x++)
                for(int y = 0; y < 4; y++)
                {
                    if(s.parts[x][y]!=0)map[s.x+x][s.y-y] = s.color;
                }
            //reset shape
            s.y = 23;
            s.x = 4;
            s.color = (int)(2 + rand() % 5);
            memcpy(s.parts,shapes[next],sizeof(plus));
            next = (int)(rand() % 7);
            score+=10;
        }else s.y--;
        }//end down check
        
        //check for movement
        if(kbhit())
        {
            char c = getchar();
            bool canMove = true;
            if(c == 'a')
            {
                for(int x = 0; x < 4; x++)
                    for(int y = 0; y < 4; y++)
                    {
                        if((s.x+x-1 == -1 && s.parts[x][y]) || (s.parts[x][y] && map[s.x+x-1][s.y-y] != 0))canMove = false;
                    }
                if (canMove)s.x--;
            }
            if(c == 'd')
            {
                for(int x = 0; x < 4; x++)
                    for(int y = 0; y < 4; y++)
                    {
                        if((s.x+x+1 == 12 && s.parts[x][y]) || (s.parts[x][y] && map[s.x+x+1][s.y-y] != 0))canMove = false;
                    }
                if(canMove)s.x++;
            }
            if(c == 's')
            {
                for(int x = 0; x < 4; x++)
                    for(int y = 0; y < 4; y++)
                    {
                        if(((s.y-(y+1)) == -1 && s.parts[x][y]) || (map[s.x+x][s.y-1-y] != 0 && s.parts[x][y]))canMove = false;
                    }
                if(canMove)s.y--;
            }
            if(c == 'r')
            {
                short int temp[4][4];
                for(int i = 0; i < 4; i++)
                {
                    short int flip[4] = {s.parts[3][i],s.parts[2][i],s.parts[1][i],s.parts[0][i]};
                    memcpy(temp+i,flip,sizeof(flip));
                }
                for(int x = 0; x < 4; x++)
                    for(int y = 0; y < 4; y++)
                        if((temp[x][y] && map[s.x+x][s.y-y]) || (temp[x][y] && (s.x+x > 11)))canMove = false;
                if(canMove)memcpy(s.parts,temp,sizeof(temp));
            }
        }
        //place map in draw buffer
        for(int x = 0; x < MW; x++)
            for(int y = 0; y < MH; y++)
            {
                display[x+1][y+1] = map[x][y];
            }
        
        //draw shape (separate from map)
        for(int x = 0; x < 4; x++)
            for(int y = 0; y < 4; y++)
            {
                if(s.parts[x][y]!=0)display[s.x+1+x][s.y+1-y] = s.color;
            }
        //check for complete rows
        int rowct = 0;
        for(int y = MH-1; y >= 0; y--)
        {
            short int row[12];
            for(int i = 0; i<12; i++)row[i] = map[i][y];//,sizeof(short int));
            if(isFull(row))
            {
                rowct++;
                for(int i = y; i < MH-1; i++)
                {
                    for(int x = 0; x < MW; x++)
                    {
                        map[x][i] = map[x][i+1];
                    }
                }
            }
            if(rowct!=0)score+=500*pow(2,rowct+1);
        }
        
        //check to see if loss
        short int row[12];
        for(int i = 0; i<12; i++)row[i] = map[i][MH-2];
        if(!isEmpty(row))
        {
            printf(NRM);
            clear();
            printf(POS,13,4);
            printf(NRM "YOU LOSE!");
            printf(POS,14,1);
            printf("SCORE: %d",score);
            sleep(3);
            clear();
            printf(POS,26,1);
            exit(1);
        }
        
        timer+=1;
        drawScreen(); //draw screen with everything in place
    }        
    return 0;
}
