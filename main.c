#include "SOIL.h"                       //Extra
#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>                      //Extra
#include <time.h>                       //Extra
#include "pacman.h"                     //Extra
//static int up = 0;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

//Defines the variables and functions that will control the game
//GLOBAL
Pacman* pac;
Scene* sen;
Virus* vi[4];
int Start_Game = 0;

void design();
void StartGame();
void EndGame();

// function that creates and configures the OpenGL graphics window
// Default
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;


    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Pacman Game",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,    //Default size of windows is assigned
                          CW_USEDEFAULT,
                          800,                //Changing this Window Size won't affect
                          800,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);
    // Calls the function that initializes the game
    StartGame();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */
            // glClearColor specifies the red, green, blue, and alpha values used by
            // glClear to clear the color buffers. Values specified by glClearColor
            // are clamped to the range [0,1].
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
            // This loop controls each frame in the game.
            // Design the game here
            design();
            //https://stackoverflow.com/questions/36379743/why-does-this-c-opengl-program-run-twice
            glPopMatrix();

            SwapBuffers(hDC);

            Sleep(1);
        }
    }
    // Coming Out of loop that draws frames
    // Call GameOver
    EndGame();

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}
// Function that checks if the keyboard has been pressed
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE: //Press ESC button to exit
                    PostQuitMessage(0);
                    break;
                case VK_RIGHT: //Press Right arrow to move Pacman to right
                    pacman_ChangeDirection(pac,0,sen);
                    break;
                case VK_DOWN: //Press Down arrow to move Pacman Downwards
                    pacman_ChangeDirection(pac,1,sen);
                    break;
                case VK_LEFT: //Press Left arrow to move Pacman to left
                    pacman_ChangeDirection(pac,2,sen);
                    break;
                case VK_UP: //Press Up arrow to move Pacman Upwards
                    pacman_ChangeDirection(pac,3,sen);
                    break;
                //case 'M':
                    //up = 1;
                    //break;
                //case 'N':
                    //up = 0;
                    //break;
                case 'P':   //If P is pressed
                    if(Start_Game == 2) //If game has ended
                    {
                        EndGame();      //Endgame
                        StartGame();    //Reinitialize the game         //Scoreboard to be added here
                    }
                    Start_Game = 1;     //Game has started flag raised
                    break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
// Function that configures OpenGL
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    // Calls the function that loads the game's textures
    Textures();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); /* Linear Filtering */
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); /* Linear Filtering */


}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}


// Functions that control and design the game


// Function that draws each component of the game
void design(){
    scene_design(sen);
    //If Game has Started
    if(Start_Game == 0)
    {
        design_tell(0);     //Show the Starting screen
        return;
    }

    if(pacman_alive(pac))
    {
        pacman_movement(pac, sen);
        pacman_design(pac);
        int i;
        for(i=0; i<4; i++)
        {
            virus_movement(vi[i], sen, pac);
            virus_design(vi[i]);
        }
    }
    else                        //Show the Ending screen
    {
        design_tell(1);
        Start_Game = 2;         //Flag set to EndGame
    }

}
// Function that starts the game map and the initial positions of the characters
void StartGame()
{
    //srand(time(NULL));
    sen = scene_read("map.txt"/*,"map1.txt",up*/);        //Reading the Scene from map.txt
    pac = pacman_create(9,11);                       //Spawning the Pacman
    int i;
    for(i=0; i<4; i++)
        vi[i] = virus_create(9,9);      //Spawning all the virus

    Start_Game = 0;                     //Game Started Flag
}
// Function that releases game data
void EndGame(){
    int i;
    for(i=0; i<4; i++)
        virus_destroy(vi[i]);

    pacman_destroy(pac);
    scene_destroy(sen);
}
