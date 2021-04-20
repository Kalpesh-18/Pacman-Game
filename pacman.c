#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include "SOIL.h"
#include "pacman.h"
//Passing pointers everywhere hence using -> operator other option *().
// Size of each block in the game matrix
// Size of graphics images(png files) is 80 X 80 pixels
// of which 70 X 70 pixels is translated onto the screen
#define block 70
// Size of the game matrix
// A Game Arena of only 20 X 20 will be considered in map.txt file
#define N 20
// Size of each block of the game matrix on the screen
#define SIZE 0.1f    //f is used for single precision floating point
// Functions that convert the row and column of the matrix to a coordinate of [-1,1]
// So that map translation from text file to OpenGL graphics is done using this
#define MAT2X(j) ((j)*0.1f-1)       //Column
#define MAT2Y(i) (0.9-(i)*0.1f)     //Row

// Structures used to control the game

// Structure that is used to store point location
struct TPoint{
    int x,y;
};

// Structure used to store direction unit vectors
// RightUpLeftBottom
const struct TPoint directions[4] = {{1,0},{0,1},{-1,0},{0,-1}};

// Pacman Structure
struct TPacman{
    int status;                       //Pacman status open mouth or closed
    int xi,yi,x,y;                    //Pacman's Co-ordinate and velocities
    int direction,step,partial;       //Current direction, step, partial (distance covered in partial animation)
    int spots;                        //When Pacman eats spots(food field)(Score)
    int invincible;                   //When Pacman eats SANITIZER
    int alive;                        //Pacman is alive or affected by virus
    int animation;                    //Pacman animation
};


//COVID Structure
struct TVirus{
    int status;                       //Virus status
    int xi,yi,x,y;                    //Virus Co-ordinate and velocities
    int direction,step,partial;       //Current direction, step, partial (distance covered in partial animation)
	int decided_cross,started_back;   //When pacman eats virus it goes back to origin position
	int current_index;                //Current index
	int *path;                        //For chasing pacman
};


//Vertex Structure
struct TVertice{
    int x,y;                          //Vertex location
    int neighbors[4];                 //What each vertex contains in the surrounding
};

//Scene Structure
struct TScene{
    int map[N][N];                    //20 X 20 Map Array taken from map.txt
    int no_tablets;                   //Total number of Food fields
    int NV;                           //Number of intersection points
    struct TVertice *graph;           //Stores the locations of intersections/ Crossovers
};


// Textures
//For Reference :- https://open.gl/textures

GLuint pacmanTex2d[12];    //12 PACMAN Textures in Graphics folder
GLuint virusTex2d[12];     //12 VIRUS Textures in Graphics folder
GLuint mapTex2d[14];       //12 MAP Textures in Graphics folder

//Upload Starting and Ending Screen of the Game
GLuint tellStart, tellGameOver;


static void designGraphic(float column,float line, GLuint tex);
static GLuint uploadArchTexture(char *str);
static void DesignTypescreen(float x, float y, float size, GLuint tex);

/* Function that loads all textures in the game
   Used sprintf() as the string is stored in buffer
   Hence Memory isn't used extensively also it gets
   automatically reinitialized to NULL */


   //replace with 3
   //position [14,11] , [14,15]
/*
void Stay_home(Scene *sen)
{
    if(up == 0)
    {
        designGraphic(MAT2X(14),MAT2Y(11),mapTex2d[3]);
        designGraphic(MAT2X(14),MAT2Y(15),mapTex2d[3]);
        //&sen->map[14,11] = 3;
        //&sen->map[14,15] = 3;
        up = 1;
    }
    else
    {
        designGraphic(MAT2X(14),MAT2Y(11),mapTex2d[0]);
        designGraphic(MAT2X(14),MAT2Y(15),mapTex2d[0]);
        //&sen->map[14,11] = 1;
        //&sen->map[14,15] = 1;
        up = 0;
    }
    return;
}
*/

void Textures(){
    int i;
    char str[50];
    for(i=0; i<12; i++){
        // "./" is used for current working dir
        sprintf(str,".//graphics//virus%d.png",i);                     //Loading Virus Textures
        virusTex2d[i] = uploadArchTexture(str);                        //
    }                                                                  //
                                                                       //
    for(i=0; i<12; i++){                                               //
        sprintf(str,".//graphics//pacman%d.png",i);                    //Loading Pacman Textures
        pacmanTex2d[i] = uploadArchTexture(str);                       //
    }                                                                  //
                                                                       //
    for(i=0; i<14; i++){                                               //
        sprintf(str,".//graphics//map%d.png",i);                       //Loading Map Textures
        mapTex2d[i] = uploadArchTexture(str);                          //
    }                                                                  //
                                                                       //
    tellStart = uploadArchTexture(".//graphics//start.png");           //Loading StartPage Textures
    tellGameOver = uploadArchTexture(".//graphics//gameover.png");;    //Loading EndPage Textures

}

// Function that loads a texture file from Graphics folder
// Takes files location as string and loads the image
static GLuint uploadArchTexture(char *str)
{
    // http://www.lonesock.net/soil.html
    GLuint tex = SOIL_load_OGL_texture(
            str,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );

    /* check for an error during the load process */
    if(0 == tex){
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }

    return tex;
}

// Function that receives a row and column from the matrix and a code
// texture and draw a square on the screen with that texture
void designGraphic(float column,float line, GLuint tex){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);                                                  // Placing each 70 X 70 pixels photo
        glTexCoord2f(0.0f,0.0f); glVertex2f(column, line);              // onto 1.1 f X 1.1 f cube
        glTexCoord2f(1.0f,0.0f); glVertex2f(column+SIZE, line);         // from 0.0 f left corner
        glTexCoord2f(1.0f,1.0f); glVertex2f(column+SIZE, line+SIZE);    // to 1.1 f right corner
        glTexCoord2f(0.0f,1.0f); glVertex2f(column, line+SIZE);         // 0.1 is used to maintain continuity
    glEnd();                                                            // So that when object travel from one
                                                                        // block to another block it doesn't
    glDisable(GL_BLEND);                                                // seem discrete
    glDisable(GL_TEXTURE_2D);

}

void DesignTypescreen(float x, float y, float size, GLuint tex){

    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,1.0f); glVertex2f(x - size, y + size);  //For loading start and end screen
        glTexCoord2f(1.0f,1.0f); glVertex2f(x + size, y + size);
        glTexCoord2f(1.0f,0.0f); glVertex2f(x + size, y - size);
        glTexCoord2f(0.0f,0.0f); glVertex2f(x - size, y - size);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}


//Function used to tell which design to print
void design_tell(int type){
    if(type == 0)
        DesignTypescreen(0, 0, 1.0, tellStart);
    else
        DesignTypescreen(0, 0, 1.0, tellGameOver);
}


//Scene


static int scene_Crossover(int x, int y, Scene* sen);
static int scene_CheckDirection(int mat[N][N], int y, int x, int direction);
static void scene_BuildGraph(Scene* sen);


// Function that loads the scenario data from a text file
Scene* scene_read(char *file)/*, char *file1, int up*/
{              //
    int i,j;

    FILE *f = fopen(file,"r");
    //FILE *q = fopen(file1,"r");                            //File Handling

    if(f == NULL){                                        //If file is not loaded properly
        printf("Error loading scenery!!\n");
        system("pause");
        exit(1);
    }

    Scene* sen = (Scene*)malloc(sizeof(Scene));
    sen->no_tablets = 0;                                    //number of tablets in the scene (food field)

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
        {
            //if(up == 1)
            //{
                //fscanf(q,"%d",&sen->map[i][j]);
            //}
            fscanf(f,"%d",&sen->map[i][j]);                 //Scanning the map row & column wise from map.txt
            if(sen->map[i][j] == 1 || sen->map[i][j] == 2)  //1 == Filled tablet        2 == Sanitizer
                sen->no_tablets++;                          //Total number of tablet in the map design
        }
    //fclose(q);
    fclose(f);

    scene_BuildGraph(sen);

    return sen;
}

// Releases the data associated with the scenario
void scene_destroy(Scene* sen){
    free(sen->graph);
    free(sen);
}

// Go through the game matrix drawing the graphics
void scene_design(Scene* sen){
    int i,j;
    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            designGraphic(MAT2X(j),MAT2Y(i),mapTex2d[sen->map[i][j]]);
            //Passing the points where the respective texture should be applied
}

// Checks whether a position (x, y) of the scene is an intersection and not a wall
// In all the possible directions where Pacman and Virus can move
static int scene_Crossover(int x, int y, Scene* sen)
{
    int i, cont = 0;
    int v[4];

    for(i=0; i<4; i++)
    {               //Current position + direction (Co-ordinate wise) All direction
        if(sen->map[y + directions[i].y][x + directions[i].x] <=2)
        {                         //It's not a wall ...
            cont++;               //Less than or equal to 2 means food field (0,1,2)
            v[i] = 1;             //Marker for food field
        }
        else
        {                         //It is a wall
            v[i] = 0;             //Marker for wall
        }
    }
    //As virus moves on their own hence they need to be given change of directions is certain
    //scenario
    if(cont > 1)
    {
        if(cont == 2)
        {
            if((v[0] == v[2] && v[0]) || (v[1] == v[3] && v[1]))
                return 0;// Keep traveling is same direction no need to change the direction
            else
                return 1;
        }
        else
            return 1;
    }
    else
        return 0;
}

// Function that checks whether it is possible to walk in a certain direction
static int scene_CheckDirection(int mat[N][N], int y, int x, int direction)
{   //spanning from 0 to 19
    //so that all the fields are covered
    //Current Co-ordinates
    int xt = x;
    int yt = y;
    //while we don't encounter a wall
    //This part is only for calculating the path weight
    while(mat[yt + directions[direction].y][xt + directions[direction].x] == 0)
    {// it's not a wall ...
        yt = yt + directions[direction].y;
        xt = xt + directions[direction].x;
    }
    //if you go out of designed space //Wall
    if(mat[yt + directions[direction].y][xt + directions[direction].x] < 0)
        return -1;
        //Stop execution
    else
        //One step back of encountering wall
        return mat[yt + directions[direction].y][xt + directions[direction].x] - 1;
}

// Given a scenario, assemble the graph that will help virus
// to choose different path in crossover positionss
static void scene_BuildGraph(Scene* sen){
    int mat[N][N];  //Map Matrix
    int i,j,k, idx, cont = 0;

    for(i=1; i<N-1; i++)
    {//All rows of map matrix
        for(j=1; j<N-1; j++)
        {//All columns of map matrix
            if(sen->map[i][j] <= 2)
            {// it's not a wall ...
                if(scene_Crossover(j,i,sen))
                {                           //Crossover
                    cont++;                 //1, 2, 3.......all the crossovers
                    mat[i][j] = cont;
                }
                else
                    mat[i][j] = 0;
            }
            else
                mat[i][j] = -1;
        }
    }
    //Putting limit walls the Arena Boundary
    //So even it user missed it in map.txt
    //A boundary will be automatically built
    for(i=0; i < N; i++){
        mat[0][i] = -1;      //TOP BOUNDARY
        mat[i][0] = -1;      //LEFT BOUNDARY
        mat[N-1][i] = -1;    //BOTTOM BOUNDARY
        mat[i][N-1] = -1;    //RIGHT BOUNDARY
    }

    //mat[i][j] is matrix which contains
    //-1           == WALL
    //0            == FOOD FIELD
    //0-1-2-3-4-NV == CROSSOVERS
    sen->NV = cont;          //Gives limiting value to NV(Next Vertex)
    sen->graph = malloc(cont * sizeof(struct TVertice));
    //Building Another graph For Virus movements
    for(i=1; i<N-1; i++){
        for(j=1; j<N-1; j++){
            if(mat[i][j] > 0) //If its not a wall...
            {
                idx = mat[i][j] - 1;       //Only crossovers
                sen->graph[idx].x = j;     //Stores x co-ordinate of Crossover
                sen->graph[idx].y = i;     //Stores y co-ordinate of Crossover
                for(k=0; k < 4; k++)
                    sen->graph[idx].neighbors[k] = scene_CheckDirection(mat,i,j,k);
                    //Get all the neighbors of current point
            }
        }
    }
}

// Pacman

static int pacman_is_invincible(Pacman *pac);
static void pacman_dies(Pacman *pac);
static void pacman_spots_virus(Pacman *pac);
static void pacman_Animation(float column,float line,Pacman* pac);

// Function that initializes data associated with pacman
Pacman* pacman_create(int x, int y){
    Pacman* pac = (Pacman*)malloc(sizeof(Pacman));
    if(pac != NULL)
    {
        pac->invincible = 0;             //Not invincible
        pac->spots = 0;                  //No spot eaten
        pac->step = 4;                   //Speed......Pacman's speed > Virus(1 sec = travel the length of 4 dots)
        pac->alive = 1;                  //Alive
        pac->status = 0;                 //open mouth
        pac->direction = 0;              //No Direction
        pac->partial = 0;                //partial (distance covered in partial animation)
        pac->xi = x;                     //x Velocity
        pac->yi = y;                     //y Velocity
        pac->x = x;                      //x Co-ordinate
        pac->y = y;                      //y Co-ordinate
    }
    return pac;
}

// Function that releases data associated with pacman
void pacman_destroy(Pacman *pac)
{
    free(pac);
}
// Function that checks whether the pacman is alive or not
int pacman_alive(Pacman *pac){
    if(pac->alive)                  //If Pacman is alive
        return 1;
    else                            //So that end screen does'nt pop up immediately after pacman is dead
    {
        if(pac->animation > 60)     //If Pacman animation exceed the value of 60 hence is it dead
            return 0;
        else
            return 1;
    }
}
//In main.c
//Right = 0
//Down = 1
//Left = 2
//Up = 3
// Function that checks whether pacman can go in a new chosen direction
void pacman_ChangeDirection(Pacman *pac, int direction, Scene *sen)
{
    if(sen->map[pac->y + directions[direction].y][pac->x + directions[direction].x] <=2)
    {//Not a wall
        int di = abs(direction - pac->direction);
        //Eg - If Opposite and same directions
        if(di != 2 && di != 0)
            pac->partial = 0;
        pac->direction = direction;
    }
}

// Update the pacman's position
void pacman_movement(Pacman *pac, Scene *sen){
    if(pac->alive == 0)
        return;

    // Increase its position within a cell in the matrix or change cells
    if(sen->map[pac->y + directions[pac->direction].y][pac->x + directions[pac->direction].x] <=2)
    {//Not a wall                                           //In matrix
        if(pac->direction < 2)                              //Right and Down
        {
            pac->partial += pac->step;                      //Partial = partial + step(4)
            if(pac->partial >= block)
            {
                pac->x += directions[pac->direction].x;     //Re-evaluating x - co
                pac->y += directions[pac->direction].y;     //Re-evaluating y - co
                pac->partial = 0;
            }
        }
        else                                                //Left and Up
        {
            pac->partial -= pac->step;                      //Partial = partial - step(4)
            if(pac->partial <= -block)
            {
                pac->x += directions[pac->direction].x;     //Re-evaluating x - co
                pac->y += directions[pac->direction].y;     //Re-evaluating x - co
                pac->partial = 0;
            }
        }
    }

    // Eat one of the tablets on the map
    if(sen->map[pac->y][pac->x] == 1)
    {//Eats normal tablet
        pac->spots += 10;
        sen->no_tablets--;
    }
    if(sen->map[pac->y][pac->x] == 2)
    {//Eats Sanitizer
        pac->spots += 50;
        pac->invincible = 1000;       //Invincible animation
        sen->no_tablets--;
    }
    // Remove the food field from the map
    sen->map[pac->y][pac->x] = 0;
}

// Function that draws the pacman
//In main.c
//Right = 0
//Down = 1
//Left = 2
//Up = 3
void pacman_design(Pacman *pac)
{
    float line, column;
    float step = (pac->partial/(float)block);
    // Check the position
    if(pac->direction == 0 || pac->direction == 2)      //Change in X-axis
    {
        line = pac->y;
        column = pac->x + step;
    }
    else                                                //Change in Y-axis
    {
        line = pac->y + step;
        column = pac->x;
    }

    if(pac->alive)
    {
        // Choose the graphic based on the direction
        int idx = 2*pac->direction;

        // Choose whether to draw with open or closed mouth
        if(pac->status < 15)
            designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[idx]);//open
        else
            designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[idx+1]);//closed

        // Switch between open or closed mouth
        pac->status = (pac->status+1) % 30;
        // In one second the mouth is opened and closed once
        // Hence 0.5 second
        // Pacman is invincible for 1000/60 secs
        if(pac->invincible > 0)
            pac->invincible--;
    }
    else
    {
        // Shows animation of death
        pacman_Animation(column,line,pac);
    }
}

static int pacman_is_invincible(Pacman *pac)
{
    return pac->invincible > 0;
}
//Pacman dies
static void pacman_dies(Pacman *pac)
{
    if(pac->alive)
    {
        pac->alive = 0;
        pac->animation = 0;
    }
}

//Pacman eats virus
static void pacman_spots_virus(Pacman *pac)
{
    pac->spots += 100;
}

static void pacman_Animation(float column,float line,Pacman* pac)
{
    pac->animation++;
    // Check which graphics should be designed to give the effect that the pacman is disappearing when he dies
    if(pac->animation < 15)                                                 //After instant pacman dies
        designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[8]);
    else
        if(pac->animation < 30)                                             //After (1/15)seconds
            designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[9]);
        else
            if(pac->animation < 45)                                         //After (1/30)seconds == 0.5 sec
                designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[10]);
            else                                                            //After (1/45)seconds
                designGraphic(MAT2X(column),MAT2Y(line), pacmanTex2d[11]);
}

// Virus

static void virus_move(Virus *vi, int direction, Scene *sen);
static int virus_DirectionGraph(Virus *vi, Scene *sen);
static int virus_DistanceGraph(Scene *sen, int noA, int noB);
static int virus_SortbyLength(Virus *vi, Scene *sen);
static int virus_Pacman(Virus *vi, Pacman *pac, Scene *sen, int direcao);
static void virus_SearchLittlePath(Virus *vi, Scene *sen);
static int virus_lookingfor(Virus *vi, Pacman *pac, Scene *sen);
static int virus_VirusDead(Virus *vi, Scene *sen);

// Function that initializes the data associated with a virus
Virus* virus_create(int x, int y){
    Virus* vi = (Virus*)malloc(sizeof(Virus));
    if(vi != NULL){
        vi->step = 3;                   //Speed of virus
        vi->decided_cross = 0;          // !Crossing: saw the pacman, head in that direction
        vi->started_back = 0;           //Started going back to home
        vi->current_index = 0;          //Current index of virus
        vi->status = 0;                 //Status for choosing graphics
        vi->direction = 0;              //Direction of heading
        vi->partial = 0;                //partial (distance covered in partial animation)
        vi->xi = x;                     //X-velocity
        vi->yi = y;                     //Y-velocity
        vi->x = x;                      //X-cord
        vi->y = y;                      //Y-cord
        vi->path = NULL;                //Initially no  path to follow back
    }
    return vi;
}

// Function that releases the data associated with a virus
void virus_destroy(Virus *vi){
    if(vi->path != NULL)
        free(vi->path);
    free(vi);
}

// Function that draws a virus
void virus_design(Virus *vi){
    float line, column;
    float step = (vi->partial/(float)block);

    // Check the position
    if(vi->direction == 0 || vi->direction == 2)
    {                   //Change in X-axis
        line = vi->y;
        column = vi->x + step;
    }
    else
    {
        line = vi->y + step;                                        //Change in X-axis
        column = vi->x;
    }
    // Choose the graphic based on direction and status (alive, dead, vulnerable)
    int idx = 3*vi->direction + vi->status;
    designGraphic(MAT2X(column),MAT2Y(line), virusTex2d[idx]);
}

// Update the position of a virus

void virus_movement(Virus *vi, Scene *sen, Pacman *pac){
    int d;
    if(vi->status == 1)
    {
        // If the virus is dead, it returns by the graph path
        d = virus_VirusDead(vi, sen);
    }
    else
    {// status is 0 or 2

        // Run away or chase the pacman

        if(pacman_is_invincible(pac))
            vi->status = 2; //Runaway From Pacman
        else
            vi->status = 0; //Chase Pacman

        // Calls the function that calculates the new direction where the virus
        // must move based on the map and the pacman's position
        d = virus_lookingfor(vi, pac, sen);
        // What to do if you touch the pacman?
        if(pac->x == vi->x && pac->y == vi->y){
            if(pacman_is_invincible(pac))
            {
                vi->status = 1;         //virus dies
                pacman_spots_virus(pac);
                vi->started_back = 0;
            }
            else                        //pacman dies
            {
                if(pacman_alive(pac))
                    pacman_dies(pac);
            }
        }
    }
    // Move and draw the virus on the screen
    virus_move(vi,d,sen);
}

//Updates the position of a virus
static void virus_move(Virus *vi, int direction, Scene *sen){
    int xt = vi->x;
    int yt = vi->y;

    // Increase its position within a cell in the matrix or change cells
    if(sen->map[vi->y + directions[direction].y][vi->x + directions[direction].x] <=2)
    {//Not a wall
        if(direction == vi->direction)                          //If direction assigned is equal to current direction
        {
            if(vi->direction < 2)                               //Right and Down
            {
                vi->partial += vi->step;                        //Partial = partial + step(3)
                if(vi->partial >= block)
                {
                    vi->x += directions[direction].x;           //Re-evaluating x - co
                    vi->y += directions[direction].y;           //Re-evaluating y - co
                    vi->partial = 0;
                }
            }
            else                                                //Left and Up
            {
                vi->partial -= vi->step;                        //Partial = partial - step(3)
                if(vi->partial <= -block)
                {
                    vi->x += directions[direction].x;           //Re-evaluating x - co
                    vi->y += directions[direction].y;           //Re-evaluating y - co
                    vi->partial = 0;
                }
            }
        }
        else
        {// change of direction ...
            if(abs(direction - vi->direction) != 2)
                vi->partial = 0;

            vi->direction = direction;
        }
    }
    //Condition to make sure that virus doesn't get stuck on the edges
    if(xt != vi->x || yt != vi->y)
        vi->decided_cross = 0;
}

// Function that assists in choosing the path when the virus dies
//In main.c
//Right = 0
//Down = 1
//Left = 2
//Up = 3
static int virus_DirectionGraph(Virus *vi, Scene *sen){
    if(sen->graph[vi->current_index].x == sen->graph[vi->path[vi->current_index]].x)
    {
        if(sen->graph[vi->current_index].y > sen->graph[vi->path[vi->current_index]].y)
            return 3;   //You have to go upside to reach home
        else
            return 1;   //You have to go downside to reach home
    }
    else
    {
        if(sen->graph[vi->current_index].x > sen->graph[vi->path[vi->current_index]].x)
            return 2;   //You have to go left to reach home
        else
            return 0;   //You have to go right to reach home
    }

}

// Function that assists in choosing the path when the virus dies
// Assigning weight to edges
static int virus_DistanceGraph(Scene *sen, int noA, int noB)
{
    return fabs(sen->graph[noA].x - sen->graph[noB].x) + fabs(sen->graph[noA].y - sen->graph[noB].y);
}

// When the virus finds an intersection, it draws a new direction
// The trend and keep moving forward, sometimes changing direction and
// almost never go back the same way
static int virus_SortbyLength(Virus *vi, Scene *sen){
    int i,j,k,bigger;
    int Weight[4], dir[4];

    for(i=0; i<4; i++)
        Weight[i] = rand() % 10 + 1; //Random number less than equal to 10 + 1
    //Current direction marking
    Weight[vi->direction] = 7;
    //Current direction is marked as 7
    //If direction is 0(right) value would be given to 2(left)
    //Opposite direction marked as 3
    Weight[(vi->direction + 2) % 4] = 3;

    // Sort the weights for each direction
    for(j=0; j<4; j++)
    {
        bigger = 0;
        for(i=0; i<4; i++)
        {
            if(Weight[i] > bigger)
            {
                bigger = Weight[i];
                k = i;                  //Storing direction of greatest weight
            }
        }
        dir[j] = k;                     //Giving direction of greatest weight
        Weight[k] = 0;                  //Marking greatest direction
    }

    // Choose the first valid direction
    i = 0;
    //For T intersection
    while(sen->map[vi->y + directions[dir[i]].y][vi->x + directions[dir[i]].x] > 2)
    {
        //while not a wall as next object
        i++;
    }
    return dir[i];
}

// Function that groups all cases of choosing the direction of the virus
static int virus_lookingfor(Virus *vi, Pacman *pac, Scene *sen){
    int d, i;
    if(scene_Crossover(vi->x, vi->y, sen))
    {
        if(!vi->decided_cross)
        {
            // Crossing: saw the pacman, and heading in that direction
            d = -1;
            for(i=0; i<4; i++)
                if(virus_Pacman(vi, pac, sen, i))
                    d = i;

            // Crossing: DID NOT see the pacman, draw a direction
            if(d == -1)
                d = virus_SortbyLength(vi, sen);
            else
            {
                // Crossing: saw the pacman, but he is invincible
                // TO RUN AWAY!! Draw a new direction
                if(pacman_is_invincible(pac))
                {
                    i = d;
                    while(i == d)
                        d = virus_SortbyLength(vi, sen);
                }
            }
            vi->decided_cross = 1;
        }
        else
            d = vi->direction;
    }
    else
    {
        // It is NOT an intersection: it continues on the same path
        vi->decided_cross = 0;
        d = vi->direction;
        if(pacman_is_invincible(pac))
        {
            // If you saw the pacman and he is invincible: he goes in the opposite direction
            if(virus_Pacman(vi, pac, sen, d))
                d = (d + 2) % 4;
        }
        // reverse direction ...
        if(sen->map[vi->y + directions[d].y][vi->x + directions[d].x] > 2)
            d = (d + 2) % 4;
    }
    return d;
}


// Function that handles cases where the virus sees the pacman
static int virus_Pacman(Virus *vi, Pacman *pac, Scene *sen, int direction)
{
    int continued = 0;
    if(direction == 0 || direction == 2)
    {
        if(pac->y == vi->y)
            continued = 1;
    }
    else
    {
        if(pac->x == vi->x)
            continued = 1;
    }

    if(continued)
    {
        int xt = vi->x;
        int yt = vi->y;
        while(sen->map[yt + directions[direction].y][xt + directions[direction].x] <= 2)
        {
            yt = yt + directions[direction].y;
            xt = xt + directions[direction].x;

            if(xt == pac->x && yt == pac->y)
                return 1;
        }
    }
    return 0;
}
// Function that assists in choosing the path when the virus dies
static void virus_SearchLittlePath(Virus *vi, Scene *sen)
{
    int i, k, indice_no;
    int continued, d;
    int *dist;

    dist = (int*)malloc(sen->NV*sizeof(int));
    if(vi->path == NULL)
        vi->path = (int*)malloc(sen->NV*sizeof(int));

    // start calculating shortest path ...
    for(i=0; i<sen->NV; i++)
    {
        dist[i] = 10000;                                            //All the dist index are given 10000 value
        vi->path[i] = -1;                                           //All the path index are given value -1
        if(sen->graph[i].x == vi->xi && sen->graph[i].y == vi->yi)  //If Virus is at home position
            indice_no = i;                                          //Store the index number of that intersection point

        if(sen->graph[i].x == vi->x && sen->graph[i].y == vi->y)    //If Virus is at one of the intersection points
            vi->current_index = i;                                  //Store the index number of that point
    }                                                               //So that virus would always make sure that it head in that direction

    dist[indice_no] = 0;                                            //Distance of home index is 0
    // calculate shortest path ...
    continued = 1;
    while(continued)
    {
        continued = 0;
        for(i=0; i<sen->NV; i++)
        {
            for(k=0; k<4; k++)
            {
                if(sen->graph[i].neighbors[k] >= 0)                 //If neighboring position are free
                {   //Distance(graph[i] - the next intersection point in k direction)
                    d = virus_DistanceGraph(sen, i, sen->graph[i].neighbors[k]);
                    //Choosing the minimum distance of all the possible directions
                    if(dist[sen->graph[i].neighbors[k]] > (dist[i] + d))
                    {   //Making it as smallest distance for other comparisions
                        dist[sen->graph[i].neighbors[k]] = (dist[i] + d);
                        //Storing the index of the point in graph[i] which is at the shortest distance
                        vi->path[sen->graph[i].neighbors[k]] = i;
                        continued = 1;
                    }
                }
            }
        }
    }
    //distance is calculated at every intersection point
    free(dist);
}

// Function that assists in choosing the path when the virus dies
static int virus_VirusDead(Virus *vi, Scene *sen){
    int d;
    //If Virus dies and is not at intersection point
    //It will only execute once
    if(!vi->started_back)
    {   //While moving forward if it lands on intersection point
        if(scene_Crossover(vi->x, vi->y, sen))
        {
            vi->started_back = 1;               //Raise the flag that now it will follow LittlePath
            virus_SearchLittlePath(vi, sen);    //Get the shortest Intersection to intersection path
            vi->decided_cross = 1;              //Raise flag that it is at crossing position
            d = virus_DirectionGraph(vi, sen);  //Get the direction to reach the shortest path
        }
        else//If not on intersection point
        {
            d = vi->direction;  //Keep moving in the same direction
            if(sen->map[vi->y + directions[d].y][vi->x + directions[d].x] > 2)//A wall
                d = (d + 2) % 4;                                              //Reverse the direction
        }
    }
    else//Already following the LittlePath
    {// make your way back ...
        if(vi->x != vi->xi || vi->y != vi->yi)//Not reached home yet
        {
            if(scene_Crossover(vi->x, vi->y, sen))//At intersection point
            {
                if(vi->decided_cross)
                    d = vi->direction;
                else
                {// check which direction to take ...
                    //Makes sure the Virus is always traveling to home
                    vi->current_index = vi->path[vi->current_index];
                    d = virus_DirectionGraph(vi, sen);
                    vi->decided_cross = 1;                              //Raise flag that it is at crossing position
                }
            }
            else// Not at intersection point
            {
                d = vi->direction;      //Get to an intersection point
                vi->decided_cross = 0;  //Raise flag that it is not at crossing position
            }
        }
        else//Reached Home
        {
            vi->status = 0;
            d = vi->direction;
        }
    }
    return d;
}
