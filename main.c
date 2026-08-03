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

#define   NBRE_POINT_MAX       7000
#define   NBRE_SEG_MAX         7000
#define   NBRE_OBJET_MAX        100
#define   NBRE_FACE_MAX        7000
#define   NBRE_FACE_MAX_SCENE  7000

#define   DISTANCE_FOCAL     512
#define   RES_VERT           600
#define   RES_HORIZ          800
#define   RES_VERT_DIV_2     300
#define   RES_HORIZ_DIV_2    400

#define   HOR_FIELD_OF_VIEW          1.326406f     // the formula is 2*atanf(RES_HORIZ_DIV_2 / (float)DISTANCE_FOCAL)
#define   VER_FIELD_OF_VIEW          1.06003f      // the formula is 2*atanf(RES_VERT_DIV_2 / (float)DISTANCE_FOCAL) 
#define   HOR_FIELD_OF_VIEW_DIV_2    0.663203f  
#define   VER_FIELD_OF_VIEW_DIV_2    0.530015f 
#define   HOR_FIELD_OF_VIEW_RATE     0.78125f      // the formula is RES_HORIZ_DIV_2 / (float)DISTANCE_FOCAL)
#define   VER_FIELD_OF_VIEW_RATE     0.5859375f    // the formula is RES_VERT_DIV_2 / (float)DISTANCE_FOCAL)

#define   HORIZON_END           250000
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
Mix_Chunk*     scale         =   NULL   ;

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

typedef  struct  objet  Objet   ;
typedef  struct  face
{
	Point*         vertices[3]  ;      // addresses of the visual vertices

	Point          nrmOrg       ;      // normal vector of the original triangle
	Point          nrmWrd       ;      // normal vector of the world triangle
	Point          nrmVsl       ;	   // normal vector of the visual triangle

	int            uv[3][2]     ;      // UV coordinates of the face triangle
	SDL_Surface*   texture      ;      // texture map of the face triangle
	Objet*         owner        ;      // pointer to the object owner of the face
}  Face  ;

typedef  struct  objet
{
	// 3 types of points are used in the 3D pipeline:
	// ptsOrg: is the original point, it is the starting point of the 3D pipeline
	// ptsWrd: is the point after transformation (rotation scale and translation) in the world
	// ptsVsl: is the point after the transformation relative to the camera

	int     nbrePts                     ;     // number of vertices in the 3D object
	Point   ptsOrg[NBRE_POINT_MAX]      ;	  // original vertices without any transformation
	Point   ptsWrd[NBRE_POINT_MAX]      ;     // vertices after transformation in the world
	Point   ptsVsl[NBRE_POINT_MAX]      ;     // vertices transformed to be visualized by the camera

	int     nbreSegment                 ;	  // number of segments
	Point*  segments[NBRE_SEG_MAX][2]   ;	  // segments used to draw wireframe 3D object

	int     nbreFace                    ;	  // number of faces
	Face    faces[NBRE_FACE_MAX]        ;     // triangles used to draw 3D object
	SDL_Surface*   texture              ;	  // texture map of the 3D object

	Point   center     ;              // absolute position of the 3D object inside the world
	float   angleX     ;	          // absolute X rotation of the 3D object inside the world
	float   angleY     ;	          // absolute Y rotation of the 3D object inside the world
	float   angleZ     ;	          // absolute Z rotation of the 3D object inside the world

	float   scale      ;              // scale of the 3D object

	Point   sphereCenter    ;         // a sphere that warp the object used for object culling
	int     radius          ;         // this is the original radius of the object, it should be scaled by the scale value.

	bool    isVisible       ;         // is the object visible or not depending on the frustum culling
}  Objet  ;

struct   Camera
{
	float   angleX             ;
	float   angleY             ;
	float   angleZ             ;

	int     posX               ;
	int     posY               ;
	int     posZ               ;
}   camera  ;

typedef  struct   buttons
{
	int   left     ;
	int   right    ;
	int   up       ;
	int   down     ;
	int   pageUp   ;
	int   pageDown ;

	int   W        ;
	int   A        ;
	int   S        ;
	int   D        ;
	int   Q        ;
	int   E        ;

	int   plus     ;
	int   minus    ;

	int   F1       ;
	int   F2       ;
	int   F3       ;
	int   F4       ;
	int   F5       ;
	int   F6       ;
	int   F7       ;
	int   F8       ;
	int   F9       ;
	int   F10      ;
	int   F11      ;
	int   F12      ;

	int   escape   ;
}  Button         ;

////-------------------------------variables globales-------------------------------------/////

void    initialisation(void)    ;
void    initSDL(void)           ;
void    attendreTouche(void)    ;
void    dessinerEtoiles(void)   ;
void    dessinerLignes(void)    ;
void    drawBackground(void)    ;
void    drawHUD(void)			;
void    displayScene()          ;
void    cleanUp()               ;
void    loadCube(Objet*  cube)  ;
void    addObjectToScene(Objet*  objet)       ;
void    removeObjectFromScene(Objet*  objet)  ;
void    afficheObjetMesh(Objet*  mesh)        ;
void    drawButtons(Button* buttons)          ;
void    movementSoundEffect(Button* buttons)  ;
void    frustumCulling(Objet* objet)	      ;
void    PlayerMovement(Button* buttons, Objet* player)               ;
void    transformToCameraPerspective(Objet* objet)                   ;
void    cameraMovement(Button* buttons, int speed, float spin)       ;
static  inline  void    localRotationScale(Objet * objet, float deltaX, float deltaY, float deltaZ, float deltaScale)  ;
static  inline  void    translation(Objet * objet , int Dx , int Dy , int Dz)        ;
static  inline  void    swap(int * a , int * b)                                      ;
static  inline  void    Mix_PlayChannel_Bridge(int ch, MIX_Audio* audio, int loops)  ;
void    ligne(int x0, int y0, int x1, int y1, Uint32  couleur)    ;
void    animationRadar(int X , int Y , float R)                   ;
void    animationTexte(void)                                      ;
int     handleInputs(Button* buttons)                             ;
int     twoPointsDistance(Point* p1, Point* p2)                   ;
static  inline  void     setPixel(int  X , int  Y , Uint32  couleur)      ;
static  inline  Uint32   getPixel(int  X , int  Y , SDL_Surface*  image)  ;
static  inline  void     triTableau(int tableau[][2] , int * position , int action , int y)    ;
static  inline  void     delTableau(int tableau[][2] , int * taille , int action)              ;
SDL_Surface*     chargerImage(const  char*  file)          ;
bool      loadOBJfile(const  char*  path, Objet*  objet, int posX, int posY, int posZ, float angleX, float angleY, float angleZ, float scale)   ;
Point     calculateFaceNormal(Point* nrm, Point* v1, Point* v2, Point* v3)     ;
void      chargementFichirs()        ;
bool      Mix_OpenAudio()            ;
void      painterAlgorithmSort()     ;

////------------------------------Static program variables-----------------------------------/////

Face*        facesQueue[NBRE_FACE_MAX_SCENE]    ;
int          nbreFaceScene      =   0           ;

Objet*       allObjet[NBRE_OBJET_MAX]      ;
int          nbreOjectScene     =   0      ;

Objet*       controlledPlayer   =   NULL   ;

////--------------------------------Fonction principale-------------------------------------/////

int   main(int  argc , char**  argv)
{
	Button      buttons  =   { 0 }   ;
	
	int         quitter  =  1   ;
		
	int         FPS  =   0      ;
	int         i    =   1      ;
	int         temps           ;

	///////-----------------------------Initialisation-----------------------------////////
	
	initialisation()            ;
		
	///////--------------------------------------test------------------------------------////////

	//ligne(-100 , 200 , 750 , 200 , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;		
	//dessinerEtoiles()         ;
	//dessinerLignes()          ;

	///////------------------------------Main loop---------------------------------////////

	while(quitter)
	{
		//  get the initial time in the beginning of the frame
		temps       =   SDL_GetTicks()            ;
		
		///////---------------First phase :  capture all the keyboard inputs---------------///////
		
		quitter  =   handleInputs(&buttons)    ;
				
		///////-------------------Second phase :  display background  --------------------///////
		
		drawBackground()   ;
		
		///////-----------Third phase : Translation/Rotation/Scale of the objects-------------////////////

		// for(int i = 0 ; i < nbreOjectScene ; i++)
		// {
		// 	localRotationScale(allObjet[i], angleX, angleY, angleZ, scale)     ;
		// 	translation(allObjet[i], Dx, Dy, Dz)	                                    ;
		// }
			
		///////-----------Fourth phase : Camera and Player movement --------------------///////
		
		//PlayerMovement(&buttons, controlledPlayer)  ;
		cameraMovement(&buttons, 500, 0.05f)        ;

		///////-----------Fifth phase : 3D pipeline displaying all the objects in the scene--------------////////////
		///////-----------Pipeline : (original object -> world oject -> camera perspective) -------------////////////
		
		//ligne(-20 , 367 , -20 , 367 , SDL_MapRGB(affichage->format, 5 , 200 , 128))  ;
		//afficheObjetMesh(controlledPlayer)    ;
		
		displayScene()  ;
		
		///////-----------Sixth phase :  display Buttons and  HUD elements---------------///////
		
		drawHUD()               ;		
		drawButtons(&buttons)   ;
		
		///////-----------Seventh phase : emit sound effect---------------///////

		movementSoundEffect(&buttons)  ;

		///////-----------Final phase :  update the frame buffer---------------///////
		
		// 1. Upload the pixels the CPU just calculated to the GPU
		SDL_UpdateTexture(screenTex, NULL, affichage->pixels, affichage->pitch);

		// 2. Clear the GPU's memory (just in case)
		SDL_RenderClear(renderer);

		// 3. Copy our software-rendered image to the screen
		SDL_RenderTexture(renderer, screenTex, NULL, NULL);

		// 4. Show it!
		SDL_RenderPresent(renderer);
		
		///////-----------------------Frame rate calculation----------------------------///////
		
		// Calculate and ceiling the Frame rate of the game
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

void  initSDL(void) {

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

void  attendreTouche(void)
{
	SDL_Event event             ;

	do
    	SDL_WaitEvent(&event)   ;
	while (event.type != SDL_EVENT_QUIT && event.type != SDL_EVENT_KEY_DOWN)   ;
}

int   handleInputs(Button* buttons)
{
	SDL_Event event         ;

	SDL_PumpEvents()        ;
		
	const bool *keystates = SDL_GetKeyboardState(NULL)  ;

	if(keystates[SDL_SCANCODE_UP])
	{
		buttons->up     =   1   ;
	}
	else
	{
		buttons->up     =   0   ;
	}
	
	if(keystates[SDL_SCANCODE_DOWN])
	{
		buttons->down     =   1   ;
	}
	else
	{
		buttons->down     =   0   ;
	}
	
	if(keystates[SDL_SCANCODE_RIGHT])
	{
		buttons->right     =   1   ;
	}
	else
	{
		buttons->right     =   0   ;
	}
	
	if(keystates[SDL_SCANCODE_LEFT])
	{
		buttons->left     =   1   ;
	}
	else
	{
		buttons->left     =   0   ;
	}
	
	if(keystates[SDL_SCANCODE_PAGEUP])
	{
		buttons->pageUp =     1    ;
	}
	else
	{
		buttons->pageUp =     0    ;
	}
	
	if(keystates[SDL_SCANCODE_PAGEDOWN])
	{
		buttons->pageDown   =   1    ;
	}
	else
	{
		buttons->pageDown   =   0    ;
	}
	
	if(keystates[SDL_SCANCODE_W])// && (controlledPlayer->center.z < 12100))
	{
		buttons->W     =   1    ;	
	}
	else
	{
		buttons->W      =   0      ;
	}
	
	if(keystates[SDL_SCANCODE_S])//  && (controlledPlayer->center.z > 500))
	{
		buttons->S     =   1    ;	
	}
	else
	{
		buttons->S     =   0    ;	
	}
	
	if(keystates[SDL_SCANCODE_D])//  && (controlledPlayer->center.x < 4500))
	{
		buttons->D     =   1    ;
		
	}
	else
	{
		buttons->D      =   0      ;
	}
	
	if(keystates[SDL_SCANCODE_A])//  && (controlledPlayer->center.x > -4500))
	{
		buttons->A      =   1      ;
	}
	else
	{
		buttons->A      =   0      ;
	}
	
	if(keystates[SDL_SCANCODE_Q])//  && (controlledPlayer->center.y > -4500))
	{
		buttons->Q      =   1      ;
	}
	else
	{
		buttons->Q      =   0      ;
	}
	
	if(keystates[SDL_SCANCODE_E])//  && (controlledPlayer->center.y < 4500))
	{
		buttons->E     =   1    ;	
	}
	else
	{
		buttons->E     =   0    ;
	}
	
	if(keystates[SDL_SCANCODE_KP_PLUS] && (controlledPlayer->scale < 2.0f))
	{
		buttons->plus     =   1    ;
		//removeObjectFromScene(controlledPlayer)   ;
	}
	else
	{
		buttons->plus     =   0    ;
	}
	
	if(keystates[SDL_SCANCODE_KP_MINUS] && (controlledPlayer->scale > 0.5f))
	{	
		buttons->minus     =   1    ;
		//addObjectToScene(controlledPlayer)   ;
	}
	else
	{
		buttons->minus     =   0    ;
	}
	
	if(keystates[SDL_SCANCODE_ESCAPE])
	{
		buttons->escape     =   1    ;
		return                  0    ; 
	}
	else
	{
		buttons->escape     =   0    ;
	}
		
	while(SDL_PollEvent(&event)) 
	{
		if (event.type == SDL_EVENT_QUIT) 
		{
			return   0  ;
		}

		if (event.type == SDL_EVENT_KEY_DOWN) 
		{
			// For F1-F12 keys, we still use Keycodes (.key.key)
			if(event.key.scancode == SDL_SCANCODE_F1)
			{
				buttons->F1      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F2)
			{
				buttons->F2      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F3)
			{
				buttons->F3      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F4)
			{
				buttons->F4      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F5)
			{
				buttons->F5      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F6)
			{
				buttons->F6      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F7)
			{
				buttons->F7      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F8)
			{
				buttons->F8      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F9)
			{
				buttons->F9      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F10)
			{
				buttons->F10      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F11)
			{
				buttons->F11      =   1    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F12)
			{
				buttons->F12      =   1    ;
			}
		}
		
		if (event.type == SDL_EVENT_KEY_UP)
		{
			if(event.key.scancode == SDL_SCANCODE_F1)
			{
				buttons->F1      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F2)
			{
				buttons->F2      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F3)
			{
				buttons->F3      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F4)
			{
				buttons->F4      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F5)
			{
				buttons->F5      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F6)
			{
				buttons->F6      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F7)
			{
				buttons->F7      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F8)
			{
				buttons->F8      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F9)
			{
				buttons->F9      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F10)
			{
				buttons->F10      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F11)
			{
				buttons->F11      =   0    ;
			}

			if(event.key.scancode == SDL_SCANCODE_F12)
			{
				buttons->F12      =   0    ;
			}
		}
	}

	return   1  ;
}

void  drawHUD(void)
{
	SDL_Rect    rectSrc         ;
	SDL_Rect    rectDst         ;
	
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
	
	animationRadar(allObjet[0]->center.x , allObjet[0]->center.z , allObjet[0]->scale)   ;
	
	rectSrc.h   =    40      ;
	rectSrc.w   =    40      ;
}

void  drawButtons(Button* buttons)
{
	SDL_Rect    rectSrc         ;
	SDL_Rect    rectDst         ;
	
	if(buttons->up == 1)
	{
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

	if(buttons->down == 1)
	{
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

	if(buttons->right == 1)
	{
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
	
	if(buttons->left == 1)
	{
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

	if(buttons->pageUp == 1)
	{		
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
	
	if(buttons->pageDown == 1)
	{		
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

	if(buttons->W == 1)
	{		
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
	
	if(buttons->S == 1)
	{
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
	
	if(buttons->A == 1)
	{
		rectSrc.x   =   0      ;
		rectSrc.y   =   80     ;

		rectDst.x   =    40    ;
		rectDst.y   =   560    ;
		
		SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
	}
	else
	{
		rectSrc.x   =   0      ;
		rectSrc.y   =   120    ;

		rectDst.x   =    40    ;
		rectDst.y   =   560    ;
		
		SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
	}
	
	if(buttons->D == 1)
	{
		rectSrc.x   =   120    ;
		rectSrc.y   =   80     ;

		rectDst.x   =   140    ;
		rectDst.y   =   560    ;
		
		SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;		
	}
	else
	{
		rectSrc.x   =   120    ;
		rectSrc.y   =   120    ;
		
		rectDst.x   =   140    ;
		rectDst.y   =   560    ;
		
		SDL_BlitSurface(bouttons, &rectSrc, affichage, &rectDst)     ;
	}
	
	if(buttons->Q == 1)
	{
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
	
	if(buttons->E == 1)
	{
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

	if(buttons->plus == 1)
	{
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
	
	if(buttons->minus == 1)
	{
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

	return    ;
}

void  movementSoundEffect(Button* buttons)
{
	if(buttons->Q == 1 || buttons->S == 1 || buttons->D == 1 || buttons->A == 1 || buttons->W == 1  || buttons->E == 1)
	{
		Mix_Resume(1)    ;
	}
	else
	{
		Mix_Pause(1)     ;
	}
	
	if(buttons->up == 1 || buttons->down == 1 || buttons->left == 1 || buttons->right == 1 || buttons->pageUp == 1 || buttons->pageDown == 1)
	{
		Mix_Resume(2)    ;
	}
	else
	{
		Mix_Pause(2)     ;
	}
	
	if(buttons->plus == 1 || buttons->minus == 1)
	{
		Mix_Resume(3)    ;
	}
	else
	{
		Mix_Pause(3)     ;
	}

	if(buttons->F1 == 1)
	{
		Mix_PlayChannel(4 , sonF1 , 0)     ;
	}

	if(buttons->F2 == 1)
	{
		Mix_PlayChannel(4 , sonF2 , 0)     ;
	}

	if(buttons->F3 == 1)
	{
		Mix_PlayChannel(4 , sonF3 , 0)     ;
	}

	if(buttons->F4 == 1)
	{
		Mix_PlayChannel(4 , sonF4 , 0)     ;
	}

	if(buttons->F5 == 1)
	{
		Mix_PlayChannel(4 , sonF5 , 0)     ;
	}

	if(buttons->F6 == 1)
	{
		Mix_PlayChannel(4 , sonF6 , 0)     ;
	}

	if(buttons->F7 == 1)
	{
		Mix_PlayChannel(4 , sonF7 , 0)     ;
	}

	if(buttons->F8 == 1)
	{
		Mix_PlayChannel(4 , sonF8 , 0)     ;
	}

	if(buttons->F9 == 1)
	{
		Mix_PlayChannel(4 , sonF9 , 0)     ;
	}

	if(buttons->F10 == 1)
	{
		Mix_PlayChannel(4 , sonF10 , 0)     ;
	}

	if(buttons->F11 == 1)
	{
		Mix_PlayChannel(4 , sonF11 , 0)     ;
	}

	if(buttons->F12 == 1)
	{
		Mix_PlayChannel(4 , sonF12 , 0)     ;
	}

	return   ;
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

void    drawBackground(void)
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
	if (Y < 0 || Y >= image->h || X < 0 || X >= image->w) return    0xFFFFFFFF   ;
	return    *((Uint32*)(image->pixels) + (image->w * Y) + X)    ;
}

void   ligne(int x0, int y0, int x1, int y1 , Uint32  couleur)
{
	if(x0 == x1 && y0 == y1)
	{
		setPixel(x0, y0, couleur)   ;
		return   ;
	}
	
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
	scale         =  Mix_LoadWAV("zoom.wav")               ;
	
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
	
	if(scale == NULL)
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
	Mix_FreeChunk(scale)            ;
	
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

	Point points[8]   =      { {  500 ,  500 , -500 , 0 , 0 } , { -500 ,  500 , -500 , 0 , 0 } , { -500 , -500 , -500 , 0 , 0 } , 
                               {  500 , -500 , -500 , 0 , 0 } , {  500 ,  500 ,  500 , 0 , 0 } , { -500 ,  500 ,  500 , 0 , 0 } , 
                               { -500 , -500 ,  500 , 0 , 0 } , {  500 , -500 ,  500 , 0 , 0 } }    ;


	Point normales[12]   =   { {    0 ,    0 , -100 , 0 , 0 } , {    0 ,    0 , -100 , 0 , 0 } , {  100 ,    0 ,    0 , 0 , 0 } , 
                               {  100 ,    0 ,    0 , 0 , 0 } , {    0 ,    0 ,  100 , 0 , 0 } , {    0 ,    0 ,  100 , 0 , 0 } , 
                               { -100 ,    0 ,    0 , 0 , 0 } , { -100 ,    0 ,    0 , 0 , 0 } , {    0 , -100 ,    0 , 0 , 0 } ,
                               {    0 , -100 ,    0 , 0 , 0 } , {    0 ,  100 ,    0 , 0 , 0 } , {    0 ,  100 ,    0 , 0 , 0 } }  ;
	
	
	cube->nbrePts       =   8     ;
	cube->center.x      =   0     ;
	cube->center.y      =   0     ;
	cube->center.z      =   3000  ;

	for(int i = 0 ; i < cube->nbrePts ; i++)
	{
		cube->ptsVsl[i].x    =    cube->ptsWrd[i].x   =    cube->ptsOrg[i].x    =    points[i].x    ;
		cube->ptsVsl[i].y    =    cube->ptsWrd[i].y   =    cube->ptsOrg[i].y    =    points[i].y    ;
		cube->ptsVsl[i].z    =    cube->ptsWrd[i].z   =    cube->ptsOrg[i].z    =    points[i].z    ;
	}

	// for(int i = 0 ; i < cube->nbrePts ; i++)
	// {
	// 	cube->ptsWrd[i].x   =    cube->ptsOrg[i].x + cube->center.x    ;
	// 	cube->ptsWrd[i].y   =    cube->ptsOrg[i].y + cube->center.y    ;
	// 	cube->ptsWrd[i].z   =    cube->ptsOrg[i].z + cube->center.z    ;
	// }

	cube->nbreSegment       =   12                  ;
	cube->segments[0][0]    =   &(cube->ptsVsl[0])   ;
	cube->segments[0][1]    =   &(cube->ptsVsl[1])   ;
	cube->segments[1][0]    =   &(cube->ptsVsl[1])   ;
	cube->segments[1][1]    =   &(cube->ptsVsl[2])   ;
	cube->segments[2][0]    =   &(cube->ptsVsl[2])   ;
	cube->segments[2][1]    =   &(cube->ptsVsl[3])   ;
	cube->segments[3][0]    =   &(cube->ptsVsl[3])   ;
	cube->segments[3][1]    =   &(cube->ptsVsl[0])   ;
	cube->segments[4][0]    =   &(cube->ptsVsl[4])   ;
	cube->segments[4][1]    =   &(cube->ptsVsl[5])   ;
	cube->segments[5][0]    =   &(cube->ptsVsl[5])   ;
	cube->segments[5][1]    =   &(cube->ptsVsl[6])   ;
	cube->segments[6][0]    =   &(cube->ptsVsl[6])   ;
	cube->segments[6][1]    =   &(cube->ptsVsl[7])   ;
	cube->segments[7][0]    =   &(cube->ptsVsl[7])   ;
	cube->segments[7][1]    =   &(cube->ptsVsl[4])   ;
	cube->segments[8][0]    =   &(cube->ptsVsl[0])   ;
	cube->segments[8][1]    =   &(cube->ptsVsl[4])   ;
	cube->segments[9][0]    =   &(cube->ptsVsl[1])   ;
	cube->segments[9][1]    =   &(cube->ptsVsl[5])   ;
	cube->segments[10][0]   =   &(cube->ptsVsl[2])   ;
	cube->segments[10][1]   =   &(cube->ptsVsl[6])   ;
	cube->segments[11][0]   =   &(cube->ptsVsl[3])   ;
	cube->segments[11][1]   =   &(cube->ptsVsl[7])   ;

	cube->nbreFace                   =   12                   ;
	cube->faces[0].vertices[0]       =   &(cube->ptsVsl[0])   ;
	cube->faces[0].vertices[1]       =   &(cube->ptsVsl[2])   ;
	cube->faces[0].vertices[2]       =   &(cube->ptsVsl[1])   ;
	cube->faces[1].vertices[0]       =   &(cube->ptsVsl[0])   ;
	cube->faces[1].vertices[1]       =   &(cube->ptsVsl[3])   ;
	cube->faces[1].vertices[2]       =   &(cube->ptsVsl[2])   ;
	cube->faces[2].vertices[0]       =   &(cube->ptsVsl[0])   ;
	cube->faces[2].vertices[1]       =   &(cube->ptsVsl[4])   ;
	cube->faces[2].vertices[2]       =   &(cube->ptsVsl[3])   ;
	cube->faces[3].vertices[0]       =   &(cube->ptsVsl[3])   ;
	cube->faces[3].vertices[1]       =   &(cube->ptsVsl[4])   ;
	cube->faces[3].vertices[2]       =   &(cube->ptsVsl[7])   ;
	cube->faces[4].vertices[0]       =   &(cube->ptsVsl[7])   ;
	cube->faces[4].vertices[1]       =   &(cube->ptsVsl[4])   ;
	cube->faces[4].vertices[2]       =   &(cube->ptsVsl[5])   ;
	cube->faces[5].vertices[0]       =   &(cube->ptsVsl[5])   ;
	cube->faces[5].vertices[1]       =   &(cube->ptsVsl[6])   ;
	cube->faces[5].vertices[2]       =   &(cube->ptsVsl[7])   ;
	cube->faces[6].vertices[0]       =   &(cube->ptsVsl[6])   ;
	cube->faces[6].vertices[1]       =   &(cube->ptsVsl[5])   ;
	cube->faces[6].vertices[2]       =   &(cube->ptsVsl[1])   ;
	cube->faces[7].vertices[0]       =   &(cube->ptsVsl[1])   ;
	cube->faces[7].vertices[1]       =   &(cube->ptsVsl[2])   ;
	cube->faces[7].vertices[2]       =   &(cube->ptsVsl[6])   ;
	cube->faces[8].vertices[0]       =   &(cube->ptsVsl[2])   ;
	cube->faces[8].vertices[1]       =   &(cube->ptsVsl[3])   ;
	cube->faces[8].vertices[2]       =   &(cube->ptsVsl[6])   ;
	cube->faces[9].vertices[0]       =   &(cube->ptsVsl[3])   ;
	cube->faces[9].vertices[1]       =   &(cube->ptsVsl[7])   ;
	cube->faces[9].vertices[2]       =   &(cube->ptsVsl[6])   ;
	cube->faces[10].vertices[0]      =   &(cube->ptsVsl[1])   ;
	cube->faces[10].vertices[1]      =   &(cube->ptsVsl[5])   ;
	cube->faces[10].vertices[2]      =   &(cube->ptsVsl[4])   ;
	cube->faces[11].vertices[0]      =   &(cube->ptsVsl[0])   ;
	cube->faces[11].vertices[1]      =   &(cube->ptsVsl[1])   ;
	cube->faces[11].vertices[2]      =   &(cube->ptsVsl[4])   ;

	for(int  i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->faces[i].nrmVsl.x    =    cube->faces[i].nrmWrd.x    =    cube->faces[i].nrmOrg.x    =     normales[i].x    ;
		cube->faces[i].nrmVsl.y    =    cube->faces[i].nrmWrd.y    =    cube->faces[i].nrmOrg.y    =     normales[i].y    ;
		cube->faces[i].nrmVsl.z    =    cube->faces[i].nrmWrd.z    =    cube->faces[i].nrmOrg.z    =     normales[i].z    ;	
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
	cube->scale              =   0.6        ;

	cube->texture   =  chargerImage("texture.png")       ;

	for(int i = 0 ; i < cube->nbreFace ; i++)
	{
		cube->faces[i].texture  =  cube->texture     ;
		cube->faces[i].owner    =  cube              ;
	}

	cube->sphereCenter.x   =   0   ;
	cube->sphereCenter.y   =   0   ;	
	cube->sphereCenter.z   =   0   ;

	cube->radius   =  (int)sqrt(500*500 + 500*500 + 500*500)   ;

	cube->	isVisible   =   false   ;

	return   ;
}

void  addObjectToScene(Objet*  objet)
{
		for(int  i = 0 ; i < objet->nbreFace ; i++)
		{
			facesQueue[nbreFaceScene]    =    &objet->faces[i]    ;
			//printf("x=%d, y=%d, z=%d\n", facesQueue[nbreFaceScene]->vertices[0]->x, facesQueue[nbreFaceScene]->vertices[0]->y, facesQueue[nbreFaceScene]->vertices[0]->z)  ;
			nbreFaceScene++    ;		
		}

	//printf("The number of faces =  %d\n", nbreFaceScene) ;

	return    ;
}

void  removeObjectFromScene(Objet*  objet)
{
		for(int  i = 0 ; i < nbreFaceScene ; i++)
		{
			if(facesQueue[i]->owner == objet)
			{
				facesQueue[i--]    =   facesQueue[nbreFaceScene-1]    ;
				nbreFaceScene--    ;
				//printf("x=%d, y=%d, z=%d\n", facesQueue[nbreFaceScene]->vertices[0]->x, facesQueue[nbreFaceScene]->vertices[0]->y, facesQueue[nbreFaceScene]->vertices[0]->z)  ;
			}	
		}
	//printf("The number of faces =  %d\n", nbreFaceScene) ;

	return    ;
}

void    initialisation(void)
{
	///-----------------------------3D objects initialization and scene loading--------------------------------//////
	
	Objet*    raziel    =  malloc(sizeof(Objet))  ;
	Objet*    dino      =  malloc(sizeof(Objet))  ;
	Objet*    kratos    =  malloc(sizeof(Objet))  ;
	Objet*    cube      =  malloc(sizeof(Objet))  ;
	Objet*    terrain   =  malloc(sizeof(Objet))  ;

	Objet*    room00    =  malloc(sizeof(Objet))  ;
	Objet*    room01    =  malloc(sizeof(Objet))  ;
	Objet*    room02    =  malloc(sizeof(Objet))  ;
	Objet*    room03    =  malloc(sizeof(Objet))  ;
	Objet*    room04    =  malloc(sizeof(Objet))  ;
	Objet*    room05    =  malloc(sizeof(Objet))  ;
	Objet*    room06    =  malloc(sizeof(Objet))  ;
	Objet*    room07    =  malloc(sizeof(Objet))  ;
	Objet*    room08    =  malloc(sizeof(Objet))  ;
	Objet*    room09    =  malloc(sizeof(Objet))  ;
	Objet*    room10    =  malloc(sizeof(Objet))  ;
	Objet*    room11    =  malloc(sizeof(Objet))  ;
	Objet*    room12    =  malloc(sizeof(Objet))  ;
	Objet*    room13    =  malloc(sizeof(Objet))  ;
	Objet*    room14    =  malloc(sizeof(Objet))  ;
	Objet*    room15    =  malloc(sizeof(Objet))  ;
	Objet*    room16    =  malloc(sizeof(Objet))  ;
	Objet*    room17    =  malloc(sizeof(Objet))  ;
	Objet*    room18    =  malloc(sizeof(Objet))  ;
	Objet*    room19    =  malloc(sizeof(Objet))  ;
	Objet*    room20    =  malloc(sizeof(Objet))  ;
	Objet*    room21    =  malloc(sizeof(Objet))  ;
	Objet*    room22    =  malloc(sizeof(Objet))  ;
	Objet*    room23    =  malloc(sizeof(Objet))  ;
	Objet*    room24    =  malloc(sizeof(Objet))  ;
	Objet*    room25    =  malloc(sizeof(Objet))  ;
	Objet*    room26    =  malloc(sizeof(Objet))  ;
	Objet*    room27    =  malloc(sizeof(Objet))  ;
	Objet*    room28    =  malloc(sizeof(Objet))  ;
	Objet*    room29    =  malloc(sizeof(Objet))  ;
	Objet*    room30    =  malloc(sizeof(Objet))  ;
	Objet*    room31    =  malloc(sizeof(Objet))  ;
	Objet*    room32    =  malloc(sizeof(Objet))  ;
	Objet*    room33    =  malloc(sizeof(Objet))  ;
	Objet*    room34    =  malloc(sizeof(Objet))  ;
	Objet*    room35    =  malloc(sizeof(Objet))  ;
	Objet*    room36    =  malloc(sizeof(Objet))  ;
	Objet*    room37    =  malloc(sizeof(Objet))  ;
	Objet*    room38    =  malloc(sizeof(Objet))  ;
	Objet*    room39    =  malloc(sizeof(Objet))  ;
	Objet*    room40    =  malloc(sizeof(Objet))  ;

	//controlledPlayer =  room00   ;
	
	loadCube(cube)              ;
	//loadOBJfile("Raziel/Raziel.obj", raziel, 0, 400, 1000, PI, 0.0, 0.0, 1.0)            ;	
	//loadOBJfile("assets/dino/dino.obj", dino, 0, 20000, 50000,  PI, 0.0, 0.0, 1.0)       ;
	//loadOBJfile("assets/kratos/kratos.obj", kratos, 0, 20000, 50000, PI, 0.0, 0.0, 1.0)  ;
	//loadOBJfile("assets/terrain.obj", terrain, 0, 0, 5000, 0.0, 0.0, 0.0, 1.0)           ;	
	loadOBJfile("assets/TR1-level1/room00.obj", room00, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room01.obj", room01, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room02.obj", room02, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room03.obj", room03, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room04.obj", room04, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room05.obj", room05, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room06.obj", room06, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room07.obj", room07, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room08.obj", room08, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room09.obj", room09, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room10.obj", room10, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room11.obj", room11, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room12.obj", room12, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room13.obj", room13, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room14.obj", room14, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room15.obj", room15, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room16.obj", room16, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room17.obj", room17, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room18.obj", room18, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room19.obj", room19, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room20.obj", room20, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room21.obj", room21, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room22.obj", room22, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room23.obj", room23, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room24.obj", room24, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room25.obj", room25, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room26.obj", room26, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room27.obj", room27, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room28.obj", room28, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room29.obj", room29, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;	
	loadOBJfile("assets/TR1-level1/room30.obj", room30, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room31.obj", room31, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room32.obj", room32, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room33.obj", room33, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room34.obj", room34, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room35.obj", room35, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room36.obj", room36, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room37.obj", room37, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room38.obj", room38, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room39.obj", room39, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;
	loadOBJfile("assets/TR1-level1/room40.obj", room40, 0, 0, 0, 0.0, 0.0, 0.0, 1.0)     ;

	allObjet[0]   =   room00  ;
	allObjet[1]   =   room01  ;
	allObjet[2]   =   room02  ;
	allObjet[3]   =   room03  ;
	allObjet[4]   =   room04  ;
	allObjet[5]   =   room05  ;
	allObjet[6]   =   room06  ;
	allObjet[7]   =   room07  ;
	allObjet[8]   =   room08  ;
	allObjet[9]   =   room09  ;
	allObjet[10]  =   room10  ;
	allObjet[11]  =   room11  ;
	allObjet[12]  =   room12  ;
	allObjet[13]  =   room13  ;
	allObjet[14]  =   room14  ;
	allObjet[15]  =   room15  ;
	allObjet[16]  =   room16  ;
	allObjet[17]  =   room17  ;
	allObjet[18]  =   room18  ;
	allObjet[19]  =   room19  ;	
	allObjet[20]  =   room20  ;
	allObjet[21]  =   room21  ;
	allObjet[22]  =   room22  ;
	allObjet[23]  =   room23  ;
	allObjet[24]  =   room24  ;
	allObjet[25]  =   room25  ;
	allObjet[26]  =   room26  ;
	allObjet[27]  =   room27  ;
	allObjet[28]  =   room28  ;
	allObjet[29]  =   room29  ;
	allObjet[30]  =   room30  ;
	allObjet[31]  =   room31  ;
	allObjet[32]  =   room32  ;
	allObjet[33]  =   room33  ;
	allObjet[34]  =   room34  ;
	allObjet[35]  =   room35  ;
	allObjet[36]  =   room36  ;
	allObjet[37]  =   room37  ;
	allObjet[38]  =   room38  ;
	allObjet[39]  =   room39  ;
	allObjet[40]  =   room40  ;


	//allObjet[0]   =   cube     ;
	//allObjet[0]   =   terrain  ;
	//allObjet[0]   =   raziel  ;
	//allObjet[0]     =   dino    ;
	//allObjet[0]     =   kratos    ;

	nbreOjectScene   =   41     ;	 
	
	//addObjectToScene(allObjet[0])   ;
	//addObjectToScene(allObjet[1])   ;
	
	//////------------------------Camera Initialization ------------------------------//////

	camera.posX   =    +340000    ;       // X you go right if it grows up
	camera.posY   =     -10000    ;	    // Y is reversed, you go down if it grows up
	camera.posZ   =    -560000    ;		// Z is going far ahead if it grows up

	camera.angleY   =   0.0f        ;
	camera.angleX   =   0.0f        ;
	camera.angleZ   =   0.0f        ;	
	
	//////------------------------Chargement des fichiers et des bibliothèques--------------------------//////

	initSDL()                 ;
	
	chargementFichirs()       ;

	////------------------------------------------Dessin------------------------------------------/////

	drawBackground()   ;
	
	////------------------------------------------Musique------------------------------------------/////
	
	// Mix_AllocateChannels(16)    ;
	
	Mix_PlayMusic(music, -1)              ;
	Mix_VolumeMusic(MIX_MAX_VOLUME/8)     ;
	
	Mix_Volume(1,MIX_MAX_VOLUME)          ;
	Mix_Volume(2,MIX_MAX_VOLUME/2)        ;
	Mix_Volume(3,MIX_MAX_VOLUME/4)        ;
	
	Mix_PlayChannel(1 , transl , -1)    ;
	Mix_PlayChannel(2 , rotation , -1)  ;
	Mix_PlayChannel(3 , scale , -1)     ;
	
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
		if(mesh->ptsVsl[i].z != 0)
		{	
			mesh->ptsVsl[i].X  =  ((mesh->ptsVsl[i].x * DISTANCE_FOCAL) / mesh->ptsVsl[i].z) + (RES_HORIZ / 2)    ;	
			mesh->ptsVsl[i].Y  =  ((mesh->ptsVsl[i].y * DISTANCE_FOCAL) / mesh->ptsVsl[i].z) + (RES_VERT  / 2)    ;				
			// if(mesh->ptsVsl[i].X  < 0 || mesh->ptsVsl[i].X  >= RES_HORIZ || mesh->ptsVsl[i].Y  < 0 || mesh->ptsVsl[i].Y  >= RES_VERT)
			// {
			// 	printf("vertex :  %d, (X=%d, Y = %d)\n", i, mesh->ptsVsl[i].X, mesh->ptsVsl[i].Y)  ;
			// }
		}
	}


	////--------------------------dessin des mesh---------------------------------//////

	for(i = 0 ; i < mesh->nbreSegment ; i++)
	{		
		//printf("Segment = %i, (X1=%d, Y1=%d)-----(X2=%d, Y2=%d)\n", i,mesh->segments[i][0]->X, mesh->segments[i][0]->Y, mesh->segments[i][1]->X, mesh->segments[i][1]->Y)   ;
		ligne(mesh->segments[i][0]->X , mesh->segments[i][0]->Y , mesh->segments[i][1]->X , mesh->segments[i][1]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;
	}

	return   ;
}

void    displayScene()
{
	int    j , k     ;

	////--------------------------------projection---------------------------------//////

	for(int  objectCnt = 0 ; objectCnt < nbreOjectScene ; objectCnt++)
	{
		// Frustum culling, then changing perspective of the object from absolute world to camera perspective
		frustumCulling(allObjet[objectCnt])                 ;		
		transformToCameraPerspective(allObjet[objectCnt])   ;		
		
		for(int  i = 0 ; i < allObjet[objectCnt]->nbrePts ; i++)
		{
			// The classic perspective formula: (3D_Coord(x or y) * Focal_Length / Depth(z)) + Screen_Offset
			if(allObjet[objectCnt]->ptsVsl[i].z != 0)
			{
				allObjet[objectCnt]->ptsVsl[i].X  =  ((allObjet[objectCnt]->ptsVsl[i].x * DISTANCE_FOCAL) / allObjet[objectCnt]->ptsVsl[i].z) + (RES_HORIZ / 2)    ;	
				allObjet[objectCnt]->ptsVsl[i].Y  =  ((allObjet[objectCnt]->ptsVsl[i].y * DISTANCE_FOCAL) / allObjet[objectCnt]->ptsVsl[i].z) + (RES_VERT  / 2)    ;
				//printf("Object = %d | X = %d; Y = %d | x = %d, y = %d, z = %d, \n", objectCnt, allObjet[objectCnt]->ptsVsl[i].X , allObjet[objectCnt]->ptsVsl[i].Y , allObjet[objectCnt]->ptsVsl[i].x , allObjet[objectCnt]->ptsVsl[i].y , allObjet[objectCnt]->ptsVsl[i].z);
			}
		}
	}
	
	////-------------------------- 2. DEPTH SORTING ---------------------------------//////
	
	// Sorts all faces from back-to-front so that near objects are drawn over far objects.	
	painterAlgorithmSort()  ;
	
	////--------------------------dessin des faces---------------------------------//////

	// Main loop to process every face in the global scene list.
	for(int i = 0 ; i < nbreFaceScene ; i++)
	{		
		if((((facesQueue[i]->nrmVsl.x * (facesQueue[i]->vertices[0]->X - (RES_HORIZ / 2))) + (facesQueue[i]->nrmVsl.y * (facesQueue[i]->vertices[0]->Y - (RES_VERT  / 2))) + (facesQueue[i]->nrmVsl.z * DISTANCE_FOCAL)) < 0) // Back-Face Culling : using the dot-product Ux * Vx + Uy * Vy + Uz * Vz, if it is positif they are pointing to the same direction.
		&& ((facesQueue[i]->vertices[0]->X > 0) || (facesQueue[i]->vertices[1]->X > 0) || (facesQueue[i]->vertices[2]->X > 0)) && ((facesQueue[i]->vertices[0]->X < (RES_HORIZ-1)) || (facesQueue[i]->vertices[1]->X < (RES_HORIZ-1)) || (facesQueue[i]->vertices[2]->X < (RES_HORIZ-1))) 
		&& ((facesQueue[i]->vertices[0]->Y > 0) || (facesQueue[i]->vertices[1]->Y > 0) || (facesQueue[i]->vertices[2]->Y > 0)) && ((facesQueue[i]->vertices[0]->Y < (RES_VERT-1)) || (facesQueue[i]->vertices[1]->Y < (RES_VERT-1)) || (facesQueue[i]->vertices[2]->Y < (RES_VERT-1)))  // Checks if the triangle is at least partially within the screen boundaries.
		&& ((facesQueue[i]->vertices[0]->z > DISTANCE_FOCAL)) && ((facesQueue[i]->vertices[1]->z > DISTANCE_FOCAL)) && ((facesQueue[i]->vertices[2]->z > DISTANCE_FOCAL))) // Checks if the triangle is too close or behind the camera
		{	
			//printf("face[%i] = (%d,%d,%d)(%d,%d)-(%d,%d,%d)(%d,%d)-(%d,%d,%d)(%d,%d)\n", i, facesQueue[i]->vertices[0]->x, facesQueue[i]->vertices[0]->y, facesQueue[i]->vertices[0]->z, facesQueue[i]->vertices[0]->X , facesQueue[i]->vertices[0]->Y , facesQueue[i]->vertices[1]->x, facesQueue[i]->vertices[1]->y, facesQueue[i]->vertices[1]->z, facesQueue[i]->vertices[1]->X , facesQueue[i]->vertices[1]->Y , facesQueue[i]->vertices[2]->x, facesQueue[i]->vertices[2]->y, facesQueue[i]->vertices[2]->z, facesQueue[i]->vertices[2]->X , facesQueue[i]->vertices[2]->Y )   ;
			
			int    haut   =   0      ;
			int    bas , millieu     ;
			int    baleillage , dis  ;
			
			// Identifies which vertex is the Top (haut), Middle (millieu), and Bottom (bas) of the triangle.
			if(facesQueue[i]->vertices[0]->Y > facesQueue[i]->vertices[1]->Y)
			{
				haut   =   1   ;
			}
			
			if(facesQueue[i]->vertices[haut]->Y > facesQueue[i]->vertices[2]->Y)
			{
				haut   =   2   ;
			}
			
			if(facesQueue[i]->vertices[(haut+1)%3]->Y > facesQueue[i]->vertices[(haut+2)%3]->Y)
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
			
			int    X0    =   facesQueue[i]->vertices[haut]->X       ;
			int    Y0    =   facesQueue[i]->vertices[haut]->Y       ;
			int    X1    =   facesQueue[i]->vertices[millieu]->X    ;
			int    Y1    =   facesQueue[i]->vertices[millieu]->Y    ;
			int    X2    =   facesQueue[i]->vertices[bas]->X        ;
			int    Y2    =   facesQueue[i]->vertices[bas]->Y        ;
			
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
			
			int    XV0    =   facesQueue[i]->uv[haut][0]     ;
			int    YV0    =   facesQueue[i]->uv[haut][1]     ;
			int    XV1    =   facesQueue[i]->uv[bas][0]      ;
			int    YV1    =   facesQueue[i]->uv[bas][1]      ;
			
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
			int    XH1    =   facesQueue[i]->uv[millieu][0]      ;
			int    YH1    =   facesQueue[i]->uv[millieu][1]      ;
			
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
						{//if (debug)  printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH , facesQueue[i]->texture))      ;
							
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
						{//if (debug)  printf(" j=%i,y=%i,xH=%i,yH=%i\n" ,j,y,xH,yH)   ;
							setPixel(j , y ,getPixel(xH , yH , facesQueue[i]->texture))      ;
							
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
			
			///////------ Excellent way to debug by highlighting the triangle in the 3D world----///////
			
			//ligne( X0 , Y0 , X1 , Y1 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;
			//ligne( X1 , Y1 , X2 , Y2 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;
			//ligne( X0 , Y0 , X2 , Y2 , SDL_MapRGB(affichage->format, 5 , 2 , 128))    ;	

			//int i =  530  ;  //(SDL_GetTicks()/50) % allObjet[0]->nbreFace  ;
			//for(int i = 0 ; i < 10 ; i++) 
			// ligne(allObjet[0]->faces[i].vertices[0]->X , allObjet[0]->faces[i].vertices[0]->Y , allObjet[0]->faces[i].vertices[1]->X , allObjet[0]->faces[i].vertices[1]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;
			// ligne(allObjet[0]->faces[i].vertices[1]->X , allObjet[0]->faces[i].vertices[1]->Y , allObjet[0]->faces[i].vertices[2]->X , allObjet[0]->faces[i].vertices[2]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;
			// ligne(allObjet[0]->faces[i].vertices[2]->X , allObjet[0]->faces[i].vertices[2]->Y , allObjet[0]->faces[i].vertices[0]->X , allObjet[0]->faces[i].vertices[0]->Y , SDL_MapRGB(affichage->format, 5 , 200 , 128))    ;
			
			// printf("normale = (%i, %i, %i)\n", allObjet[0]->faces[530].nrmVsl.x, allObjet[0]->faces[530].nrmVsl.y, allObjet[0]->faces[530].nrmVsl.z)   ;

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
	
	objet->center.x    +=   Dx     ;
	objet->center.y    +=   Dy     ;
	objet->center.z    +=   Dz     ;
	
	for(i = 0 ; i < objet->nbrePts ; i++)
	{
		objet->ptsWrd[i].x    +=  Dx     ;
		objet->ptsWrd[i].y    +=  Dy     ;
		objet->ptsWrd[i].z    +=  Dz     ;
	}

	objet->sphereCenter.x   +=   Dx     ;
	objet->sphereCenter.y   +=   Dy     ;
	objet->sphereCenter.z   +=   Dz     ;
	
	return     ;
}

static inline  void    localRotationScale(Objet * objet, float angleX, float angleY, float angleZ, float Scale)
{
	objet->angleX   =   angleX     ;
	objet->angleY   =   angleY     ;
	objet->angleZ   =   angleZ     ;

	objet->scale   =   Scale    ;
	
	////---------------------------------rotation--------------------------------------//////
	
	for(int i = 0 ; i < objet->nbrePts ; i++)
	{
		
		// par raport a l'axe Z
		objet->ptsWrd[i].x   =    (objet->ptsOrg[i].x * cos(objet->angleZ)) - (objet->ptsOrg[i].y * sin(objet->angleZ))  ;
		objet->ptsWrd[i].y   =    (objet->ptsOrg[i].x * sin(objet->angleZ)) + (objet->ptsOrg[i].y * cos(objet->angleZ))  ;
		
		// par raport a l'axe Y
		objet->ptsWrd[i].z   =    (objet->ptsOrg[i].z * cos(objet->angleY)) - (objet->ptsWrd[i].x * sin(objet->angleY))   ;
		objet->ptsWrd[i].x   =    (objet->ptsOrg[i].z * sin(objet->angleY)) + (objet->ptsWrd[i].x * cos(objet->angleY))   ;
		
		// z value save is mandatory, otherwire its value will be altered in the next instruction, and useless in the insrtuction after
		int    z   =  objet->ptsWrd[i].z   ;  
		
		// par raport a l'axe X
		objet->ptsWrd[i].z   =    (objet->ptsWrd[i].y * sin(objet->angleX)) + (z * cos(objet->angleX))   ;
		objet->ptsWrd[i].y   =    (objet->ptsWrd[i].y * cos(objet->angleX)) - (z * sin(objet->angleX))   ;
		
	}
	
	for(int i = 0 ; i < objet->nbreFace ; i++)
	{
		
		// par raport a l'axe Z
		objet->faces[i].nrmWrd.x   =    (objet->faces[i].nrmOrg.x * cos(objet->angleZ)) - (objet->faces[i].nrmOrg.y * sin(objet->angleZ))  ;
		objet->faces[i].nrmWrd.y   =    (objet->faces[i].nrmOrg.x * sin(objet->angleZ)) + (objet->faces[i].nrmOrg.y * cos(objet->angleZ))  ;
		
		// par raport a l'axe Y
		objet->faces[i].nrmWrd.z   =    (objet->faces[i].nrmOrg.z * cos(objet->angleY)) - (objet->faces[i].nrmWrd.x * sin(objet->angleY))   ;
		objet->faces[i].nrmWrd.x   =    (objet->faces[i].nrmOrg.z * sin(objet->angleY)) + (objet->faces[i].nrmWrd.x * cos(objet->angleY))   ;
		
		int    z   =  objet->faces[i].nrmWrd.z   ;
		
		// par raport a l'axe X
		objet->faces[i].nrmWrd.z   =    (objet->faces[i].nrmWrd.y * sin(objet->angleX)) + (z * cos(objet->angleX))   ;
		objet->faces[i].nrmWrd.y   =    (objet->faces[i].nrmWrd.y * cos(objet->angleX)) - (z * sin(objet->angleX))   ;
		
	}
	
	/////-----------------------------chagement d'echelle--------------------------------/////
	
	for(int i = 0 ;  i < objet->nbrePts ; i++)
	{
		objet->ptsWrd[i].x    =  (int)(objet->ptsWrd[i].x * objet->scale) + objet->center.x    ;
		objet->ptsWrd[i].y    =  (int)(objet->ptsWrd[i].y * objet->scale) + objet->center.y    ;
		objet->ptsWrd[i].z    =  (int)(objet->ptsWrd[i].z * objet->scale) + objet->center.z    ;
	}
		
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

bool loadOBJfile(const  char*  path, Objet*  objet, int posX, int posY, int posZ, float angleX, float angleY, float angleZ, float scale)
{
    // extract directory from path to handle relative texture paths
	FILE*  file    =    fopen(path, "r")    ;	
	char   dirPath[256]                     ;

	int    maxX, minX, maxY, minY, maxZ, minZ   ;
	bool   firstVertex     =    true            ;

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

	objet->center.x   =  posX    ;
	objet->center.y   =  posY    ;
	objet->center.z   =  posZ    ;

	objet->angleX   =   angleX   ;
	objet->angleY   =   angleY   ;
	objet->angleZ   =   angleZ   ;
	objet->scale    =   scale    ;

	objet->	isVisible   =   false   ;

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
					//printf("texturePath = %s\n", texturePath)   ;
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
            objet->ptsOrg[objet->nbrePts].x    =   objet->ptsVsl[objet->nbrePts].x    =   objet->ptsWrd[objet->nbrePts].x   =   (int)(x * 1000)   ;
            objet->ptsOrg[objet->nbrePts].y    =   objet->ptsVsl[objet->nbrePts].y    =   objet->ptsWrd[objet->nbrePts].y   =   (int)(y * 1000)   ;
            objet->ptsOrg[objet->nbrePts].z    =   objet->ptsVsl[objet->nbrePts].z    =   objet->ptsWrd[objet->nbrePts].z   =   (int)(z * 1000)   ;

			// initialize the bounding box borders with the first vertex
			if(firstVertex)
			{
				maxX  =   minX   =   objet->ptsOrg[objet->nbrePts].x   ;
				maxY  =   minY   =   objet->ptsOrg[objet->nbrePts].y   ;
				maxZ  =   minZ   =   objet->ptsOrg[objet->nbrePts].z   ;				
				
				firstVertex      =    false               ;
			}

			// We calculate the sphere bounding box
			if(objet->ptsOrg[objet->nbrePts].x > maxX)   maxX   =   objet->ptsOrg[objet->nbrePts].x   ;
			if(objet->ptsOrg[objet->nbrePts].x < minX)   minX   =   objet->ptsOrg[objet->nbrePts].x   ;
			if(objet->ptsOrg[objet->nbrePts].y > maxY)   maxY   =   objet->ptsOrg[objet->nbrePts].y   ;
			if(objet->ptsOrg[objet->nbrePts].y < minY)   minY   =   objet->ptsOrg[objet->nbrePts].y   ;
			if(objet->ptsOrg[objet->nbrePts].z > maxZ)   maxZ   =   objet->ptsOrg[objet->nbrePts].z   ;
			if(objet->ptsOrg[objet->nbrePts].z < minZ)   minZ   =   objet->ptsOrg[objet->nbrePts].z   ;		

			//objet->ptsWrd[objet->nbrePts].z   +=   objet->center.z  ;
            
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
			//printf("UV %d: (%d, %d)\n", uvCount, UV[uvCount][0], UV[uvCount][1])   ;
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
				objet->faces[objet->nbreFace].vertices[0]    =    &objet->ptsVsl[v1[0] - 1]    ;
				objet->faces[objet->nbreFace].vertices[1]    =    &objet->ptsVsl[v2[0] - 1]    ;
				objet->faces[objet->nbreFace].vertices[2]    =    &objet->ptsVsl[v3[0] - 1]    ;

				calculateFaceNormal(&objet->faces[objet->nbreFace].nrmOrg, objet->faces[objet->nbreFace].vertices[0], objet->faces[objet->nbreFace].vertices[1], objet->faces[objet->nbreFace].vertices[2])   ;

				objet->faces[objet->nbreFace].nrmVsl     =   objet->faces[objet->nbreFace].nrmWrd     =   objet->faces[objet->nbreFace].nrmOrg     ;

				// Store the segments for the face		
				
				// objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[0]     ;
				// objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[1]     ;
				// objet->nbreSegment++    ;

				// objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[1]     ;
				// objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[2]     ;
				// objet->nbreSegment++    ;

				// objet->segments[objet->nbreSegment][0]    =    objet->faces[objet->nbreFace].vertices[2]     ;
				// objet->segments[objet->nbreSegment][1]    =    objet->faces[objet->nbreFace].vertices[0]     ;
				// objet->nbreSegment++    ;
				//printf("Segment %d\n", objet->nbreSegment);
				
				// Store the texture coordinate
				objet->faces[objet->nbreFace].uv[0][0]    =   UV[v1[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[0][1]    =   UV[v1[1] - 1][1]   ;
				objet->faces[objet->nbreFace].uv[1][0]    =   UV[v2[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[1][1]    =   UV[v2[1] - 1][1]   ;
				objet->faces[objet->nbreFace].uv[2][0]    =   UV[v3[1] - 1][0]   ;
				objet->faces[objet->nbreFace].uv[2][1]    =   UV[v3[1] - 1][1]   ;

				objet->faces[objet->nbreFace].texture     =   objet->texture     ;
				objet->faces[objet->nbreFace].owner       =   objet              ;
				//printf("Face %d (x=%d, y=%d, z=%d).\n", objet->nbreFace, v1[0], v2[0], v3[0])   ;
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

	objet->sphereCenter.x   =   (maxX + minX) / 2  ;
	objet->sphereCenter.y   =   (maxY + minY) / 2  ;
	objet->sphereCenter.z   =   (maxZ + minZ) / 2  ;

	objet->radius   =  (int)sqrt(((maxX - objet->sphereCenter.x) * (long long)(maxX - objet->sphereCenter.x)) + ((maxY - objet->sphereCenter.y) * (long long)(maxY - objet->sphereCenter.y)) + ((maxZ - objet->sphereCenter.z) * (long long)(maxZ - objet->sphereCenter.z)))   ;	
	
    fclose(file)   ;

    return   true  ;
}

int    twoPointsDistance(Point* p1, Point* p2)
{
	return   (int)sqrt((p1->x - p2->x) * (long long)(p1->x - p2->x) + (p1->y - p2->y) * (long long)(p1->y - p2->y) + (p1->z - p2->z) * (long long)(p1->z - p2->z))   ;	
}

void    frustumCulling(Objet* objet)
{
	// calculate the relative sphere center of the object to the camera
	Point   relativeSphereCenter   ; 
	Point   relativeCenter         ;               // it is imperative to calculate the relative center of the object to the camera, because the sphere center is relative to the object center, and not to the world origin
	Point   origin   =   {   0,   0,   0   }   ;   // the origin of the camera after transformation of the world

	relativeCenter.x   =    objet->center.x - camera.posX   ;
	relativeCenter.y   =    objet->center.y - camera.posY   ;
	relativeCenter.z   =    objet->center.z - camera.posZ   ;		

	relativeSphereCenter.x     =   objet->sphereCenter.x + relativeCenter.x    ;
	relativeSphereCenter.y     =   objet->sphereCenter.y + relativeCenter.y    ;
	relativeSphereCenter.z     =   objet->sphereCenter.z + relativeCenter.z    ;
	
	// calculate the relative sphere center after a rotation of the camera
	int  x   =  relativeSphereCenter.x   ;   // x needs to be saved, because its value change in the 2nd instruction		
	relativeSphereCenter.x   =    (x * cos(-camera.angleY)) - (relativeSphereCenter.z * sin(-camera.angleY))  ;
	relativeSphereCenter.z   =    (x * sin(-camera.angleY)) + (relativeSphereCenter.z * cos(-camera.angleY))  ;

	int  y   =  relativeSphereCenter.y   ; 	
	relativeSphereCenter.y   =    (y * cos(camera.angleX)) - (relativeSphereCenter.z * sin(camera.angleX))   ;
	relativeSphereCenter.z   =    (y * sin(camera.angleX)) + (relativeSphereCenter.z * cos(camera.angleX))   ;

	x   =  relativeSphereCenter.x   ;
	relativeSphereCenter.x   =    (x * cos(camera.angleZ)) - (relativeSphereCenter.y * sin(camera.angleZ))  ;
	relativeSphereCenter.y   =    (x * sin(camera.angleZ)) + (relativeSphereCenter.y * cos(camera.angleZ))  ;

	bool   becomeVisible    =   false   ;

	// test if the sphere center is behind the camera
	if(relativeSphereCenter.z < 0)
	{
		if(twoPointsDistance(&relativeSphereCenter, &origin) < objet->radius)
		{
			becomeVisible    =   true    ;			
		}
		else
		{
			becomeVisible    =   false   ;			
		}
	}
	else
	{
		// test if the object is too far away from the camera, outside the horizon
		if(relativeSphereCenter.z - objet->radius > HORIZON_END)
		{
			becomeVisible    =   false   ;			
		}
		else
		{
			int   dist   =   (int)(DISTANCE_FOCAL/(float)RES_HORIZ_DIV_2 * relativeSphereCenter.x + relativeSphereCenter.z) / sqrt((DISTANCE_FOCAL * DISTANCE_FOCAL) / (float)(RES_HORIZ_DIV_2 * RES_HORIZ_DIV_2) + 1) ; // calculate the distance from the sphere center to the camera frustum horizontal left plane
			
			// test if the sphere center is outside the camera frustum horizontal left plane
			if((dist < 0) && (abs(dist) > objet->radius))
			{
				becomeVisible    =   false   ;				
			}
			else
			{
				dist   =   (int)(DISTANCE_FOCAL/(float)RES_HORIZ_DIV_2 * relativeSphereCenter.x - relativeSphereCenter.z) / sqrt((DISTANCE_FOCAL * DISTANCE_FOCAL) / (float)(RES_HORIZ_DIV_2 * RES_HORIZ_DIV_2) + 1) ; // calculate the distance from the sphere center to the camera frustum horizontal right plane
				
				// test if the sphere center is outside the camera frustum horizontal right plane
				if((dist > 0) && (abs(dist) > objet->radius))
				{
					becomeVisible    =   false   ;					
				}
				else
				{
					dist   =   (int)(DISTANCE_FOCAL/(float)RES_VERT_DIV_2 * relativeSphereCenter.y + relativeSphereCenter.z) / sqrt((DISTANCE_FOCAL * DISTANCE_FOCAL) / (float)(RES_VERT_DIV_2 * RES_VERT_DIV_2) + 1) ; // calculate the distance from the sphere center to the camera frustum vertical top plane
					
					// test if the sphere center is outside the camera frustum vertical top plane
					if((dist < 0) && (abs(dist) > objet->radius))
					{
						becomeVisible    =   false   ;						
					}
					else
					{
						dist   =   (int)(DISTANCE_FOCAL/(float)RES_VERT_DIV_2 * relativeSphereCenter.y - relativeSphereCenter.z) / sqrt((DISTANCE_FOCAL * DISTANCE_FOCAL) / (float)(RES_VERT_DIV_2 * RES_VERT_DIV_2) + 1) ; // calculate the distance from the sphere center to the camera frustum vertical bottom plane
						
						// test if the sphere center is outside the camera frustum vertical bottom plane
						if((dist > 0) && (abs(dist) > objet->radius))
						{
							becomeVisible    =   false   ;							
						}
						else
						{
							// The sphere center is inside the camera frustum, so the object should be visible
							becomeVisible    =   true    ;							
						}
					}
				}
			}
		}
	}

	// update the visibility status of the object and add/remove it from the scene accordingly
	if((objet->isVisible == false)&&(becomeVisible == true))
	{
		objet->isVisible   =    true  ;

		addObjectToScene(objet)       ;
	}
	else
	{
		if((objet->isVisible == true)&&(becomeVisible == false))
		{
			objet->isVisible   =    false  ;

			removeObjectFromScene(objet)   ;
		}
	}                

	return   ;
}

void   painterAlgorithmSort()
{
	// calculate the average Z value for each face
	int   averageZ[NBRE_FACE_MAX_SCENE]      ;

	for (int i = 0 ; i < nbreFaceScene ; i++)
	{
		averageZ[i]   =   (facesQueue[i]->vertices[0]->z + facesQueue[i]->vertices[1]->z + facesQueue[i]->vertices[2]->z + facesQueue[i]->vertices[2]->z)    ;
	}

	// Sort in descending order based on their average Z value (depth) using insertion sort algorithm
	for (int i = 1 ; i < nbreFaceScene ; i++) 
	{
		Face*  key       =    facesQueue[i]        ;
		int    keyAvg    =    averageZ[i]          ;

		int j = i  ;
		while(j > 0 && keyAvg > averageZ[j - 1]) 		
		{			
			facesQueue[j]     =   facesQueue[j - 1]     ;
			averageZ[j]       =   averageZ[j - 1]       ;

			j--   ;
		}

		facesQueue[j]     =   key     ;
		averageZ[j]       =   keyAvg  ;
	}

	return  ;
}

// I will use the movement by buttons temporarily, the buttons will be separated in the future
void    cameraMovement(Button* buttons, int speed, float spin)  
{
	if(buttons->W == 1)
	{
		camera.posZ   +=   speed * cos(camera.angleY)   ;
		camera.posX   -=   speed * sin(camera.angleY)   ;
		camera.posY   +=   speed * sin(camera.angleX)   ;
	}
	
	if(buttons->S == 1)
	{
		camera.posZ   -=   speed * cos(camera.angleY)   ;
		camera.posX   +=   speed * sin(camera.angleY)   ;
		camera.posY   -=   speed * sin(camera.angleX)   ;
	}
	
	if(buttons->A == 1)
	{
		camera.posX   -=   speed * cos(camera.angleY)   ;
		camera.posZ   -=   speed * sin(camera.angleY)   ;
		// the sidewalk displacement is not fully implemented, but it aggreable to use for now
	}
	
	if(buttons->D == 1)
	{
		camera.posX   +=   speed * cos(camera.angleY)   ;
		camera.posZ   +=   speed * sin(camera.angleY)   ;
		// the sidewalk displacement is not fully implemented, but it aggreable to use for now
	}

	if(buttons->Q == 1)
	{
		camera.posY   -=   speed   ;
	}
	
	if(buttons->E == 1)
	{
		camera.posY   +=   speed   ;
	}

	if(buttons->right == 1)	
	{
		camera.angleY   -=   spin   ;
	}

	if(buttons->left == 1)	
	{
		camera.angleY   +=   spin   ;
	}

	if(buttons->up == 1)	
	{
		camera.angleX   -=   spin   ;
	}

	if(buttons->down == 1)	
	{
		camera.angleX   +=   spin   ;
	}

	if(buttons->pageUp == 1)	
	{
		camera.angleZ   -=   spin   ;
	}

	if(buttons->pageDown == 1)	
	{
		camera.angleZ   +=   spin   ;
	}

	return   ;
}

void   transformToCameraPerspective(Objet* objet)
{
	Point   relativeCenter   ;

	if(objet->isVisible == false) 
		return   ;

	relativeCenter.x   =    objet->center.x - camera.posX   ;
	relativeCenter.y   =    objet->center.y - camera.posY   ;
	relativeCenter.z   =    objet->center.z - camera.posZ   ;	
	
	for(int i = 0 ; i < objet->nbrePts ; i++)
	{
		objet->ptsVsl[i].x     =   objet->ptsWrd[i].x + relativeCenter.x    ;
		objet->ptsVsl[i].y     =   objet->ptsWrd[i].y + relativeCenter.y    ;
		objet->ptsVsl[i].z     =   objet->ptsWrd[i].z + relativeCenter.z    ;		
	}

	// rotation relative to the camera
	for(int i = 0 ; i < objet->nbrePts ; i++)
	{
		int  x   =  objet->ptsVsl[i].x   ;   // x needs to be saved, because its value change in the 2nd instruction
		
		objet->ptsVsl[i].x   =    (x * cos(-camera.angleY)) - (objet->ptsVsl[i].z * sin(-camera.angleY))  ;
		objet->ptsVsl[i].z   =    (x * sin(-camera.angleY)) + (objet->ptsVsl[i].z * cos(-camera.angleY))  ;

		int  y   =  objet->ptsVsl[i].y   ; 
		
		objet->ptsVsl[i].y   =    (y * cos(camera.angleX)) - (objet->ptsVsl[i].z * sin(camera.angleX))   ;
		objet->ptsVsl[i].z   =    (y * sin(camera.angleX)) + (objet->ptsVsl[i].z * cos(camera.angleX))   ;

		x   =  objet->ptsVsl[i].x   ;

		objet->ptsVsl[i].x   =    (x * cos(camera.angleZ)) - (objet->ptsVsl[i].y * sin(camera.angleZ))  ;
		objet->ptsVsl[i].y   =    (x * sin(camera.angleZ)) + (objet->ptsVsl[i].y * cos(camera.angleZ))  ;
	}

	// rotation of the normal vectors relative to the camera
	for(int i = 0 ; i < objet->nbreFace ; i++)
	{
		int  x   =  objet->faces[i].nrmWrd.x   ;   // x needs to be saved, because its value change in the 2nd instruction
		
		objet->faces[i].nrmVsl.x   =    (x * cos(-camera.angleY)) - (objet->faces[i].nrmWrd.z * sin(-camera.angleY))  ;
		objet->faces[i].nrmVsl.z   =    (x * sin(-camera.angleY)) + (objet->faces[i].nrmWrd.z * cos(-camera.angleY))  ;

		int  y   =  objet->faces[i].nrmWrd.y   ; 
		
		objet->faces[i].nrmVsl.y   =    (y * cos(camera.angleX)) - (objet->faces[i].nrmVsl.z * sin(camera.angleX))   ;
		objet->faces[i].nrmVsl.z   =    (y * sin(camera.angleX)) + (objet->faces[i].nrmVsl.z * cos(camera.angleX))   ;

		x   =  objet->faces[i].nrmVsl.x   ;

		objet->faces[i].nrmVsl.x   =    (x * cos(camera.angleZ)) - (objet->faces[i].nrmVsl.y * sin(camera.angleZ))  ;
		objet->faces[i].nrmVsl.y   =    (x * sin(camera.angleZ)) + (objet->faces[i].nrmVsl.y * cos(camera.angleZ))  ;		
	}

	return   ;
}

void    PlayerMovement(Button* buttons, Objet* player)
{
	static   float   rotX = 0.0f   ;
	static   float   rotY = 0.0f   ;
	static   float   rotZ = 0.0f   ;
	
	if(player != NULL)
	{		
		if(buttons->up == 1)
		{
			rotX   +=   0.01f   ;
		}
		
		if(buttons->down == 1)
		{
			rotX   -=   0.01f   ;
		}
		
		if(buttons->right == 1)	
		{
			rotZ   +=   0.01f   ;			
		}

		if(buttons->left == 1)	
		{
			rotZ   -=   0.01f   ;			
		}

		if(buttons->pageUp == 1)
		{
			rotY   +=   0.01f   ;			
		}
		
		if(buttons->pageDown == 1)
		{
			rotY   -=   0.01f   ;				
		}

		localRotationScale(player, rotX, rotY, rotZ, 1.0f)   ;
	}

	return   ;
}
