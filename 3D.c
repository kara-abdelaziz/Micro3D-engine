#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  <SDL/SDL.h>
#include  <SDL/SDL_ttf.h>
#include  <SDL/SDL_image.h>
#include  <SDL/SDL_mixer.h>

#define   NBRE_POINT_MAX     100
#define   NBRE_SEG_MAX       100
#define   NBRE_OBJET_MAX     100
#define   NBRE_FACE_MAX      100

#define   DISTANCE           512
#define   RES_VERT           600
#define   RES_HORIZ          800
#define   RES_VERT_DIV_2     300
#define   RES_HORIZ_DIV_2    400

#define   FRAMES_PER_SECOND  300


////-------------------------variables globales-------------------------------------/////


SDL_Surface*   affichage     =   NULL   ;
SDL_Surface*   arrierePlan   =   NULL   ;
SDL_Surface*   message       =   NULL   ;
SDL_Surface*   bouttons      =   NULL   ;
SDL_Surface*   paneau        =   NULL   ;
SDL_Surface*   map           =   NULL   ;
SDL_Surface*   entete        =   NULL   ;
SDL_Surface*   grid          =   NULL   ;
SDL_Surface*   radar         =   NULL   ;
SDL_Surface*   texte         =   NULL   ;

Mix_Music*     music         =   NULL   ;

Mix_Chunk*     transl        =   NULL   ;
Mix_Chunk*     rotation      =   NULL   ;
Mix_Chunk*     echell        =   NULL   ;

Mix_Chunk*     sonF1         =   NULL   ;
Mix_Chunk*     sonF2         =   NULL   ;
Mix_Chunk*     sonF3         =   NULL   ;
Mix_Chunk*     sonF4         =   NULL   ;
Mix_Chunk*     sonF5         =   NULL   ;
Mix_Chunk*     sonF6         =   NULL   ;
Mix_Chunk*     sonF7         =   NULL   ;
Mix_Chunk*     sonF8         =   NULL   ;
Mix_Chunk*     sonF9         =   NULL   ;
Mix_Chunk*     sonF10        =   NULL   ;
Mix_Chunk*     sonF11        =   NULL   ;
Mix_Chunk*     sonF12        =   NULL   ;

TTF_Font*      font          =   NULL   ;

SDL_Color      textColor     =  { 100, 100, 200 }   ;

////-------------------------Structures de données-----------------------------------/////

typedef  struct  point
{
	int   x  ;
	int   y  ;
	int   z  ;

	int   X  ;
	int   Y  ;
}  Point  ;

typedef  struct  objet
{
	int     nbrePts                     ;
	Point   points[NBRE_POINT_MAX]      ;
	Point   ptsOrg[NBRE_POINT_MAX]      ;

	int     nbreSegment                 ;
	Point*  segments[NBRE_SEG_MAX][2]   ;

	int     nbreFace                       ;
	Point*  faces[NBRE_FACE_MAX][4]        ;
	Point   normale[NBRE_FACE_MAX]         ;
	Point   nrmOrg[NBRE_FACE_MAX]          ;
	int     faceTxtr[NBRE_FACE_MAX][3][2]  ;
	SDL_Surface*   texture                 ;
	
	Point   centre             ;
	float   angleX             ;
	float   angleY             ;
	float   angleZ             ;
	float   echell             ;

	int     radius             ;
}  Objet  ;

typedef  struct  objets
{
	int      nbreObjet                    ;
	Objet    toutObjet[NBRE_OBJET_MAX]    ;
}  Objets  ;


////-------------------------------variables globales-------------------------------------/////


void    initialisation(Objet* cube)  ;
void    initSDL(void)           ;
void    attendreTouche(void)    ;
void    dessinerEtoiles(void)   ;
void    dessinerLignes(void)    ;
void    dessinEnv2D(void)       ;
inline  void    translation(Objet * objet , int Dx , int Dy , int Dz)    ;
inline  void    changementEchell_rotation(Objet * objet)                 ;
inline  void    swap(int * a , int * b)     ;
void    ligne(int x0, int y0, int x1, int y1, Uint32  couleur)    ;
void    afficheObjetMesh(Objet*  cube)                            ;
void    afficheObjetTexture(Objet*  cube)                         ;
void    animationRadar(int X , int Y , float R)                   ;
void    animationTexte(void)                                      ;
inline  void     setPixel(int  X , int  Y , Uint32  couleur)      ;
inline  Uint32   getPixel(int  X , int  Y , SDL_Surface*  image)  ;
inline  void     triTableau(int tableau[][2] , int * position , int action , int y)    ;
inline  void     delTableau(int tableau[][2] , int * taille , int action)              ;
SDL_Surface*     chargerImage(char*  file)        ;
void             chargementFichirs()              ;
void             cleanUp()                        ;
void   triangle(int X0 , int Y0 , int X1 , int Y1 ,int X2 ,int Y2) ;



////--------------------------------Fonction principale-------------------------------------/////
//#undef main

int   main(int  argc , char**  argv)
{
	int         quitter  =  1   ;
	Objet       cube            ;
	
	Uint8 *     keystates       ;
	SDL_Rect    rectSrc         ;
	SDL_Rect    rectDst         ;
	
	int         FPS  =   0      ;
	int         i    =   1      ;
	int         temps           ;
	
	initialisation(&cube);
	
	///////--------------------------------------test------------------------------------////////


	//ligne(-100 , 200 , 750 , 200 , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;	

	//setPixel( 800 , 600 , SDL_MapRGB(affichage->format , 5 , 200 , 128))   ;

	//sleep(1000)  ;
	
	//dessinerEtoiles()         ;
	//dessinerLignes()          ;

	//SDL_UpdateRect(affichage, 0, 0, 0, 0)   ;	

	while(quitter)
	{
		temps       =   SDL_GetTicks()            ;
		
		SDL_PumpEvents()                          ;
		
		SDL_Event       event                     ;
		keystates   =   SDL_GetKeyState(NULL)     ;
		
		
		int   sonTransl , sonRotation , sonEchell ;
		
		/////-------------affichage de toute la scène par le changement du buffer------------///////
		
		dessinEnv2D()   ;
		
		changementEchell_rotation(&cube)    ;
		
		//afficheObjetMesh(&cube)    ;
		afficheObjetTexture(&cube)  ;
		
		/////---------------------------dessin des images 2D--------------------------------///////
		
		rectDst.x    =    0    ;
		rectDst.y    =    500  ;
		
		SDL_BlitSurface(paneau, NULL, affichage, &rectDst)     ;
		
		rectDst.x    =    0    ;
		rectDst.y    =    127  ;
		
		SDL_BlitSurface(grid, NULL, affichage, &rectDst)       ;
		
		rectDst.x    =    0    ;
		rectDst.y    =    80   ;
		
		SDL_BlitSurface(map, NULL, affichage, &rectDst)        ;
		
		rectDst.x    =    0    ;
		rectDst.y    =    0    ;
		
		SDL_BlitSurface(entete, NULL, affichage, &rectDst)     ;
		
		animationTexte()                                              ;
		animationRadar(cube.centre.x , cube.centre.z , cube.echell)   ;
		
		rectSrc.h   =    40      ;
		rectSrc.w   =    40      ;
		
		if(keystates[SDLK_UP])
		{
			changementEchell_rotation(&cube)     ;
			cube.angleX     -=   0.05            ;
			
			rectSrc.x   =   40     ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   315    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   40     ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   315    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_DOWN])
		{
			changementEchell_rotation(&cube)     ;
			cube.angleX     +=   0.05            ;
			
			rectSrc.x   =   80     ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   315    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   80     ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   315    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_RIGHT])
        	{
			changementEchell_rotation(&cube)     ;
			cube.angleY     -=   0.05            ;
			
			rectSrc.x   =   120    ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   365    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   120    ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   365    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_LEFT])
		{
			changementEchell_rotation(&cube)     ;
			cube.angleY     +=   0.05            ;
			
			rectSrc.x   =   0      ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   265    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   0      ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   265    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_PAGEUP])
		{
			changementEchell_rotation(&cube)     ;
			cube.angleZ     -=   0.05            ;
			
			rectSrc.x   =   160    ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   265    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   160    ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   265    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_PAGEDOWN])
		{
			changementEchell_rotation(&cube)     ;
			cube.angleZ     +=   0.05            ;
			
			rectSrc.x   =   200    ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   365    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   200    ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   365    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_w] && (cube.centre.z < 12100))
		{
			translation(&cube , 0 , 0 ,  5)    ;
			
			rectSrc.x   =   40     ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   90     ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   40     ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   90     ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_s] && (cube.centre.z > 1500))
		{
			translation(&cube , 0 , 0 , -5)    ;
			
			rectSrc.x   =   80     ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   90     ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   80     ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   90     ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_d] && (cube.centre.x < 4500))
		{
			translation(&cube ,  5 , 0 , 0)    ;
			
			rectSrc.x   =   0      ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   140    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   0      ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   140    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_a] && (cube.centre.x > -4500))
        	{
			translation(&cube , -5 , 0 , 0)    ;
			
			rectSrc.x   =   120    ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   40     ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   120    ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   40     ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_q] && (cube.centre.y > -4500))
		{
			translation(&cube , 0 , -5 , 0)    ;
			
			rectSrc.x   =   160    ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   40     ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   160    ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   40     ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_e] && (cube.centre.y < 4500))
        	{
			translation(&cube , 0 ,  5 , 0)    ;
			
			rectSrc.x   =   200    ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   140    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
			
		}
		else
		{
			rectSrc.x   =   200    ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   140    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_KP_PLUS] && (cube.echell < 1.1f))
		{
			cube.echell     +=   0.002        ;
			changementEchell_rotation(&cube)  ;
			
			rectSrc.x   =   240    ;
			rectSrc.y   =   0      ;
			
			rectDst.x   =   500-20    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   240    ;
			rectSrc.y   =   40     ;
			
			rectDst.x   =   500-20    ;
			rectDst.y   =   520    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_KP_MINUS] && (cube.echell > 0.5f))
        	{
			cube.echell     -=   0.002        ;
			changementEchell_rotation(&cube)  ;
			
			rectSrc.x   =   240    ;
			rectSrc.y   =   80     ;
			
			rectDst.x   =   500-20    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		else
		{
			rectSrc.x   =   240    ;
			rectSrc.y   =   120    ;
			
			rectDst.x   =   500-20    ;
			rectDst.y   =   560    ;
			
			SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
		}
		
		if(keystates[SDLK_ESCAPE])
        	{
			quitter  =   0    ;
		}
		
		if(keystates[SDLK_q] || keystates[SDLK_s] || keystates[SDLK_d] || keystates[SDLK_a] || keystates[SDLK_z] || keystates[SDLK_e])
		{
			Mix_Resume(1)    ;
		}
		else
		{
			Mix_Pause(1)     ;
		}
		
		if(keystates[SDLK_UP] || keystates[SDLK_DOWN] || keystates[SDLK_LEFT] || keystates[SDLK_RIGHT] || keystates[SDLK_PAGEUP] || keystates[SDLK_PAGEDOWN])
		{
			Mix_Resume(2)    ;
		}
		else
		{
			Mix_Pause(2)     ;
		}
		
		if(keystates[SDLK_KP_PLUS] || keystates[SDLK_KP_MINUS])
		{
			Mix_Resume(3)    ;
		}
		else
		{
			Mix_Pause(3)     ;
		}
		
		while(SDL_PollEvent(&event))
        	{
			if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_F1)
				{
					Mix_PlayChannel(4 , sonF1 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F2)
				{
					Mix_PlayChannel(5 , sonF2 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F3)
				{
					Mix_PlayChannel(6 , sonF3 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F4)
				{
					Mix_PlayChannel(7 , sonF4 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F5)
				{
					Mix_PlayChannel(8 , sonF5 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F6)
				{
					Mix_PlayChannel(9 , sonF6 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F7)
				{
					Mix_PlayChannel(10 , sonF7 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F8)
				{
					Mix_PlayChannel(11 , sonF8 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F9)
				{
					Mix_PlayChannel(12 , sonF9 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F10)
				{
					Mix_PlayChannel(13 , sonF10 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F11)
				{
					Mix_PlayChannel(14 , sonF11 , 0)     ;
				}
				
				if(event.key.keysym.sym == SDLK_F12)
				{
					Mix_PlayChannel(15 , sonF12 , 0)     ;
				}
			}
		}
		
		SDL_Flip(affichage)      ;
		
		if((SDL_GetTicks() - temps) < (1000 / FRAMES_PER_SECOND))
		{
			SDL_Delay((1000 / FRAMES_PER_SECOND)- (SDL_GetTicks() - temps))     ;
		}
		
		//printf("FPS = %i\n", (FPS += 1000 / (SDL_GetTicks() - temps))/i++)   ;
	}
	
	//attendreTouche()          ;

	/////-----------------------nétoyage avant la ferméture du programme---------------------------///////

	cleanUp()                 ;
	
	return    EXIT_SUCCESS    ;

}


///////-----------------------------------Autres fonctions------------------------------------//////////

void   initSDL(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) 
	{
		fprintf(stderr, "Erreur à l'initialisation de la SDL : %s\n", SDL_GetError())  ;
		exit(EXIT_FAILURE)                                                             ;
	}

	if (TTF_Init() < 0)
	{
		fprintf(stderr, "Erreur à l'initialisation de la SDL : %s\n", TTF_GetError())  ;
		exit(EXIT_FAILURE)                                                             ;
	}
	
	if (Mix_OpenAudio(44100 , MIX_DEFAULT_FORMAT , 2 , 1024) < 0)
	{
		fprintf(stderr, "Erreur à l'initialisation du Mixer SDL : %s\n", Mix_GetError())  ;
		exit(EXIT_FAILURE)                                                                ;
	}
	
	atexit(SDL_Quit)     ;

	affichage = SDL_SetVideoMode(800, 600, 32, SDL_DOUBLEBUF | SDL_FULLSCREEN)     ;

	if (affichage == NULL) 
	{
		fprintf(stderr, "Impossible d'activer le mode graphique : %s\n", SDL_GetError())   ;
		exit(EXIT_FAILURE)                                                                 ;
	}

	SDL_WM_SetCaption("SnakeAs 3d", NULL)       ;

}

void   attendreTouche(void)
{
	SDL_Event event      ;

	do
    		SDL_WaitEvent(&event)                                 ;
	while (event.type != SDL_QUIT && event.type != SDL_KEYDOWN)   ;
}

void   dessinerEtoiles(void)
{
	int i     ;
	for (i = 0; i < 100; i++)
	{
		setPixel(rand() % 800, rand() % 600 , SDL_MapRGB(affichage->format,rand() % 128 + 128, rand() % 128 + 128, rand() % 128 + 128))  ;
		SDL_UpdateRect(affichage, 0, 0, 0, 0)   ;
	}
}

void    dessinerLignes(void)
{
	int i     ;
	for (i = 0; i < 100 ; i++)
	{
		ligne(rand() % 800 , rand() % 600 , rand() % 800 , rand() % 600 , SDL_MapRGB(affichage->format,rand() % 128 + 128 , rand() % 128 + 128, rand() % 128 + 128))  ;
		SDL_UpdateRect(affichage, 0, 0, 0, 0)     ;
	}
}

void    dessinEnv2D(void)
{
	SDL_BlitSurface(arrierePlan, NULL, affichage, NULL)       ;
	SDL_BlitSurface(message, NULL, affichage, NULL)           ;
}

inline   void      setPixel(int  X , int  Y , Uint32  couleur)
{
	 *((Uint32*)(affichage->pixels) + (affichage->w * Y) + X) = couleur   ;
	 //printf("yoooo")   ;
}

inline   Uint32    getPixel(int  X , int  Y , SDL_Surface*  image)
{
	 return    *((Uint32*)(image->pixels) + (image->w * Y) + X)    ;
	 //printf("yoooo")   ;
}

void   ligne(int x0, int y0, int x1, int y1 , Uint32  couleur)
{
	int  ponte  =  (abs(y1 - y0) > abs(x1 - x0))   ; 
	if (ponte)
	{
		swap(&x0, &y0)   ;
		swap(&x1, &y1)   ;
	}

	if (x0 > x1)
	{
		swap(&x0, &x1)   ;
		swap(&y0, &y1)   ;
	}

	int    Dx  =  x1 - x0       ;
	int    Dy  =  abs(y1 - y0)  ;

	int    erreur  =     Dx / 2   ;
	int    pas_y                  ;
	int    y                      ;
	int    x                      ;

	if (y0 < y1)
	{
		pas_y   =  1   ;
	}
	else
	{
		pas_y   = -1   ;
	}
	
	if(x0 < 0)
	{
		erreur  =  erreur - x0 * Dy             ; 
		y0      =  y0 + pas_y * erreur / Dx     ;
		erreur  =  erreur % Dx          ;
		x0      =  0                    ;

	}
	else
	{
		if(x0 >= (ponte ? RES_VERT : RES_HORIZ))
		{
			x0  =   ponte ?  RES_VERT : RES_HORIZ         ;
			x1  =  (ponte ?  RES_VERT : RES_HORIZ) - 1    ;

			y0  =  0       ;
			y1  =  0       ;
		}
	}

	if(x1 < 0)
	{
		x0  =   0       ;
		x1  =  -1       ;

		y0  =   0       ;
		y1  =   0       ;
	}
	else
	{
		if(x1 >= (ponte ? RES_VERT : RES_HORIZ ))
		{
			int   erreur_tmp   =   erreur    ;
			
			erreur_tmp    =  erreur_tmp + ((ponte ? RES_VERT : RES_HORIZ) - x0 - 1) * Dy    ;
			y1      =  y0 + pas_y * erreur_tmp / Dx                                         ;
			x1      =  (ponte ? RES_VERT : RES_HORIZ) - 1                                   ;
		}
	}	

	if(y0 < 0)
	{
		if(y1 < 0)
		{
			x0  =   0       ;
			x1  =  -1       ;

			y0  =   0       ;
			y1  =   0       ;
		}
		else
		{
			erreur  =  erreur - y0 * Dx     ;
			x0      =  x0 + erreur / Dy     ;
			erreur  =  Dy - erreur % Dy     ;
			y0      =  0                    ;
		}
	}
	else
	{
		if(y0 >= (ponte ? RES_HORIZ : RES_VERT))
		{
			if(y1 >= (ponte ? RES_HORIZ : RES_VERT))
			{
				x0  =   0       ;
				x1  =  -1       ;

				y0  =   0       ;
				y1  =   0       ;
			}
			else
			{
				erreur  =  erreur + (y0 - (ponte ? RES_HORIZ : RES_VERT) + 1) * Dx        ;
				x0      =  x0 + erreur / Dy                     ;
				erreur  =  Dy - erreur % Dy                     ;
				y0      =  (ponte ? RES_HORIZ : RES_VERT) - 1   ;
			}
		}
	}
	

	if(y1 < 0)
	{
		int   erreur_tmp   =   erreur    ;

		erreur_tmp  =  erreur_tmp - y1 * Dx     ;
		x1          =  x1 - erreur_tmp / Dy     ;
		y1          =  0                        ;
	}
	else
	{
		if(y1 >= (ponte ? RES_HORIZ : RES_VERT))
		{
			int   erreur_tmp   =   erreur    ;

			erreur_tmp  =  erreur_tmp + (y1 - (ponte ? RES_HORIZ : RES_VERT) + 1) * Dx     ;
			x1      =  x1 - erreur_tmp / Dy                     ;
			y1      =  (ponte ? RES_HORIZ : RES_VERT) - 1   ;
		}
	}

	y    =   y0   ;

	for(x = x0 ; x <= x1 ; x++)
	{
		if(ponte)
		{
			setPixel(y,x,couleur)   ;
		}
		else
		{
			setPixel(x,y,couleur)   ;
		}

		erreur   = erreur - Dy   ;

		if(erreur < 0)
		{
			y       = y + pas_y        ;
			erreur  = erreur + Dx      ;
		}

	}

	return    ;
}

inline   void   swap(int * a , int * b)
{
	*a ^= *b   ;
	*b ^= *a   ;
	*a ^= *b   ;

	return   ;
}

SDL_Surface*     chargerImage(char*  file) 
{ 
	
	SDL_Surface*    imageChargee     = NULL   ;
	SDL_Surface*    imageClibree     = NULL   ;

	imageChargee    = IMG_Load(file);

	if(imageChargee != NULL) 
	{ 
		imageClibree     =    SDL_DisplayFormat(imageChargee)      ;
		SDL_FreeSurface(imageChargee)                              ;
		
	        if(imageClibree != NULL)
        	{
			SDL_SetColorKey( imageClibree, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_MapRGB( imageClibree->format, 0xFF, 0, 0xFF ) );
        	}
	}

	return     imageClibree     ; 
}

void    chargementFichirs()
{ 
	int    test   =  0    ;

	bouttons      =  chargerImage("clavier.png")           ;
	paneau        =  chargerImage("paneau.png")            ;
	map           =  chargerImage("map.png")               ;
	entete        =  chargerImage("entete.png")            ;
	grid          =  chargerImage("grid.png")              ;
	radar         =  chargerImage("radar.png")             ;
	texte         =  chargerImage("texte.png")             ;
	arrierePlan   =  chargerImage("black_lagoon.png")      ;
	
	music         =  Mix_LoadMUS("Foxy_Doll.mp3")          ;
	
	transl        =  Mix_LoadWAV("move.wav")               ;
	rotation      =  Mix_LoadWAV("rotate.wav")             ;
	echell        =  Mix_LoadWAV("zoom.wav")               ;
	
	sonF1         =  Mix_LoadWAV("F1.wav")                 ;
	sonF2         =  Mix_LoadWAV("F2.wav")                 ;
	sonF3         =  Mix_LoadWAV("F3.wav")                 ;
	sonF4         =  Mix_LoadWAV("F4.wav")                 ;
	sonF5         =  Mix_LoadWAV("F5.wav")                 ;
	sonF6         =  Mix_LoadWAV("F6.wav")                 ;
	sonF7         =  Mix_LoadWAV("F7.wav")                 ;
	sonF8         =  Mix_LoadWAV("F8.wav")                 ;
	sonF9         =  Mix_LoadWAV("F9.wav")                 ;
	sonF10        =  Mix_LoadWAV("F10.wav")                ;
	sonF11        =  Mix_LoadWAV("F11.wav")                ;
	sonF12        =  Mix_LoadWAV("F12.wav")                ;
	
	font          =  TTF_OpenFont("Prototype.ttf", 12)     ;
	
	if(bouttons == NULL)
	{
		test   =   1    ;
	}
	
	if(arrierePlan == NULL)
	{
		test   =   1    ;
	}
	
	if(paneau == NULL)
	{
		test   =   1    ;
	}
	
	if(map == NULL)
	{
		test   =   1    ;
	}
	
	if(entete == NULL)
	{ 
		test   =   1    ;
	}
	
	if(grid == NULL)
	{
		test   =   1    ;
	}
	
	if(radar == NULL)
	{
		test   =   1    ;
	}
	
	if(texte == NULL)
	{
		test   =   1    ;
	}
	
	if(music == NULL)
	{
		test   =   1    ;
	}
	
	if(transl == NULL)
	{
		test   =   1    ;
	}
	
	if(rotation == NULL)
	{
		test   =   1    ;
	}
	
	if(echell == NULL)
	{
		test   =   1    ;
	}
	
	if(font == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF1 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF2 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF3 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF4 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF5 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF6 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF7 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF8 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF9 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF10 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF11 == NULL)
	{
		test   =   1    ;
	}
	
	if(sonF12 == NULL)
	{
		test   =   1    ;
	}
	
	if(test)
	{
		fprintf(stderr, "Erreur lors du chargement des fichiers.\n")  ;
		exit(EXIT_FAILURE)                                            ;
	}
	
	SDL_SetAlpha(grid, SDL_SRCALPHA | SDL_RLEACCEL, 128)      ;
	//SDL_SetAlpha(radar, SDL_SRCALPHA | SDL_RLEACCEL, 128)     ;
	
	//message = TTF_RenderText_Solid( font, "Test pour sdl_ttf", textColor )    ;
	
	return   ;
}

void    cleanUp()
{
	SDL_FreeSurface(arrierePlan)    ;
	SDL_FreeSurface(message)        ;
	SDL_FreeSurface(bouttons)       ;
	SDL_FreeSurface(paneau)         ;
	SDL_FreeSurface(map)            ;
	SDL_FreeSurface(entete)         ;
	SDL_FreeSurface(grid)           ;
	SDL_FreeSurface(radar)          ;
	SDL_FreeSurface(texte)          ;
	
	Mix_HaltChannel(1)              ;
	Mix_HaltChannel(2)              ;
	Mix_HaltChannel(3)              ;
	
	Mix_HaltChannel(4)              ;
	Mix_HaltChannel(5)              ;
	Mix_HaltChannel(6)              ;
	Mix_HaltChannel(7)              ;
	Mix_HaltChannel(8)              ;
	Mix_HaltChannel(9)              ;
	Mix_HaltChannel(10)             ;
	Mix_HaltChannel(11)             ;
	Mix_HaltChannel(12)             ;
	Mix_HaltChannel(13)             ;
	Mix_HaltChannel(14)             ;
	Mix_HaltChannel(15)             ;
	
	Mix_FreeChunk(transl)           ;
	Mix_FreeChunk(rotation)         ;
	Mix_FreeChunk(echell)           ;
	
	Mix_FreeChunk(sonF1)            ;
	Mix_FreeChunk(sonF2)            ;
	Mix_FreeChunk(sonF3)            ;
	Mix_FreeChunk(sonF4)            ;
	Mix_FreeChunk(sonF5)            ;
	Mix_FreeChunk(sonF6)            ;
	Mix_FreeChunk(sonF7)            ;
	Mix_FreeChunk(sonF8)            ;
	Mix_FreeChunk(sonF9)            ;
	Mix_FreeChunk(sonF10)           ;
	Mix_FreeChunk(sonF11)           ;
	Mix_FreeChunk(sonF12)           ;
	
	Mix_FadeOutMusic(2000)          ;
	SDL_Delay(2000)                 ;
	Mix_FreeMusic(music)            ;
	Mix_CloseAudio()                ;
	
	TTF_CloseFont(font)             ;
	
	TTF_Quit()                      ;
}

void    initialisation(Objet*  cube)
{
	//////-------------------------------Le cube 3D------------------------------//////

	int      i      ;
	
	Point points[8]   =  { {  500 ,  500 , -500 , 0 , 0 } , { -500 ,  500 , -500 , 0 , 0 } , { -500 , -500 , -500 , 0 , 0 } , 
                               {  500 , -500 , -500 , 0 , 0 } , {  500 ,  500 ,  500 , 0 , 0 } , { -500 ,  500 ,  500 , 0 , 0 } , 
                               { -500 , -500 ,  500 , 0 , 0 } , {  500 , -500 ,  500 , 0 , 0 } }    ;


	Point normales[12]=  { {    0 ,    0 , -100 , 0 , 0 } , {    0 ,    0 , -100 , 0 , 0 } , {  100 ,    0 ,    0 , 0 , 0 } , 
                               {  100 ,    0 ,    0 , 0 , 0 } , {    0 ,    0 ,  100 , 0 , 0 } , {    0 ,    0 ,  100 , 0 , 0 } , 
                               { -100 ,    0 ,    0 , 0 , 0 } , { -100 ,    0 ,    0 , 0 , 0 } , {    0 , -100 ,    0 , 0 , 0 } ,
                               {    0 , -100 ,    0 , 0 , 0 } , {    0 ,  100 ,    0 , 0 , 0 } , {    0 ,  100 ,    0 , 0 , 0 } }  ;
	
	
	cube->nbrePts       =   8     ;
	cube->centre.x      =   0     ;
	cube->centre.y      =   0     ;
	cube->centre.z      =   2000  ;

	for(i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->ptsOrg[i].x    =     points[i].x    ;
		cube->ptsOrg[i].y    =     points[i].y    ;
		cube->ptsOrg[i].z    =     points[i].z    ;
	}

	for(i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->points[i].x   =    cube->ptsOrg[i].x + cube->centre.x    ;
		cube->points[i].y   =    cube->ptsOrg[i].y + cube->centre.y    ;
		cube->points[i].z   =    cube->ptsOrg[i].z + cube->centre.z    ;
	}

	cube->nbreSegment       =   12                  ;
	cube->segments[0][0]    =   &(cube->points[0])   ;
	cube->segments[0][1]    =   &(cube->points[1])   ;
	cube->segments[1][0]    =   &(cube->points[1])   ;
	cube->segments[1][1]    =   &(cube->points[2])   ;
	cube->segments[2][0]    =   &(cube->points[2])   ;
	cube->segments[2][1]    =   &(cube->points[3])   ;
	cube->segments[3][0]    =   &(cube->points[3])   ;
	cube->segments[3][1]    =   &(cube->points[0])   ;
	cube->segments[4][0]    =   &(cube->points[4])   ;
	cube->segments[4][1]    =   &(cube->points[5])   ;
	cube->segments[5][0]    =   &(cube->points[5])   ;
	cube->segments[5][1]    =   &(cube->points[6])   ;
	cube->segments[6][0]    =   &(cube->points[6])   ;
	cube->segments[6][1]    =   &(cube->points[7])   ;
	cube->segments[7][0]    =   &(cube->points[7])   ;
	cube->segments[7][1]    =   &(cube->points[4])   ;
	cube->segments[8][0]    =   &(cube->points[0])   ;
	cube->segments[8][1]    =   &(cube->points[4])   ;
	cube->segments[9][0]    =   &(cube->points[1])   ;
	cube->segments[9][1]    =   &(cube->points[5])   ;
	cube->segments[10][0]   =   &(cube->points[2])   ;
	cube->segments[10][1]   =   &(cube->points[6])   ;
	cube->segments[11][0]   =   &(cube->points[3])   ;
	cube->segments[11][1]   =   &(cube->points[7])   ;

	cube->nbreFace          =   12                   ;
	cube->faces[0][0]       =   &(cube->points[0])   ;
	cube->faces[0][1]       =   &(cube->points[2])   ;
	cube->faces[0][2]       =   &(cube->points[1])   ;
	cube->faces[1][0]       =   &(cube->points[0])   ;
	cube->faces[1][1]       =   &(cube->points[3])   ;
	cube->faces[1][2]       =   &(cube->points[2])   ;
	cube->faces[2][0]       =   &(cube->points[0])   ;
	cube->faces[2][1]       =   &(cube->points[4])   ;
	cube->faces[2][2]       =   &(cube->points[3])   ;
	cube->faces[3][0]       =   &(cube->points[3])   ;
	cube->faces[3][1]       =   &(cube->points[4])   ;
	cube->faces[3][2]       =   &(cube->points[7])   ;
	cube->faces[4][0]       =   &(cube->points[7])   ;
	cube->faces[4][1]       =   &(cube->points[4])   ;
	cube->faces[4][2]       =   &(cube->points[5])   ;
	cube->faces[5][0]       =   &(cube->points[5])   ;
	cube->faces[5][1]       =   &(cube->points[6])   ;
	cube->faces[5][2]       =   &(cube->points[7])   ;
	cube->faces[6][0]       =   &(cube->points[6])   ;
	cube->faces[6][1]       =   &(cube->points[5])   ;
	cube->faces[6][2]       =   &(cube->points[1])   ;
	cube->faces[7][0]       =   &(cube->points[1])   ;
	cube->faces[7][1]       =   &(cube->points[2])   ;
	cube->faces[7][2]       =   &(cube->points[6])   ;
	cube->faces[8][0]       =   &(cube->points[2])   ;
	cube->faces[8][1]       =   &(cube->points[3])   ;
	cube->faces[8][2]       =   &(cube->points[6])   ;
	cube->faces[9][0]       =   &(cube->points[3])   ;
	cube->faces[9][1]       =   &(cube->points[7])   ;
	cube->faces[9][2]       =   &(cube->points[6])   ;
	cube->faces[10][0]      =   &(cube->points[1])   ;
	cube->faces[10][1]      =   &(cube->points[5])   ;
	cube->faces[10][2]      =   &(cube->points[4])   ;
	cube->faces[11][0]      =   &(cube->points[0])   ;
	cube->faces[11][1]      =   &(cube->points[1])   ;
	cube->faces[11][2]      =   &(cube->points[4])   ;


	for(i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->normale[i].x    =    cube->nrmOrg[i].x    =     normales[i].x    ;
		cube->normale[i].y    =    cube->nrmOrg[i].y    =     normales[i].y    ;
		cube->normale[i].z    =    cube->nrmOrg[i].z    =     normales[i].z    ;
	}

	for(i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->faces[i][3]   =    &(cube->normale[i])    ;
	}

	cube->faceTxtr[0][0][0]     =    255 + 181  ;
	cube->faceTxtr[0][0][1]     =    511 + 181  ;
	cube->faceTxtr[0][1][0]     =      0 + 181  ;
	cube->faceTxtr[0][1][1]     =    255 + 181  ;
	cube->faceTxtr[0][2][0]     =      0 + 181  ;
	cube->faceTxtr[0][2][1]     =    511 + 181  ;
	cube->faceTxtr[1][0][0]     =    255 + 181  ;
	cube->faceTxtr[1][0][1]     =    511 + 181  ;
	cube->faceTxtr[1][1][0]     =    255 + 181  ;
	cube->faceTxtr[1][1][1]     =    255 + 181  ;
	cube->faceTxtr[1][2][0]     =      0 + 181  ;
	cube->faceTxtr[1][2][1]     =    255 + 181  ;
	cube->faceTxtr[2][0][0]     =    255 + 181  ;
	cube->faceTxtr[2][0][1]     =    511 + 181  ;
	cube->faceTxtr[2][1][0]     =    511 + 181  ;
	cube->faceTxtr[2][1][1]     =    511 + 181  ;
	cube->faceTxtr[2][2][0]     =    255 + 181  ;
	cube->faceTxtr[2][2][1]     =    255 + 181  ;
	cube->faceTxtr[3][0][0]     =    255 + 181  ;
	cube->faceTxtr[3][0][1]     =    255 + 181  ;
	cube->faceTxtr[3][1][0]     =    511 + 181  ;
	cube->faceTxtr[3][1][1]     =    511 + 181  ;
	cube->faceTxtr[3][2][0]     =    511 + 181  ;
	cube->faceTxtr[3][2][1]     =    255 + 181  ;
	cube->faceTxtr[4][0][0]     =    511 + 181  ;
	cube->faceTxtr[4][0][1]     =    255 + 181  ;
	cube->faceTxtr[4][1][0]     =    511 + 181  ;
	cube->faceTxtr[4][1][1]     =    511 + 181  ;
	cube->faceTxtr[4][2][0]     =    767 + 181  ;
	cube->faceTxtr[4][2][1]     =    511 + 181  ;
	cube->faceTxtr[5][0][0]     =    767 + 181  ;
	cube->faceTxtr[5][0][1]     =    511 + 181  ;
	cube->faceTxtr[5][1][0]     =    767 + 181  ;
	cube->faceTxtr[5][1][1]     =    255 + 181  ;
	cube->faceTxtr[5][2][0]     =    511 + 181  ;
	cube->faceTxtr[5][2][1]     =    255 + 181  ;
	cube->faceTxtr[6][0][0]     =    767 + 181  ;
	cube->faceTxtr[6][0][1]     =    255 + 181  ;
	cube->faceTxtr[6][1][0]     =    767 + 181  ;
	cube->faceTxtr[6][1][1]     =    511 + 181  ;
	cube->faceTxtr[6][2][0]     =   1023 + 181  ;
	cube->faceTxtr[6][2][1]     =    511 + 181  ;
	cube->faceTxtr[7][0][0]     =   1023 + 181  ;
	cube->faceTxtr[7][0][1]     =    511 + 181  ;
	cube->faceTxtr[7][1][0]     =   1023 + 181  ;
	cube->faceTxtr[7][1][1]     =    255 + 181  ;
	cube->faceTxtr[7][2][0]     =    767 + 181  ;
	cube->faceTxtr[7][2][1]     =    255 + 181  ;
	cube->faceTxtr[8][0][0]     =      0 + 181  ;
	cube->faceTxtr[8][0][1]     =    255 + 181  ;
	cube->faceTxtr[8][1][0]     =    255 + 181  ;
	cube->faceTxtr[8][1][1]     =    255 + 181  ;
	cube->faceTxtr[8][2][0]     =      0 + 181  ;
	cube->faceTxtr[8][2][1]     =      0 + 181  ;
	cube->faceTxtr[9][0][0]     =    255 + 181  ;
	cube->faceTxtr[9][0][1]     =    255 + 181  ;
	cube->faceTxtr[9][1][0]     =    255 + 181  ;
	cube->faceTxtr[9][1][1]     =      0 + 181  ;
	cube->faceTxtr[9][2][0]     =      0 + 181  ;
	cube->faceTxtr[9][2][1]     =      0 + 181  ;
	cube->faceTxtr[10][0][0]    =      0 + 181  ;
	cube->faceTxtr[10][0][1]    =    511 + 181  ;
	cube->faceTxtr[10][1][0]    =      0 + 181  ;
	cube->faceTxtr[10][1][1]    =    767 + 181  ;
	cube->faceTxtr[10][2][0]    =    255 + 181  ;
	cube->faceTxtr[10][2][1]    =    767 + 181  ;
	cube->faceTxtr[11][0][0]    =    255 + 181  ;
	cube->faceTxtr[11][0][1]    =    511 + 181  ;
	cube->faceTxtr[11][1][0]    =      0 + 181  ;
	cube->faceTxtr[11][1][1]    =    511 + 181  ;
	cube->faceTxtr[11][2][0]    =    255 + 181  ;
	cube->faceTxtr[11][2][1]    =    767 + 181  ;
	
	cube->angleX     =   0.0        ;
	cube->angleY     =   0.0        ;
	cube->angleZ     =   0.0        ;
	cube->echell     =   0.6        ;


	//////------------------------Chargement des fichiers et des bibliothèques--------------------------//////

	initSDL()                 ;
	
	chargementFichirs()       ;
	
	cube->texture   =  chargerImage("texture.png")       ;

	////------------------------------------------Dessin------------------------------------------/////

	dessinEnv2D()   ;
	
	////------------------------------------------Musique------------------------------------------/////
	
	Mix_AllocateChannels(16)    ;
	
	Mix_PlayMusic(music, -1)              ;
	Mix_VolumeMusic(MIX_MAX_VOLUME/8)     ;
	
	Mix_Volume(1,MIX_MAX_VOLUME)          ;
	Mix_Volume(2,MIX_MAX_VOLUME/2)        ;
	Mix_Volume(3,MIX_MAX_VOLUME/4)        ;
	
	Mix_PlayChannel(1 , transl , -1)    ;
	Mix_PlayChannel(2 , rotation , -1)  ;
	Mix_PlayChannel(3 , echell , -1)    ;
	
	Mix_Pause(1)     ;
	Mix_Pause(2)     ;
	Mix_Pause(3)     ;
	
}

void    afficheObjetMesh(Objet*  cube)
{
	int    i      ;

	////--------------------------------projection---------------------------------//////

	for(i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->points[i].X  =  ((cube->points[i].x * DISTANCE) / cube->points[i].z) + (RES_HORIZ / 2)    ;	
		cube->points[i].Y  =  ((cube->points[i].y * DISTANCE) / cube->points[i].z) + (RES_VERT  / 2)    ;		
	}

	////--------------------------dessin des mesh---------------------------------//////

	for(i = 0 ; i < cube->nbreSegment ; i++)
	{		
		ligne(cube->segments[i][0]->X , cube->segments[i][0]->Y , cube->segments[i][1]->X , cube->segments[i][1]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;	
	}

	return   ;
}

void    afficheObjetTexture(Objet*  cube)
{
	int    i , j , k    ;

	////--------------------------------projection---------------------------------//////

	for(i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->points[i].X  =  ((cube->points[i].x * DISTANCE) / cube->points[i].z) + (RES_HORIZ / 2)    ;	
		cube->points[i].Y  =  ((cube->points[i].y * DISTANCE) / cube->points[i].z) + (RES_VERT  / 2)    ;
	}

	////--------------------------dessin des faces---------------------------------//////

	for(i = 0 ; i < cube->nbreFace ; i++)
	{		
		if((((cube->faces[i][3]->x * (cube->faces[i][0]->X - (RES_HORIZ / 2))) + (cube->faces[i][3]->y * (cube->faces[i][0]->Y - (RES_VERT  / 2))) + (cube->faces[i][3]->z * DISTANCE)) < 0) && ((cube->faces[i][0]->X > 0) || (cube->faces[i][1]->X > 0) || (cube->faces[i][2]->X > 0)) && ((cube->faces[i][0]->X < (RES_HORIZ-1)) || (cube->faces[i][1]->X < (RES_HORIZ-1)) || (cube->faces[i][2]->X < (RES_HORIZ-1))) && ((cube->faces[i][0]->Y > 0) || (cube->faces[i][1]->Y > 0) || (cube->faces[i][2]->Y > 0)) && ((cube->faces[i][0]->Y < (RES_VERT-1)) || (cube->faces[i][1]->Y < (RES_VERT-1)) || (cube->faces[i][2]->Y < (RES_VERT-1))))
		{	
			int    haut   =   0      ;
			int    bas , millieu     ;
			int    baleillage , dis  ;
			
			
			if(cube->faces[i][0]->Y > cube->faces[i][1]->Y)
			{
				haut   =   1   ;
			}
			
			if(cube->faces[i][haut]->Y > cube->faces[i][2]->Y)
			{
				haut   =   2   ;
			}
			
			if(cube->faces[i][(haut+1)%3]->Y > cube->faces[i][(haut+2)%3]->Y)
			{
				millieu    =   (haut+2)%3   ;
				bas        =   (haut+1)%3   ;
				//baleillage =   -1           ;
			}
			else
			{
				millieu    =   (haut+1)%3   ;
				bas        =   (haut+2)%3   ;
				//baleillage =   1            ;
			}
			
			///////---------------------------------------------------------------//////////
			
			int    X0    =   cube->faces[i][haut]->X       ;
			int    Y0    =   cube->faces[i][haut]->Y       ;
			int    X1    =   cube->faces[i][millieu]->X    ;
			int    Y1    =   cube->faces[i][millieu]->Y    ;
			int    X2    =   cube->faces[i][bas]->X        ;
			int    Y2    =   cube->faces[i][bas]->Y        ;
			
			int    Dx0  =  X1 - X0   ;
			int    Dy0  =  Y1 - Y0   ;
			
			int    Dx1  =  X2 - X1   ;
			int    Dy1  =  Y2 - Y1   ;
			
			int    Dx2  =  X2 - X0   ;
			int    Dy2  =  Y2 - Y0   ;
			
			if(!Dy2) continue       ;
			
			int    reste0     ;
			int    erreur0    ;
			int    pas_C_0    ;
			int    pas_L_0    ;
			
			if(Dy0)
			{
				reste0    =     abs(Dx0) % Dy0 ;
				erreur0   =     Dy0 / 2        ;
				pas_C_0   =     Dx0 / Dy0      ;
			
				if (X0 < X1)
				{
					pas_L_0   =  pas_C_0 + 1   ;
				}
				else
				{
					pas_L_0   =  pas_C_0 - 1   ;
				}
			}
			
			int    reste1     ;
			int    erreur1    ;
			int    pas_C_1    ;
			int    pas_L_1    ;
			
			if(Dy1)
			{
				reste1    =     abs(Dx1) % Dy1 ;
				erreur1   =     Dy1 / 2        ;
				pas_C_1   =     Dx1 / Dy1      ;
			
				if (X1 < X2)
				{
					pas_L_1   =  pas_C_1 + 1   ;
				}
				else
				{
					pas_L_1   =  pas_C_1 - 1   ;
				}
			}
			
			int    reste2    =     abs(Dx2) % Dy2 ;
			int    erreur2   =     Dy2 / 2        ;
			int    pas_C_2   =     Dx2 / Dy2      ;
			int    pas_L_2                        ;
			
			if (X0 < X2)
			{
				pas_L_2   =  pas_C_2 + 1   ;
			}
			else
			{
				pas_L_2   =  pas_C_2 - 1   ;
			}
			
			int    x0    =  X0   ;
			int    x1    =  X1   ;
			int    x2    =  X0   ;
			
			///////---------------------------------------------------------------//////////
			
			int    XV0    =   cube->faceTxtr[i][haut][0]     ;
			int    YV0    =   cube->faceTxtr[i][haut][1]     ;
			int    XV1    =   cube->faceTxtr[i][bas][0]      ;
			int    YV1    =   cube->faceTxtr[i][bas][1]      ;
			
			int    DVx   =   XV1 - XV0   ;
			int    DVy   =   YV1 - YV0   ;
			
			int    DV    =   Y2 - Y0     ;
			
			int    resteVx    =     abs(DVx) % DV  ;
			int    erreurVx   =     DV  / 2        ;
			int    distVx     =     DVx / DV       ;
			int    DistVx                          ;
			
			if (XV0 < XV1)
			{
				DistVx   =  distVx + 1   ;
			}
			else
			{
				DistVx   =  distVx - 1   ;
			}
			
			int    resteVy    =     abs(DVy) % DV  ;
			int    erreurVy   =     DV  / 2        ;
			int    distVy     =     DVy / DV       ;
			int    DistVy                          ;
			
			if (YV0 < YV1)
			{
				DistVy   =  distVy + 1   ;
			}
			else
			{
				DistVy   =  distVy - 1   ;
			}
			
			int    xV    =  XV0    ;
			int    yV    =  YV0    ;
			
			///////---------------------------------------------------------------//////////
			
			int    XH0    =   XV0 + ((Y1-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((Y1-Y0)*resteVx)-DV/2)/DV) + (((((Y1-Y0)*resteVx)-DV/2)%DV)>0))     ;
			int    YH0    =   YV0 + ((Y1-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((Y1-Y0)*resteVy)-DV/2)/DV) + (((((Y1-Y0)*resteVy)-DV/2)%DV)>0))     ;
			int    XH1    =   cube->faceTxtr[i][millieu][0]      ;
			int    YH1    =   cube->faceTxtr[i][millieu][1]      ;
			
			int    DHx   =   XH1 - XH0   ;
			int    DHy   =   YH1 - YH0   ;
			
			int    DH    =   X1 - (X0 + ((Y1-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((Y1-Y0)*reste2)-Dy2/2)/Dy2) + (((((Y1-Y0)*reste2)-Dy2/2)%Dy2)>0)))     ;
			
			if(DH > 0)
			{
				baleillage   =   1    ;
			}
			else
			{
				baleillage   =  -1    ;
			}
			
			DH     =  abs(DH)      ;
			
			if(DH < 3) continue       ;
			
			int    resteHx    =     abs(DHx) % DH  ;
			int    erreurHx   =     DH             ;
			int    distHx     =     DHx / DH       ;
			int    DistHx                          ;
			
			if (XH0 < XH1)
			{
				DistHx   =  distHx + 1   ;
			}
			else
			{
				DistHx   =  distHx - 1   ;
			}
			
			int    resteHy    =     abs(DHy) % DH  ;
			int    erreurHy   =     DH             ;
			int    distHy     =     DHy / DH       ;
			int    DistHy                          ;
			
			if (YH0 < YH1)
			{
				DistHy   =  distHy + 1   ;
			}
			else
			{
				DistHy   =  distHy - 1   ;
			}
			
			int    xH        ;
			int    yH        ;
			
			///////---------------------------------------------------------------//////////
			
			int    y , x3 , x4         ;
			int    erreur3 , erreur4   ;
			int    Dy3     , Dy4       ;
			int    reste3  , reste4    ;
			int    pas_C_3 , pas_C_4   ;
			int    pas_L_3 , pas_L_4   ;
			
			int    xVV       , yVV         ;
			int    erreurVVx , erreurVVy   ;
			int    DVV                     ;
			int    resteVVx  , resteVVy    ;
			int    distVVx   , distVVy     ;
			int    DistVVx   , DistVVy     ;
			
			///////---------------------------------------------------------------//////////
			
			int    Xv0       ;
			int    Yv0       ;
			int    Xv1       ;
			int    Yv1       ;
			
			int    Dvx       ;
			int    Dvy       ;
			
			int    Dv        ;
			
			int    restevx     ;
			int    erreurvx    ;
			int    distvx      ;
			int    Distvx      ;
			
			int    restevy     ;
			int    erreurvy    ;
			int    distvy      ;
			int    Distvy      ;
			
			int    xv        ;
			int    yv        ;
			
			///////---------------------------------------------------------------//////////
			
			int    bordure[10][2]   =   { 0 }  ;             //0  : ...
			                                                 //1  : initialisation Y0_Y1
			                                                 //2  : initialisation Y1_Y2
			                                                 //3  : initialisation bord 0
			                                                 //4  : initialisation YV0_YV1
			                                                 //5  : initialisation Yv0_Yv1
			                                                 //6  : fin de la boucle
			                                                 //7  : ...
			                                                 //8  : ...
			                                                 //9  : ...
			k     =   0       ;
			y     =   Y0      ;
			
			if(Y0 != Y1)
			{
				triTableau(bordure , &k , 1 , Y0)   ;
			}
				
			triTableau(bordure , &k , 4 , Y0)   ;
				
			if(Y1 != Y2)
			{
				triTableau(bordure , &k , 2 , Y1)   ;
			}
			
			triTableau(bordure , &k , 6 , Y2+1)   ;
			
			///////---------------------------------------------------------------//////////
			
			int   HM_gauche     ;
			int   HB_gauche     ;
			int   MB_gauche     ;
			int   HM_droite     ;
			int   HB_droite     ;
			int   MB_droite     ;
			
			if(X0 < 0)
			{
				if(X1 < 0)
				{
					HB_gauche  =   Y2 - (((float)Dy2 / Dx2) * X2)    ;
					
					if(Y1 == Y2)
					{
						MB_gauche  =   Y1   ;
					}
					else
					{
						MB_gauche  =   Y2 - (((float)Dy1 / Dx1) * X2)    ;
					}
					
					if(baleillage == 1)
					{
						dis        =  MB_gauche      ;
						y          =  MB_gauche      ;
						
						int   Dis  =  -(X0 + ((MB_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv0  =  XV0 + ((MB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv0  =  YV0 + ((MB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Xv1  =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						Yv1  =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						Dvx   =   Xv1 - Xv0   ;
						Dvy   =   Yv1 - Yv0   ;
						
						Dv    =   HB_gauche - MB_gauche ;
						
						if(!Dv)  Dv = 1    ;
						
						Dv     =  abs(Dv)               ;
						
						restevx    =     abs(Dvx) % Dv  ;
						erreurvx   =     Dv / 2         ;
						distvx     =     Dvx / Dv       ;
						Distvx                          ;
						
						if (Xv0 < Xv1)
						{
							Distvx   =  distvx + 1   ;
						}
						else
						{
							Distvx   =  distvx - 1   ;
						}
						
						restevy    =     abs(Dvy) % Dv  ;
						erreurvy   =     Dv / 2         ;
						distvy     =     Dvy / Dv       ;
						Distvy                          ;
						
						if (Yv0 < Yv1)
						{
							Distvy   =  distvy + 1   ;
						}
						else
						{
							Distvy   =  distvy - 1   ;
						}
						
						xv    =     Xv0    ;
						yv    =     Yv0    ;
						
						x1    =     0      ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 4)                ;
						
						triTableau(bordure , &k , 5 , MB_gauche)    ;
						
						xV   =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						yV   =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						x2    =     0      ;
						
						triTableau(bordure , &k , 4 , HB_gauche)    ;
					}
					else
					{
						y    =  HB_gauche      ;
						
						xV   =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						yV   =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 4)                ;
						delTableau(bordure , &k , 2)                ;
						
						triTableau(bordure , &k , 3 , HB_gauche)    ;
						
						x2    =     0      ;
						
						triTableau(bordure , &k , 4 , HB_gauche)    ;
						
						if(Y1 != Y2)
						{
							x1    =     0      ;
							
							triTableau(bordure , &k , 2 , MB_gauche)    ;
						}
					}
				}
				else
				{
					if(X2 < 0)
					{
						if(Y0 == Y1)
						{
							HM_gauche  =   Y0   ;
						}
						else
						{
							HM_gauche  =   Y1 - (((float)Dy0 / Dx0) * X1)    ;
						}
						
						if(Y2 == Y1)
						{
							MB_gauche  =   Y1   ;
						}
						else
						{
							MB_gauche  =   Y1 - (((float)Dy1 / Dx1) * X1)    ;
						}
						
						dis        =  HM_gauche      ;
						y          =  HM_gauche      ;
						
						int   Dis  =  -(X0 + ((HM_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv0  =  XV0 + ((HM_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv0  =  YV0 + ((HM_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Dis  =  -(X0 + ((MB_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv1  =  XV0 + ((MB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv1  =  YV0 + ((MB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Dvx   =   Xv1 - Xv0   ;
						Dvy   =   Yv1 - Yv0   ;
						
						Dv    =   MB_gauche - HM_gauche ;
						
						if(!Dv)  Dv = 1    ;
						
						Dv     =  abs(Dv)               ;
						
						restevx    =     abs(Dvx) % Dv  ;
						erreurvx   =     Dv / 2         ;
						distvx     =     Dvx / Dv       ;
						Distvx                          ;
						
						if (Xv0 < Xv1)
						{
							Distvx   =  distvx + 1   ;
						}
						else
						{
							Distvx   =  distvx - 1   ;
						}
						
						restevy    =     abs(Dvy) % Dv  ;
						erreurvy   =     Dv / 2         ;
						distvy     =     Dvy / Dv       ;
						Distvy                          ;
						
						if (Yv0 < Yv1)
						{
							Distvy   =  distvy + 1   ;
						}
						else
						{
							Distvy   =  distvy - 1   ;
						}
						
						xv    =     Xv0    ;
						yv    =     Yv0    ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 2)                ;
						delTableau(bordure , &k , 4)                ;
						
						if(Y0 != Y1)
						{
							x0    =     0      ;
							
							triTableau(bordure , &k , 1 , HM_gauche)    ;
						}
						
						triTableau(bordure , &k , 2 , Y1)           ;

						triTableau(bordure , &k , 5 , HM_gauche)    ;
						
						triTableau(bordure , &k , 6 , MB_gauche)    ;
					}
					else
					{
						if(Y0 == Y1)
						{
							HM_gauche  =   Y0   ;
						}
						else
						{
							HM_gauche  =   Y1 - (((float)Dy0 / Dx0) * X1)    ;
						}
						
						HB_gauche  =   Y2 - (((float)Dy2 / Dx2) * X2)    ;
						
						if(baleillage == 1)
						{
							dis        =  HM_gauche      ;
							y          =  HM_gauche      ;
							
							int   Dis  =  -(X0 + ((HM_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HM_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
							
							Yv0  =  YV0 + ((HM_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Xv1  =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))    ;
							
							Yv1  =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))    ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HB_gauche - HM_gauche ;
							
							if(!Dv) Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							delTableau(bordure , &k , 1)                ;
							delTableau(bordure , &k , 2)                ;
							delTableau(bordure , &k , 4)                ;
							
							if(Y0 != Y1)
							{
								x0    =     0      ;
								
								triTableau(bordure , &k , 1 , HM_gauche)    ;
							}
							
							triTableau(bordure , &k , 2 , Y1)    ;
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HM_gauche)    ;
							
							x2    =     0      ;
							
							xV   =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))    ;
							
							yV   =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))     ;
							
							triTableau(bordure , &k , 4 , HB_gauche)    ;
						}
						else
						{
							y    =  HB_gauche      ;
							
							xV   =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))     ;
							
							yV   =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))     ;
							
							delTableau(bordure , &k , 1)                ;
							delTableau(bordure , &k , 2)                ;
							delTableau(bordure , &k , 4)                ;
							
							triTableau(bordure , &k , 3 , HB_gauche)    ;
							
							x2    =     0      ;
							
							triTableau(bordure , &k , 4 , HB_gauche)    ;
							
							x0    =     0      ;
							
							triTableau(bordure , &k , 1 , HM_gauche)    ;
							
							triTableau(bordure , &k , 2 , Y1)           ;
						}
					}
				}
			}
			else
			{
				if(X1 < 0)
				{
					if(X2 < 0)
					{
						if(Y0 == Y1)
						{
							HM_gauche  =   Y0   ;
						}
						else
						{
							HM_gauche  =   Y0 - (((float)Dy0 / Dx0) * X0)    ;
						}
						
						HB_gauche  =   Y0 - (((float)Dy2 / Dx2) * X0)    ;
						
						if(baleillage == 1)
						{
							dis        =  HB_gauche      ;
							
							int   Dis  =  -(X0 + ((HM_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))   ;
							
							Yv0  =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))   ;
							
							Xv1  =  XV0 + ((HM_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
							
							Yv1  =  YV0 + ((HM_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HM_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HM_gauche - HB_gauche ;
							
							if(!Dv)  Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							delTableau(bordure , &k , 2)                ;
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HB_gauche)    ;
							
							triTableau(bordure , &k , 6 , HM_gauche)    ;
							
						}
						else
						{
							delTableau(bordure , &k , 2)                ;
							
							triTableau(bordure , &k , 3 , HM_gauche)    ;
							
							triTableau(bordure , &k , 6 , HB_gauche)    ;
						}
					}
					else
					{
						if(Y0 == Y1)
						{
							HM_gauche  =   Y0   ;
						}
						else
						{
							HM_gauche  =   Y0 - (((float)Dy0 / Dx0) * X0)    ;
						}
						
						if(Y2 == Y1)
						{
							MB_gauche  =   Y1   ;
						}
						else
						{
							MB_gauche  =   Y2 - (((float)Dy1 / Dx1) * X2)    ;
						}
						
						delTableau(bordure , &k , 2)                ;
						
						triTableau(bordure , &k , 3 , HM_gauche)    ;
						
						if(Y1 != Y2)
						{
							x1    =     0      ;
							
							triTableau(bordure , &k , 2 , MB_gauche)    ;
						}
					}
				}
				else
				{
					if(X2 < 0)
					{
						HB_gauche  =   Y0 - (((float)Dy2 / Dx2) * X0)    ;
						
						if(Y2 == Y1)
						{
							MB_gauche  =   Y1   ;
						}
						else
						{
							MB_gauche  =   Y1 - (((float)Dy1 / Dx1) * X1)    ;
						}
					
						if(baleillage == 1)
						{
							dis        =    HB_gauche    ;
							
							int   Dis  =  -(X0 + ((MB_gauche-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_gauche-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_gauche-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVx)-DV/2)%DV)>0))   ;
							
							Yv0  =  YV0 + ((HB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((HB_gauche-Y0)*resteVy)-DV/2)%DV)>0))   ;
								
							Xv1  =  XV0 + ((MB_gauche-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_gauche-Y0)*resteVx)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
							Yv1  =  YV0 + ((MB_gauche-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_gauche-Y0)*resteVy)-DV/2)/DV) + (((((MB_gauche-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HB_gauche - MB_gauche ;
							
							if(!Dv)  Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HB_gauche)    ;
							
							triTableau(bordure , &k , 6 , MB_gauche)    ;
						}
						else
						{
							triTableau(bordure , &k , 3 , MB_gauche)    ;
							
							triTableau(bordure , &k , 6 , HB_gauche)    ;
						}
					}
				}
			}
			
			
			if(X0 >= RES_HORIZ)
			{
				if(X1 >= RES_HORIZ)
				{
					HB_droite  =   Y2 + (((float)Dy2 / Dx2) * (RES_HORIZ-X2))    ;
					
					if(Y1 == Y2)
					{
						MB_droite  =   Y1   ;
					}
					else
					{
						MB_droite  =   Y2 + (((float)Dy1 / Dx1) * (RES_HORIZ-X2))    ;
					}
					
					if(baleillage == 1)
					{
						y    =  HB_droite     ;
						
						xV   =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						yV   =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 4)                ;
						delTableau(bordure , &k , 2)                ;
						
						triTableau(bordure , &k , 3 , HB_droite)    ;
						
						x2    =     RES_HORIZ - 1      ;
						
						triTableau(bordure , &k , 4 , HB_droite)    ;
						
						if(Y1 != Y2)
						{
							x1    =     RES_HORIZ - 1      ;
							
							triTableau(bordure , &k , 2 , MB_droite)    ;
						}
					}
					else
					{
						dis        =    MB_droite      ;
						y          =    MB_droite      ;
						
						int   Dis  =  -RES_HORIZ + (X0 + ((MB_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv0  =  XV0 + ((MB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_droite-Y0)*resteVx)-DV/2)/DV) + (((((MB_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv0  =  YV0 + ((MB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_droite-Y0)*resteVy)-DV/2)/DV) + (((((MB_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Xv1  =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						Yv1  =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						Dvx   =   Xv1 - Xv0   ;
						Dvy   =   Yv1 - Yv0   ;
						
						Dv    =   HB_droite - MB_droite ;
						
						if(!Dv)  Dv = 1    ;
						
						Dv     =  abs(Dv)               ;
						
						restevx    =     abs(Dvx) % Dv  ;
						erreurvx   =     Dv / 2         ;
						distvx     =     Dvx / Dv       ;
						Distvx                          ;
						
						if (Xv0 < Xv1)
						{
							Distvx   =  distvx + 1   ;
						}
						else
						{
							Distvx   =  distvx - 1   ;
						}
						
						restevy    =     abs(Dvy) % Dv  ;
						erreurvy   =     Dv / 2         ;
						distvy     =     Dvy / Dv       ;
						Distvy                          ;
						
						if (Yv0 < Yv1)
						{
							Distvy   =  distvy + 1   ;
						}
						else
						{
							Distvy   =  distvy - 1   ;
						}
						
						xv    =     Xv0    ;
						yv    =     Yv0    ;
						
						x1    =     RES_HORIZ-1      ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 4)                ;
						
						triTableau(bordure , &k , 5 , MB_droite)    ;
						
						xV   =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))     ;
						
						yV   =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))     ;
						
						x2    =     RES_HORIZ-1      ;
						
						triTableau(bordure , &k , 4 , HB_droite)    ;
					}
				}
				else
				{
					if(X2 >= RES_HORIZ)
					{
						if(Y0 == Y1)
						{
							HM_droite  =   Y0   ;
						}
						else
						{
							HM_droite  =   Y1 + (((float)Dy0 / Dx0) * (RES_HORIZ-X1))    ;
						}
						
						if(Y2 == Y1)
						{
							MB_droite  =   Y1   ;
						}
						else
						{
							MB_droite  =   Y1 + (((float)Dy1 / Dx1) * (RES_HORIZ-X1))    ;
						}
						
						dis        =  HM_droite      ;
						y          =  HM_droite      ;
						
						int   Dis  =  -RES_HORIZ + (X0 + ((HM_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv0  =  XV0 + ((HM_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_droite-Y0)*resteVx)-DV/2)/DV) + (((((HM_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv0  =  YV0 + ((HM_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_droite-Y0)*resteVy)-DV/2)/DV) + (((((HM_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Dis  =  -RES_HORIZ + (X0 + ((MB_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
						
						Xv1  =  XV0 + ((MB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_droite-Y0)*resteVx)-DV/2)/DV) + (((((MB_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
						Yv1  =  YV0 + ((MB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_droite-Y0)*resteVy)-DV/2)/DV) + (((((MB_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
						
						Dvx   =   Xv1 - Xv0   ;
						Dvy   =   Yv1 - Yv0   ;
						
						Dv    =   MB_droite - HM_droite ;
						
						if(!Dv)  Dv = 1    ;
						
						Dv     =  abs(Dv)               ;
						
						restevx    =     abs(Dvx) % Dv  ;
						erreurvx   =     Dv / 2         ;
						distvx     =     Dvx / Dv       ;
						Distvx                          ;
						
						if (Xv0 < Xv1)
						{
							Distvx   =  distvx + 1   ;
						}
						else
						{
							Distvx   =  distvx - 1   ;
						}
						
						restevy    =     abs(Dvy) % Dv  ;
						erreurvy   =     Dv / 2         ;
						distvy     =     Dvy / Dv       ;
						Distvy                          ;
						
						if (Yv0 < Yv1)
						{
							Distvy   =  distvy + 1   ;
						}
						else
						{
							Distvy   =  distvy - 1   ;
						}
						
						xv    =     Xv0    ;
						yv    =     Yv0    ;
						
						delTableau(bordure , &k , 1)                ;
						delTableau(bordure , &k , 2)                ;
						delTableau(bordure , &k , 4)                ;
						
						if(Y0 != Y1)
						{
							x0    =     RES_HORIZ - 1    ;
							
							triTableau(bordure , &k , 1 , HM_droite)    ;
						}
						
						triTableau(bordure , &k , 2 , Y1)           ;
						
						triTableau(bordure , &k , 5 , HM_droite)    ;
						
						triTableau(bordure , &k , 6 , MB_droite)    ;
					}
					else
					{
						if(Y0 == Y1)
						{
							HM_droite  =   Y0   ;
						}
						else
						{
							HM_droite  =   Y1 + (((float)Dy0 / Dx0) * (RES_HORIZ-X1))    ;
						}
						
						HB_droite  =   Y2 + (((float)Dy2 / Dx2) * (RES_HORIZ-X2))    ;
						
						if(baleillage == 1)
						{
							y    =  HB_droite      ;
							
							xV   =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))     ;
							
							yV   =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))     ;
							
							delTableau(bordure , &k , 1)                ;
							delTableau(bordure , &k , 2)                ;
							delTableau(bordure , &k , 4)                ;
							
							triTableau(bordure , &k , 3 , HB_droite)    ;
							
							x2    =     RES_HORIZ - 1    ;
							
							triTableau(bordure , &k , 4 , HB_droite)    ;
							
							x0    =     RES_HORIZ - 1    ;
							
							triTableau(bordure , &k , 1 , HM_droite)    ;
							
							triTableau(bordure , &k , 2 , Y1)           ;
						}
						else
						{
							dis        =  HM_droite      ;
							y          =  HM_droite      ;
							
							int   Dis  =  -RES_HORIZ + (X0 + ((HM_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HM_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_droite-Y0)*resteVx)-DV/2)/DV) + (((((HM_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
							
							Yv0  =  YV0 + ((HM_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_droite-Y0)*resteVy)-DV/2)/DV) + (((((HM_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Xv1  =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))    ;
							
							Yv1  =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))    ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HB_droite - HM_droite ;
							
							if(!Dv) Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							delTableau(bordure , &k , 1)                ;
							delTableau(bordure , &k , 2)                ;
							delTableau(bordure , &k , 4)                ;
							
							if(Y0 != Y1)
							{
								x0    =     RES_HORIZ - 1    ;
								
								triTableau(bordure , &k , 1 , HM_droite)    ;
							}
							
							triTableau(bordure , &k , 2 , Y1)    ;
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HM_droite)    ;
							
							x2    =     RES_HORIZ - 1    ;
							
							xV   =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))    ;
							
							yV   =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))     ;
							
							triTableau(bordure , &k , 4 , HB_droite)    ;
						}
					}
				}
			}
			else
			{
				if(X1 >= RES_HORIZ)
				{
					if(X2 >= RES_HORIZ)
					{
						if(Y0 == Y1)
						{
							HM_droite  =   Y0   ;
						}
						else
						{
							HM_droite  =   Y0 + (((float)Dy0 / Dx0) * (RES_HORIZ-X0))    ;
						}
						
						HB_droite  =   Y0 + (((float)Dy2 / Dx2) * (RES_HORIZ-X0))    ;
						
						if(baleillage == 1)
						{
							delTableau(bordure , &k , 2)    ;
							
							triTableau(bordure , &k , 3 , HM_droite)    ;
							
							triTableau(bordure , &k , 6 , HB_droite)    ;
						}
						else
						{
							dis     =    HB_droite     ;
							
							int   Dis  =  -RES_HORIZ + (X0 + ((HM_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((HM_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((HM_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))   ;
							
							Yv0  =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))   ;
							
							Xv1  =  XV0 + ((HM_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HM_droite-Y0)*resteVx)-DV/2)/DV) + (((((HM_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
							
							Yv1  =  YV0 + ((HM_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HM_droite-Y0)*resteVy)-DV/2)/DV) + (((((HM_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HM_droite - HB_droite ;
							
							if(!Dv)  Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							delTableau(bordure , &k , 2)                ;
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HB_droite)    ;
							
							triTableau(bordure , &k , 6 , HM_droite)    ;
						}
					}
					else
					{
						if(Y0 == Y1)
						{
							HM_droite  =   Y0   ;
						}
						else
						{
							HM_droite  =   Y0 + (((float)Dy0 / Dx0) * (RES_HORIZ-X0))    ;
						}
						
						if(Y2 == Y1)
						{
							MB_droite  =   Y1   ;
						}
						else
						{
							MB_droite  =   Y2 + (((float)Dy1 / Dx1) * (RES_HORIZ-X2))    ;
						}
						
						delTableau(bordure , &k , 2)                ;
						
						triTableau(bordure , &k , 3 , HM_droite)    ;
						
						if(Y1 != Y2)
						{
							x1    =     RES_HORIZ - 1      ;
							
							triTableau(bordure , &k , 2 , MB_droite)    ;
						}
					}
				}
				else
				{
					if(X2 >= RES_HORIZ)
					{
						HB_droite  =   Y0 + (((float)Dy2 / Dx2) * (RES_HORIZ-X0))    ;
					
						if(Y2 == Y1)
						{
							MB_droite  =   Y1   ;
						}
						else
						{
							MB_droite  =   Y1 + (((float)Dy1 / Dx1) * (RES_HORIZ-X1))    ;
						}
						
						if(baleillage == 1)
						{
							triTableau(bordure , &k , 3 , MB_droite)    ;
							
							triTableau(bordure , &k , 6 , HB_droite)    ;
						}
						else
						{
							dis     =    HB_droite     ;
							
							int   Dis  =  -RES_HORIZ + (X0 + ((MB_droite-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((MB_droite-Y0)*reste2)-Dy2/2)/Dy2) + (((((MB_droite-Y0)*reste2)-Dy2/2)%Dy2)>0)))   ;
							
							Xv0  =  XV0 + ((HB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((HB_droite-Y0)*resteVx)-DV/2)/DV) + (((((HB_droite-Y0)*resteVx)-DV/2)%DV)>0))   ;
							
							Yv0  =  YV0 + ((HB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((HB_droite-Y0)*resteVy)-DV/2)/DV) + (((((HB_droite-Y0)*resteVy)-DV/2)%DV)>0))   ;
								
							Xv1  =  XV0 + ((MB_droite-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((MB_droite-Y0)*resteVx)-DV/2)/DV) + (((((MB_droite-Y0)*resteVx)-DV/2)%DV)>0))  + (Dis*distHx) + ((XH0<XH1)?1:-1) * ((((Dis*resteHx)-DH/2)/DH) + ((((Dis*resteHx)-DH/2)%DH)>0))   ;
						
							Yv1  =  YV0 + ((MB_droite-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((MB_droite-Y0)*resteVy)-DV/2)/DV) + (((((MB_droite-Y0)*resteVy)-DV/2)%DV)>0))  + (Dis*distHy) + ((YH0<YH1)?1:-1) * ((((Dis*resteHy)-DH/2)/DH) + ((((Dis*resteHy)-DH/2)%DH)>0))   ;
							
							Dvx   =   Xv1 - Xv0   ;
							Dvy   =   Yv1 - Yv0   ;
							
							Dv    =   HB_droite - MB_droite ;
							
							if(!Dv)  Dv = 1    ;
							
							Dv     =  abs(Dv)               ;
							
							restevx    =     abs(Dvx) % Dv  ;
							erreurvx   =     Dv / 2         ;
							distvx     =     Dvx / Dv       ;
							Distvx                          ;
							
							if (Xv0 < Xv1)
							{
								Distvx   =  distvx + 1   ;
							}
							else
							{
								Distvx   =  distvx - 1   ;
							}
							
							restevy    =     abs(Dvy) % Dv  ;
							erreurvy   =     Dv / 2         ;
							distvy     =     Dvy / Dv       ;
							Distvy                          ;
							
							if (Yv0 < Yv1)
							{
								Distvy   =  distvy + 1   ;
							}
							else
							{
								Distvy   =  distvy - 1   ;
							}
							
							xv    =     Xv0    ;
							yv    =     Yv0    ;
							
							triTableau(bordure , &k , 5 , HB_droite)    ;
							
							triTableau(bordure , &k , 6 , MB_droite)    ;
						}
					}
				}
			}
			
			///////---------------------------------------------------------------//////////
			
			
			if(Y2 >= RES_VERT)
			{
				triTableau(bordure , &k , 6 , RES_VERT)    ;
			}
			
			///////---------------------------------------------------------------//////////
			
			
			if(y < 0)
			{
				y     =   0       ;
				k     =   0       ;
				
				while(bordure[k][1] < 0)
				{
					switch(bordure[k][0])
					{
						case  1   :     x0    =   X0 + ((-Y0)*pas_C_0) + ((X0<X1)?1:-1) * (((((-Y0)*reste0)-Dy0/2)/Dy0) + (((((-Y0)*reste0)-Dy0/2)%Dy0)>0))   ;
								break                   ;
							
						case  2   :     x1    =   X1 + ((-Y1)*pas_C_1) + ((X1<X2)?1:-1) * (((((-Y1)*reste1)-Dy1/2)/Dy1) + (((((-Y1)*reste1)-Dy1/2)%Dy1)>0))   ;
								break                   ;
							
						case  4   :     x2    =   X0 + ((-Y0)*pas_C_2) + ((X0<X2)?1:-1) * (((((-Y0)*reste2)-Dy2/2)/Dy2) + (((((-Y0)*reste2)-Dy2/2)%Dy2)>0))   ;
								xV    =   XV0 + ((-Y0)*distVx) + ((XV0<XV1)?1:-1) * (((((-Y0)*resteVx)-DV/2)/DV) + (((((-Y0)*resteVx)-DV/2)%DV)>0))     ;
								yV    =   YV0 + ((-Y0)*distVy) + ((YV0<YV1)?1:-1) * (((((-Y0)*resteVy)-DV/2)/DV) + (((((-Y0)*resteVy)-DV/2)%DV)>0))     ;
								break                   ;
							
						case  5   :     xv    =   Xv0 + ((-dis)*distvx) + ((Xv0<Xv1)?1:-1) * (((((-dis)*restevx)-Dv/2)/Dv) + (((((-dis)*restevx)-Dv/2)%Dv)>0))     ;
								yv    =   Yv0 + ((-dis)*distvy) + ((Yv0<Yv1)?1:-1) * (((((-dis)*restevy)-Dv/2)/Dv) + (((((-dis)*restevy)-Dv/2)%Dv)>0))     ;
								break                   ;
							
						default   :     break                   ;
					}
					
					bordure[k][1]    =   0      ;
					k++                         ;
				}
			}
			
			///////---------------------------------------------------------------//////////
			
			
			//printf("%i,%i,%i,%i,%i,%i\n" ,X0,Y0,X1,Y1,X2,Y2)   ;
			int    boucle   =    1   ;
			k               =    0   ;
			
			while(boucle)
			{
				
				//0  : ...
				//1  : initialisation Y0_Y1
				//2  : initialisation Y1_Y2
				//3  : initialisation bord 0
				//4  : initialisation YV0_YV1
				//5  : initialisation Yv0_Yv1
				//6  : fin de la boucle
				
				switch(bordure[k][0])
				{
					case  1   :     x4        =   x0        ;
							erreur4   =   erreur0   ;
							Dy4       =   Dy0       ;
							reste4    =   reste0    ;
							pas_C_4   =   pas_C_0   ;
							pas_L_4   =   pas_L_0   ;
							break                   ;
							
					case  2   :     x4        =   x1        ;//printf("x1=%i\n",x1);
							erreur4   =   erreur1   ;
							Dy4       =   Dy1       ;
							reste4    =   reste1    ;
							pas_C_4   =   pas_C_1   ;
							pas_L_4   =   pas_L_1   ;
							break                   ;
							
					case  3   :     x4        =   (baleillage==1) ? RES_HORIZ-1 : 0   ;
							erreur4   =   0         ;
							Dy4       =   0         ;
							reste4    =   0         ;
							pas_C_4   =   0         ;
							pas_L_4   =   0         ;
							break                   ;
							
					case  4   :     x3        =   x2        ;
							erreur3   =   erreur2   ;
							Dy3       =   Dy2       ;
							reste3    =   reste2    ;
							pas_C_3   =   pas_C_2   ;
							pas_L_3   =   pas_L_2   ;
							
							xVV       =   xV        ;
							yVV       =   yV        ;
							erreurVVx =   erreurVx  ;
							erreurVVy =   erreurVy  ;
							DVV       =   DV        ;
							resteVVx  =   resteVx   ;
							resteVVy  =   resteVy   ;
							distVVx   =   distVx    ;
							distVVy   =   distVy    ;
							DistVVx   =   DistVx    ;
							DistVVy   =   DistVy    ;
							break                   ;
							
					case  5   :     x3        =   (baleillage==1) ? 0 : RES_HORIZ-1   ;
							erreur3   =   0         ;
							Dy3       =   0         ;
							reste3    =   0         ;
							pas_C_3   =   0         ;
							pas_L_3   =   0         ;
							
							xVV       =   xv        ;
							yVV       =   yv        ;
							erreurVVx =   erreurvx  ;
							erreurVVy =   erreurvy  ;
							DVV       =   Dv        ;
							resteVVx  =   restevx   ;
							resteVVy  =   restevy   ;
							distVVx   =   distvx    ;
							distVVy   =   distvy    ;
							DistVVx   =   Distvx    ;
							DistVVy   =   Distvy    ;
							break                   ;
							
					case  6   :     boucle    = 0           ;
							continue                ;
							break                   ;
							
					default   :     boucle    = 0           ;
							continue                ;
				}
				
				k++       ;
				
				for(; y < bordure[k][1] ; y++)
				{
					
					//setPixel(x4 , y , SDL_MapRGB(affichage->format , 5 , 200 , 128))   ;
					//setPixel(x3 , y , getPixel(xVV , yVV , image))                       ;
					
					xH    =    xVV   ;
					yH    =    yVV   ;
					
					erreurHx   =     DH / 2       ;
					erreurHy   =     DH / 2       ;
					
					if(baleillage == 1)
					{
						for(j = x3 ; j <= x4 ; j++)
						{//	printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH , cube->texture))      ;
							
							erreurHx   -=  resteHx       ;
							erreurHy   -=  resteHy       ;
							
							if(erreurHx < 0)
							{
								xH        +=  DistHx  ;
								erreurHx  +=  DH      ;
							}
							else
							{
								xH        +=  distHx  ;
							}
							
							if(erreurHy < 0)
							{
								yH        +=  DistHy   ;
								erreurHy  +=  DH       ;
							}
							else
							{
								yH        +=  distHy   ;
							}
						}
					}
					else
					{
						for(j = x3 ; j >= x4 ; j--)
						{//	printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH , cube->texture))      ;
							
							erreurHx   -=  resteHx       ;
							erreurHy   -=  resteHy       ;
							
							if(erreurHx < 0)
							{
								xH        +=  DistHx  ;
								erreurHx  +=  DH      ;
							}
							else
							{
								xH        +=  distHx  ;
							}
							
							if(erreurHy < 0)
							{
								yH        +=  DistHy   ;
								erreurHy  +=  DH       ;
							}
							else
							{
								yH        +=  distHy   ;
							}
						}
					}
					
					/////--------------------------------------------------------------/////
					
					erreur3   -=  reste3       ;
					erreur4   -=  reste4       ;
					
					if(erreur3 < 0)
					{
						x3       += pas_L_3   ;
						erreur3  += Dy3       ;
					}
					else
					{
						x3       += pas_C_3   ;
					}
					
					if(erreur4 < 0)
					{
						x4       += pas_L_4   ;
						erreur4  += Dy4       ;
					}
					else
					{
						x4       += pas_C_4   ;
					}
					
					/////--------------------------------------------------------------/////
					
					erreurVVx   -=  resteVVx       ;
					erreurVVy   -=  resteVVy       ;
					
					if(erreurVVx < 0)
					{
						xVV        +=  DistVVx  ;
						erreurVVx  +=  DVV      ;
					}
					else
					{
						xVV        +=  distVVx  ;
					}
					
					if(erreurVVy < 0)
					{
						yVV        +=  DistVVy   ;
						erreurVVy  +=  DVV       ;
					}
					else
					{
						yVV        +=  distVVy   ;
					}
				}
			}
			
			//ligne( X0 , Y0 , X1 , Y1 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;
			//ligne( X1 , Y1 , X2 , Y2 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;
			//ligne( X0 , Y0 , X2 , Y2 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;
		}
	}
	
	return   ;
}


inline   void   triTableau(int tableau[][2] , int * position , int action , int y)
{
	int    i  =   *position   ;
	
	tableau[*position][0]   =   action     ;
	tableau[*position][1]   =   y          ;
	
	while((tableau[i][1] < tableau[i-1][1]) && (i != 0))
	{
		swap((int *)&tableau[i][0] , (int *)&tableau[i-1][0])   ;
		swap((int *)&tableau[i][1] , (int *)&tableau[i-1][1])   ;
		i--                                   ;
	}
	
	(*position)++    ;
	
	return     ;
}

inline   void   delTableau(int tableau[][2] , int * taille , int action)
{
	int    i       ;
	
	for(i = 0 ; i < (*taille) ; i++)
	{
		if(tableau[i][0] == action)
		{
			swap((int *)&tableau[i][0] , (int *)&tableau[i+1][0])   ;
			swap((int *)&tableau[i][1] , (int *)&tableau[i+1][1])   ;
		}
	}
	
	if(tableau[i][0] == action)
	{
		tableau[i][0]   =   0     ;
		tableau[i][1]   =   0     ;
		(*taille)--               ;
	}
	return     ;
}

inline  void    translation(Objet * objet , int Dx , int Dy , int Dz)
{
	int   i     ;
	
	objet->centre.x    +=   Dx     ;
	objet->centre.y    +=   Dy     ;
	objet->centre.z    +=   Dz     ;
	
	for(i = 0 ; i < objet->nbrePts ; i++)
	{
		objet->points[i].x    +=  Dx     ;
		objet->points[i].y    +=  Dy     ;
		objet->points[i].z    +=  Dz     ;
	}
	
	return     ;
}

inline  void    changementEchell_rotation(Objet * objet)
{
	int    i  =  0      ;
	
	////---------------------------------rotation--------------------------------------//////
	
	for(i = 0 ; i < objet->nbrePts ; i++)
	{
		
		// par raport a l'axe Z
		objet->points[i].x   =    (objet->ptsOrg[i].x * cos(objet->angleZ)) - (objet->ptsOrg[i].y * sin(objet->angleZ))  ;
		objet->points[i].y   =    (objet->ptsOrg[i].x * sin(objet->angleZ)) + (objet->ptsOrg[i].y * cos(objet->angleZ))  ;
		
		// par raport a l'axe Y
		objet->points[i].z   =    (objet->ptsOrg[i].z * cos(objet->angleY)) - (objet->points[i].x * sin(objet->angleY))   ;
		objet->points[i].x   =    (objet->ptsOrg[i].z * sin(objet->angleY)) + (objet->points[i].x * cos(objet->angleY))   ;
		
		int    z   =  objet->points[i].z   ;
		
		// par raport a l'axe X
		objet->points[i].z   =    (objet->points[i].y * sin(objet->angleX)) + (z * cos(objet->angleX))   ;
		objet->points[i].y   =    (objet->points[i].y * cos(objet->angleX)) - (z * sin(objet->angleX))   ;
		
	}
	
	for(i = 0 ; i < objet->nbreFace ; i++)
	{
		
		// par raport a l'axe Z
		objet->normale[i].x   =    (objet->nrmOrg[i].x * cos(objet->angleZ)) - (objet->nrmOrg[i].y * sin(objet->angleZ))  ;
		objet->normale[i].y   =    (objet->nrmOrg[i].x * sin(objet->angleZ)) + (objet->nrmOrg[i].y * cos(objet->angleZ))  ;
		
		// par raport a l'axe Y
		objet->normale[i].z   =    (objet->nrmOrg[i].z * cos(objet->angleY)) - (objet->normale[i].x * sin(objet->angleY))   ;
		objet->normale[i].x   =    (objet->nrmOrg[i].z * sin(objet->angleY)) + (objet->normale[i].x * cos(objet->angleY))   ;
		
		int    z   =  objet->normale[i].z   ;
		
		// par raport a l'axe X
		objet->normale[i].z   =    (objet->normale[i].y * sin(objet->angleX)) + (z * cos(objet->angleX))   ;
		objet->normale[i].y   =    (objet->normale[i].y * cos(objet->angleX)) - (z * sin(objet->angleX))   ;
		
	}
	
	/////-----------------------------chagement d'echelle--------------------------------/////
	
	for(i = 0 ;  i < objet->nbrePts ; i++)
	{
		objet->points[i].x    =  (int)(objet->points[i].x * objet->echell) + objet->centre.x    ;
		objet->points[i].y    =  (int)(objet->points[i].y * objet->echell) + objet->centre.y    ;
		objet->points[i].z    =  (int)(objet->points[i].z * objet->echell) + objet->centre.z    ;
	}
	
	//objet->radius   *=  lamda      ;
	
	return     ;
}

void    animationRadar(int X , int Y , float R)
{
	static      int   i    =   0    ;
	
	SDL_Rect    rectSrc    ;
	SDL_Rect    rectDst    ;
	
	rectSrc.x    =    (int)((R-0.5f)/0.23333f) * 40     ;
	rectSrc.y    =    0    ;
	
	rectSrc.h    =    40   ;
	rectSrc.w    =    40   ;
	
	rectDst.x    =    55  + (X / 100)  ;
	rectDst.y    =    252 - (Y / 100)  ;
	
	SDL_BlitSurface(radar, &rectSrc, affichage, &rectDst)     ;
	
	rectSrc.x    =    ((i++)%60)/20 * 40  ;
	rectSrc.y    =    40    ;
	
	rectDst.x    =    55  + (X / 100)  ;
	rectDst.y    =    252 - (Y / 100)  ;
	
	SDL_BlitSurface(radar, &rectSrc, affichage, &rectDst)     ;
	
	return   ;
}

void    animationTexte(void)
{
	static      int   i    =   0    ;
	
	SDL_Rect    rectSrc    ;
	SDL_Rect    rectDst    ;
	
	rectSrc.x    =    (i++)%825   ;
	rectSrc.y    =    0           ;
	
	rectSrc.h    =    60   ;
	rectSrc.w    =    800  ;
	
	rectDst.x    =    0    ;
	rectDst.y    =    0    ;
	
	SDL_BlitSurface(texte, &rectSrc, affichage, &rectDst)     ;
	
	return      ;
}

/*

# bool CObjet3D::_LoadOBJ(const char *filename) // charge un objet Wavefront 3D (.obj)
# {
# FILE *fp = fopen(filename,"rb");
# if(!fp) return false;
# DEBUGLOG1("Chargement du fichier WAVEFRONT %s",filename)
#
# SFace3D face;
# SGroup3D grp;
# vec3 pt3D;
# vec2 pt2D;
# char *buf,buftmp[64];
# long i,v,g; // compteur pour le stockage des données lors de la seconde passe
# unsigned int lenbuf;
#
# grp.Actif2D = true;
# grp.Actif3D = true;
# while (!feof(fp)) {
# buf = ReadLine(fp);
# lenbuf = strlen((const char *)buf);
# if (lenbuf > 0) {
# sscanf(buf,"%s",buftmp);
# if(!strcmp((const char *)buftmp,"#")) { // on a trouvé un commentaire, on passe
# } else if(!strcmp((const char *)buftmp,"mtllib")) { // on a trouvé un matérial à charger ?
# materials->Load(&buf[7]); // on charge le fichier associé
# } else if(!strcmp((const char *)buftmp,"v")) { // on a trouvé une vertice ?
# sscanf(&buf[2],"%f%f%f",&pt3D.x,&pt3D.y,&pt3D.z);
# this->_pVertices.push_back(pt3D);
# } else if(!strcmp((const char *)buftmp,"vt")) { // on a trouvé une coordonnée de texture ?
# sscanf(&buf[2],"%f%f",&pt2D.x,&pt2D.y);
# this->_pTexCoords.push_back(pt2D);
# } else if(!strcmp((const char *)buftmp,"vn")) { // on a trouvé une normale ?
# sscanf(&buf[2],"%f%f%f",&pt3D.x,&pt3D.y,&pt3D.z);
# this->_pNormals.push_back(pt3D);
# } else if(!strcmp((const char *)buftmp,"g")) { // on a trouvé un groupe ?
# grp.Name = &buf[2];
# grp.Material = 0;
# this->_pGroups.push_back(grp);
# } else if(!strcmp((const char *)buftmp,"usemtl")) { // on a trouvé un matérial à utiliser ?
# if(this->_pGroups.size() <= 0) {
# grp.Name = "No Name";
# grp.Material = 0;
# this->_pGroups.push_back(grp);
# g = 0;
# } else {
# g = this->_pGroups.size() - 1;
# }
# this->_pGroups[g].Material = materials->GetId(&buf[7]); // on récupère son id
# } else if(!strcmp((const char *)buftmp,"f")) { // on a trouvé une face ?
# if(this->_pGroups.size() <= 0) {
# grp.Name = "No Name";
# grp.Material = 0;
# this->_pGroups.push_back(grp);
# g = 0;
# } else {
# g = this->_pGroups.size() - 1;
# }
# for(i=0; (buf[i] < '0') || (buf[i] > '9') ;i++); // on se positionne à la première valeur
# for(v=0; v < 3 ;v++) { // triangles donc composés de 3 vertices
# face.Vertices[v] = 0;
# for(; (buf[i] >= '0') && (buf[i] <= '9') ;i++) { // on la récupère
# face.Vertices[v] *= 10; // première vertice
# face.Vertices[v] += buf[i]-0x30; // 0x30 est la valeur ascii du caractère '0'
# }
# face.Vertices[v]--; // indice n'est pas de 1 à nbFaces mais de 0 à nbFaces-1
# NEXT_INDICE; // on se positionne à la valeur suivante
# face.TexCoords[v] = 0;
# for(; (buf[i] >= '0') && (buf[i] <= '9') ;i++) { // on la récupère
# face.TexCoords[v] *= 10; // première coordonnée de texture
# face.TexCoords[v] += buf[i]-0x30;
# }
# face.TexCoords[v]--; // indice n'est pas de 1 à nbFaces mais de 0 à nbFaces-1
# NEXT_INDICE; // ect ... il y a 9 indices à récupérer
# face.Normals[v] = 0;
# for(; (buf[i] >= '0') && (buf[i] <= '9') ;i++) {
# face.Normals[v] *= 10; // première normale
# face.Normals[v] += buf[i]-0x30;
# }
# face.Normals[v]--; // indice n'est pas de 1 à nbFaces mais de 0 à nbFaces-1
# if(v < 2) NEXT_INDICE;
# }
# this->_pGroups[g].pFaces.push_back(face); // on enregistre la face récupérée
# }
# }
# delete[] buf;
# }
# fclose(fp);
# DEBUGLOG2("Composition finale de l'objet : %ul vertices\n %ul groupes",this->_pVertices.size(),this->_pGroups.size())
# DEBUGLOG2(" %ul faces\n %ul materials",this->GetNumFaces(),this->GetNumMaterials())
# return true;
# } 

*/
