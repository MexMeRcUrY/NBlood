//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#include "credits.h"
#include "build.h"
#include "compat.h"
#include "SmackerDecoder.h"
#include "fx_man.h"
#include "keyboard.h"
#include "common_game.h"
#include "blood.h"
#include "config.h"
#include "controls.h"
#include "globals.h"
#include "resource.h"
#include "screen.h"
#include "sound.h"
#include "view.h"
#include "sdltheora/sdltheora.h"
#include <SDL_video.h>
#include "renderlayer.h" //getwindow

char Wait(int nTicks)
{
	totalclock = 0;
	while (totalclock < nTicks)
	{
		gameHandleEvents();
		if (keyGetScan())
			return FALSE;
	}
	return TRUE;
}

char DoFade(char r, char g, char b, int nTicks)
{
	dassert(nTicks > 0);
	scrSetupFade(r, g, b);
	totalclock = gFrameClock = 0;
	do
	{
		while (totalclock < gFrameClock) { gameHandleEvents(); };
		gFrameClock += 2;
		scrNextPage();
		scrFadeAmount(divscale16(ClipHigh((int)totalclock, nTicks), nTicks));
		if (keyGetScan())
			return FALSE;
	} while (totalclock <= nTicks);
	return TRUE;
}

char DoUnFade(int nTicks)
{
	dassert(nTicks > 0);
	scrSetupUnfade();
	totalclock = gFrameClock = 0;
	do
	{
		while (totalclock < gFrameClock) { gameHandleEvents(); };
		gFrameClock += 2;
		scrNextPage();
		scrFadeAmount(0x10000 - divscale16(ClipHigh((int)totalclock, nTicks), nTicks));
		if (keyGetScan())
			return FALSE;
	} while (totalclock <= nTicks);
	return TRUE;
}

void credLogosDos(void)
{
	char bShift = keystatus[sc_LeftShift] | keystatus[sc_RightShift];
	if (bShift || !gCutScenes)
		return;

	videoClearScreen(0);
	DoUnFade(1);
	
	char* smkMovieFullPath = new char[MAX_PATH];
	char* ogvMovieFullPath = new char[MAX_PATH];
	char* nBloodMovieFullPath = new char[MAX_PATH];
	char logoVideo[] = "movie\\MONOLITH.SMK";
	getCutScenePath(logoVideo, nBloodMovieFullPath, ogvMovieFullPath, smkMovieFullPath);
	bool moviePlayed = false;

	moviePlayed = credPlaySmk("LOGO.SMK", "logo811m.wav", 300);
	if (!moviePlayed)
		moviePlayed = credPlayTheora(ogvMovieFullPath);
	if (!moviePlayed)
	{
		rotatesprite(160 << 16, 100 << 16, 65536, 0, 2050, 0, 0, 0x4a, 0, 0, xdim - 1, ydim - 1);
		scrNextPage();
		sndStartSample("THUNDER2", 128, -1);
		if (Wait(360))
		{
			if (videoGetRenderMode() == REND_CLASSIC)
				DoFade(0, 0, 0, 60);
		}
	}

	credReset();
	char gtiVideo[] = "movie\\GTI.SMK";
	getCutScenePath(gtiVideo, nBloodMovieFullPath, ogvMovieFullPath, smkMovieFullPath);
	moviePlayed = credPlaySmk("GTI.SMK", "gti.wav", 301);
	if (!moviePlayed)
		moviePlayed = credPlayTheora(ogvMovieFullPath);
	if (!moviePlayed)
	{
		rotatesprite(160 << 16, 100 << 16, 65536, 0, 2052, 0, 0, 0x0a, 0, 0, xdim - 1, ydim - 1);
		scrNextPage();
		sndStartSample("THUNDER2", 128, -1);
		if (Wait(360))
		{
			if (videoGetRenderMode() == REND_CLASSIC)
				DoFade(0, 0, 0, 60);
		}
	}

	credReset();

	rotatesprite(160 << 16, 100 << 16, 65536, 0, 2518, 0, 0, 0x4a, 0, 0, xdim - 1, ydim - 1);
	scrNextPage();
	sndStartSample("THUNDER2", 128, -1);
	sndPlaySpecialMusicOrNothing(MUS_INTRO);
	Wait(360);
	sndFadeSong(4000);
}

void credReset(void)
{
	videoClearScreen(0);
	scrNextPage();
	DoFade(0, 0, 0, 1);
	scrSetupUnfade();
	DoUnFade(1);
}

int credKOpen4Load(char*& pzFile)
{
	int nLen = strlen(pzFile);
	for (int i = 0; i < nLen; i++)
	{
		if (pzFile[i] == '\\')
			pzFile[i] = '/';
	}
	int nHandle = kopen4loadfrommod(pzFile, 0);
	if (nHandle == -1)
	{
		// Hack
		if (nLen >= 3 && isalpha(pzFile[0]) && pzFile[1] == ':' && pzFile[2] == '/')
		{
			pzFile += 3;
			nHandle = kopen4loadfrommod(pzFile, 0);
		}
	}
	return nHandle;
}

#define kSMKPal 5
#define kSMKTile (MAXTILES-1)

char credPlaySmk(const char* _pzSMK, const char* _pzWAV, int nWav)
{
#if 0
	CSMKPlayer smkPlayer;
	if (dword_148E14 >= 0)
	{
		if (toupper(*pzSMK) == 'A' + dword_148E14)
		{
			if (Redbook.sub_82258() == 0 || Redbook.sub_82258() > 20)
				return FALSE;
		}
		Redbook.sub_82554();
	}
	smkPlayer.sub_82E6C(pzSMK, pzWAV);
#endif
	if (Bstrlen(_pzSMK) == 0)
		return false;
	char* pzSMK = Xstrdup(_pzSMK);
	char* pzWAV = Xstrdup(_pzWAV);
	char* pzSMK_ = pzSMK;
	char* pzWAV_ = pzWAV;
	int nHandleSMK = credKOpen4Load(pzSMK);
	if (nHandleSMK == -1)
	{
		Bfree(pzSMK_);
		Bfree(pzWAV_);
		return FALSE;
	}
	kclose(nHandleSMK);
	SmackerHandle hSMK = Smacker_Open(pzSMK);
	if (!hSMK.isValid)
	{
		Bfree(pzSMK_);
		Bfree(pzWAV_);
		return FALSE;
	}
	uint32_t nWidth, nHeight;
	Smacker_GetFrameSize(hSMK, nWidth, nHeight);
	uint8_t palette[768];
	uint8_t* pFrame = (uint8_t*)Xmalloc(nWidth * nHeight);
	walock[kSMKTile] = CACHE1D_PERMANENT;
	waloff[kSMKTile] = (intptr_t)pFrame;
	tileSetSize(kSMKTile, nHeight, nWidth);

	if (!pFrame)
	{
		Smacker_Close(hSMK);
		Bfree(pzSMK_);
		Bfree(pzWAV_);
		return FALSE;
	}
	int nFrameRate = Smacker_GetFrameRate(hSMK);
	int nFrames = Smacker_GetNumFrames(hSMK);

	Smacker_GetPalette(hSMK, palette);
	paletteSetColorTable(kSMKPal, palette);
	videoSetPalette(gBrightness >> 2, kSMKPal, 8 + 2);

	auto const oyxaspect = yxaspect;

	int nScale;
	int32_t nStat;

	if (nWidth <= 320 && nHeight <= 200)
	{
		if ((nWidth / (nHeight * 1.2f)) > (1.f * xdim / ydim))
			nScale = divscale16(320 * xdim * 3, nWidth * ydim * 4);
		else
			nScale = divscale16(200, nHeight);

		nStat = 2 | 4 | 8 | 64;
	}
	else
	{
		// DOS Blood v1.11: 320x240, 320x320, 640x400, and 640x480 SMKs all display 1:1 and centered in a 640x480 viewport
		nScale = tabledivide32(scale(65536, ydim << 2, xdim * 3), ((max(nHeight, 240 + 1u) + 239) / 240));
		nStat = 2 | 4 | 8 | 64 | 1024;
		renderSetAspect(viewingrange, 65536);
	}

	if (nWav)
		sndStartWavID(nWav, FXVolume);
	else
	{
		int nHandleWAV = credKOpen4Load(pzWAV);
		if (nHandleWAV != -1)
		{
			kclose(nHandleWAV);
			sndStartWavDisk(pzWAV, FXVolume);
		}
	}

	UpdateDacs(0, true);

	gameHandleEvents();
	ClockTicks nStartTime = totalclock;

	ctrlClearAllInput();

	int nFrame = 0;
	do
	{
		gameHandleEvents();
		if (scale((int)(totalclock - nStartTime), nFrameRate, kTicRate) < nFrame)
			continue;

		if (KB_KeyPressed(sc_Escape) || (nFrame > nFrameRate && ctrlCheckAllInput()))
			break;

		videoClearScreen(0);
		Smacker_GetPalette(hSMK, palette);
		paletteSetColorTable(kSMKPal, palette);
		videoSetPalette(gBrightness >> 2, kSMKPal, 0);
		tileInvalidate(kSMKTile, 0, 1 << 4);  // JBF 20031228
		Smacker_GetFrame(hSMK, pFrame);

		rotatesprite_fs(160 << 16, 100 << 16, nScale, 512, kSMKTile, 0, 0, nStat);

		videoNextPage();

		ctrlClearAllInput();
		nFrame++;
		Smacker_GetNextFrame(hSMK);
	} while (nFrame < nFrames);

	Smacker_Close(hSMK);
	ctrlClearAllInput();
	FX_StopAllSounds();
	renderSetAspect(viewingrange, oyxaspect);
	videoSetPalette(gBrightness >> 2, 0, 8 + 2);
	walock[kSMKTile] = 0;
	waloff[kSMKTile] = 0;
	tileSetSize(kSMKTile, 0, 0);
	Bfree(pFrame);
	Bfree(pzSMK_);
	Bfree(pzWAV_);

	return TRUE;
}

char credPlayTheora(const char* ogvideo)
{
	struct stat buf;
	
	if (stat(ogvideo, &buf) != 0)
	{
		return false;
	}

	uint32_t posX = 0, posY = 0, width = 0, height = 0;
	HWND hWindow = win_gethwnd(); // blood main window
	if (validmodecnt)
	{
		//max valid resolution
		width = validmode[0].xdim; 
		height = validmode[0].ydim;
	}
	else
	{
		width = 640;
		height = 480;
	}
	int quit = 0;
	SDL_Event e;
	SDL_Window* win = NULL;
	SDL_Renderer* renderer = NULL;
	RECT Rect;
	GetWindowRect(hWindow, &Rect);
	int const display = r_displayindex < SDL_GetNumVideoDisplays() ? r_displayindex : 0;

	if (gSetup.fullscreen)
	{
		//cannot hanlde 2 windows in fullscreen. so hide main one while movie playing
		ShowWindow(hWindow, SW_HIDE);
		win = SDL_CreateWindow("", posX, posY, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{
		MapWindowPoints(HWND_DESKTOP, GetParent(hWindow), (LPPOINT)&Rect, 2);
		posX = Rect.left;
		posY = Rect.top;
		width = gSetup.xdim;
		height = gSetup.ydim;
		win = SDL_CreateWindow("", posX + 9 , posY + 30, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
	
	}

	if (win == NULL)
	{
		return false;//SDL_GetError();
	}
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		printf("Renderer SDL Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		win = NULL;
	}
	else
	{
		//Initialize renderer color
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	}

	intptr_t my_video = THR_Load(ogvideo, renderer);
	SDL_Texture* video_texture = THR_UpdateVideo(my_video);
	ctrlClearAllInput();
	while (!quit)
	{

		gameHandleEvents();
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = 1; 
			}
		}

		// Use SDL_RenderCopy to blit the texture normally
		//Clear screen
		SDL_RenderClear(renderer);
		//Render texture to screen
		SDL_RenderCopy(renderer, video_texture, NULL, NULL);
		//Update screen
		SDL_RenderPresent(renderer);

		if (THR_IsPlaying(my_video) == 0 || KB_KeyPressed(sc_Escape))
		{
			quit = 1;
			THR_DestroyVideo(my_video, win);
			THR_Quit();
			//focus main blood window hidden in fullscreen
			ShowWindow(hWindow, SW_SHOWNORMAL);
			SetForegroundWindow(hWindow);
			SetFocus(hWindow);
			videoClearScreen(0);
		}
		else
			video_texture = THR_UpdateVideo(my_video);

		ctrlClearAllInput();
	}

	return TRUE;
}

