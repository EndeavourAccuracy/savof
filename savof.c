/* savof v0.9 (May 2016)
 * Copyright (C) 2016 Norbert de Jonge <mail@norbertdejonge.nl>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see [ www.gnu.org/licenses/ ].
 *
 * To properly read this code, set your program's tab stop to: 2.
 */

/*========== Includes ==========*/
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#include <windows.h>
#undef PlaySound
#endif

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
/*========== Includes ==========*/

/*========== Defines ==========*/
#define PROGRAM_NAME "savof"
#define PROGRAM_VERSION "v0.9 (May 2016)"
#define COPYRIGHT "Copyright (C) 2016 Norbert de Jonge"
#define SAV_FILE "PRINCE.SAV"
#define HOF_FILE "PRINCE.HOF"
#define EXIT_NORMAL 0
#define EXIT_ERROR 1
#define MAX_PATHS 1000
#define MAX_FILE 100
#define MAX_DATA 720
#define MAX_NAME 25
#define MAX_OPTION 100
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540
#define NUM_SOUNDS 20
#define REFRESH 30
#define MAX_TEXT 500
#define NO_WRAP 0
#define MAX_TOWRITE 720
#define MAX_HEX 100
#define MAX_TIME 20

#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#define SEP '\\'
#else
#define SEP '/'
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
/*========== Defines ==========*/

int iDebug;
int iNoAudio;
int iSAVMinutes, iSAVTicks, iSAVLevel, iSAVHP;
int iHOFEntries;
char arHOFNames[6 + 2][MAX_NAME + 2];
int arHOFMinutes[6 + 2];
int arHOFTicks[6 + 2];
TTF_Font *font1;
TTF_Font *font2;
TTF_Font *font3;
SDL_Window *window;
SDL_Renderer *screen;
int iXPos, iYPos;
int iFullscreen;
int iInput;
SDL_Cursor *curArrow;
SDL_Cursor *curText;
char sPath[MAX_PATHS + 2];
char sSAVFile[MAX_FILE + 2];
char sHOFFile[MAX_FILE + 2];

/*** Used for conversions. ***/
int iSAVMinutes1, iSAVMinutes2;
int iSAVTicks1, iSAVTicks2;
int iSAVLevel1, iSAVLevel2;
int iSAVHP1, iSAVHP2;
int iHOFEntries1, iHOFEntries2;
int arHOFMinutes1[6 + 2], arHOFMinutes2[6 + 2];
int arHOFTicks1[6 + 2], arHOFTicks2[6 + 2];

unsigned int gamespeed;
Uint32 looptime;

struct sample {
	Uint8 *data;
	Uint32 dpos;
	Uint32 dlen;
} sounds[NUM_SOUNDS];

/*** text ***/
SDL_Color color_bl = {0x00, 0x00, 0x00, 255};
SDL_Color color_wh = {0xff, 0xff, 0xff, 255};
SDL_Color color_red = {0xff, 0x00, 0x00, 255};
SDL_Rect offset;
SDL_Surface *message;
SDL_Texture *messaget;

/*** buttons ***/
int iQuitDown;
int iSAVCreate, iSAVDisabled, iSAVDown;
int iHOFCreate, iHOFDisabled, iHOFDown;

SDL_Texture *imgback;
SDL_Texture *imgquit1, *imgquit2;
SDL_Texture *imgcsav1, *imgcsav2, *imgosav0, *imgosav1, *imgosav2;
SDL_Texture *imgchof1, *imgchof2, *imgohof0, *imgohof1, *imgohof2;
SDL_Texture *imgcheck;
SDL_Texture *imgfaded;
SDL_Texture *imginput;

void StringToUpper (char *sInput, char *sOutput);
unsigned long BytesAsLU (unsigned char *sData, int iBytes);
int ReadFromFile (int iFd, char *sWhat, int iSize, unsigned char *sRetString);
void ShowUsage (void);
void LoadSAVValues (void);
void LoadHOFValues (void);
void GetOptionValue (char *sArgv, char *sValue);
void Quit (void);
void InitScreen (void);
void LoadFonts (void);
void MixAudio (void *unused, Uint8 *stream, int iLen);
void ShowScreen (void);
void Zoom (void);
void PlaySound (char *sFile);
void PreLoad (char *sPathFile, SDL_Texture **imgImage);
void ShowImage (int iThing, int iLocation, int iFromImageX,
	int iFromImageY, int iFromImageWidth, int iFromImageHeight);
void CustomRenderCopy (SDL_Texture* src, char *srcn, SDL_Rect* srcrect,
	SDL_Rect *dstrect);
void CenterNumber (int iNumber, int iX, int iY,
	SDL_Color fore, SDL_Color back);
void DisplayText (int iStartX, int iStartY, char sText[MAX_TEXT + 2],
	TTF_Font *font, SDL_Color color, int iWrapWidth);
void SSLittleEndianToHexToInts (int iValue, int *iHexRight, int *iHexLeft);
int InArea (int iUpperLeftX, int iUpperLeftY, int iWidth, int iHeight);
int Check (void);
void Time (int iMinutes, int iTicks, char *sTime);
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iSAVHOF);
void Save (int iSAVHOF);
void WriteCharByChar (int iFd, unsigned char *sString, int iLength);

/*****************************************************************************/
int main (int argc, char *argv[])
/*****************************************************************************/
{
	DIR *dDir;
	struct dirent *stDirent;
	char sRegular[MAX_FILE + 2];
	char sToUpper[MAX_FILE + 2];
	int iOption;

	/*** Defaults. ***/
	iDebug = 0;
	iNoAudio = 0;
	snprintf (sPath, MAX_PATHS, "%s", ".");
	snprintf (sSAVFile, MAX_FILE, "%s", "");
	snprintf (sHOFFile, MAX_FILE, "%s", "");
	iFullscreen = 0;
	iInput = 0;
	SDL_SetCursor (curArrow);

	/*** Parse command-line options. ***/
	if (argc > 1)
	{
		for (iOption = 1; iOption <= argc - 1; iOption++)
		{
			if ((strcmp (argv[iOption], "-h") == 0) ||
				(strcmp (argv[iOption], "-?") == 0) ||
				(strcmp (argv[iOption], "--help") == 0))
			{
				ShowUsage();
			}
			else if ((strcmp (argv[iOption], "-v") == 0) ||
				(strcmp (argv[iOption], "--version") == 0))
			{
				printf ("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
				exit (EXIT_NORMAL);
			}
			else if ((strcmp (argv[iOption], "-d") == 0) ||
				(strcmp (argv[iOption], "--debug") == 0))
			{
				iDebug = 1;
			}
			else if ((strncmp (argv[iOption], "-p=", 3) == 0) ||
				(strncmp (argv[iOption], "--path=", 7) == 0))
			{
				GetOptionValue (argv[iOption], sPath);
				if (iDebug == 1)
					{ printf ("[ INFO ] Using path: %s\n", sPath); }
			}
			else if ((strcmp (argv[iOption], "-f") == 0) ||
				(strcmp (argv[iOption], "--fullscreen") == 0))
			{
				iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
			}
			else if ((strcmp (argv[iOption], "-n") == 0) ||
				(strcmp (argv[iOption], "--noaudio") == 0))
			{
				iNoAudio = 1;
			}
			else
			{
				ShowUsage();
			}
		}
	}

	/*** Are PRINCE.SAV or PRINCE.HOF in the current directory? ***/
	dDir = opendir (sPath);
	if (dDir != NULL)
	{
		while ((stDirent = readdir (dDir)) != NULL)
		{
			snprintf (sRegular, MAX_FILE, "%s", stDirent->d_name);
			StringToUpper (sRegular, sToUpper);
			if ((strcmp (sRegular, ".") != 0) &&
				(strcmp (sRegular, "..") != 0))
			{
				if (strcmp (sToUpper, SAV_FILE) == 0)
					{ snprintf (sSAVFile, MAX_FILE, "%s", sRegular); }
				if (strcmp (sToUpper, HOF_FILE) == 0)
					{ snprintf (sHOFFile, MAX_FILE, "%s", sRegular); }
			}
		}
	} else {
		printf ("[FAILED] Could not open path \"%s\": %s!\n",
			sPath, strerror (errno));
		exit (EXIT_ERROR);
	}
	closedir (dDir);

	LoadSAVValues();
	LoadHOFValues();

	InitScreen();
	Quit();

	return 0;
}
/*****************************************************************************/
void StringToUpper (char *sInput, char *sOutput)
/*****************************************************************************/
{
	int iLoop;

	for (iLoop = 0; sInput[iLoop] != '\0'; iLoop++)
	{
		sOutput[iLoop] = toupper (sInput[iLoop]);
	}
	sOutput[iLoop] = '\0';
}
/*****************************************************************************/
unsigned long BytesAsLU (unsigned char *sData, int iBytes)
/*****************************************************************************/
{
	unsigned long luReturn;
	char sString[MAX_DATA + 2];
	char sTemp[MAX_DATA + 2];
	int iTemp;

	snprintf (sString, MAX_DATA, "%s", "");
	for (iTemp = iBytes - 1; iTemp >= 0; iTemp--)
	{
		snprintf (sTemp, MAX_DATA, "%s%02x", sString, sData[iTemp]);
		snprintf (sString, MAX_DATA, "%s", sTemp);
	}
	luReturn = strtoul (sString, NULL, 16);

	return (luReturn);
}
/*****************************************************************************/
int ReadFromFile (int iFd, char *sWhat, int iSize, unsigned char *sRetString)
/*****************************************************************************/
{
	int iLength;
	int iRead;
	char sRead[1 + 2];
	int iEOF;

	if ((iDebug == 1) && (strcmp (sWhat, "") != 0))
	{
		printf ("[  OK  ] Loading: %s\n", sWhat);
	}
	iLength = 0;
	iEOF = 0;
	do {
		iRead = read (iFd, sRead, 1);
		switch (iRead)
		{
			case -1:
				printf ("[FAILED] Could not read: %s!\n", strerror (errno));
				exit (EXIT_ERROR);
				break;
			case 0:
				iEOF = 1;
				break;
			default:
				sRetString[iLength] = sRead[0];
				iLength++;
				break;
		}
	} while ((iLength < iSize) && (iEOF == 0));
	sRetString[iLength] = '\0';

	return (iLength);
}
/*****************************************************************************/
void ShowUsage (void)
/*****************************************************************************/
{
	printf ("%s %s\n%s\n\n", PROGRAM_NAME, PROGRAM_VERSION, COPYRIGHT);
	printf ("Usage:\n");
	printf ("  %s [OPTIONS]\n\nOptions:\n", PROGRAM_NAME);
	printf ("  -h, -?,    --help           display this help and exit\n");
	printf ("  -v,        --version        output version information and"
		" exit\n");
	printf ("  -d,        --debug          also show debug information\n");
	printf ("  -p='PATH', --path='PATH'    search for PRINCE.SAV and"
		" PRINCE.HOF in PATH\n");
	printf ("  -f,        --fullscreen     start in fullscreen mode\n");
	printf ("  -n,        --noaudio        do not play sound effects\n\n");
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void LoadSAVValues (void)
/*****************************************************************************/
{
	int iFd;
	unsigned char sTwoBytes[2 + 2];

	/*** Defaults. ***/
	iSAVMinutes = 60;
	iSAVTicks = 719;
	iSAVLevel = 1;
	iSAVHP = 3;
	iSAVDisabled = 0;
	iSAVCreate = 1;

	if (strcmp (sSAVFile, "") != 0)
	{
		iFd = open (sSAVFile, O_RDONLY|O_BINARY);
		if (iFd != -1)
		{
			ReadFromFile (iFd, "Minutes", 2, sTwoBytes);
			iSAVMinutes = BytesAsLU (sTwoBytes, 2);
			ReadFromFile (iFd, "Ticks", 2, sTwoBytes);
			iSAVTicks = BytesAsLU (sTwoBytes, 2);
			ReadFromFile (iFd, "Level", 2, sTwoBytes);
			iSAVLevel = BytesAsLU (sTwoBytes, 2);
			ReadFromFile (iFd, "HP", 2, sTwoBytes);
			iSAVHP = BytesAsLU (sTwoBytes, 2);
			iSAVDisabled = 1;
			iSAVCreate = 0;
		} else {
			printf ("[ WARN ] Could not open \"%s\": %s!\n",
				sSAVFile, strerror (errno));
			snprintf (sSAVFile, MAX_FILE, "%s", SAV_FILE);
		}
		close (iFd);
	} else {
		snprintf (sSAVFile, MAX_FILE, "%s", SAV_FILE);
	}
}
/*****************************************************************************/
void LoadHOFValues (void)
/*****************************************************************************/
{
	int iFd;
	unsigned char sTwoBytes[2 + 2];
	int iEntry;
	int iChar;

	/*** Defaults. ***/
	iHOFEntries = 0;
	for (iEntry = 1; iEntry <= 6; iEntry++)
	{
		for (iChar = 0; iChar < MAX_NAME; iChar++)
			{ arHOFNames[iEntry][iChar] = '\0'; }
		arHOFMinutes[iEntry] = 0;
		arHOFTicks[iEntry] = 0;
	}
	iHOFDisabled = 0;
	iHOFCreate = 1;

	if (strcmp (sHOFFile, "") != 0)
	{
		iFd = open (sHOFFile, O_RDONLY|O_BINARY);
		if (iFd != -1)
		{
			ReadFromFile (iFd, "Entries", 2, sTwoBytes);
			iHOFEntries = BytesAsLU (sTwoBytes, 2);
			for (iEntry = 1; iEntry <= 6; iEntry++)
			{
				ReadFromFile (iFd, "Name", MAX_NAME,
					(unsigned char*)arHOFNames[iEntry]);
				ReadFromFile (iFd, "Minutes", 2, sTwoBytes);
				arHOFMinutes[iEntry] = BytesAsLU (sTwoBytes, 2);
				ReadFromFile (iFd, "Ticks", 2, sTwoBytes);
				arHOFTicks[iEntry] = BytesAsLU (sTwoBytes, 2);
			}
			iHOFDisabled = 1;
			iHOFCreate = 0;
			if ((iHOFEntries != Check()) || (iHOFEntries < 0) || (iHOFEntries > 6))
			{
				printf ("[ WARN ] Empty or incorrect number of entries.\n");
				if ((iHOFEntries < 0) || (iHOFEntries > 6)) { iHOFEntries = 0; }
			}
		} else {
			printf ("[ WARN ] Could not open \"%s\": %s!\n",
				sHOFFile, strerror (errno));
			snprintf (sHOFFile, MAX_FILE, "%s", HOF_FILE);
		}
		close (iFd);
	} else {
		snprintf (sHOFFile, MAX_FILE, "%s", HOF_FILE);
	}
}
/*****************************************************************************/
void GetOptionValue (char *sArgv, char *sValue)
/*****************************************************************************/
{
	int iTemp;
	char sTemp[MAX_OPTION + 2];

	iTemp = strlen (sArgv) - 1;
	snprintf (sValue, MAX_OPTION, "%s", "");
	while (sArgv[iTemp] != '=')
	{
		snprintf (sTemp, MAX_OPTION, "%c%s", sArgv[iTemp], sValue);
		snprintf (sValue, MAX_OPTION, "%s", sTemp);
		iTemp--;
	}
}
/*****************************************************************************/
void Quit (void)
/*****************************************************************************/
{
	TTF_CloseFont (font1);
	TTF_CloseFont (font2);
	TTF_CloseFont (font3);
	TTF_Quit();
	SDL_Quit();
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void InitScreen (void)
/*****************************************************************************/
{
	SDL_AudioSpec fmt;
	SDL_Surface *imgicon;
	SDL_Event event;
	int iOldXPos, iOldYPos;
	int iHOFEntriesNew;
	const Uint8 *keystate;
	int iSmall, iLarge;
	int iEntry, iY;
	int iInputOld;
	int iAddInput;
	char sHOFNameTemp[MAX_NAME + 2];

	if (SDL_Init (SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0)
	{
		printf ("[FAILED] Unable to init SDL: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	atexit (SDL_Quit);

	window = SDL_CreateWindow (PROGRAM_NAME " " PROGRAM_VERSION,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, iFullscreen);
	if (window == NULL)
	{
		printf ("[FAILED] Unable to create a window: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	screen = SDL_CreateRenderer (window, -1, 0);
	if (screen == NULL)
	{
		printf ("[FAILED] Unable to set video mode: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	/*** Some people may prefer linear, but we're going old school. ***/
	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (iFullscreen != 0)
	{
		SDL_RenderSetLogicalSize (screen, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	if (TTF_Init() == -1)
	{
		printf ("[FAILED] Could not initialize TTF: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}

	LoadFonts();

	curArrow = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	if (curArrow == NULL)
	{
		printf ("[FAILED] Could not load cursor: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	curText = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_IBEAM);
	if (curText == NULL)
	{
		printf ("[FAILED] Could not load cursor: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}

	if (iNoAudio != 1)
	{
		fmt.freq = 44100;
		fmt.format = AUDIO_S16;
		fmt.channels = 2;
		fmt.samples = 512;
		fmt.callback = MixAudio;
		fmt.userdata = NULL;
		if (SDL_OpenAudio (&fmt, NULL) < 0)
		{
			printf ("[FAILED] Unable to open audio: %s!\n", SDL_GetError());
			exit (EXIT_ERROR);
		}
		SDL_PauseAudio (0);
	}

	imgicon = IMG_Load ("png/savof_icon.png");
	SDL_SetWindowIcon (window, imgicon);

	PreLoad ("png/background.png", &imgback);
	PreLoad ("png/button_quit_1.png", &imgquit1);
	PreLoad ("png/button_quit_2.png", &imgquit2);
	PreLoad ("png/button_create_sav_1.png", &imgcsav1);
	PreLoad ("png/button_create_sav_2.png", &imgcsav2);
	PreLoad ("png/button_overwrite_sav_0.png", &imgosav0);
	PreLoad ("png/button_overwrite_sav_1.png", &imgosav1);
	PreLoad ("png/button_overwrite_sav_2.png", &imgosav2);
	PreLoad ("png/button_create_hof_1.png", &imgchof1);
	PreLoad ("png/button_create_hof_2.png", &imgchof2);
	PreLoad ("png/button_overwrite_hof_0.png", &imgohof0);
	PreLoad ("png/button_overwrite_hof_1.png", &imgohof1);
	PreLoad ("png/button_overwrite_hof_2.png", &imgohof2);
	PreLoad ("png/check.png", &imgcheck);
	PreLoad ("png/faded_entry.png", &imgfaded);
	PreLoad ("png/text_input.png", &imginput);

	ShowScreen();
	while (1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN: /*** http://wiki.libsdl.org/SDL_Keycode ***/
					switch (event.key.keysym.sym)
					{
						case SDLK_f:
							if (iInput == 0)
							{
								Zoom();
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_h:
							if ((iInput == 0) && (iHOFDisabled == 0)) { Save (2); }
							break;
						case SDLK_q:
							if (iInput == 0) { Quit(); }
							break;
						case SDLK_s:
							if ((iInput == 0) && (iSAVDisabled == 0)) { Save (1); }
							break;
						case SDLK_ESCAPE:
							Quit(); break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
							if ((event.key.keysym.mod & KMOD_LALT) ||
								(event.key.keysym.mod & KMOD_RALT))
							{
								Zoom();
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_BACKSPACE:
							if (iInput != 0)
							{
								if (strlen (arHOFNames[iInput]) > 0)
								{
									arHOFNames[iInput]
										[strlen (arHOFNames[iInput]) - 1] = '\0';
									PlaySound ("wav/hum_adj.wav");
									iHOFDisabled = 0;
								}
							}
							break;
						case SDLK_0:
						case SDLK_KP_0:
							if ((iInput == 0) && (iHOFEntries != 0))
							{
								iHOFEntries = 0;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_1:
						case SDLK_KP_1:
							if ((iInput == 0) && (iHOFEntries != 1))
							{
								iHOFEntries = 1;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_2:
						case SDLK_KP_2:
							if ((iInput == 0) && (iHOFEntries != 2))
							{
								iHOFEntries = 2;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_3:
						case SDLK_KP_3:
							if ((iInput == 0) && (iHOFEntries != 3))
							{
								iHOFEntries = 3;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_4:
						case SDLK_KP_4:
							if ((iInput == 0) && (iHOFEntries != 4))
							{
								iHOFEntries = 4;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_5:
						case SDLK_KP_5:
							if ((iInput == 0) && (iHOFEntries != 5))
							{
								iHOFEntries = 5;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						case SDLK_6:
						case SDLK_KP_6:
							if ((iInput == 0) && (iHOFEntries != 6))
							{
								iHOFEntries = 6;
								PlaySound ("wav/check_box.wav");
								iHOFDisabled = 0;
							}
							break;
						default: break;
					}
					ShowScreen();
					ShowScreen(); /*** Workaround for a(n SDL?) bug. ***/
					break;
				case SDL_MOUSEMOTION:
					iOldXPos = iXPos;
					iOldYPos = iYPos;
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if ((iOldXPos == iXPos) && (iOldYPos == iYPos)) { break; }

					/*** input ***/
					iInputOld = iInput;
					iInput = 0;
					if (InArea (113, 199, 355, 17) == 1) { iInput = 1; }
					if (InArea (113, 242, 355, 17) == 1) { iInput = 2; }
					if (InArea (113, 285, 355, 17) == 1) { iInput = 3; }
					if (InArea (113, 328, 355, 17) == 1) { iInput = 4; }
					if (InArea (113, 371, 355, 17) == 1) { iInput = 5; }
					if (InArea (113, 414, 355, 17) == 1) { iInput = 6; }
					if (iInputOld != iInput)
					{
						if (iInput == 0)
						{
							SDL_SetCursor (curArrow);
							SDL_StopTextInput();
						} else {
							SDL_SetCursor (curText);
							SDL_StartTextInput();
						}
						ShowScreen();
					}

					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (858, 491, 85, 32) == 1) /*** button: Quit ***/
							{ if (iQuitDown != 1) { iQuitDown = 1; } }
						if (InArea (17, 491, 170, 32) == 1) /*** button: SAV ***/
							{ if (iSAVDown != 1) { iSAVDown = 1; } }
						if (InArea (202, 491, 170, 32) == 1) /*** button: HOF ***/
							{ if (iHOFDown != 1) { iHOFDown = 1; } }
					}
					ShowScreen();
					break;
				case SDL_MOUSEBUTTONUP:
					iQuitDown = 0;
					iSAVDown = 0;
					iHOFDown = 0;

					if (InArea (858, 491, 85, 32) == 1) /*** button: Quit ***/
						{ Quit(); }

					/*** entries ***/
					iHOFEntriesNew = iHOFEntries;
					if (InArea (539, 200, 15, 15) == 1) { iHOFEntriesNew = 0; }
					if (InArea (477, 200, 15, 15) == 1) { iHOFEntriesNew = 1; }
					if (InArea (477, 243, 15, 15) == 1) { iHOFEntriesNew = 2; }
					if (InArea (477, 286, 15, 15) == 1) { iHOFEntriesNew = 3; }
					if (InArea (477, 329, 15, 15) == 1) { iHOFEntriesNew = 4; }
					if (InArea (477, 372, 15, 15) == 1) { iHOFEntriesNew = 5; }
					if (InArea (477, 415, 15, 15) == 1) { iHOFEntriesNew = 6; }
					if (iHOFEntriesNew != iHOFEntries)
					{
						iHOFEntries = iHOFEntriesNew;
						PlaySound ("wav/check_box.wav");
						iHOFDisabled = 0;
					}

					/*** Check Shift for iSmall and iLarge. ***/
					keystate = SDL_GetKeyboardState (NULL);
					if ((keystate[SDL_SCANCODE_LSHIFT]) ||
						(keystate[SDL_SCANCODE_RSHIFT]))
						{ iSmall = 100; iLarge = 1000; }
							else { iSmall = 1; iLarge = 10; }

					/*** SAV, Minutes ***/
					PlusMinus (&iSAVMinutes, 113, 114, 0, 0xFFFF, 0 - iLarge, 1);
					PlusMinus (&iSAVMinutes, 128, 114, 0, 0xFFFF, 0 - iSmall, 1);
					PlusMinus (&iSAVMinutes, 227, 114, 0, 0xFFFF, iSmall, 1);
					PlusMinus (&iSAVMinutes, 242, 114, 0, 0xFFFF, iLarge, 1);
					/*** SAV, Ticks ***/
					PlusMinus (&iSAVTicks, 326, 114, 0, 0xFFFF, 0 - iLarge, 1);
					PlusMinus (&iSAVTicks, 341, 114, 0, 0xFFFF, 0 - iSmall, 1);
					PlusMinus (&iSAVTicks, 440, 114, 0, 0xFFFF, iSmall, 1);
					PlusMinus (&iSAVTicks, 455, 114, 0, 0xFFFF, iLarge, 1);
					/*** SAV, Level ***/
					PlusMinus (&iSAVLevel, 113, 137, 0, 0xFFFF, 0 - iLarge, 1);
					PlusMinus (&iSAVLevel, 128, 137, 0, 0xFFFF, 0 - iSmall, 1);
					PlusMinus (&iSAVLevel, 227, 137, 0, 0xFFFF, iSmall, 1);
					PlusMinus (&iSAVLevel, 242, 137, 0, 0xFFFF, iLarge, 1);
					/*** SAV, HP ***/
					PlusMinus (&iSAVHP, 326, 137, 0, 0xFFFF, 0 - iLarge, 1);
					PlusMinus (&iSAVHP, 341, 137, 0, 0xFFFF, 0 - iSmall, 1);
					PlusMinus (&iSAVHP, 440, 137, 0, 0xFFFF, iSmall, 1);
					PlusMinus (&iSAVHP, 455, 137, 0, 0xFFFF, iLarge, 1);

					/*** HOF, Minutes and Ticks ***/
					for (iEntry = 1; iEntry <= 6; iEntry++)
					{
						iY = 176 + (iEntry * 43);
						PlusMinus (&arHOFMinutes[iEntry], 113, iY,
							0, 0xFFFF, 0 - iLarge, 2);
						PlusMinus (&arHOFMinutes[iEntry], 128, iY,
							0, 0xFFFF, 0 - iSmall, 2);
						PlusMinus (&arHOFMinutes[iEntry], 227, iY,
							0, 0xFFFF, iSmall, 2);
						PlusMinus (&arHOFMinutes[iEntry], 242, iY,
							0, 0xFFFF, iLarge, 2);

						PlusMinus (&arHOFTicks[iEntry], 326, iY,
							0, 0xFFFF, 0 - iLarge, 2);
						PlusMinus (&arHOFTicks[iEntry], 341, iY,
							0, 0xFFFF, 0 - iSmall, 2);
						PlusMinus (&arHOFTicks[iEntry], 440, iY,
							0, 0xFFFF, iSmall, 2);
						PlusMinus (&arHOFTicks[iEntry], 455, iY,
							0, 0xFFFF, iLarge, 2);
					}

					if (InArea (17, 491, 170, 32) == 1) /*** button: SAV ***/
						{ if (iSAVDisabled == 0) { Save (1); } }
					if (InArea (202, 491, 170, 32) == 1) /*** button: HOF ***/
						{ if (iHOFDisabled == 0) { Save (2); } }

					ShowScreen();
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowScreen(); } break;
				case SDL_QUIT:
					Quit(); break;
				case SDL_TEXTINPUT:
					iAddInput = 0;
					if (iInput != 0)
					{
						if (strlen (arHOFNames[iInput]) < MAX_NAME)
								{ iAddInput = 1; }
					}
					if (iAddInput == 1)
					{
						snprintf (sHOFNameTemp, MAX_NAME, "%s", arHOFNames[iInput]);
						/*** Added "+1" for \0. ***/
						snprintf (arHOFNames[iInput], MAX_NAME + 1, "%s%s",
							sHOFNameTemp, event.text.text);
						PlaySound ("wav/hum_adj.wav");
						iHOFDisabled = 0;
						ShowScreen();
					}

					break;
				default:
					break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
}
/*****************************************************************************/
void LoadFonts (void)
/*****************************************************************************/
{
	font1 = TTF_OpenFont ("ttf/Bitstream_Vera_Sans_Bold.ttf", 15);
	if (font1 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
	font2 = TTF_OpenFont ("ttf/Bitstream_Vera_Sans_Bold.ttf", 20);
	if (font2 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
	font3 = TTF_OpenFont ("ttf/Terminus_TTF_Bold.ttf", 14);
	if (font3 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
}
/*****************************************************************************/
void MixAudio (void *unused, Uint8 *stream, int iLen)
/*****************************************************************************/
{
	int iTemp;
	int iAmount;

	if (unused != NULL) { } /*** To prevent warnings. ***/

	SDL_memset (stream, 0, iLen); /*** SDL2 ***/
	for (iTemp = 0; iTemp < NUM_SOUNDS; iTemp++)
	{
		iAmount = (sounds[iTemp].dlen-sounds[iTemp].dpos);
		if (iAmount > iLen)
		{
			iAmount = iLen;
		}
		SDL_MixAudio (stream, &sounds[iTemp].data[sounds[iTemp].dpos], iAmount,
			SDL_MIX_MAXVOLUME);
		sounds[iTemp].dpos += iAmount;
	}
}
/*****************************************************************************/
void ShowScreen (void)
/*****************************************************************************/
{
	char sText[MAX_TEXT + 2];
	int iEntry, iChar;
	char sTemp[MAX_TOWRITE + 2];
	char sTime[MAX_TIME + 2];
	int iY;
	SDL_Color color;

	/*** Used for showing. ***/
	char sSAVMinutes[MAX_TOWRITE + 2];
	char sSAVTicks[MAX_TOWRITE + 2];
	char sSAVLevel[MAX_TOWRITE + 2];
	char sSAVHP[MAX_TOWRITE + 2];
	char sShowSAV[MAX_TOWRITE + 2];
	char sHOFEntries[MAX_TOWRITE + 2];
	char sHOFNames[6 + 2][MAX_TOWRITE + 2];
	char sHOFMinutes[6 + 2][MAX_TOWRITE + 2];
	char sHOFTicks[6 + 2][MAX_TOWRITE + 2];
	char sShowHOF[MAX_TOWRITE + 2];

	/*** background ***/
	ShowImage (1, 1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	/*** input ***/
	if (iInput != 0)
	{
		ShowImage (16, 12, 0, 0, 355, 17);
	}

	/*** about ***/
	snprintf (sText, MAX_TEXT, "%s", "Create, view and modify Prince of Persia for DOS save (PRINCE.SAV) and hall of fame (PRINCE.HOF) files. Ticks are frames (1/12 sec.) left of the current minute. Hold Shift to add or remove 100/1000.");
	DisplayText (40, 40, sText, font1, color_bl, 880);

	/*** SAV: Input. ***/
	if ((iSAVMinutes == 0) || (iSAVMinutes > 0x7FFF))
		{ color = color_red; } else { color = color_bl; }
	CenterNumber (iSAVMinutes, 141, 114, color, color_wh);
	if ((iSAVTicks > 719) || (iSAVTicks == 0))
		{ color = color_red; } else { color = color_bl; }
	CenterNumber (iSAVTicks, 354, 114, color, color_wh);
	Time (iSAVMinutes, iSAVTicks, sTime);
	DisplayText (515, 116, sTime, font1, color_bl, NO_WRAP);
	if ((iSAVLevel > 15) || (iSAVLevel == 0))
		{ color = color_red; } else { color = color_bl; }
	CenterNumber (iSAVLevel, 141, 137, color, color_wh);
	if (iSAVHP == 0) { color = color_red; } else { color = color_bl; }
	CenterNumber (iSAVHP, 354, 137, color, color_wh);

	/*** SAV: Convert data. ***/
	SSLittleEndianToHexToInts (iSAVMinutes, &iSAVMinutes1, &iSAVMinutes2);
	SSLittleEndianToHexToInts (iSAVTicks, &iSAVTicks1, &iSAVTicks2);
	SSLittleEndianToHexToInts (iSAVLevel, &iSAVLevel1, &iSAVLevel2);
	SSLittleEndianToHexToInts (iSAVHP, &iSAVHP1, &iSAVHP2);

	/*** SAV: Show as hexadecimals. ***/
	snprintf (sSAVMinutes, MAX_TOWRITE, "%02X %02X", iSAVMinutes2, iSAVMinutes1);
	snprintf (sSAVTicks, MAX_TOWRITE, "%02X %02X", iSAVTicks2, iSAVTicks1);
	snprintf (sSAVLevel, MAX_TOWRITE, "%02X %02X", iSAVLevel2, iSAVLevel1);
	snprintf (sSAVHP, MAX_TOWRITE, "%02X %02X", iSAVHP2, iSAVHP1);
	snprintf (sShowSAV, MAX_TOWRITE, "%s %s %s %s",
		sSAVMinutes, sSAVTicks, sSAVLevel, sSAVHP);
	DisplayText (640, 112, sShowSAV, font3, color_bl, 280);

	/*** HOF: Input. ***/
	for (iEntry = 1; iEntry <= 6; iEntry++)
	{
		switch (iEntry)
		{
			case 1: iY = 219; break;
			case 2: iY = 262; break;
			case 3: iY = 305; break;
			case 4: iY = 348; break;
			case 5: iY = 391; break;
			case 6: iY = 434; break;
		}
		if (strcmp (arHOFNames[iEntry], "") != 0)
		{
			DisplayText (140, iY - 20, arHOFNames[iEntry],
				font1, color_bl, NO_WRAP);
		}
		if ((arHOFMinutes[iEntry] == 0) || (arHOFMinutes[iEntry] > 0x7FFF))
			{ color = color_red; } else { color = color_bl; }
		CenterNumber (arHOFMinutes[iEntry], 141, iY, color, color_wh);
		if ((arHOFTicks[iEntry] > 719) || (arHOFTicks[iEntry] == 0))
			{ color = color_red; } else { color = color_bl; }
		CenterNumber (arHOFTicks[iEntry], 354, iY, color, color_wh);
		Time (arHOFMinutes[iEntry], arHOFTicks[iEntry], sTime);
		DisplayText (515, iY + 2, sTime, font1, color_bl, NO_WRAP);
	}

	/*** HOF: Convert data. ***/
	SSLittleEndianToHexToInts (iHOFEntries, &iHOFEntries1, &iHOFEntries2);
	for (iEntry = 1; iEntry <= 6; iEntry++)
	{
		/*** No need to convert arHOFNames. ***/
		SSLittleEndianToHexToInts (arHOFMinutes[iEntry], &arHOFMinutes1[iEntry],
			&arHOFMinutes2[iEntry]);
		SSLittleEndianToHexToInts (arHOFTicks[iEntry], &arHOFTicks1[iEntry],
			&arHOFTicks2[iEntry]);
	}

	/*** HOF: Show as hexadecimals. ***/
	snprintf (sHOFEntries, MAX_TOWRITE, "%02X %02X", iHOFEntries2, iHOFEntries1);
	for (iEntry = 1; iEntry <= 6; iEntry++)
	{
		snprintf (sHOFNames[iEntry], MAX_TOWRITE, "%s", "");
		for (iChar = 0; iChar < MAX_NAME; iChar++)
		{
			if (iChar == 0)
			{
				snprintf (sTemp, MAX_TOWRITE, "%02X", arHOFNames[iEntry][iChar]);
			} else {
				snprintf (sTemp, MAX_TOWRITE, "%s %02X", sHOFNames[iEntry],
					arHOFNames[iEntry][iChar]);
			}
			snprintf (sHOFNames[iEntry], MAX_TOWRITE, "%s", sTemp);
		}
		snprintf (sHOFMinutes[iEntry], MAX_TOWRITE, "%02X %02X",
			arHOFMinutes2[iEntry], arHOFMinutes1[iEntry]);
		snprintf (sHOFTicks[iEntry], MAX_TOWRITE, "%02X %02X",
			arHOFTicks2[iEntry], arHOFTicks1[iEntry]);
	}
	snprintf (sShowHOF, MAX_TOWRITE, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		sHOFEntries,
		sHOFNames[1], sHOFMinutes[1], sHOFTicks[1],
		sHOFNames[2], sHOFMinutes[2], sHOFTicks[2],
		sHOFNames[3], sHOFMinutes[3], sHOFTicks[3],
		sHOFNames[4], sHOFMinutes[4], sHOFTicks[4],
		sHOFNames[5], sHOFMinutes[5], sHOFTicks[5],
		sHOFNames[6], sHOFMinutes[6], sHOFTicks[6]);
	DisplayText (640, 230-33, sShowHOF, font3, color_bl, 280);

	/*** button: Quit ***/
	if (iQuitDown != 1)
		{ ShowImage (2, 2, 0, 0, 85, 32); }
			else { ShowImage (3, 2, 0, 0, 85, 32); }

	/*** button: SAV ***/
	if (iSAVCreate == 1)
	{
		/*** Never disabled. ***/
		if (iSAVDown != 1)
			{ ShowImage (4, 3, 0, 0, 170, 32); }
				else { ShowImage (5, 3, 0, 0, 170, 32); }
	} else {
		if (iSAVDisabled == 1)
		{
			ShowImage (6, 3, 0, 0, 170, 32);
		} else {
			if (iSAVDown != 1)
				{ ShowImage (7, 3, 0, 0, 170, 32); }
					else { ShowImage (8, 3, 0, 0, 170, 32); }
		}
	}

	/*** button: HOF ***/
	if (iHOFCreate == 1)
	{
		/*** Never disabled. ***/
		if (iHOFDown != 1)
			{ ShowImage (9, 4, 0, 0, 170, 32); }
				else { ShowImage (10, 4, 0, 0, 170, 32); }
	} else {
		if (iHOFDisabled == 1)
		{
			ShowImage (11, 4, 0, 0, 170, 32);
		} else {
			if (iHOFDown != 1)
				{ ShowImage (12, 4, 0, 0, 170, 32); }
					else { ShowImage (13, 4, 0, 0, 170, 32); }
		}
	}

	/*** check ***/
	ShowImage (14, 5, 0, 0, 15, 15);

	/*** faded ***/
	if (iHOFEntries < 1) { ShowImage (15, 6, 0, 0, 552, 40); }
	if (iHOFEntries < 2) { ShowImage (15, 7, 0, 0, 552, 40); }
	if (iHOFEntries < 3) { ShowImage (15, 8, 0, 0, 552, 40); }
	if (iHOFEntries < 4) { ShowImage (15, 9, 0, 0, 552, 40); }
	if (iHOFEntries < 5) { ShowImage (15, 10, 0, 0, 552, 40); }
	if (iHOFEntries < 6) { ShowImage (15, 11, 0, 0, 552, 40); }

	/*** refresh screen ***/
	SDL_RenderPresent (screen);
}
/*****************************************************************************/
void Zoom (void)
/*****************************************************************************/
{
	if (iFullscreen == 0)
		{ iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP; }
			else { iFullscreen = 0; }

	SDL_SetWindowFullscreen (window, iFullscreen);
	SDL_SetWindowSize (window, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_RenderSetLogicalSize (screen, SCREEN_WIDTH, SCREEN_HEIGHT);
	TTF_CloseFont (font1);
	TTF_CloseFont (font2);
	TTF_CloseFont (font3);
	LoadFonts();
}
/*****************************************************************************/
void PlaySound (char *sFile)
/*****************************************************************************/
{
	int iIndex;
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	if (iNoAudio == 1) { return; }
	for (iIndex = 0; iIndex < NUM_SOUNDS; iIndex++)
	{
		if (sounds[iIndex].dpos == sounds[iIndex].dlen)
		{
			break;
		}
	}
	if (iIndex == NUM_SOUNDS) { return; }

	if (SDL_LoadWAV (sFile, &wave, &data, &dlen) == NULL)
	{
		printf ("[FAILED] Could not load %s: %s!\n", sFile, SDL_GetError());
		exit (EXIT_ERROR);
	}
	SDL_BuildAudioCVT (&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, 2,
		44100);
	/*** The "+ 1" is a workaround for SDL bug #2274. ***/
	cvt.buf = (Uint8 *)malloc (dlen * (cvt.len_mult + 1));
	memcpy (cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio (&cvt);
	SDL_FreeWAV (data);

	if (sounds[iIndex].data)
	{
		free(sounds[iIndex].data);
	}
	SDL_LockAudio();
	sounds[iIndex].data = cvt.buf;
	sounds[iIndex].dlen = cvt.len_cvt;
	sounds[iIndex].dpos = 0;
	SDL_UnlockAudio();
}
/*****************************************************************************/
void PreLoad (char *sPathFile, SDL_Texture **imgImage)
/*****************************************************************************/
{
	*imgImage = IMG_LoadTexture (screen, sPathFile);
	if (!*imgImage)
	{
		printf ("[FAILED] IMG_LoadTexture: %s\n", IMG_GetError());
		exit (EXIT_ERROR);
	}
}
/*****************************************************************************/
void ShowImage (int iThing, int iLocation, int iFromImageX,
	int iFromImageY, int iFromImageWidth, int iFromImageHeight)
/*****************************************************************************/
{
	SDL_Rect dest;
	SDL_Rect loc;

	loc.x = iFromImageX;
	loc.y = iFromImageY;
	loc.w = iFromImageWidth;
	loc.h = iFromImageHeight;

	switch (iLocation)
	{
		case 1: dest.x = 0; dest.y = 0; break; /*** background ***/
		case 2: dest.x = 858; dest.y = 491; break; /*** button: Quit ***/
		case 3: dest.x = 17; dest.y = 491; break; /*** button: SAV ***/
		case 4: dest.x = 202; dest.y = 491; break; /*** button: HOF ***/
		case 5: /*** check ***/
			switch (iHOFEntries)
			{
				case 0: dest.x = 539; dest.y = 200; break;
				case 1: dest.x = 477; dest.y = 200; break;
				case 2: dest.x = 477; dest.y = 243; break;
				case 3: dest.x = 477; dest.y = 286; break;
				case 4: dest.x = 477; dest.y = 329; break;
				case 5: dest.x = 477; dest.y = 372; break;
				case 6: dest.x = 477; dest.y = 415; break;
			}
			break;
		case 6: dest.x = 40; dest.y = 199; break; /*** entry 1 ***/
		case 7: dest.x = 40; dest.y = 242; break; /*** entry 2 ***/
		case 8: dest.x = 40; dest.y = 285; break; /*** entry 3 ***/
		case 9: dest.x = 40; dest.y = 328; break; /*** entry 4 ***/
		case 10: dest.x = 40; dest.y = 371; break; /*** entry 5 ***/
		case 11: dest.x = 40; dest.y = 414; break; /*** entry 6 ***/
		case 12: /*** input ***/
			dest.x = 113;
			switch (iInput)
			{
				case 1: dest.y = 199; break;
				case 2: dest.y = 242; break;
				case 3: dest.y = 285; break;
				case 4: dest.y = 328; break;
				case 5: dest.y = 371; break;
				case 6: dest.y = 414; break;
			}
			break;
	}

	switch (iThing)
	{
		case 1: /*** background ***/
			CustomRenderCopy (imgback, "imgback", &loc, &dest); break;
		case 2: /*** button: Quit, normal ***/
			CustomRenderCopy (imgquit1, "imgquit1", &loc, &dest); break;
		case 3: /*** button: Quit, down ***/
			CustomRenderCopy (imgquit2, "imgquit2", &loc, &dest); break;
		case 4: /*** button: SAV, create, normal ***/
			CustomRenderCopy (imgcsav1, "imgcsav1", &loc, &dest); break;
		case 5: /*** button: SAV, create, down ***/
			CustomRenderCopy (imgcsav2, "imgcsav2", &loc, &dest); break;
		case 6: /*** button: SAV, overwrite, disabled ***/
			CustomRenderCopy (imgosav0, "imgosav0", &loc, &dest); break;
		case 7: /*** button: SAV, overwrite, normal ***/
			CustomRenderCopy (imgosav1, "imgosav1", &loc, &dest); break;
		case 8: /*** button: SAV, overwrite, down ***/
			CustomRenderCopy (imgosav2, "imgosav2", &loc, &dest); break;
		case 9: /*** button: HOF, create, normal ***/
			CustomRenderCopy (imgchof1, "imgchof1", &loc, &dest); break;
		case 10: /*** button: HOF, create, down ***/
			CustomRenderCopy (imgchof2, "imgchof2", &loc, &dest); break;
		case 11: /*** button: HOF, overwrite, disabled ***/
			CustomRenderCopy (imgohof0, "imgohof0", &loc, &dest); break;
		case 12: /*** button: HOF, overwrite, normal ***/
			CustomRenderCopy (imgohof1, "imgohof1", &loc, &dest); break;
		case 13: /*** button: HOF, overwrite, down ***/
			CustomRenderCopy (imgohof2, "imgohof2", &loc, &dest); break;
		case 14: /*** check ***/
			CustomRenderCopy (imgcheck, "imgcheck", &loc, &dest); break;
		case 15: /*** faded ***/
			CustomRenderCopy (imgfaded, "imgfaded", &loc, &dest); break;
		case 16: /*** input ***/
			CustomRenderCopy (imginput, "imginput", &loc, &dest); break;
	}
}
/*****************************************************************************/
void CustomRenderCopy (SDL_Texture* src, char *srcn, SDL_Rect* srcrect,
	SDL_Rect *dstrect)
/*****************************************************************************/
{
	SDL_Rect stuff;
	int iW, iH;

	SDL_QueryTexture (src, NULL, NULL, &iW, &iH);

	stuff.x = dstrect->x;
	stuff.y = dstrect->y;
	dstrect->w = iW;
	dstrect->h = iH;
	stuff.w = dstrect->w;
	stuff.h = dstrect->h;
	if (SDL_RenderCopy (screen, src, srcrect, &stuff) != 0)
	{
		printf ("[ WARN ] SDL_RenderCopy (%s): %s\n", srcn, SDL_GetError());
	}
}
/*****************************************************************************/
void CenterNumber (int iNumber, int iX, int iY,
	SDL_Color fore, SDL_Color back)
/*****************************************************************************/
{
	char sText[MAX_TEXT + 2];

	snprintf (sText, MAX_TEXT, "%i", iNumber);
	message = TTF_RenderText_Shaded (font2, sText, fore, back);
	messaget = SDL_CreateTextureFromSurface (screen, message);
	if ((iNumber >= 0) && (iNumber <= 9))
	{
		offset.x = iX + 1 + (5 * 7);
	} else if ((iNumber >= 10) && (iNumber <= 99)) {
		offset.x = iX + 1 + (4 * 7);
	} else if ((iNumber >= 100) && (iNumber <= 999)) {
		offset.x = iX + 1 + (3 * 7);
	} else if ((iNumber >= 1000) && (iNumber <= 9999)) {
		offset.x = iX + 1 + (2 * 7);
	} else {
		offset.x = iX + 1 + (1 * 7);
	}
	offset.y = iY - 1;
	offset.w = message->w; offset.h = message->h;
	CustomRenderCopy (messaget, "message", NULL, &offset);
	SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
}
/*****************************************************************************/
void DisplayText (int iStartX, int iStartY, char sText[MAX_TEXT + 2],
	TTF_Font *font, SDL_Color color, int iWrapWidth)
/*****************************************************************************/
{
	if (strcmp (sText, "") != 0)
	{
		message = TTF_RenderText_Blended_Wrapped (font, sText, color, iWrapWidth);
		messaget = SDL_CreateTextureFromSurface (screen, message);
		offset.x = iStartX;
		offset.y = iStartY;
		offset.w = message->w; offset.h = message->h;
		CustomRenderCopy (messaget, "message", NULL, &offset);
		SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
	} else {
		printf ("[ WARN ] Tried to display an empty text.\n");
	}
}
/*****************************************************************************/
void SSLittleEndianToHexToInts (int iValue, int *iHexRight, int *iHexLeft)
/*****************************************************************************/
{
	char sHex[MAX_HEX];
	char sHexRight[MAX_HEX];
	char sHexLeft[MAX_HEX];

	snprintf (sHex, MAX_HEX, "%04x", iValue);
	snprintf (sHexRight, MAX_HEX, "%c%c", sHex[0], sHex[1]);
	snprintf (sHexLeft, MAX_HEX, "%c%c", sHex[2], sHex[3]);
	*iHexLeft = (int)strtol(sHexLeft, NULL, 16);
	*iHexRight = (int)strtol(sHexRight, NULL, 16);
}
/*****************************************************************************/
int InArea (int iUpperLeftX, int iUpperLeftY, int iWidth, int iHeight)
/*****************************************************************************/
{
	if ((iUpperLeftX <= iXPos) &&
		(iUpperLeftX + iWidth >= iXPos) &&
		(iUpperLeftY <= iYPos) &&
		(iUpperLeftY + iHeight >= iYPos))
	{
		return (1);
	} else {
		return (0);
	}
}
/*****************************************************************************/
int Check (void)
/*****************************************************************************/
{
	int iEntry;
	int iChar;

	for (iEntry = 6; iEntry >= 1; iEntry--)
	{
		for (iChar = 0; iChar < MAX_NAME; iChar++)
		{
			if (arHOFNames[iEntry][iChar] != '\0')
			{
				return (iEntry);
			}
		}
	}

	return (0);
}
/*****************************************************************************/
void Time (int iMinutes, int iTicks, char *sTime)
/*****************************************************************************/
{
	int iShowMinutes;
	int iShowSeconds;
	char sShowSeconds[MAX_TIME];

	iShowMinutes = iMinutes - 1;
	iShowSeconds = trunc ((double)iTicks / 12);
	if (iShowSeconds < 10)
	{
		snprintf (sShowSeconds, MAX_TIME, "0%i", iShowSeconds);
	} else {
		snprintf (sShowSeconds, MAX_TIME, "%i", iShowSeconds);
	}
	snprintf (sTime, MAX_TIME, "%i:%s", iShowMinutes, sShowSeconds);
}
/*****************************************************************************/
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iSAVHOF)
/*****************************************************************************/
{
	if ((InArea (iX, iY, 13, 20) == 1) &&
		(((iChange < 0) && (*iWhat > iMin)) ||
		((iChange > 0) && (*iWhat < iMax))))
	{
		*iWhat = *iWhat + iChange;
		if ((iChange < 0) && (*iWhat < iMin)) { *iWhat = iMin; }
		if ((iChange > 0) && (*iWhat > iMax)) { *iWhat = iMax; }
		switch (iSAVHOF)
		{
			case 1: iSAVDisabled = 0; break;
			case 2: iHOFDisabled = 0; break;
		}
		PlaySound ("wav/plus_minus.wav");
		return (1);
	} else { return (0); }
}
/*****************************************************************************/
void Save (int iSAVHOF)
/*****************************************************************************/
{
	char sPathFile[MAX_PATHS + MAX_FILE + 2];
	char sToWrite[MAX_DATA + 2];
	int iFd;
	int iEntry;

	/*** SAV ***/
	if (iSAVHOF == 1)
	{
		/*** sPathFile ***/
		if (strcmp (sPath, ".") == 0) /*** Default. ***/
		{
			snprintf (sPathFile, MAX_PATHS + MAX_FILE, "%s", sSAVFile);
		} else { /*** Custom. ***/
			if (sPath[strlen (sPath) - 1] == SEP)
			{ /*** Custom ends with a separator. ***/
				snprintf (sPathFile, MAX_PATHS + MAX_FILE, "%s%s", sPath, sSAVFile);
			} else { /*** Custom does not end with a separator. ***/
				snprintf (sPathFile, MAX_PATHS + MAX_FILE,
					"%s%c%s", sPath, SEP, sSAVFile);
			}
		}

		iFd = open (sPathFile, O_WRONLY|O_TRUNC|O_CREAT|O_BINARY, 0600);
		if (iFd != -1)
		{
			snprintf (sToWrite, MAX_DATA, "%c%c%c%c%c%c%c%c",
				iSAVMinutes2, iSAVMinutes1,
				iSAVTicks2, iSAVTicks1,
				iSAVLevel2, iSAVLevel1,
				iSAVHP2, iSAVHP1);
			WriteCharByChar (iFd, (unsigned char*)sToWrite, 8);
			close (iFd);

			PlaySound ("wav/save.wav");
			iSAVDisabled = 1;
			iSAVCreate = 0;
		} else {
			printf ("[ WARN ] Cannot create \"%s\": %s!\n",
				sPathFile, strerror (errno));
		}
	}

	/*** HOF ***/
	if (iSAVHOF == 2)
	{
		/*** sPathFile ***/
		if (strcmp (sPath, ".") == 0) /*** Default. ***/
		{
			snprintf (sPathFile, MAX_PATHS + MAX_FILE, "%s", sHOFFile);
		} else { /*** Custom. ***/
			if (sPath[strlen (sPath) - 1] == SEP)
			{ /*** Custom ends with a separator. ***/
				snprintf (sPathFile, MAX_PATHS + MAX_FILE, "%s%s", sPath, sHOFFile);
			} else { /*** Custom does not end with a separator. ***/
				snprintf (sPathFile, MAX_PATHS + MAX_FILE,
					"%s%c%s", sPath, SEP, sHOFFile);
			}
		}

		iFd = open (sPathFile, O_WRONLY|O_TRUNC|O_CREAT|O_BINARY, 0600);
		if (iFd != -1)
		{
			snprintf (sToWrite, MAX_DATA, "%c%c",
				iHOFEntries2, iHOFEntries1);
			WriteCharByChar (iFd, (unsigned char*)sToWrite, 2);
			for (iEntry = 1; iEntry <= 6; iEntry++)
			{
				WriteCharByChar (iFd, (unsigned char*)arHOFNames[iEntry], 25);
				snprintf (sToWrite, MAX_DATA, "%c%c%c%c",
					arHOFMinutes2[iEntry], arHOFMinutes1[iEntry],
					arHOFTicks2[iEntry], arHOFTicks1[iEntry]);
				WriteCharByChar (iFd, (unsigned char*)sToWrite, 4);
			}
			close (iFd);

			PlaySound ("wav/save.wav");
			iHOFDisabled = 1;
			iHOFCreate = 0;
		} else {
			printf ("[ WARN ] Cannot create \"%s\": %s!\n",
				sPathFile, strerror (errno));
		}
	}
}
/*****************************************************************************/
void WriteCharByChar (int iFd, unsigned char *sString, int iLength)
/*****************************************************************************/
{
	int iTemp;
	char sToWrite[MAX_TOWRITE + 2];

	for (iTemp = 0; iTemp < iLength; iTemp++)
	{
		snprintf (sToWrite, MAX_TOWRITE, "%c", sString[iTemp]);
		write (iFd, sToWrite, 1);
	}
}
/*****************************************************************************/
