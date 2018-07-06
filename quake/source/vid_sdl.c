// vid_sdl.h -- sdl video driver 

#include <sys/stat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "quakedef.h"
#include "d_local.h"

// R2-Tec
#include "sys_r2-tec.h"
static TTF_Font *gTTFFont = NULL;

viddef_t vid; // global video state
unsigned short d_8to16table[256];

extern uint8_t main_bg[157735];

#define DISABLE_CDROM

// Quake Screen...
static SDL_Surface *hwscreen = NULL;
static SDL_Surface *screen = NULL;

int min_vid_width = 320;

int VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes = 0;
byte *VGA_pagebase;

/* static qboolean mouse_avail;
static float mouse_x, mouse_y;
static int mouse_oldbuttonstate = 0; */

//vars for mlook
float start_yaw;
float yaw_modifier=0;

void drawImage(SDL_Surface *scr, SDL_Surface *image, int x, int y)
{
#if 1
	SDL_Rect dstRect;
	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = image->w;
	dstRect.h = image->h;
	SDL_BlitSurface(image, NULL, scr, &dstRect);
#else
  {
    if(SDL_MUSTLOCK(hwscreen)) SDL_LockSurface(hwscreen);
    int x, y;
    SDL_Surface *p = SDL_ConvertSurface(image, scr->format, 0);
    uint32_t *s = (uint32_t*)p->pixels;
    uint32_t *d = (uint32_t*)hwscreen->pixels;
    for(y=0; y<240; y++){
      for(x=0; x<160; x++){
        *d++ = *s++;
      }
      d+= 160;
    }
    if(SDL_MUSTLOCK(hwscreen)) SDL_UnlockSurface(hwscreen);
    SDL_FreeSurface(p);
  }
#endif
}

// No support for option menus
void (*vid_menudrawfn)(void) = NULL;
void (*vid_menukeyfn)(int key) = NULL;

void VID_SetPalette (unsigned char *palette) {
    int i;
    SDL_Color colors[256];

    for ( i=0; i<256; ++i ) {
        colors[i].r = *palette++;
        colors[i].g = *palette++;
        colors[i].b = *palette++;
    }
    //SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
    SDL_SetColors(screen, colors, 0, 256);
}

void VID_ShiftPalette (unsigned char *palette) {
    VID_SetPalette(palette);
}

void VID_Init (unsigned char *palette) {
	cpu_init();
	
	int pnum, chunk;
	byte *cache;
    int cachesize;

	// Quake(SDL) Menu...
	SDL_Surface *background=NULL;

	int quit = false;
	int option = 0;
	int handle = -1;

	TTF_Init();
	gTTFFont = TTF_OpenFont("q_sys/gfx/dpquake.ttf", 14);

  // Load the SDL library
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0){
    Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
  }

  //if (!(hwscreen = SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE|SDL_ASYNCBLIT|SDL_ANYFORMAT|SDL_HWPALETTE)))
  if(!(hwscreen = SDL_SetVideoMode(320, 480, 16, SDL_SWSURFACE|SDL_ASYNCBLIT|SDL_ANYFORMAT|SDL_HWPALETTE))){
      Sys_Error("VID: Couldn't set video mode: %s\n", SDL_GetError());
  }

	// Background
	background = IMG_Load("q_sys/gfx/main.png");
	if(!background){
		printf("IMG_Load failed: %s\n", IMG_GetError());
	}

	SDL_Color red = { 210, 0, 0, 0 };
	SDL_Color white = { 245, 245, 245, 0 };
	SDL_Color black = { 0, 0, 0, 0 };

	// initialize the mouse
	SDL_ShowCursor(0);

	while (!quit) {
		// Background
    if(background){
		  drawImage(hwscreen, background, 0, 0);
    }
#if 0
		TXT_Printf(gTTFFont, hwscreen, 1, 3, black, "---------------=======( Quake Menu )=======---------------");
		TXT_Printf(gTTFFont, hwscreen, 0, 2, red, "---------------=======( Quake Menu )=======---------------");
		TXT_Printf(gTTFFont, hwscreen, 21, 31, black, "CPU Overclocking:");
		TXT_Printf(gTTFFont, hwscreen, 20, 30, white, "CPU Overclocking:");
		TXT_Printf(gTTFFont, hwscreen, 20, 79, white, "Modification:");
		TXT_Printf(gTTFFont, hwscreen, 21, 78, black, "Modification:");
		TXT_Printf(gTTFFont, hwscreen, 21, 128, black, "Launch Quake");
		TXT_Printf(gTTFFont, hwscreen, 20, 127, white, "Launch Quake");
#else
		TXT_Printf(gTTFFont, hwscreen, 0, 2, white, "=== Quake Menu ===");
		TXT_Printf(gTTFFont, hwscreen, 20, 30, white, "CPU Overclocking:");
		TXT_Printf(gTTFFont, hwscreen, 20, 79, white, "Modification:");
		TXT_Printf(gTTFFont, hwscreen, 20, 127, white, "Launch Quake");
#endif
		/* Set current CPU Speed */
#if 0
		if (r2_cpu == 0) {
			TXT_Printf(gTTFFont, hwscreen, 29, 55, black, "- 336 MHz");
			TXT_Printf(gTTFFont, hwscreen, 28, 54, red, "- 336 MHz");
		} else if (r2_cpu == 1) {
			TXT_Printf(gTTFFont, hwscreen, 29, 55, black, "- 364 MHz");
			TXT_Printf(gTTFFont, hwscreen, 28, 54, red, "- 364 MHz");
		} else if (r2_cpu == 2) {
			TXT_Printf(gTTFFont, hwscreen, 29, 55, black, "- 392 MHz");
			TXT_Printf(gTTFFont, hwscreen, 28, 54, red, "- 392 MHz");
		} else if (r2_cpu == 3) {
			TXT_Printf(gTTFFont, hwscreen, 29, 55, black, "- 420 MHz");
			TXT_Printf(gTTFFont, hwscreen, 28, 54, red, "- 420 MHz");
		}
#else
		if (r2_cpu == 0) {
			TXT_Printf(gTTFFont, hwscreen, 28, 54, white, "- 336 MHz");
		} else if (r2_cpu == 1) {
			TXT_Printf(gTTFFont, hwscreen, 28, 54, white, "- 364 MHz");
		} else if (r2_cpu == 2) {
			TXT_Printf(gTTFFont, hwscreen, 28, 54, white, "- 392 MHz");
		} else if (r2_cpu == 3) {
			TXT_Printf(gTTFFont, hwscreen, 28, 54, white, "- 420 MHz");
		}
#endif

#if 0
		/* Set current Mod */
		if (r2_mod == 0) {
			TXT_Printf(gTTFFont, hwscreen, 29, 103, black, "- Quake (Default)");
			TXT_Printf(gTTFFont, hwscreen, 28, 102, red, "- Quake (Default)");
		} else if (r2_mod == 1) {
			TXT_Printf(gTTFFont, hwscreen, 29, 103, black, "- Scourge of Armagon");
			TXT_Printf(gTTFFont, hwscreen, 28, 102, red, "- Scourge of Armagon");
		} else if (r2_mod == 2) {
			TXT_Printf(gTTFFont, hwscreen, 29, 103, black, "- Dissolution of Eternity");
			TXT_Printf(gTTFFont, hwscreen, 28, 102, red, "- Dissolution of Eternity");
		}
#else
		if (r2_mod == 0) {
			TXT_Printf(gTTFFont, hwscreen, 28, 102, white, "- Quake (Default)");
		} else if (r2_mod == 1) {
			TXT_Printf(gTTFFont, hwscreen, 28, 102, white, "- Scourge of Armagon");
		} else if (r2_mod == 2) {
			TXT_Printf(gTTFFont, hwscreen, 28, 102, white, "- Dissolution of Eternity");
		}
#endif

#if 0
		/* Set current Option */
		if(option == 0) {
			TXT_Printf(gTTFFont, hwscreen, 11, 31, black, ">");
			TXT_Printf(gTTFFont, hwscreen, 10, 30, red, ">");
		} else if(option == 1) {
			TXT_Printf(gTTFFont, hwscreen, 11, 79, black, ">");
			TXT_Printf(gTTFFont, hwscreen, 10, 78, red, ">");
		} else if(option == 2) {
			TXT_Printf(gTTFFont, hwscreen, 11, 127, black, ">");
			TXT_Printf(gTTFFont, hwscreen, 10, 126, red, ">");
		}
#else
		if(option == 0) {
			TXT_Printf(gTTFFont, hwscreen, 10, 30, white, ">");
		} else if(option == 1) {
			TXT_Printf(gTTFFont, hwscreen, 10, 78, white, ">");
		} else if(option == 2) {
			TXT_Printf(gTTFFont, hwscreen, 10, 126, white, ">");
		}
#endif
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
		      SDL_FillRect(hwscreen, NULL, SDL_MapRGB(hwscreen->format, 0,0,0));
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							option--;

							if(option <= 0)
								option = 0;
						break;

						case SDLK_LEFT:
							if(option == 0) {
								r2_cpu--;

								if(r2_cpu <= 0)
									r2_cpu = 0;
							} else if(option == 1) {
								r2_mod--;

								if(r2_mod <= 0)
									r2_mod = 0;
							} 
						break;

						case SDLK_DOWN:
							option++;

							if(option >= 3)
								option = 2;
						break;

						case SDLK_RIGHT:
							if(option == 0) {
								r2_cpu++;

								if(r2_cpu >= 4)
									r2_cpu = 3;
							} else if(option == 1) {
								r2_mod++;

								if(r2_mod >= 3)
									r2_mod = 2;
							}
						break;

						case SDLK_LCTRL:
		          SDL_FillRect(hwscreen, NULL, SDL_MapRGB(hwscreen->format, 0,0,0));
							if(option == 2) {
								// Set up display mode (width and height)
								vid.width = 320;
								vid.height = 240;

								if ((pnum=COM_CheckParm("-winsize"))) {
									if (pnum >= com_argc-2)
										Sys_Error("VID: -winsize <width> <height>\n");

									vid.width = Q_atoi(com_argv[pnum+1]);
									vid.height = Q_atoi(com_argv[pnum+2]);

									if (!vid.width || !vid.height)
										Sys_Error("VID: Bad window width/height\n");
								}

								if (r2_cpu == 1) {
									cpu_set_clock(336);
								} else if (r2_cpu == 2) {
									cpu_set_clock(364);
								} else if (r2_cpu == 3) {
									cpu_set_clock(392);
								} else if (r2_cpu == 3) {
									cpu_set_clock(420);
								}

								if (r2_mod == 0) {
									if (Sys_FileOpenRead("id1/pak0.pak", &handle) < 0) {
										Sys_Error("/id1/pak0.pak was not found.\n");
										return;
									} else {
										Sys_FileClose(handle);
									}
								} else if (r2_mod == 1) {
									if (Sys_FileOpenRead("hipnotic/pak0.pak", &handle) < 0) {
										Sys_Error("/hipnotic/pak0.pak was not found.\n");
										return;
									} else {
										Sys_FileClose(handle);
									}
								} else if (r2_mod == 2) {
									if (Sys_FileOpenRead("rogue/pak0.pak", &handle) < 0) {
										Sys_Error("/rogue/pak0.pak was not found.\n");
										return;
									} else {
										Sys_FileClose(handle);
									}
								}

								// Initialize display 
								screen = SDL_CreateRGBSurface(SDL_SWSURFACE, vid.width, vid.height, 8, 0, 0, 0, 0);
								quit = true;
							}
						break;

						default:
						break;
					}
				break;
			}
		}

	    //SDL_UpdateRect(hwscreen, 0, 0, 0, 0);
		SDL_Flip(hwscreen);
	}

	VID_SetPalette(palette);
	SDL_WM_SetCaption("Quake", "quake");

	// now know everything we need to know about the buffer
	VGA_width = vid.conwidth = vid.width;
	VGA_height = vid.conheight = vid.height;
	vid.aspect = 1.0; 
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	VGA_pagebase = vid.buffer = screen->pixels;
	VGA_rowbytes = vid.rowbytes = screen->pitch;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.direct = 0;
    
	// allocate z buffer and surface cache
	chunk = vid.width * vid.height * sizeof (*d_pzbuffer);
	cachesize = D_SurfaceCacheForRes (vid.width, vid.height);
	chunk += cachesize;
	d_pzbuffer = Hunk_HighAllocName(chunk, "video");

	if (d_pzbuffer == NULL)
		Sys_Error ("Not enough memory for video mode\n");

	// initialize the cache memory 
	cache = (byte *) d_pzbuffer + vid.width * vid.height * sizeof (*d_pzbuffer);
	D_InitCaches (cache, cachesize);
}

void    VID_Shutdown (void) {
    SDL_Quit();
}

void    VID_Update (vrect_t *rects) {
    SDL_Rect *sdlrects;
    int n, i;
    vrect_t *rect;

    // Two-pass system, since Quake doesn't do it the SDL way...

    // First, count the number of rectangles
    n = 0;
    for (rect = rects; rect; rect = rect->pnext)
        ++n;

    // Second, copy them to SDL rectangles and update
    if (!(sdlrects = (SDL_Rect *)alloca(n*sizeof(*sdlrects))))
        Sys_Error("Out of memory");
    i = 0;

	for (rect = rects; rect; rect = rect->pnext) {
        sdlrects[i].x = rect->x;
        sdlrects[i].y = rect->y;
        sdlrects[i].w = rect->width;
        sdlrects[i].h = rect->height;
        ++i;
    }

	//SDL_Flip(screen);
	//SDL_BlitSurface(screen, 0, hwscreen, 0);
	//SDL_Flip(hwscreen);
  {
    SDL_Surface* p = SDL_ConvertSurface(screen, hwscreen->format, 0);
    SDL_SoftStretch(p, NULL, hwscreen, NULL);
    SDL_Flip(hwscreen);
    SDL_FreeSurface(p);
  }
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
    Uint8 *offset;


    if (!screen) return;
    if ( x < 0 ) x = screen->w+x-1;
    offset = (Uint8 *)screen->pixels + y*screen->pitch + x;
    while ( height-- )
    {
        memcpy(offset, pbitmap, width);
        offset += screen->pitch;
        pbitmap += width;
    }
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
    if (!screen) return;
    if (x < 0) x = screen->w+x-1;
    //SDL_UpdateRect(screen, x, y, width, height);
}


/*
================
Sys_SendKeyEvents
================
*/

void Sys_SendKeyEvents(void)
{
    SDL_Event event;
    int sym, state;
    int modstate;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                sym = event.key.keysym.sym;
                state = event.key.state;
                modstate = SDL_GetModState();
                switch(sym)
                {
                   case SDLK_DELETE: sym = K_DEL; break;
                   case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
                   case SDLK_F1: sym = K_F1; break;
                   case SDLK_F2: sym = K_F2; break;
                   case SDLK_F3: sym = K_F3; break;
                   case SDLK_F4: sym = K_F4; break;
                   case SDLK_F5: sym = K_F5; break;
                   case SDLK_F6: sym = K_F6; break;
                   case SDLK_F7: sym = K_F7; break;
                   case SDLK_F8: sym = K_F8; break;
                   case SDLK_F9: sym = K_F9; break;
                   case SDLK_F10: sym = K_F10; break;
                   case SDLK_F11: sym = K_F11; break;
                   case SDLK_F12: sym = K_F12; break;
                   case SDLK_BREAK:
                   case SDLK_PAUSE: sym = K_PAUSE; break;
                   case SDLK_UP: sym = K_UPARROW; break;
                   case SDLK_DOWN: sym = K_DOWNARROW; break;
                   case SDLK_RIGHT: sym = K_RIGHTARROW; break;
                   case SDLK_LEFT: sym = K_LEFTARROW; break;
                   case SDLK_INSERT: sym = K_INS; break;
                   case SDLK_HOME: sym = K_HOME; break;
                   case SDLK_END: sym = K_END; break;
                   case SDLK_PAGEUP: sym = K_PGUP; break;
                   case SDLK_PAGEDOWN: sym = K_PGDN; break;
                   case SDLK_RSHIFT:
                   case SDLK_LSHIFT: sym = K_SHIFT; break;
                   case SDLK_RCTRL:
                   case SDLK_LCTRL: sym = K_CTRL; break;
                   case SDLK_RALT:
                   case SDLK_LALT: sym = K_ALT; break;
                   case SDLK_KP0: 
                       if(modstate & KMOD_NUM) sym = K_INS; 
                       else sym = SDLK_0;
                       break;
                   case SDLK_KP1:
                       if(modstate & KMOD_NUM) sym = K_END;
                       else sym = SDLK_1;
                       break;
                   case SDLK_KP2:
                       if(modstate & KMOD_NUM) sym = K_DOWNARROW;
                       else sym = SDLK_2;
                       break;
                   case SDLK_KP3:
                       if(modstate & KMOD_NUM) sym = K_PGDN;
                       else sym = SDLK_3;
                       break;
                   case SDLK_KP4:
                       if(modstate & KMOD_NUM) sym = K_LEFTARROW;
                       else sym = SDLK_4;
                       break;
                   case SDLK_KP5: sym = SDLK_5; break;
                   case SDLK_KP6:
                       if(modstate & KMOD_NUM) sym = K_RIGHTARROW;
                       else sym = SDLK_6;
                       break;
                   case SDLK_KP7:
                       if(modstate & KMOD_NUM) sym = K_HOME;
                       else sym = SDLK_7;
                       break;
                   case SDLK_KP8:
                       if(modstate & KMOD_NUM) sym = K_UPARROW;
                       else sym = SDLK_8;
                       break;
                   case SDLK_KP9:
                       if(modstate & KMOD_NUM) sym = K_PGUP;
                       else sym = SDLK_9;
                       break;
                   case SDLK_KP_PERIOD:
                       if(modstate & KMOD_NUM) sym = K_DEL;
                       else sym = SDLK_PERIOD;
                       break;
                   case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
                   case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
                   case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
                   case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
                   case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
                   case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;
                }
                // If we're not directly handled and still above 255
                // just force it to 0
                if(sym > 255) sym = 0;
                Key_Event(sym, state);
                break;

            /* case SDL_MOUSEMOTION:
                if ( (event.motion.x != (vid.width/2)) ||
                     (event.motion.y != (vid.height/2)) ) {
                    mouse_x = event.motion.xrel*10;
                    mouse_y = event.motion.yrel*10;
                    if ( (event.motion.x < ((vid.width/2)-(vid.width/4))) ||
                         (event.motion.x > ((vid.width/2)+(vid.width/4))) ||
                         (event.motion.y < ((vid.height/2)-(vid.height/4))) ||
                         (event.motion.y > ((vid.height/2)+(vid.height/4))) ) {
                        SDL_WarpMouse(vid.width/2, vid.height/2);
                    }
                }
                break; */

            case SDL_QUIT:
                CL_Disconnect ();
                Host_ShutdownServer(false);        
                Sys_Quit ();
                break;
            default:
                break;
        }
    }
}

void IN_Init (void) {
    /* if ( COM_CheckParm ("-nomouse") )
        return;

	mouse_x = mouse_y = 0.0;
    mouse_avail = 1; */
}

void IN_Shutdown (void) {
    // mouse_avail = 0;
}

void IN_Commands (void) {
    /* int i;
    int mouse_buttonstate;
   
    if (!mouse_avail)
		return;
   
    i = SDL_GetMouseState(NULL, NULL); */
    /* Quake swaps the second and third buttons */
    /* mouse_buttonstate = (i & ~0x06) | ((i & 0x02)<<1) | ((i & 0x04)>>1);
    for (i=0 ; i<3 ; i++) {
        if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, true);

        if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, false);
    }
    mouse_oldbuttonstate = mouse_buttonstate; */
}

void IN_Move (usercmd_t *cmd) {
    /* if (!mouse_avail)
        return;
   
    mouse_x *= sensitivity.value;
    mouse_y *= sensitivity.value;
   
    if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
        cmd->sidemove += m_side.value * mouse_x;
    else
        cl.viewangles[YAW] -= m_yaw.value * mouse_x;
    if (in_mlook.state & 1)
        V_StopPitchDrift ();
   
    if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * mouse_y;
        if (cl.viewangles[PITCH] > 80)
            cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70)
            cl.viewangles[PITCH] = -70;
    } else {
        if ((in_strafe.state & 1) && noclip_anglehack)
            cmd->upmove -= m_forward.value * mouse_y;
        else
            cmd->forwardmove -= m_forward.value * mouse_y;
    }
    mouse_x = mouse_y = 0.0; */
}

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void) {
    return 0;
}

void GpError(char *text, int hold){
}