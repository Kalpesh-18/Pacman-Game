#ifndef PACMAN_H_INCLUDED
#define PACMAN_H_INCLUDED
//=========================================================
typedef struct TPacman Pacman;
typedef struct TVirus Virus;
typedef struct TScene Scene;

//=========================================================
void Textures();

//=========================================================
Scene* scene_read(char *file);/*,char *file2,int up*/
void scene_destroy(Scene* sen);
void scene_design(Scene* sen);
void design_tell(int type);

//=========================================================
Pacman* pacman_create(int x, int y);
void pacman_destroy(Pacman *pac);
int pacman_alive(Pacman *pac);
void pacman_design(Pacman *pac);
void pacman_ChangeDirection(Pacman *pac, int direction, Scene *sen);
void pacman_movement(Pacman *pac, Scene *sen);
//void Stay_home(Scene *sen);
////=========================================================
Virus* virus_create(int x, int y);
void virus_destroy(Virus *vi);
void virus_movement(Virus *vi, Scene *sen, Pacman *pac);
void virus_design(Virus *vi);

#endif // PACMAN_H_INCLUDED
