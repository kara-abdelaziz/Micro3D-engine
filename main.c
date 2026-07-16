#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

/////////////////////////// Mixer migration from SDL1.2 to SDL3 ///////////////////////////

typedef     MIX_Audio Mix_Chunk  ;       // In the new Mixer, everything is MIX_Audio
typedef     MIX_Audio Mix_Music  ;

#define Mix_LoadWAV(path)                 MIX_LoadAudio(gMixer, path, true)
#define Mix_LoadMUS(path)                 MIX_LoadAudio(gMixer, path, true)
#define Mix_FreeChunk(chunk)              MIX_DestroyAudio(chunk)
#define Mix_FreeMusic(music)              MIX_DestroyAudio(music)

#define Mix_PlayChannel(ch, chunk, loop)  Mix_PlayChannel_Bridge(ch, chunk, loop)
#define Mix_PlayMusic(music, loop)        Mix_PlayChannel_Bridge(0, music, loop)

// Map the volume/pause for the track
#define Mix_Pause(ch)                     MIX_PauseTrack(gTracks[ch])
#define Mix_Resume(ch)                    MIX_ResumeTrack(gTracks[ch])
#define Mix_HaltChannel(ch)               MIX_StopTrack(gTracks[ch], 0)

#define MIX_MAX_VOLUME 128                // The old SDL 1.2/2.0 maximum volume

#define Mix_VolumeMusic(vol)              MIX_SetTrackGain(gTracks[0], (float)(vol) / 128.0f)
#define Mix_Volume(ch, vol)               MIX_SetTrackGain(gTracks[ch], (float)(vol) / 128.0f)

#define Mix_FadeOutMusic(ms)              MIX_StopTrack(gTracks[0], ms)

#define SDL_MapRGB(fmt, r, g, b)          SDL_MapRGB(SDL_GetPixelFormatDetails(fmt), NULL, r, g, b)

///////////////////////////////////////////////////////////////////////////////////////////

#define   PI 3.14159265358979323846

#define   NBRE_POINT_MAX       5000
#define   NBRE_SEG_MAX         6000
#define   NBRE_OBJET_MAX        100
#define   NBRE_FACE_MAX        2000
#define   NBRE_FACE_MAX_SCENE  3000

#define   DISTANCE_FOCAL     512
#define   RES_VERT           600
#define   RES_HORIZ          800
#define   RES_VERT_DIV_2     300
#define   RES_HORIZ_DIV_2    400

#define   FRAMES_PER_SECOND         60 

////-------------------------variables globales-------------------------------------/////

// The "affichage" in SDL 1.2 was the screen.
// In SDL3, "affichage" is a CPU buffer we create manually.
SDL_Window*   window      = NULL;
SDL_Renderer* renderer    = NULL;
SDL_Surface*  affichage   = NULL; // This is your CPU pixel buffer
SDL_Texture*  screenTex   = NULL; // This is the "bridge" to the GPU

SDL_Surface*   arrierePlan   =   NULL   ;
SDL_Surface*   message       =   NULL   ;
SDL_Surface*   bouttons      =   NULL   ;
SDL_Surface*   paneau        =   NULL   ;
SDL_Surface*   map           =   NULL   ;
SDL_Surface*   entete        =   NULL   ;
SDL_Surface*   grid          =   NULL   ;
SDL_Surface*   radar         =   NULL   ;
SDL_Surface*   texte         =   NULL   ;

MIX_Mixer*     gMixer        =   NULL   ;
MIX_Track*     gTracks[5]               ;
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

typedef  struct  face
{
	Point*         vertices[3]  ;
	Point          normale      ;
	Point          nrmOrg       ;
	int            uv[3][2]     ;
	SDL_Surface*   texture      ;
}  Face  ;

typedef  struct  objet
{
	int     nbrePts                     ;
	Point   points[NBRE_POINT_MAX]      ;
	Point   ptsOrg[NBRE_POINT_MAX]      ;

	int     nbreSegment                 ;
	Point*  segments[NBRE_SEG_MAX][2]   ;

	int     nbreFace                    ;
	Face    faces[NBRE_FACE_MAX]        ;
	SDL_Surface*   texture              ;
	
	Point   centre             ;
	float   angleX             ;
	float   angleY             ;
	float   angleZ             ;
	float   echell             ;

	int     radius             ;
}  Objet  ;

////-------------------------------variables globales-------------------------------------/////

void    initialisation(void)    ;
void    initSDL(void)           ;
void    attendreTouche(void)    ;
void    dessinerEtoiles(void)   ;
void    dessinerLignes(void)    ;
void    dessinEnv2D(void)       ;
void    displayScene()          ;
void    loadScene()             ;
void    cleanUp()               ;
void    loadCube(Objet*  cube)  ;
void    afficheObjetMesh(Objet*  mesh)    ;
static  inline  void    translation(Objet * objet , int Dx , int Dy , int Dz)        ;
static  inline  void    changementEchell_rotation(Objet * objet)                     ;
static  inline  void    swap(int * a , int * b)                                      ;
static  inline  void    Mix_PlayChannel_Bridge(int ch, MIX_Audio* audio, int loops)  ;
void    ligne(int x0, int y0, int x1, int y1, Uint32  couleur)    ;
void    animationRadar(int X , int Y , float R)                   ;
void    animationTexte(void)                                      ;
static  inline  void     setPixel(int  X , int  Y , Uint32  couleur)      ;
static  inline  Uint32   getPixel(int  X , int  Y , SDL_Surface*  image)  ;
static  inline  void     triTableau(int tableau[][2] , int * position , int action , int y)    ;
static  inline  void     delTableau(int tableau[][2] , int * taille , int action)              ;
SDL_Surface*     chargerImage(const  char*  file)          ;
bool      loadOBJfile(const  char*  path, Objet*  objet)   ;
Point     calculateFaceNormal(Point* nrm, Point* v1, Point* v2, Point* v3)     ;
void      chargementFichirs()        ;
bool      Mix_OpenAudio()            ;
void      painterAlgorithmSort()     ;

////------------------------------Static program variables-----------------------------------/////

Face*        faces[NBRE_FACE_MAX_SCENE]    ;
int          nbreFaceScene      =   0      ;

Objet*       allObjet[NBRE_OBJET_MAX]      ;
int          nbreOjectScene     =   0      ;

Objet*       controlledPlayer   =   NULL   ;

////--------------------------------Fonction principale-------------------------------------/////

int   main(int  argc , char**  argv)
{
	int         quitter  =  1   ;
	
	//Uint8 *     keystates     ;
	SDL_Rect    rectSrc         ;
	SDL_Rect    rectDst         ;

	int         FPS  =   0      ;
	int         i    =   1      ;
	int         temps           ;
	int         Dx   =   0      ;
	int         Dy   =   0      ;
	int         Dz   =   0      ;

	initialisation()            ;
	
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

		const bool *keystates = SDL_GetKeyboardState(NULL)  ;
		
		
		int   sonTransl , sonRotation , sonEchell ;
		
		/////-------------affichage de toute la scène par le changement du buffer------------///////
		
		dessinEnv2D()   ;

		changementEchell_rotation(allObjet[1])      ;
		allObjet[1]->angleX  +=  0.05               ;
		allObjet[1]->angleY  +=  0.05               ;
		allObjet[1]->angleZ  +=  0.05               ;
		
		changementEchell_rotation(controlledPlayer)     ;
		translation(controlledPlayer, Dx, Dy, Dz)	    ;

		Dx   =   0      ;
		Dy   =   0      ;
		Dz   =   0      ;
		
		//afficheObjetMesh(controlledPlayer)    ;
		displayScene()  ;
		
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
		animationRadar(controlledPlayer->centre.x , controlledPlayer->centre.z , controlledPlayer->echell)   ;
		
		rectSrc.h   =    40      ;
		rectSrc.w   =    40      ;
		
		if(keystates[SDL_SCANCODE_UP])
		{
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleX     -=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_DOWN])
		{
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleX     +=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_RIGHT])
        {
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleY     -=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_LEFT])
		{
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleY     +=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_PAGEUP])
		{
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleZ     -=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_PAGEDOWN])
		{
			//changementEchell_rotation(&cube)     ;
			controlledPlayer->angleZ     +=   0.05            ;
			
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
		
		if(keystates[SDL_SCANCODE_W] && (controlledPlayer->centre.z < 12100))
		{
			//translation(&cube , 0 , 0 ,  5)    ;
			Dz    +=   5           ;
			
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
		
		if(keystates[SDL_SCANCODE_S] && (controlledPlayer->centre.z > 500))
		{
			//translation(&cube , 0 , 0 , -5)    ;
			Dz    -=   5           ;
			
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
		
		if(keystates[SDL_SCANCODE_D] && (controlledPlayer->centre.x < 4500))
		{
			//translation(&cube ,  5 , 0 , 0)    ;
			Dx    +=   5           ;
			
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
		
		if(keystates[SDL_SCANCODE_A] && (controlledPlayer->centre.x > -4500))
        {
			//translation(&cube , -5 , 0 , 0)    ;
			Dx    -=   5           ;
			
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
		
		if(keystates[SDL_SCANCODE_Q] && (controlledPlayer->centre.y > -4500))
		{
			//translation(&cube , 0 , -5 , 0)    ;
			Dy    -=   5           ;
			
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
		
		if(keystates[SDL_SCANCODE_E] && (controlledPlayer->centre.y < 4500))
        	{
			//translation(&cube , 0 ,  5 , 0)    ;
			Dy    +=   5           ;			
			
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
		
		if(keystates[SDL_SCANCODE_KP_PLUS] && (controlledPlayer->echell < 2.0f))
		{
			controlledPlayer->echell     +=   0.002        ;
			//changementEchell_rotation(&cube)  ;
			
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
		
		if(keystates[SDL_SCANCODE_KP_MINUS] && (controlledPlayer->echell > 0.5f))
        {
			controlledPlayer->echell     -=   0.002        ;
			//changementEchell_rotation(&cube)  ;
			
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
		
		if(keystates[SDL_SCANCODE_ESCAPE])
        {
			quitter  =   0    ;
		}
		
		if(keystates[SDL_SCANCODE_Q] || keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_E])
		{
			Mix_Resume(1)    ;
		}
		else
		{
			Mix_Pause(1)     ;
		}
		
		if(keystates[SDL_SCANCODE_UP] || keystates[SDL_SCANCODE_DOWN] || keystates[SDL_SCANCODE_LEFT] || keystates[SDL_SCANCODE_RIGHT] || keystates[SDL_SCANCODE_PAGEUP] || keystates[SDL_SCANCODE_PAGEDOWN])
		{
			Mix_Resume(2)    ;
		}
		else
		{
			Mix_Pause(2)     ;
		}
		
		if(keystates[SDL_SCANCODE_KP_PLUS] || keystates[SDL_SCANCODE_KP_MINUS])
		{
			Mix_Resume(3)    ;
		}
		else
		{
			Mix_Pause(3)     ;
		}
		
		while(SDL_PollEvent(&event)) 
		{
    		if (event.type == SDL_EVENT_QUIT) 
			{
        		quitter = 0;
    		}

    		if (event.type == SDL_EVENT_KEY_DOWN) 
			{
        		// For F1-F12 keys, we still use Keycodes (.key.key)
				if(event.key.scancode == SDL_SCANCODE_F1)
				{
					Mix_PlayChannel(4 , sonF1 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F2)
				{
					Mix_PlayChannel(4 , sonF2 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F3)
				{
					Mix_PlayChannel(4 , sonF3 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F4)
				{
					Mix_PlayChannel(4 , sonF4 , 0)     ;
				}
				if(event.key.scancode == SDL_SCANCODE_F5)
				{
					Mix_PlayChannel(4 , sonF5 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F6)
				{
					Mix_PlayChannel(4 , sonF6 , 0)     ;
				}
				
				if(event.key.scancode == SDL_SCANCODE_F7)
				{
					Mix_PlayChannel(4 , sonF7 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F8)
				{
					Mix_PlayChannel(4 , sonF8 , 0)     ;
				}
				if(event.key.scancode == SDL_SCANCODE_F9)
				{
					Mix_PlayChannel(4 , sonF9 , 0)     ;
				}

				if(event.key.scancode == SDL_SCANCODE_F10)
				{
					Mix_PlayChannel(4 , sonF10 , 0)    ;
				}
				
				if(event.key.scancode == SDL_SCANCODE_F11)
				{
					Mix_PlayChannel(4 , sonF11 , 0)    ;
				}

				if(event.key.scancode == SDL_SCANCODE_F12)
				{
					Mix_PlayChannel(4 , sonF12 , 0)    ;
				}
            }
		}

		// 1. Upload the pixels the CPU just calculated to the GPU
		SDL_UpdateTexture(screenTex, NULL, affichage->pixels, affichage->pitch);

		// 2. Clear the GPU's memory (just in case)
		SDL_RenderClear(renderer);

		// 3. Copy our software-rendered image to the screen
		SDL_RenderTexture(renderer, screenTex, NULL, NULL);

		// 4. Show it!
		SDL_RenderPresent(renderer);


		Uint64 now = SDL_GetTicks()     ;
		Uint64 elapsed = now - temps    ;

		if (elapsed < (1000 / FRAMES_PER_SECOND)) 
		{
    		SDL_Delay((1000 / FRAMES_PER_SECOND) - (Uint32)elapsed);
		}

		//printf("FPS = %i\n", (FPS += 1000 / elapsed)/i++)   ;
	}
	
	//attendreTouche()          ;

	/////-----------------------nétoyage avant la ferméture du programme---------------------------///////

	cleanUp()                 ;
	
	return    EXIT_SUCCESS    ;

}


///////-----------------------------------Autres fonctions------------------------------------//////////

void initSDL(void) {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) 
	{
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        exit(EXIT_FAILURE)   ;
    }

    if (!TTF_Init()) 
	{
        SDL_Log("TTF_Init Error: %s", SDL_GetError());
        exit(EXIT_FAILURE)   ;
    }

	if (!Mix_OpenAudio()) 
	{
		SDL_Log("Mix_OpenAudio Error: %s", SDL_GetError())  ;
		exit(EXIT_FAILURE)   ;
	}	

    // SDL3 window creation
    window = SDL_CreateWindow("3D Software Engine", RES_HORIZ, RES_VERT, 0);
    
    // We create a renderer just to copy our CPU pixels to the screen
    renderer = SDL_CreateRenderer(window, NULL);

    // IMPORTANT: This is your raw pixel buffer (equivalent to old SDL_Surface)
    // We create it in RAM so the CPU can access it quickly
    affichage = SDL_CreateSurface(RES_HORIZ, RES_VERT, SDL_PIXELFORMAT_XRGB8888);

    // This texture lives on the GPU. We will copy 'affichage' into it every frame.
    screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, RES_HORIZ, RES_VERT);

    if (!affichage || !screenTex) {
        SDL_Log("Buffer Creation Error: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void   attendreTouche(void)
{
	SDL_Event event             ;

	do
    	SDL_WaitEvent(&event)   ;
	while (event.type != SDL_EVENT_QUIT && event.type != SDL_EVENT_KEY_DOWN)   ;
}

void   dessinerEtoiles(void)
{
	int i     ;
	for (i = 0; i < 100; i++)
	{
		setPixel(rand() % 800, rand() % 600 , SDL_MapRGB(affichage->format,rand() % 128 + 128, rand() % 128 + 128, rand() % 128 + 128))  ;
		//SDL_UpdateRect(affichage, 0, 0, 0, 0)   ;
	}
}

void    dessinerLignes(void)
{
	int i     ;
	for (i = 0; i < 100 ; i++)
	{
		ligne(rand() % 800 , rand() % 600 , rand() % 800 , rand() % 600 , SDL_MapRGB(affichage->format,rand() % 128 + 128 , rand() % 128 + 128, rand() % 128 + 128))  ;
		//SDL_UpdateRect(affichage, 0, 0, 0, 0)     ;
	}
}

void    dessinEnv2D(void)
{
	SDL_BlitSurface(arrierePlan, NULL, affichage, NULL)       ;
	SDL_BlitSurface(message, NULL, affichage, NULL)           ;
}

static inline void setPixel(int X, int Y, Uint32 couleur)
{
    // 1. Check boundaries (Note: use >= because 800 is out of bounds for an 800-wide array)
    if (X < 0 || X >= RES_HORIZ || Y < 0 || Y >= RES_VERT) 
    {
        return; // Exit the function immediately. Do NOT draw, do NOT print.
    }

    // 2. If we reach here, it is safe to write to memory
    Uint32* pixels = (Uint32*)affichage->pixels;
    
    // Use pitch/4 for absolute safety in 32-bit modes
    pixels[(Y * (affichage->pitch / 4)) + X] = couleur;
}

static inline   Uint32    getPixel(int  X , int  Y , SDL_Surface*  image)
{
	 return    *((Uint32*)(image->pixels) + (image->w * Y) + X)    ;
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

static inline   void   swap(int * a , int * b)
{
	*a ^= *b   ;
	*b ^= *a   ;
	*a ^= *b   ;

	return   ;
}


SDL_Surface*     chargerImage(const char* file) 
{
    // 1. Load the image into a temporary surface
    SDL_Surface* tempSurface   =   IMG_Load(file)   ;
    
	if (tempSurface == NULL) 
	{
        SDL_Log("Unable to load image %s! SDL_image Error: %s", file, SDL_GetError())   ;
        return NULL;
    }

    // 2. Convert it to XRGB8888 (32-bit) so it matches our engine math
    SDL_Surface*    optimizedSurface   =    SDL_ConvertSurface(tempSurface, SDL_PIXELFORMAT_ARGB8888)    ;
    SDL_DestroySurface(tempSurface);      // We don't need the original anymore

    if (optimizedSurface != NULL) 
	{
        // 3. Set Color Key (Transparency). In your code, you used Magenta (FF, 0, FF)
        // SDL3 uses SDL_SetSurfaceColorKey. 
        // We map the color using the surface format.
        Uint32   key   =    SDL_MapRGB(optimizedSurface->format, 255, 0, 255)    ;
        SDL_SetSurfaceColorKey(optimizedSurface, true, key)                      ;
		SDL_SetSurfaceBlendMode(optimizedSurface, SDL_BLENDMODE_BLEND)           ;
    }

    return optimizedSurface;
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
	arrierePlan   =  chargerImage("background.png")        ;
	
	music         =  Mix_LoadMUS("ambient_sound.mp3")      ;
	
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
		test   =   2    ;
	}
	
	if(paneau == NULL)
	{
		test   =   3    ;
	}
	
	if(map == NULL)
	{
		test   =   4    ;
	}
	
	if(entete == NULL)
	{ 
		test   =   5    ;
	}
	
	if(grid == NULL)
	{
		test   =   6    ;
	}
	
	if(radar == NULL)
	{
		test   =   7    ;
	}
	
	if(texte == NULL)
	{
		test   =   8    ;
	}
	
	if(music == NULL)
	{
		test   =   9    ;
	}
	
	if(transl == NULL)
	{
		test   =  10    ;
	}
	
	if(rotation == NULL)
	{
		test   =  11    ;
	}
	
	if(echell == NULL)
	{
		test   =  12    ;
	}
	
	if(font == NULL)
	{
		test   =  13    ;
	}
	
	if(sonF1 == NULL)
	{
		test   =  14    ;
	}
	
	if(sonF2 == NULL)
	{
		test   =  15    ;
	}
	
	if(sonF3 == NULL)
	{
		test   =  16    ;
	}
	
	if(sonF4 == NULL)
	{
		test   =  17    ;
	}
	
	if(sonF5 == NULL)
	{
		test   =  18    ;
	}
	
	if(sonF6 == NULL)
	{
		test   =  19    ;
	}
	
	if(sonF7 == NULL)
	{
		test   =  20    ;
	}
	
	if(sonF8 == NULL)
	{
		test   =  21    ;
	}
	
	if(sonF9 == NULL)
	{
		test   =  22    ;
	}
	
	if(sonF10 == NULL)
	{
		test   =  23    ;
	}
	
	if(sonF11 == NULL)
	{
		test   =  24    ;
	}
	
	if(sonF12 == NULL)
	{
		test   =  25    ;
	}
	
	if(test)
	{
		fprintf(stderr, "Erreur lors du chargement des fichiers. Test value: %i\n", test)  ;		
		exit(EXIT_FAILURE)         ;
	}
	
	//SDL_SetAlpha(grid, SDL_SRCALPHA | SDL_RLEACCEL, 128)      ;
	SDL_SetSurfaceAlphaMod(grid, 128)                           ;
	SDL_SetSurfaceBlendMode(grid, SDL_BLENDMODE_BLEND)          ;
	//SDL_SetAlpha(radar, SDL_SRCALPHA | SDL_RLEACCEL, 128)     ;
	
	//message = TTF_RenderText_Solid( font, "Test pour sdl_ttf", textColor )    ;
	
	return   ;
}

void    cleanUp()
{
	for(int i = 0; i < nbreOjectScene; i++)
	{
		free(allObjet[i])   ;
	}	
	
	SDL_DestroySurface(arrierePlan)    ;
	SDL_DestroySurface(message)        ;
	SDL_DestroySurface(bouttons)       ;
	SDL_DestroySurface(paneau)         ;
	SDL_DestroySurface(map)            ;
	SDL_DestroySurface(entete)         ;
	SDL_DestroySurface(grid)           ;
	SDL_DestroySurface(radar)          ;
	SDL_DestroySurface(texte)          ;
	
	Mix_HaltChannel(0)              ;
	Mix_HaltChannel(1)              ;
	Mix_HaltChannel(2)              ;
	Mix_HaltChannel(3)              ;	
	Mix_HaltChannel(4)              ;
	
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
	
	MIX_StopAllTracks(gMixer, 0)    ;
	MIX_DestroyAudio(music)         ;
	MIX_DestroyMixer(gMixer)        ; 
	MIX_Quit()                      ; 
	
	TTF_CloseFont(font)             ;
	
	SDL_DestroyTexture(screenTex);
    SDL_DestroySurface(affichage);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    MIX_Quit();
    SDL_Quit();
}

void    loadCube(Objet*  cube)
{
//////-------------------------------Le cube 3D------------------------------//////

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
	cube->centre.z      =   3000  ;

	for(int i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->ptsOrg[i].x    =     points[i].x    ;
		cube->ptsOrg[i].y    =     points[i].y    ;
		cube->ptsOrg[i].z    =     points[i].z    ;
	}

	for(int i = 0 ; i < cube->nbrePts ; i++)
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

	cube->nbreFace                   =   12                   ;
	cube->faces[0].vertices[0]       =   &(cube->points[0])   ;
	cube->faces[0].vertices[1]       =   &(cube->points[2])   ;
	cube->faces[0].vertices[2]       =   &(cube->points[1])   ;
	cube->faces[1].vertices[0]       =   &(cube->points[0])   ;
	cube->faces[1].vertices[1]       =   &(cube->points[3])   ;
	cube->faces[1].vertices[2]       =   &(cube->points[2])   ;
	cube->faces[2].vertices[0]       =   &(cube->points[0])   ;
	cube->faces[2].vertices[1]       =   &(cube->points[4])   ;
	cube->faces[2].vertices[2]       =   &(cube->points[3])   ;
	cube->faces[3].vertices[0]       =   &(cube->points[3])   ;
	cube->faces[3].vertices[1]       =   &(cube->points[4])   ;
	cube->faces[3].vertices[2]       =   &(cube->points[7])   ;
	cube->faces[4].vertices[0]       =   &(cube->points[7])   ;
	cube->faces[4].vertices[1]       =   &(cube->points[4])   ;
	cube->faces[4].vertices[2]       =   &(cube->points[5])   ;
	cube->faces[5].vertices[0]       =   &(cube->points[5])   ;
	cube->faces[5].vertices[1]       =   &(cube->points[6])   ;
	cube->faces[5].vertices[2]       =   &(cube->points[7])   ;
	cube->faces[6].vertices[0]       =   &(cube->points[6])   ;
	cube->faces[6].vertices[1]       =   &(cube->points[5])   ;
	cube->faces[6].vertices[2]       =   &(cube->points[1])   ;
	cube->faces[7].vertices[0]       =   &(cube->points[1])   ;
	cube->faces[7].vertices[1]       =   &(cube->points[2])   ;
	cube->faces[7].vertices[2]       =   &(cube->points[6])   ;
	cube->faces[8].vertices[0]       =   &(cube->points[2])   ;
	cube->faces[8].vertices[1]       =   &(cube->points[3])   ;
	cube->faces[8].vertices[2]       =   &(cube->points[6])   ;
	cube->faces[9].vertices[0]       =   &(cube->points[3])   ;
	cube->faces[9].vertices[1]       =   &(cube->points[7])   ;
	cube->faces[9].vertices[2]       =   &(cube->points[6])   ;
	cube->faces[10].vertices[0]      =   &(cube->points[1])   ;
	cube->faces[10].vertices[1]      =   &(cube->points[5])   ;
	cube->faces[10].vertices[2]      =   &(cube->points[4])   ;
	cube->faces[11].vertices[0]      =   &(cube->points[0])   ;
	cube->faces[11].vertices[1]      =   &(cube->points[1])   ;
	cube->faces[11].vertices[2]      =   &(cube->points[4])   ;

	for(int  i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->faces[i].normale.x    =    cube->faces[i].nrmOrg.x    =     normales[i].x    ;
		cube->faces[i].normale.y    =    cube->faces[i].nrmOrg.y    =     normales[i].y    ;
		cube->faces[i].normale.z    =    cube->faces[i].nrmOrg.z    =     normales[i].z    ;	
	}

	cube->faces[0].uv[0][0]     =    255 + 181  ;
	cube->faces[0].uv[0][1]     =    511 + 181  ;
	cube->faces[0].uv[1][0]     =      0 + 181  ;
	cube->faces[0].uv[1][1]     =    255 + 181  ;
	cube->faces[0].uv[2][0]     =      0 + 181  ;
	cube->faces[0].uv[2][1]     =    511 + 181  ;
	cube->faces[1].uv[0][0]     =    255 + 181  ;
	cube->faces[1].uv[0][1]     =    511 + 181  ;
	cube->faces[1].uv[1][0]     =    255 + 181  ;
	cube->faces[1].uv[1][1]     =    255 + 181  ;
	cube->faces[1].uv[2][0]     =      0 + 181  ;
	cube->faces[1].uv[2][1]     =    255 + 181  ;
	cube->faces[2].uv[0][0]     =    255 + 181  ;
	cube->faces[2].uv[0][1]     =    511 + 181  ;
	cube->faces[2].uv[1][0]     =    511 + 181  ;
	cube->faces[2].uv[1][1]     =    511 + 181  ;
	cube->faces[2].uv[2][0]     =    255 + 181  ;
	cube->faces[2].uv[2][1]     =    255 + 181  ;
	cube->faces[3].uv[0][0]     =    255 + 181  ;
	cube->faces[3].uv[0][1]     =    255 + 181  ;
	cube->faces[3].uv[1][0]     =    511 + 181  ;
	cube->faces[3].uv[1][1]     =    511 + 181  ;
	cube->faces[3].uv[2][0]     =    511 + 181  ;
	cube->faces[3].uv[2][1]     =    255 + 181  ;
	cube->faces[4].uv[0][0]     =    511 + 181  ;
	cube->faces[4].uv[0][1]     =    255 + 181  ;
	cube->faces[4].uv[1][0]     =    511 + 181  ;
	cube->faces[4].uv[1][1]     =    511 + 181  ;
	cube->faces[4].uv[2][0]     =    767 + 181  ;
	cube->faces[4].uv[2][1]     =    511 + 181  ;
	cube->faces[5].uv[0][0]     =    767 + 181  ;
	cube->faces[5].uv[0][1]     =    511 + 181  ;
	cube->faces[5].uv[1][0]     =    767 + 181  ;
	cube->faces[5].uv[1][1]     =    255 + 181  ;
	cube->faces[5].uv[2][0]     =    511 + 181  ;
	cube->faces[5].uv[2][1]     =    255 + 181  ;
	cube->faces[6].uv[0][0]     =    767 + 181  ;
	cube->faces[6].uv[0][1]     =    255 + 181  ;
	cube->faces[6].uv[1][0]     =    767 + 181  ;
	cube->faces[6].uv[1][1]     =    511 + 181  ;
	cube->faces[6].uv[2][0]     =   1023 + 181  ;
	cube->faces[6].uv[2][1]     =    511 + 181  ;
	cube->faces[7].uv[0][0]     =   1023 + 181  ;
	cube->faces[7].uv[0][1]     =    511 + 181  ;
	cube->faces[7].uv[1][0]     =   1023 + 181  ;
	cube->faces[7].uv[1][1]     =    255 + 181  ;
	cube->faces[7].uv[2][0]     =    767 + 181  ;
	cube->faces[7].uv[2][1]     =    255 + 181  ;
	cube->faces[8].uv[0][0]     =      0 + 181  ;
	cube->faces[8].uv[0][1]     =    255 + 181  ;
	cube->faces[8].uv[1][0]     =    255 + 181  ;
	cube->faces[8].uv[1][1]     =    255 + 181  ;
	cube->faces[8].uv[2][0]     =      0 + 181  ;
	cube->faces[8].uv[2][1]     =      0 + 181  ;
	cube->faces[9].uv[0][0]     =    255 + 181  ;
	cube->faces[9].uv[0][1]     =    255 + 181  ;
	cube->faces[9].uv[1][0]     =    255 + 181  ;
	cube->faces[9].uv[1][1]     =      0 + 181  ;
	cube->faces[9].uv[2][0]     =      0 + 181  ;
	cube->faces[9].uv[2][1]     =      0 + 181  ;
	cube->faces[10].uv[0][0]    =      0 + 181  ;
	cube->faces[10].uv[0][1]    =    511 + 181  ;
	cube->faces[10].uv[1][0]    =      0 + 181  ;
	cube->faces[10].uv[1][1]    =    767 + 181  ;
	cube->faces[10].uv[2][0]    =    255 + 181  ;
	cube->faces[10].uv[2][1]    =    767 + 181  ;
	cube->faces[11].uv[0][0]    =    255 + 181  ;
	cube->faces[11].uv[0][1]    =    511 + 181  ;
	cube->faces[11].uv[1][0]    =      0 + 181  ;
	cube->faces[11].uv[1][1]    =    511 + 181  ;
	cube->faces[11].uv[2][0]    =    255 + 181  ;
	cube->faces[11].uv[2][1]    =    767 + 181  ;
	
	cube->angleX     =   0.0        ;
	cube->angleY     =   0.0        ;
	cube->angleZ     =   0.0        ;
	cube->echell     =   0.6        ;

	cube->texture   =  chargerImage("texture.png")       ;

	for(int i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->faces[i].texture  =  cube->texture     ;
	}

	return   ;
}

void  loadScene()
{
	
	for(int  i = 0 ; i < nbreOjectScene ; i++)
	{
		for(int  j = 0 ; j < allObjet[i]->nbreFace ; j++)
		{
			faces[nbreFaceScene]    =    &allObjet[i]->faces[j]    ;
			//printf("x=%d, y=%d, z=%d\n", faces[nbreFaceScene]->vertices[0]->x, faces[nbreFaceScene]->vertices[0]->y, faces[nbreFaceScene]->vertices[0]->z)  ;
			nbreFaceScene++    ;
		}
	}

	//printf("The number of faces =  %d\n", nbreFaceScene) ;

	return    ;
}	

void    initialisation(void)
{
	///-----------------------------3D objects initialization and scene loading--------------------------------//////
	
	//Objet*    raziel   =  malloc(sizeof(Objet))  ;
	//Objet*    room01    =  malloc(sizeof(Objet))  ;
	Objet*    dino    =  malloc(sizeof(Objet))   ;
	Objet*    cube     =  malloc(sizeof(Objet))  ;
	//Objet*    terrain  =  malloc(sizeof(Objet))  ;


	controlledPlayer =  dino   ;
	
	//loadOBJfile("Raziel/Raziel.obj", raziel)   ;
	//loadOBJfile("assets/room01.obj", room01)       ;
	loadOBJfile("assets/dino/dino.obj", dino)       ;
	//loadOBJfile("assets/terrain.obj", terrain)       ;
	loadCube(cube)                             ;

	allObjet[0]   =   dino  ;
	allObjet[1]   =   cube     ;
	//allObjet[2]   =   terrain  ;

	//terrain->centre.z   =  3000      ;
	//terrain->angleX     =    PI      ;

	nbreOjectScene   =   1     ;	
	
	loadScene()   ;
	
	//////------------------------Chargement des fichiers et des bibliothèques--------------------------//////

	initSDL()                 ;
	
	chargementFichirs()       ;

	////------------------------------------------Dessin------------------------------------------/////

	dessinEnv2D()   ;
	
	////------------------------------------------Musique------------------------------------------/////
	
	// Mix_AllocateChannels(16)    ;
	
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

void    afficheObjetMesh(Objet*  mesh)
{
	int    i      ;

	////--------------------------------projection---------------------------------//////

	for(i = 0 ; i < mesh->nbrePts ; i++)
	{
		mesh->points[i].X  =  ((mesh->points[i].x * DISTANCE_FOCAL) / mesh->points[i].z) + (RES_HORIZ / 2)    ;	
		mesh->points[i].Y  =  ((mesh->points[i].y * DISTANCE_FOCAL) / mesh->points[i].z) + (RES_VERT  / 2)    ;				
	}


	////--------------------------dessin des mesh---------------------------------//////

	for(i = 0 ; i < mesh->nbreSegment ; i++)
	{		
		ligne(mesh->segments[i][0]->X , mesh->segments[i][0]->Y , mesh->segments[i][1]->X , mesh->segments[i][1]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;	
	}

	return   ;
}

void    displayScene()
{
	int    i , j , k     ;

	////--------------------------------projection---------------------------------//////

	for(int  objectCnt = 0 ; objectCnt < nbreOjectScene ; objectCnt++)
	{
		for(int  i = 0 ; i < allObjet[objectCnt]->nbrePts ; i++)
		{
			// The classic perspective formula: (3D_Coord(x or y) * Focal_Length / Depth(z)) + Screen_Offset
			if(allObjet[objectCnt]->points[i].z != 0)
			{
				allObjet[objectCnt]->points[i].X  =  ((allObjet[objectCnt]->points[i].x * DISTANCE_FOCAL) / allObjet[objectCnt]->points[i].z) + (RES_HORIZ / 2)    ;	
				allObjet[objectCnt]->points[i].Y  =  ((allObjet[objectCnt]->points[i].y * DISTANCE_FOCAL) / allObjet[objectCnt]->points[i].z) + (RES_VERT  / 2)    ;
				//printf("Object = %d | X = %d; Y = %d | x = %d, y = %d, z = %d, \n", objectCnt, allObjet[objectCnt]->points[i].X , allObjet[objectCnt]->points[i].Y , allObjet[objectCnt]->points[i].x , allObjet[objectCnt]->points[i].y , allObjet[objectCnt]->points[i].z);
			}
		}
	}
	
	////-------------------------- 2. DEPTH SORTING ---------------------------------//////
	// Sorts all faces from back-to-front so that near objects are drawn over far objects.
	
	painterAlgorithmSort()  ;
	
	////--------------------------dessin des faces---------------------------------//////

	// Main loop to process every face in the global scene list.
	for(int  i = 0 ; i < nbreFaceScene ; i++)
	{		
		if((((faces[i]->normale.x * (faces[i]->vertices[0]->X - (RES_HORIZ / 2))) + (faces[i]->normale.y * (faces[i]->vertices[0]->Y - (RES_VERT  / 2))) + (faces[i]->normale.z * DISTANCE_FOCAL)) < 0) // Back-Face Culling : using the dot-product Ux * Vx + Uy * Vy + Uz * Vz, if it is positif they are pointing to the same direction.
		&& ((faces[i]->vertices[0]->X > 0) || (faces[i]->vertices[1]->X > 0) || (faces[i]->vertices[2]->X > 0)) && ((faces[i]->vertices[0]->X < (RES_HORIZ-1)) || (faces[i]->vertices[1]->X < (RES_HORIZ-1)) || (faces[i]->vertices[2]->X < (RES_HORIZ-1))) 
		&& ((faces[i]->vertices[0]->Y > 0) || (faces[i]->vertices[1]->Y > 0) || (faces[i]->vertices[2]->Y > 0)) && ((faces[i]->vertices[0]->Y < (RES_VERT-1)) || (faces[i]->vertices[1]->Y < (RES_VERT-1)) || (faces[i]->vertices[2]->Y < (RES_VERT-1)))  // Checks if the triangle is at least partially within the screen boundaries.
		&& ((faces[i]->vertices[0]->z > DISTANCE_FOCAL)) && ((faces[i]->vertices[1]->z > DISTANCE_FOCAL)) && ((faces[i]->vertices[2]->z > DISTANCE_FOCAL))) // Checks if the triangle is too close or behind the camera
		{	
			
			
			int    haut   =   0      ;
			int    bas , millieu     ;
			int    baleillage , dis  ;
			
			// Identifies which vertex is the Top (haut), Middle (millieu), and Bottom (bas) of the triangle.
			if(faces[i]->vertices[0]->Y > faces[i]->vertices[1]->Y)
			{
				haut   =   1   ;
			}
			
			if(faces[i]->vertices[haut]->Y > faces[i]->vertices[2]->Y)
			{
				haut   =   2   ;
			}
			
			if(faces[i]->vertices[(haut+1)%3]->Y > faces[i]->vertices[(haut+2)%3]->Y)
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
			
			int    X0    =   faces[i]->vertices[haut]->X       ;
			int    Y0    =   faces[i]->vertices[haut]->Y       ;
			int    X1    =   faces[i]->vertices[millieu]->X    ;
			int    Y1    =   faces[i]->vertices[millieu]->Y    ;
			int    X2    =   faces[i]->vertices[bas]->X        ;
			int    Y2    =   faces[i]->vertices[bas]->Y        ;
			
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
			
			int    XV0    =   faces[i]->uv[haut][0]     ;
			int    YV0    =   faces[i]->uv[haut][1]     ;
			int    XV1    =   faces[i]->uv[bas][0]      ;
			int    YV1    =   faces[i]->uv[bas][1]      ;
			
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
			int    XH1    =   faces[i]->uv[millieu][0]      ;
			int    YH1    =   faces[i]->uv[millieu][1]      ;
			
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
						{//printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH<0?0:yH , faces[i]->texture))      ;
							
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
						{//printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH<0?0:yH , faces[i]->texture))      ;
							
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


static inline   void   triTableau(int tableau[][2] , int * position , int action , int y)
{
	int    i  =   *position   ;
	
	while((i > 0) && (y < tableau[i-1][1]))
	{
		tableau[i][0] = tableau[i-1][0]   ;
		tableau[i][1] = tableau[i-1][1]   ;
		i--                               ;
	}
	
	tableau[i][0]   =   action     ;
	tableau[i][1]   =   y          ;
	
	(*position)++    ;
	
	return     ;
}

static inline   void   delTableau(int tableau[][2] , int * taille , int action)
{
	int    i , j      ;
	
	for(i = 0 ; i < (*taille) ; i++)
	{
		if(tableau[i][0] == action)
		{
			for(j = i ; j < (*taille)-1 ; j++)
			{
				tableau[j][0] = tableau[j+1][0]   ;
				tableau[j][1] = tableau[j+1][1]   ;
			}

			(*taille)--    ;
			return         ;
		}
	}
}

static inline  void    translation(Objet * objet , int Dx , int Dy , int Dz)
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

static inline  void    changementEchell_rotation(Objet * objet)
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
		objet->faces[i].normale.x   =    (objet->faces[i].nrmOrg.x * cos(objet->angleZ)) - (objet->faces[i].nrmOrg.y * sin(objet->angleZ))  ;
		objet->faces[i].normale.y   =    (objet->faces[i].nrmOrg.x * sin(objet->angleZ)) + (objet->faces[i].nrmOrg.y * cos(objet->angleZ))  ;
		
		// par raport a l'axe Y
		objet->faces[i].normale.z   =    (objet->faces[i].nrmOrg.z * cos(objet->angleY)) - (objet->faces[i].normale.x * sin(objet->angleY))   ;
		objet->faces[i].normale.x   =    (objet->faces[i].nrmOrg.z * sin(objet->angleY)) + (objet->faces[i].normale.x * cos(objet->angleY))   ;
		
		int    z   =  objet->faces[i].normale.z   ;
		
		// par raport a l'axe X
		objet->faces[i].normale.z   =    (objet->faces[i].normale.y * sin(objet->angleX)) + (z * cos(objet->angleX))   ;
		objet->faces[i].normale.y   =    (objet->faces[i].normale.y * cos(objet->angleX)) - (z * sin(objet->angleX))   ;
		
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

bool Mix_OpenAudio() 
{
    if (!MIX_Init()) 
		return      false      ;

    // Open default audio device
    gMixer   =     MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL)    ;
    
	if (!gMixer) 
		return      false      ;

    // Create 5 tracks to act as your "Channels"
    for (int i = 0; i <= 4; i++) 
	{
        gTracks[i]    =    MIX_CreateTrack(gMixer)     ;
    }

    return true;
}

static inline   void   Mix_PlayChannel_Bridge(int ch, MIX_Audio* audio, int loops) 
{
    // Set the audio data to the track
    MIX_SetTrackAudio(gTracks[ch], audio)   ;
    
    // Set looping property (-1 in old SDL = infinite)
    SDL_PropertiesID      props =   SDL_CreateProperties()                                  ;
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops)    ;
    
    MIX_PlayTrack(gTracks[ch], props)   ;

    SDL_DestroyProperties(props)        ;
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

Point   calculateFaceNormal(Point* nrm, Point* v1, Point* v2, Point* v3) 
{
	// Calculate vectors
	Point   u   =   {v2->x - v1->x, v2->y - v1->y, v2->z - v1->z}     ;
	Point   v   =   {v3->x - v1->x, v3->y - v1->y, v3->z - v1->z}     ;
	
	// Cross product
	nrm->x   =   (u.y * v.z) - (u.z * v.y)     ;
	nrm->y   =   (u.z * v.x) - (u.x * v.z)     ;
	nrm->z   =   (u.x * v.y) - (u.y * v.x)     ;

	// Prevent overflow
	while(abs(nrm->x) > 10000 || abs(nrm->y) > 10000 || abs(nrm->z) > 10000)
	{
		nrm->x   /=   2   ;
		nrm->y   /=   2   ;
		nrm->z   /=   2   ;
	}
	
	// Normalize the vector
	float length   =   sqrt(nrm->x * nrm->x + nrm->y * nrm->y + nrm->z * nrm->z)     ;

	nrm->x = (int)((nrm->x / length) * 100.0f);
	nrm->y = (int)((nrm->y / length) * 100.0f);
	nrm->z = (int)((nrm->z / length) * 100.0f);
	
	return *nrm  ;
}

bool loadOBJfile(const  char*  path, Objet*  objet) 
{
    // extract directory from path to handle relative texture paths
	FILE*  file    =    fopen(path, "r")    ;	
	char   dirPath[256]                     ;

	const  char*   lastSlashPtr  =   strrchr(path, '/')  ;
	if (lastSlashPtr == NULL) 
	{
		dirPath[0]  =   '\0'   ; // No directory found
	}
	else
	{
		int   dirLength     =   lastSlashPtr - path + 1    ;
		strncpy(dirPath, path, dirLength)                  ;
		dirPath[dirLength]  =   '\0'                       ; // Null-terminate the directory path
	} 

    if (!file) 
	{
        SDL_Log("Could not open OBJ file: %s", path)   ;
        return   false   ;
    }

    objet->nbrePts       =   0   ;
    objet->nbreFace      =   0   ;
	objet->nbreSegment   =   0   ;

	objet->centre.x   =     0      ;
	objet->centre.y   =     0      ;
	objet->centre.z   =  1500      ;

	objet->angleX   =     0.0      ;
	objet->angleY   =     0.0      ;
	objet->angleZ   =     0.0      ;
	objet->echell   =     1.0      ;

    char   line[256]              ;

	int   UV[NBRE_POINT_MAX][2]   ;	
	int   uvCount   =        0    ;

    while (fgets(line, sizeof(line), file)) 
	{				
		// Parse Material Library
		if((strncmp(line, "mtllib", 6) == 0))
		{
			char   mtlFile[256]   ;
			sscanf(line, "mtllib %s", mtlFile)   ;

			char     fullMtlPath[256]            ;
            snprintf(fullMtlPath, sizeof(fullMtlPath), "%s%s", dirPath, mtlFile)    ;
			
			FILE*  f   =   fopen(fullMtlPath, "r")   ;

			if (!f) 
			{
				SDL_Log("Could not open mtlFile: %s", mtlFile)   ;
				fclose(file)     ;
				return   false   ;
			}

		    char   l[256]        ;

			while (fgets(l, sizeof(l), f)) 
			{
				if((strncmp(l, "map_Kd", 6) == 0))
				{
					// Handle texture mapping
					char      textureFile[256]    ;	
					char      texturePath[256]    ;	

					sscanf(l, "map_Kd %s", textureFile)   ;
					snprintf(texturePath, sizeof(texturePath), "%s%s", dirPath, textureFile)    ;
					
					objet->texture   =   chargerImage(texturePath)   ;					
				}
			}
		}

		// Parse Vertices
		if (line[0] == 'v' && line[1] == ' ') 
		{
            float   x, y, z   ;

            sscanf(line, "v %f %f %f", &x, &y, &z)     ;

            // We scale by 500 to match your engine's coordinate system
            objet->ptsOrg[objet->nbrePts].x    =   objet->points[objet->nbrePts].x   =   (int)(x * 1000)   ;
            objet->ptsOrg[objet->nbrePts].y    =   objet->points[objet->nbrePts].y   =   (int)(y * 1000)   ;
            objet->ptsOrg[objet->nbrePts].z    =   objet->points[objet->nbrePts].z   =   (int)(z * 1000)   ;

			objet->points[objet->nbrePts].z   +=   objet->centre.z  ;
            
			objet->nbrePts++     ;
			//printf("Vertex %d: (%d, %d, %d)\n", objet->nbrePts, objet->ptsOrg[objet->nbrePts-1].x, objet->ptsOrg[objet->nbrePts-1].y, objet->ptsOrg[objet->nbrePts-1].z)   ;
        }

		// Parse Texture Coordinates
		if ((line[0] == 'v') && (line[1] == 't')) 
		{
			float   u, v   ;

			sscanf(line, "vt %f %f", &u, &v)     ;

			// Store texture coordinates
			UV[uvCount][0]    =   (int)(u * objet->texture->w)            ;
			UV[uvCount][1]    =   (int)((1.0f - v) * objet->texture->h)   ;

			uvCount++    ;
		}

		// Parse Faces (Triangles)
		if (line[0] == 'f' && line[1] == ' ') 
		{
			int   v1[3], v2[3], v3[3]     ;

			// OBJ indices start at 1, so we subtract 1
			if((sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1[0], &v1[1], &v1[2], &v2[0], &v2[1], &v2[2], &v3[0], &v3[1], &v3[2]) == 9) || 
			   (sscanf(line, "f %d/%d %d/%d %d/%d", &v1[0], &v1[1], &v2[0], &v2[1], &v3[0], &v3[1]) == 6))
			{
				objet->faces[objet->nbreFace].vertices[0]    =    &objet->points[v1[0] - 1]    ;
				objet->faces[objet->nbreFace].vertices[1]    =    &objet->points[v2[0] - 1]    ;
				objet->faces[objet->nbreFace].vertices[2]    =    &objet->points[v3[0] - 1]    ;

				calculateFaceNormal(&objet->faces[objet->nbreFace].nrmOrg, objet->faces[objet->nbreFace].vertices[0], objet->faces[objet->nbreFace].vertices[1], objet->faces[objet->nbreFace].vertices[2])   ;

				objet->faces[objet->nbreFace].normale     =   objet->faces[objet->nbreFace].nrmOrg     ;

				// Store the segments for the face		
				
				objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[0]     ;
				objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[1]     ;
				objet->nbreSegment++    ;

				objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[1]     ;
				objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[2]     ;
				objet->nbreSegment++    ;

				objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[2]     ;
				objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[0]     ;
				objet->nbreSegment++    ;
				
				// Store the texture coordinate
				objet->faces[objet->nbreFace].uv[0][0]    =   UV[v1[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[0][1]    =   UV[v1[1] - 1][1]   ;
				objet->faces[objet->nbreFace].uv[1][0]    =   UV[v2[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[1][1]    =   UV[v2[1] - 1][1]   ;
				objet->faces[objet->nbreFace].uv[2][0]    =   UV[v3[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[2][1]    =   UV[v3[1] - 1][1]   ;

				objet->faces[objet->nbreFace].texture     =   objet->texture     ;

				objet->nbreFace++    ;
			}
			else
			{
				SDL_Log("Failed to parse face line: %s", line[0])   ;
				fclose(file)    ;
				return   false  ;
			}
		}
    }

    fclose(file)   ;

    return   true  ;
}


void   painterAlgorithmSort()
{
	// calculate the average Z value for each face
	int   averageZ[NBRE_FACE_MAX_SCENE]      ;

	for (int i = 0 ; i < nbreFaceScene ; i++)
	{
		averageZ[i]   =   (faces[i]->vertices[0]->z + faces[i]->vertices[1]->z + faces[i]->vertices[2]->z + faces[i]->vertices[2]->z)    ;
	}

	// Sort in descending order based on their average Z value (depth) using insertion sort algorithm
	for (int i = 1 ; i < nbreFaceScene ; i++) 
	{
		Face*  key       =    faces[i]        ;
		int    keyAvg    =    averageZ[i]     ;

		int j = i  ;
		while(j > 0 && keyAvg > averageZ[j - 1]) 		
		{			
			faces[j]     =   faces[j - 1]     ;
			averageZ[j]  =   averageZ[j - 1]  ;

			j--   ;
		}

		faces[j]     =   key     ;
		averageZ[j]  =   keyAvg  ;
	}

	return  ;
}
