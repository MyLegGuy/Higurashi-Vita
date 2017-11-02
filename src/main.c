/*
	TODO - Inversion
		I could actually modify the loaded texture data. That would be for the best. I would need to store the filepaths of all busts and backgrounds loaded, though. Or, I could store backups in another texture.
	
	TODO - These are films. Look at the DrawFilm function please.
		TODO - The functions I made don't work well for some reason.
	
	(OPTIONAL TODO)
		TODO - (optional) Garbage collector won't collect functions made in script files??? i.e: function hima_tips_09_b()
			Maybe I can make a system similar to the one I made in MrDude that tracks all the global variables made between two points of time.
				If I could do that, I could dispose of the variables properly.
					Or I could just not do this and ignore the small problem. That's always an option.
			(I think this problem isn't worth the effort. So few files have multiple functions that it won't be a problem. If the user runs the same file with multiple functions over and over then the old functions will become garbage anyway, so they can't run out of memory that way. They need a lot of files with lots of functions of different names)
		TODO - (Optional) Italics
			OutputLine(NULL, "　……知レバ、…巻キ込マレテシマウ…。",
			   NULL, "...<i>If she found out... she would become involved</i>...", Line_Normal);

			(Here's the problem, It'll be hard to draw non italics text and italics in the same line)
			(A possible solution is to store x cordinates to start italics text)
				// Here's the plan.
				// Make another message array, but store text that is in italics in it
		TODO - Position markup
			At the very end of Onikakushi, I think that there's a markup that looks something like this <pos=36>Keechi</pos>
	TODO - Mod libvita2d to not inlcude characters with value 1 when getting text width. (This should be easy to do. There's a for loop)
*/
#define SINGLELINEARRAYSIZE 121
#define PLAYTIPMUSIC 0
#include "GeneralGoodConfig.h"

// main.h
	void Draw();
	void RecalculateBustOrder();
	void PlayBGM(const char* filename, int _volume, int _slot);
	void LazyMessage(const char* stra, const char* strb, const char* strc, const char* strd);
	void SaveSettings();
	void XOutFunction();
	void DrawHistory(unsigned char _textStuffToDraw[][SINGLELINEARRAYSIZE]);
	//void FixPath(char* filename,unsigned char _buffer[], char type);
	void CheckTouchControls();
	void DrawTouchControlsHelp();
	void SaveGameEditor();
	void SettingsMenu();
	typedef struct grhuighruei{
		char** theArray;
		unsigned char length;
	}goodStringMallocArray;
	typedef struct grejgrkew{
		unsigned char* theArray;
		unsigned char length;
	}goodu8MallocArray;

// Libraries all need
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//
#include <Lua/lua.h>
#include <Lua/lualib.h>
#include <Lua/lauxlib.h>
//

#define LOCATION_UNDEFINED 0
#define LOCATION_CG 1
#define LOCATION_CGALT 2

/////////////////////////////////////
#define MAXBUSTS 9 // 0 through 8. The slot argument isn't changed from the script anymore.
#define MAXIMAGECHAR 20
#define MESSAGETEXTXOFFSET 20
#define MAXFILES 50
#define MAXFILELENGTH 51
#define MAXMESSAGEHISTORY 40
#define VERSIONSTRING "v2.0.0.x" // This
#define VERSIONNUMBER 2 // This
#define VERSIONCOLOR 0,208,138
#define HISTORYONONESCREEN 13
#define MINHAPPYLUAVERSION 1
#define MAXHAPPYLUAVERSION MINHAPPYLUAVERSION
#define USEUMA0 1 // Doesn't need to be unsafe for this, I guess.
////////////////////////////////////
#define MAXMUSICARRAY 10

#include "GeneralGoodExtended.h"
#include "GeneralGood.h"
#include "GeneralGoodGraphics.h"
#include "GeneralGoodText.h"
#include "GeneralGoodImages.h"
#include "GeneralGoodSound.h"

// 1 is start
// 2 adds BGM and SE volume
#define OPTIONSFILEFORMAT 2

//#define LUAREGISTER(x,y) DebugLuaReg(y);

#define LUAREGISTER(x,y) lua_pushcfunction(L,x);\
	lua_setglobal(L,y);

// Platform specific headers
	// Nothing to see here.

////////////////////////////////////////
// PLatform specific variables
///////////////////////////////////////

//////////////
// The STREAMINGASSETS variable is only used for images and sound
char* STREAMINGASSETS;
char* PRESETFOLDER;
char* SCRIPTFOLDER;
char* SAVEFOLDER;

/*
CUSTOM INCLUDES
*/
#include "GeneralGood.h"
#include "FpsCapper.h"

#define BUST_STATUS_NORMAL 0
#define BUST_STATUS_FADEIN 1 // var 1 is alpha per frame. var 2 is time where 0 alpha
#define BUST_STATUS_FADEOUT 2 // var 1 is alpha per frame
#define BUST_STATUS_FADEOUT_MOVE 3 // var 1 is alpha per frame. var 2 is x per frame. var 3 is y per frame
#define BUST_STATUS_SPRITE_MOVE 4 // var 1 is x per frame, var 2 is y per frame

typedef struct hauighrehrge{
	CrossTexture* image;
	signed int xOffset;
	signed int yOffset;
	char isActive;
	char isInvisible;
	int layer;
	signed short alpha;
	unsigned char bustStatus;
	int statusVariable;
	int statusVariable2;
	int statusVariable3;
	int statusVariable4;
	unsigned int lineCreatedOn;
	float cacheXOffsetScale;
	float cacheYOffsetScale;
}bust;

bust Busts[MAXBUSTS];

lua_State* L;
/*
	Line_ContinueAfterTyping=0; (No wait after text display, go right to next script line)
	Line_WaitForInput=1; (Wait for the player to click. Displays an arrow.)
	Line_Normal=2; (Wait for the player to click. DIsplays a page icon and almost 100% of the time has the screen cleared with the next command)
*/
#define Line_ContinueAfterTyping 0
#define Line_WaitForInput 1
#define Line_Normal 2
int endType;
signed char useVsync=0;
unsigned char currentMessages[15][SINGLELINEARRAYSIZE];
int currentLine=0;
int place=0;

int lastBGMVolume = 128;

CrossTexture* currentBackground = NULL;
CROSSMUSIC* currentMusic[MAXMUSICARRAY] = {NULL};
CROSSPLAYHANDLE currentMusicHandle[MAXMUSICARRAY] = {0};
//CROSSMUSIC* currentMusic = NULL;
CROSSSFX* soundEffects[10] = {NULL};

CROSSSFX* menuSound=NULL;
signed char menuSoundLoaded=0;

// Alpha of black rectangle over screen
int MessageBoxAlpha = 100;

signed char MessageBoxEnabled=1;

unsigned int currentScriptLine=0;

// Order of busts drawn as organized by layer
// element in array is their bust slot
// first element in array is highest up
// so, when drawing, start from the end and go backwards to draw the first element last
unsigned char bustOrder[MAXBUSTS];

unsigned char bustOrderOverBox[MAXBUSTS];

char* locationStrings[3] = {(char*)"CG/",(char*)"CG/",(char*)"CGAlt/"};

goodStringMallocArray currentPresetFileList;
goodStringMallocArray currentPresetTipList;
goodStringMallocArray currentPresetTipNameList;
goodStringMallocArray currentPresetFileFriendlyList;
goodu8MallocArray currentPresetTipUnlockList;
int16_t currentPresetChapter=0;
// Made with malloc
char* currentPresetFilename=NULL;

/*
0 - title
1 - Load preset from currentPresetFilename
2 - Open file selector for preset file selection
3 - Start Lua thread
4 - Navigation menu after preset loading
5 - tip menu
*/
#define GAMESTATUS_TITLE 0
#define GAMESTATUS_LOADPRESET 1
#define GAMESTATUS_PRESETSELECTION 2
#define GAMESTATUS_MAINGAME 3
#define GAMESTATUS_NAVIGATIONMENU 4
#define GAMESTATUS_TIPMENU 5
#define GAMESTATUS_QUIT 99
signed char currentGameStatus=GAMESTATUS_TITLE;

unsigned char nextScriptToLoad[256] = {0};
unsigned char globalTempConcat[256] = {0};

signed char tipNamesLoaded=0;
signed char chapterNamesLoaded=0;
unsigned char lastSelectionAnswer=0;

// The x position on screen of this image character
unsigned short imageCharX[MAXIMAGECHAR] = {0};
// The y position on screeb of this image character
unsigned short imageCharY[MAXIMAGECHAR] = {0};
// The character that the image character is. The values in here are one of the IMAGECHAR constants
signed char imageCharType[MAXIMAGECHAR] = {0};
// The line number the image chars are at. This is used when displaying the message in OutputLine
unsigned short imageCharLines[MAXIMAGECHAR] = {0};
// The character positions within the lines they're at. This is used when displayng the message in OutputLine. Image characters are 3 spaces in the message. This will refer to the first spot. 
unsigned short imageCharCharPositions[MAXIMAGECHAR] = {0};

#define IMAGECHARSTAR 1
#define IMAGECHARNOTE 2
#define IMAGECHARUNKNOWN 0
CrossTexture* imageCharImages[3]; // PLEASE DON'T FORGET TO CHANGE THIS IF ANOTHER IMAGE CHAR IS ADDED

unsigned char filterR;
unsigned char filterG;
unsigned char filterB;
unsigned char filterA;
unsigned char filterActive=0;

signed char autoModeOn=0;
int32_t autoModeWait=500;

signed char cpuOverclocked=0;

unsigned char graphicsLocation = LOCATION_CGALT;

unsigned char messageHistory[MAXMESSAGEHISTORY][SINGLELINEARRAYSIZE];
unsigned char oldestMessage=0;

#define TOUCHMODE_NONE 0
#define TOUCHMODE_MAINGAME 1
#define TOUCHMODE_MENU 2
#define TOUCHMODE_LEFTRIGHTSELECT 3
unsigned char easyTouchControlMode = TOUCHMODE_MENU;

char presetsAreInStreamingAssets=1;

float bgmVolume = 0.75;
float seVolume = 1.0;

int currentTextHeight;

#if PLATFORM == PLAT_WINDOWS
	CrossTexture* controlsUpImage;
	CrossTexture* controlsDownImage;
	CrossTexture* controlsLeftImage;
	CrossTexture* controlsRightImage;
	CrossTexture* controlsSelectImage;
	CrossTexture* controlsBackImage;

	CrossTexture* controlsMenuImage;
	CrossTexture* skipImage;
	CrossTexture* autoImage;
	CrossTexture* textlogImage;

	char showControls=0;
#endif

CrossTexture* loadingImage;

/*
====================================================
*/

// Give a function name to this. It will tell you if it's new.
// THIS DOES NOT ACCOUNT FOR FUNCTIONS MADE IN happy.lua
void DebugLuaReg(char* name){
	if (lua_getglobal(L,name)==0){
		//printf("%s is new!\n",name);
		printf("LUAREGISTER(L_NotYet,\"%s\")\n",name);
		lua_pop(L,-1);
		return;
	}
}

void PlayMenuSound(){
	if (menuSoundLoaded==1){
		PlaySound(menuSound,1);
	}
}

CrossTexture* SafeLoadPNG(const char* path){
	CrossTexture* _tempTex = LoadPNG((char*)path);
	if (_tempTex==NULL){
		showErrorIfNull(_tempTex);
		LazyMessage("Failed to load image",path,"What will happen now?!",NULL);
	}
	return _tempTex;
}

CrossTexture* LoadEmbeddedPNG(const char* path){
	FixPath((char*)path,globalTempConcat,TYPE_EMBEDDED);
	CrossTexture* _tempTex = LoadPNG((char*)globalTempConcat);
	if (_tempTex==NULL){
		showErrorIfNull(_tempTex);
		LazyMessage("Failed to load image",path,"What will happen now?!","THIS IS SUPPOSED TO BE EMBEDDED!");
	}
	return _tempTex;
}

void DrawMessageBox(){
	DrawRectangle(0,0,screenWidth,screenHeight,0,0,0,MessageBoxAlpha);
}

void DrawCurrentFilter(){
	DrawRectangle(0,0,screenWidth,screenHeight,filterR,filterG,filterB,filterA-100);
	//DrawRectangle(0,0,960,screenHeight,filterR,255-(filterG*filterA*.0011),filterB,255);
}

u64 waitwithCodeTarget;
void WaitWithCodeStart(int amount){
	waitwithCodeTarget = getTicks()+amount;
}

void WaitWithCodeEnd(int amount){
	if (getTicks()<waitwithCodeTarget){
		wait(waitwithCodeTarget-getTicks());
	}
}

// Draws the loading screen.
void DrawLoadingScreen(){
	StartDrawing();
	DrawTextureScaleSize(loadingImage,0,0,screenWidth,screenHeight);
	EndDrawing();
}

void SetDefaultFontSize(){
	#if TEXTRENDERER == TEXT_DEBUG
		fontSize = 1.7;
	#endif
	#if TEXTRENDERER == TEXT_FONTCACHE
		fontSize = floor(screenWidth/40);
	#endif
	#if TEXTRENDERER == TEXT_VITA2D
		fontSize=32;
	#endif
}

void ReloadFont(){
	DrawLoadingScreen();
	#if TEXTRENDERER == TEXT_FONTCACHE
		FixPath("assets/LiberationSans-Regular.ttf",globalTempConcat,TYPE_EMBEDDED);
		FC_FreeFont(fontImage);
		fontImage = NULL;
		fontImage = FC_CreateFont();
		FC_LoadFont(fontImage, mainWindowRenderer, (char*)globalTempConcat, fontSize, FC_MakeColor(255,255,255,255), TTF_STYLE_NORMAL);
		currentTextHeight = TextHeight(fontSize);
	#endif
}

char MenuControls(char _choice,int _menuMin, int _menuMax){
	if (WasJustPressed(SCE_CTRL_UP)){
		if (_choice!=_menuMin){
			return (_choice-1);
		}else{
			return _menuMax;
		}
	}
	if (WasJustPressed(SCE_CTRL_DOWN)){
		if (_choice!=_menuMax){
			return (_choice+1);
		}else{
			return _menuMin;
		}
	}
	return _choice;
}

char SafeLuaDoFile(lua_State* passedState, char* passedPath, char showMessage){
	if (checkFileExist(passedPath)==0){
		if (showMessage==1){
			LazyMessage("The LUA file",passedPath,"does not exist!","What will happen now?!");
		}
		return 0;
	}
	luaL_dofile(passedState,passedPath);
	return 1;
}

void WriteToDebugFile(const char* stuff){
	#if PLATFORM == PLAT_VITA
		char *_tempDebugFileLocationBuffer = malloc(strlen(DATAFOLDER)+strlen("log.txt"));
		strcpy(_tempDebugFileLocationBuffer,DATAFOLDER);
		strcat(_tempDebugFileLocationBuffer,"log.txt");
		FILE *fp;
		fp = fopen(_tempDebugFileLocationBuffer, "a");
		if (!fp){
			LazyMessage("Failed to open debug file.",_tempDebugFileLocationBuffer,NULL,NULL);
			return;
		}
		fprintf(fp,"%s\n",stuff);
		fclose(fp);
	#endif
}

void WriteSDLError(){
	#if RENDERER == REND_SDL || SOUNDPLAYER == SND_SDL
		WriteToDebugFile(SDL_GetError());
	#else
		WriteToDebugFile("Can't write SDL error because not using SDL.");
	#endif
}

size_t u_strlen(const unsigned char * array){
	return (const size_t)strlen((const char*)array);
}

// Returns one if they chose yes
// Returns zero if they chose no
int LazyChoice(const char* stra, const char* strb, const char* strc, const char* strd){
	int _choice=0;
	ControlsStart();
	ControlsEnd();
	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS)){
			PlayMenuSound();
			ControlsStart();
			ControlsEnd();
			return _choice;
		}
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_choice++;
			if (_choice>1){
				_choice=0;
			}
		}
		if (WasJustPressed(SCE_CTRL_UP)){
			_choice--;
			if (_choice<0){
				_choice=1;
			}
		}
		ControlsEnd();
		StartDrawing();
		if (stra!=NULL){
			GoodDrawText(32,5+currentTextHeight*(0+2),stra,fontSize);
		}
		if (strb!=NULL){
			GoodDrawText(32,5+currentTextHeight*(1+2),strb,fontSize);
		}
		if (strc!=NULL){
			GoodDrawText(32,5+currentTextHeight*(2+2),strc,fontSize);
		}
		if (strd!=NULL){
			GoodDrawText(32,5+currentTextHeight*(3+2),strd,fontSize);
		}
		GoodDrawText(0,screenHeight-32-currentTextHeight*(_choice+1),">",fontSize);
		GoodDrawText(32,screenHeight-32-currentTextHeight*2,"Yes",fontSize);
		GoodDrawText(32,screenHeight-32-currentTextHeight,"No",fontSize);
		EndDrawing();
		FpsCapWait();
	}
	return 0;
}

int FixVolumeArg(int _val){
	if (floor(_val/(float)2)>128){
		return 128;
	}else if (_val<0){
		return 0;
	}else{
		return floor(_val/(float)2);
	}
	return 255;
}

int FixBGMVolume(int _val){
	return FixVolumeArg(_val)*bgmVolume;
}
int FixSEVolume(int _val){
	return FixVolumeArg(_val)*seVolume;
}

void ClearMessageArray(){
	currentLine=0;
	int i,j;
	for (i = 0; i < 15; i++){
		if (currentMessages[i][0]!='\0'){
			strcpy((char*)messageHistory[oldestMessage],(const char*)currentMessages[i]);
			oldestMessage++;
			if (oldestMessage==MAXMESSAGEHISTORY){
				oldestMessage=0;
			}
		}
		
		for (j = 0; j < SINGLELINEARRAYSIZE; j++){
			currentMessages[i][j]='\0';
		}
	}
	for (i=0;i<MAXIMAGECHAR;i++){
		imageCharType[i]=-1;
	}
}

void SetAllMusicVolume(int _passedFixedVolume){
	int i;
	for (i = 0; i < MAXMUSICARRAY; i++){
		SetMusicVolume(currentMusicHandle[i],_passedFixedVolume);
	}
}

int GetNextCharOnLine(int _linenum){
	return u_strlen(currentMessages[_linenum]);
}

void DrawMessageText(){
	//system("cls");
	int i;
	for (i = 0; i < 15; i++){
		//printf("%s\n",currentMessages[i]);
		GoodDrawText(MESSAGETEXTXOFFSET,currentTextHeight+i*(currentTextHeight),(char*)currentMessages[i],fontSize);
	}
	for (i=0;i<MAXIMAGECHAR;i++){
		if (imageCharType[i]!=-1){
			DrawTextureScale(imageCharImages[imageCharType[i]],imageCharX[i],imageCharY[i],((double)TextWidth(fontSize,"   ")/ GetTextureWidth(imageCharImages[imageCharType[i]])),((double)TextHeight(fontSize)/GetTextureHeight(imageCharImages[imageCharType[i]])));
		}
	}
}

void PrintScreenValues(){
	system("cls");
	int i,j;
	for (i = 0; i < 15; i++){
		for (j = 0; j < SINGLELINEARRAYSIZE; j++){
			printf("%d;",currentMessages[i][j]);
		}
		printf("\n");
	}
}

int Password(int val, int _shouldHave){
	if (val==_shouldHave){
		return val+1;
	}else{
		return 0;
	}
}

void WriteIntToDebugFile(int a){
	#if PLATFORM == PLAT_VITA
		char *_tempDebugFileLocationBuffer = malloc(strlen(DATAFOLDER)+strlen("log.txt"));
		strcpy(_tempDebugFileLocationBuffer,DATAFOLDER);
		strcat(_tempDebugFileLocationBuffer,"log.txt");
		FILE *fp;
		if (!fp){
			LazyMessage("Failed to open debug file.",_tempDebugFileLocationBuffer,NULL,NULL);
			return;
		}
		fp = fopen(_tempDebugFileLocationBuffer, "a");
		fprintf(fp,"%d\n", a);
		fclose(fp);
	#endif
}

void XOutFunction(){
	if (currentGameStatus==GAMESTATUS_MAINGAME){
		//luaL_error(L,"game stopped\n");
		// Adds function to stack
		lua_getglobal(L,"quitxfunction");
		// Call funciton. Removes function from stack.
		lua_call(L, 0, 0);
	}else{
		printf("normalquit\n");
		currentGameStatus=GAMESTATUS_QUIT;
	}
}

// Does not clear the debug file at ux0:data/HIGURASHI/log.txt  , I promise.
void ClearDebugFile(){
	#if PLATFORM == PLAT_VITA
	FILE *fp;
	fp = fopen("ux0:data/HIGURASHI/log.txt", "w");
	fclose(fp);
	#endif
}

void ResetBustStruct(bust* passedBust, int canfree){
	if (canfree==1 && passedBust->image!=NULL){
		FreeTexture(passedBust->image);
	}
	passedBust->image=NULL;
	passedBust->xOffset=0;
	passedBust->yOffset=0;
	passedBust->isActive=0;
	passedBust->isInvisible=0;
	passedBust->alpha=255;
	passedBust->bustStatus = BUST_STATUS_NORMAL;
	passedBust->lineCreatedOn=0;
}

void DisposeOldScript(){
	// Frees the script main
	lua_getglobal(L,"FreeTrash");
	lua_call(L, 0, 0);
}

char StringStartWith(const char *a, const char *b){
	if(strncmp(a, b, strlen(b)) == 0) return 1;
	return 0;
}

// Give it a full script file path and it will return 1 if the file was converted beforehand
int DidActuallyConvert(char* filepath){
	if (checkFileExist(filepath)==0){
		LazyMessage("I was going to check if you converted this file,","but I can't find it!",filepath,"How strange.");
		return 1;
	}
	FILE* file = fopen(filepath, "r");
	char line[256];

	int _isConverted=0;

	StartDrawing();
	GoodDrawText(32,50,"Checking if you actually converted the script...",fontSize);
	GoodDrawText(32,200,filepath,fontSize);
	EndDrawing();

	while (fgets(line, sizeof(line), file)) {
		printf("%s", line);

		// //MyLegGuyisanoob is put in the first blank line found in converted script files
		if (StringStartWith(line,"//MyLegGuyisanoob")==1){
			_isConverted=1;
			break;
		}else if (StringStartWith(line,"function main()")==1){ // Every file should have this
			_isConverted=1;
			break;
		}
	}

	fclose(file);
	return _isConverted;
}

void SaveFontSizeFile(){
	FixPath("fontsize.noob",globalTempConcat,TYPE_DATA);
	FILE* fp = fopen((const char*)globalTempConcat,"w");
	fwrite(&fontSize,4,1,fp);
	fclose(fp);
}

void LoadFontSizeFile(){
	FixPath("fontsize.noob",globalTempConcat,TYPE_DATA);
	FILE* fp = fopen((const char*)globalTempConcat,"r");
	fread(&fontSize,4,1,fp);
	fclose(fp);
}

void DisplaypcallError(int val, const char* fourthMessage){
	switch (val){
		case LUA_ERRRUN:
			LazyMessage("lua_pcall failed with error","LUA_ERRSYNTAX, a runtime error.","Please report the bug on the thread.",fourthMessage);
		break;
		case LUA_ERRMEM:
			LazyMessage("lua_pcall failed with error","LUA_ERRMEM, an out of memory error","Please report the bug on the thread.",fourthMessage);
		break;
		case LUA_ERRERR:
			LazyMessage("lua_pcall failed with error","LUA_ERRMEM, a message handler error","Please report the bug on the thread.",fourthMessage);
		break;
		case LUA_ERRGCMM:
			LazyMessage("lua_pcall failed with error","LUA_ERRGCMM, an __gc metamethod error","Please report the bug on the thread.",fourthMessage);
		break;
		default:
			LazyMessage("lua_pcall failed with error","UNKNOWN ERROR, something that shouldn't happen.","Please report the bug on the thread.",fourthMessage);
		break;
	}
}

int _debugCount=0;
void PrintDebugCounter(){
	printf("DEBUG %d\n",_debugCount);
	_debugCount++;
}

// Returns 1 if it worked
char RunScript(const char* _scriptfolderlocation,char* filename, char addTxt){
	// Hopefully, nobody tries to call a script from a script and wants to keep the current message display.
	ClearMessageArray();
	currentScriptLine=0;
	char tempstringconcat[strlen(_scriptfolderlocation)+strlen(filename)+strlen(".txt")+1];
	strcpy(tempstringconcat,_scriptfolderlocation);
	strcat(tempstringconcat,filename);
	if (addTxt==1){
		strcat(tempstringconcat,".txt");
	}
	// Free garbage and old main function if it existed
	DisposeOldScript();
	
	
	int _loadFileResult = luaL_loadfile(L, tempstringconcat);
	if (_loadFileResult!=LUA_OK){
		switch (_loadFileResult){
			case LUA_ERRSYNTAX:
				if (DidActuallyConvert(tempstringconcat)==1){
					LazyMessage("luaL_loadfile failed with error","LUA_ERRSYNTAX","You seem to have converted the files correctly...","Please report the bug on the thread.");
				}else{
					LazyMessage("luaL_loadfile failed with error","LUA_ERRSYNTAX","You probably forgot to convert the files with the","converter. Please make sure you did.");
				}
			break;
			case LUA_ERRMEM:
				LazyMessage("luaL_loadfile failed with error","LUA_ERRMEM","This is an out of memory error.","Please report the bug on the thread.");
			break;
			case LUA_ERRGCMM:
				LazyMessage("luaL_loadfile failed with error","LUA_ERRGCMM","This is a __gc metamethod error.","Please report the bug on the thread.");
			break;
			case LUA_ERRFILE:
				LazyMessage("luaL_loadfile failed with error","LUA_ERRFILE, this means the file failed to load.","Make sure the file exists.",tempstringconcat);
			break;
			default:
				LazyMessage("luaL_loadfile failed with error","UNKNOWN ERROR!","This is weird and should NEVER HAPPEN!","Please report the bug on the thread.");
			break;
		}
		currentGameStatus=GAMESTATUS_TITLE;
		return 0;
	}
	
	int _pcallResult = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (_pcallResult!=LUA_OK){
		printf("Failed pcall!\n");
		DisplaypcallError(_pcallResult,"This is the first lua_pcall in RunScript.");
		currentGameStatus=GAMESTATUS_TITLE;
		return 0;
	}

	//if (SafeLuaDoFile(L,tempstringconcat)==0){
	//	printf("Failed to load\n");
	//}

	// Adds function to stack
	lua_getglobal(L,"main");
	// Call funciton. Removes function from stack.
	_pcallResult = lua_pcall(L, 0, 0, 0);
	if (_pcallResult!=LUA_OK){
		DisplaypcallError(_pcallResult,"This is the second lua_pcall in RunScript.");
	}
	return 1;
}

char* CombineStringsPLEASEFREE(const char* first, const char* firstpointfive, const char* second, const char* third){
	char* tempstringconcat = (char*)calloc(1,strlen(first)+strlen(firstpointfive)+strlen(second)+strlen(third)+1);
	strcpy(tempstringconcat, first);
	strcat(tempstringconcat, firstpointfive);
	strcat(tempstringconcat, second);
	strcat(tempstringconcat, third);
	return tempstringconcat;
}

signed char WaitCanSkip(int amount){
	int i=0;
	ControlsStart();
	ControlsEnd();
	for (i = 0; i < floor(amount/50); ++i){
		wait(50);
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS)){
			ControlsEnd();
			printf("Skipped with %d left\n",amount-i);
			return 1;
		}
		ControlsEnd();
	}
	wait(amount%50);
	return 0;
}

void DrawUntilX(){
	while (1){
		FpsCapStart();

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS) || isSkipping==1){
			break;
		}
		ControlsEnd();

		StartDrawing();
		Draw();
		EndDrawing();

		FpsCapWait();
	}
	ControlsEnd();
}

void LastLineLazyFix(int* _line){
	if (*_line==15){
		DrawUntilX();
		ClearMessageArray();
		*_line=0;
	}
}

void Update(){
	int i=0;
	for (i = 0; i < MAXBUSTS; i++){
		if (Busts[i].bustStatus == BUST_STATUS_FADEIN){
			if (Busts[i].statusVariable2>0){
				Busts[i].statusVariable2-=17;
				if (Busts[i].statusVariable2>4000000){
					Busts[i].statusVariable2=0;
					//Busts[i].bustStatus = BUST_STATUS_NORMAL;
				}
			}else{
				Busts[i].alpha += Busts[i].statusVariable;
				if (Busts[i].alpha>=255){
					Busts[i].alpha=255;
					Busts[i].bustStatus = BUST_STATUS_NORMAL;
				}
			}
		}
		if (Busts[i].bustStatus == BUST_STATUS_FADEOUT){
			Busts[i].alpha -= Busts[i].statusVariable;
			if (Busts[i].alpha<=0){
				Busts[i].alpha=0;
				Busts[i].isActive=0;
				ResetBustStruct(&(Busts[i]),1);
				Busts[i].bustStatus = BUST_STATUS_NORMAL;
				RecalculateBustOrder();
			}	
		}
		if (Busts[i].bustStatus == BUST_STATUS_SPRITE_MOVE){
			if (abs(Busts[i].statusVariable3-(Busts[i].xOffset+Busts[i].statusVariable))<abs(Busts[i].statusVariable3-Busts[i].xOffset)){
				Busts[i].xOffset+=Busts[i].statusVariable;
			}else{
				Busts[i].xOffset=Busts[i].statusVariable3;
			}

			if (abs(Busts[i].statusVariable4-(Busts[i].yOffset+Busts[i].statusVariable2))<abs(Busts[i].statusVariable4-Busts[i].yOffset)){
				Busts[i].yOffset+=Busts[i].statusVariable2;
			}else{
				Busts[i].yOffset=Busts[i].statusVariable4;
			}


			if ((Busts[i].xOffset == Busts[i].statusVariable3) && (Busts[i].yOffset==Busts[i].statusVariable4)){
				Busts[i].bustStatus = BUST_STATUS_NORMAL;
				printf("DONE SPRITE MOVING\n");
			}
		}
	}
}

int FixHistoryOldSub(int _val, int _scroll){
	if (_val+_scroll>=MAXMESSAGEHISTORY){
		return (_val+_scroll)-MAXMESSAGEHISTORY;
	}else{
		return _val+_scroll;
	}
}

void InBetweenLines(lua_State *L, lua_Debug *ar) {
	if (currentGameStatus==GAMESTATUS_MAINGAME){
		currentScriptLine++;
		if (isSkipping==1){
			ControlsStart();
			#if PLATFORM != PLAT_WINDOWS
				if (!IsDown(SCE_CTRL_SQUARE)){

					isSkipping=0;
				}
			#endif
			#if PLATFORM == PLAT_WINDOWS
				if ( (  !(IsDown(SCE_TOUCH) && (touchX<screenWidth*.25 && touchY<screenHeight*.20)) ) && !(IsDown(SCE_CTRL_SQUARE))  ){
					isSkipping=0;
					PlayMenuSound();
				}
			#endif
			ControlsEnd();
			if (isSkipping==1){
				endType=Line_ContinueAfterTyping;
			}
		}
		u64 _inBetweenLinesMilisecondsStart = getTicks();
		char _didPressCircle=0;
		do{
			FpsCapStart();
			ControlsStart();
			Update();
			Draw();

			if (WasJustPressed(SCE_CTRL_CROSS)){
				if (_didPressCircle==1){
					MessageBoxEnabled=1;
				}
				endType = Line_ContinueAfterTyping;
			}
			if (WasJustPressed(SCE_CTRL_CIRCLE)){
				if (_didPressCircle==1){
					MessageBoxEnabled = !MessageBoxEnabled;
				}else if (MessageBoxEnabled==1){
					MessageBoxEnabled=0;
					_didPressCircle=1;
				}
			}
			if (WasJustPressed(SCE_CTRL_SQUARE)){
				isSkipping=1;
				endType=Line_ContinueAfterTyping;
			}
			if (WasJustPressed(SCE_CTRL_TRIANGLE)){
				SettingsMenu();
			}
			if (WasJustPressed(SCE_CTRL_SELECT)){
				PlayMenuSound();
				if (autoModeOn==1){
					autoModeOn=0;
				}else{
					autoModeOn=1;
				}
			}
			if (WasJustPressed(SCE_CTRL_START)){
				DrawHistory(messageHistory);
			}
			ControlsEnd();
			FpsCapWait();
			if (autoModeOn==1){
				if (getTicks()>=(unsigned int)(_inBetweenLinesMilisecondsStart+autoModeWait)){
					// TODO - Does this is happy?
					//MessageBoxEnabled=1;
					endType = Line_ContinueAfterTyping;
				}
			}
		}while(endType==Line_Normal || endType == Line_WaitForInput);
	}
}

void GetXAndYOffset(CrossTexture* _tempImg, signed int* _tempXOffset, signed int* _tempYOffset){
	*_tempXOffset = floor((screenWidth-GetTextureWidth(_tempImg))/2);
	*_tempYOffset = floor((screenHeight-GetTextureHeight(_tempImg))/2);
	// If they're bigger than the screen, assume that they're supposed to scroll or something
	if (*_tempXOffset<0){
		*_tempXOffset=0;
	}
	if (*_tempYOffset<0){
		*_tempYOffset=0;
	}
}

float GetXOffsetScale(CrossTexture* _tempImg){
	if (GetTextureWidth(_tempImg)>screenWidth){
		return (screenWidth/640);
	}
	return (GetTextureWidth(_tempImg)/(float)640);
}

float GetYOffsetScale(CrossTexture* _tempImg){
	if (GetTextureHeight(_tempImg)>screenHeight){
		return (screenHeight/480);
	}
	return ( GetTextureHeight(_tempImg)/(float)480);
}

void DrawBackground(CrossTexture* passedBackground){


	signed int _tempXOffset;
	signed int _tempYOffset;
	GetXAndYOffset(passedBackground,&_tempXOffset,&_tempYOffset);
	DrawTexture(passedBackground,_tempXOffset,_tempYOffset);
}

void DrawBackgroundAlpha(CrossTexture* passedBackground, unsigned char passedAlpha){

	
	signed int _tempXOffset;
	signed int _tempYOffset;
	GetXAndYOffset(passedBackground,&_tempXOffset,&_tempYOffset);
	DrawTextureAlpha(passedBackground,_tempXOffset,_tempYOffset,passedAlpha);
}

void DrawBust(bust* passedBust){
	signed int _tempXOffset;
	signed int _tempYOffset;
	
	GetXAndYOffset(passedBust->image,&_tempXOffset,&_tempYOffset);
	if (passedBust->alpha==255){
		DrawTexture(passedBust->image,_tempXOffset+passedBust->xOffset*passedBust->cacheXOffsetScale,_tempYOffset+passedBust->yOffset*passedBust->cacheYOffsetScale);
	}else{
		DrawTextureAlpha(passedBust->image,_tempXOffset+passedBust->xOffset*passedBust->cacheXOffsetScale,_tempYOffset+passedBust->yOffset*passedBust->cacheYOffsetScale, passedBust->alpha);
	}	
}

void RecalculateBustOrder(){
	int i, j, k;

	for (i=0;i<MAXBUSTS;i++){
		bustOrder[i]=255;
		bustOrderOverBox[i]=255;
	}

	// This generates the orderOfAction list
	// i is the the current orderOfAction slot.
	for (i=0;i<MAXBUSTS;i++){
		// j is the current fighter we're testig
		for (j=0;j<MAXBUSTS;j++){
			// If current entity speed greater than
			if ((bustOrder[i]==255 || Busts[j].layer>Busts[bustOrder[i]].layer) && (Busts[j].layer<=31)){
				// Loops through all of orderOfAction to make sure you're not already in orderOfAction
				for (k=0;k<MAXBUSTS;k++){
					if (bustOrder[k]==j){
						break;
					}else{
						if (k==MAXBUSTS-1){
							bustOrder[i]=j;
						}
					}
				}
			}
		}
	}

	// Do another calculation for busts that have a layer greater than 31 and therefor are over the message box
	for (i=0;i<MAXBUSTS;i++){
		// j is the current fighter we're testig
		for (j=0;j<MAXBUSTS;j++){
			// If current entity speed greater than
			if ((bustOrderOverBox[i]==255 || Busts[j].layer>Busts[bustOrderOverBox[i]].layer) && (Busts[j].layer>31)){
				// Loops through all of orderOfAction to make sure you're not already in orderOfAction
				for (k=0;k<MAXBUSTS;k++){
					if (bustOrderOverBox[k]==j){
						break;
					}else{
						if (k==MAXBUSTS-1){
							bustOrderOverBox[i]=j;
						}
					}
				}
			}
		}
	}
}

void MoveFilePointerPastNewline(CROSSFILE* fp){
	unsigned char _temp;
	crossfread(&_temp,1,1,fp);
	if (_temp==13){
		crossfseek(fp,1,SEEK_CUR);
	}
}

// LAST ARG IS WHERE THE LENGTH IS STORED
unsigned char* ReadNumberStringList(CROSSFILE *fp, unsigned char* arraysize){
	int numScripts;
	char currentReadNumber[4];
	// Add null for atoi
	currentReadNumber[3]=0;

	crossfread(&currentReadNumber,3,1,fp);
	numScripts = atoi(currentReadNumber);
	MoveFilePointerPastNewline(fp);

	//fseek(fp,2,SEEK_CUR);

	
	unsigned char* _thelist;
	
	_thelist = (unsigned char*)calloc(numScripts,sizeof(char));

	int i=0;
	for (i=0;i<numScripts;i++){
		crossfread(&currentReadNumber,3,1,fp);
		_thelist[i]=atoi(currentReadNumber);



		MoveFilePointerPastNewline(fp);
	}

	(*arraysize) = numScripts;
	return _thelist;
}

char** ReadFileStringList(CROSSFILE *fp, unsigned char* arraysize){
	char currentReadLine[200];
	char currentReadNumber[4];
	// Add null for atoi
	currentReadNumber[3]=0;
	int linePosition=0;
	int numScripts;

	currentReadNumber[3]='\0';
	crossfread(&currentReadNumber,3,1,fp);
	numScripts = atoi(currentReadNumber);
	MoveFilePointerPastNewline(fp);
	

	char** _thelist;
	
	_thelist = (char**)calloc(numScripts,sizeof(char*));
	unsigned char justreadbyte=00;

	int i=0;
	for (i=0;i<numScripts;i++){
		while (currentGameStatus!=GAMESTATUS_QUIT){
			if (crossfread(&justreadbyte,1,1,fp)!=1){
				break;
			}
			// Newline char
			// By some black magic, even though newline is two bytes, I can still detect it?
			
			if (isNewLine(fp,justreadbyte)==1){
				break;
			}
			currentReadLine[linePosition]=justreadbyte;
			linePosition++;
		}
		// Add null character
		currentReadLine[linePosition]='\0';
		//_thelist[i] = 

		_thelist[i] = (char*)calloc(1,linePosition+1);
		memcpy((_thelist[i]),currentReadLine,linePosition+1);
		
		linePosition=0;
	}

	(*arraysize) = numScripts;
	return _thelist;
}

void LoadPreset(char* filename){
	//currentPresetFileList
	CROSSFILE *fp;
	fp = crossfopen(filename, "r");
	//fprintf(fp,"There are %d music deocoders available\n", Mix_GetNumMusicDecoders());
	//int i;
	
	currentPresetFileList.theArray = ReadFileStringList(fp,&currentPresetFileList.length);
	//for (i=0;i<currentPresetFileList.length;i++){
	//	printf("%s\n",currentPresetFileList.theArray[i]);
	//}
	currentPresetTipList.theArray = ReadFileStringList(fp,&currentPresetTipList.length);
	//for (i=0;i<currentPresetTipList.length;i++){
	//	printf("%s\n",currentPresetTipList.theArray[i]);
	//}
	//printf("Is %s\n",currentReadNumber);

	currentPresetTipUnlockList.theArray = ReadNumberStringList(fp,&(currentPresetTipUnlockList.length));
	//for (i=0;i<currentPresetTipUnlockList.length;i++){
	//	printf("%d\n",currentPresetTipUnlockList.theArray[i]);
	//}
	char tempreadstring[15] = {'\0'};

	// Check for the
	// tipnames
	// string
	// If it exists, read the tip's names
	if (crossfread(tempreadstring,8,1,fp)==1){
		MoveFilePointerPastNewline(fp);
		if (strcmp(tempreadstring,"tipnames")==0){
			currentPresetTipNameList.theArray = ReadFileStringList(fp,&currentPresetTipNameList.length);
			tipNamesLoaded=1;
		}else{
			tipNamesLoaded=0;
		}
	}else{
		MoveFilePointerPastNewline(fp);
		tipNamesLoaded=0;
	}

	if (crossfread(tempreadstring,12,1,fp)==1){
		MoveFilePointerPastNewline(fp);
		if (strcmp(tempreadstring,"chapternames")==0){
			currentPresetFileFriendlyList.theArray = ReadFileStringList(fp,&currentPresetFileFriendlyList.length);
			chapterNamesLoaded=1;
		}else{
			chapterNamesLoaded=0;
		}
	}else{
		MoveFilePointerPastNewline(fp);
		chapterNamesLoaded=0;
	}

	//chapterNamesLoaded
	//currentPresetFileFriendlyList


	crossfclose(fp);
}

void SetNextScriptName(){
	memset((char*)(nextScriptToLoad),'\0',sizeof(nextScriptToLoad));
	strcpy((char*)nextScriptToLoad,currentPresetFileList.theArray[currentPresetChapter]);
}

// Generates the default data paths for script, presets, etc
// Will use uma0 if possible
void ResetDataDirectory(){
	#if USEUMA0==1
		GenerateDefaultDataDirectory(&DATAFOLDER,1);
		if (!directoryExists(DATAFOLDER)){
			free(DATAFOLDER);
			GenerateDefaultDataDirectory(&DATAFOLDER,0);
		}
	#else
		GenerateDefaultDataDirectory(&DATAFOLDER,0);
	#endif
}

void LazyMessage(const char* stra, const char* strb, const char* strc, const char* strd){
	ControlsStart();
	ControlsEnd();
	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS)){
			ControlsStart();
			ControlsEnd();
			break;
		}
		ControlsEnd();
		StartDrawing();
		if (stra!=NULL){
			GoodDrawText(32,5+currentTextHeight*(0+2),stra,fontSize);
		}
		if (strb!=NULL){
			GoodDrawText(32,5+currentTextHeight*(1+2),strb,fontSize);
		}
		if (strc!=NULL){
			GoodDrawText(32,5+currentTextHeight*(2+2),strc,fontSize);
		}
		if (strd!=NULL){
			GoodDrawText(32,5+currentTextHeight*(3+2),strd,fontSize);
		}
		GoodDrawText(32,screenHeight-32-currentTextHeight,SELECTBUTTONNAME" to continue.",fontSize);
		EndDrawing();
		FpsCapWait();
	}
}

void LoadGame(){
	strcpy((char*)globalTempConcat,SAVEFOLDER);
	strcat((char*)globalTempConcat,currentPresetFilename);
	currentPresetChapter=-1;
	if (checkFileExist((char*)globalTempConcat)==1){
		FILE *fp;
		fp = fopen((const char*)globalTempConcat, "r");
		fread(&currentPresetChapter,2,1,fp);
		fclose(fp);
	}
}

void SaveGame(){
	strcpy((char*)globalTempConcat,SAVEFOLDER);
	strcat((char*)globalTempConcat,currentPresetFilename);
	FILE *fp;
	fp = fopen((const char*)globalTempConcat, "w");
	fwrite(&currentPresetChapter,2,1,fp);
	fclose(fp);
}

// We make the user set up stuff by themself, that means that there will be morons who don't do it right.
// I must protect said morons
// Returns 0 for everything good
// Returns 1 for nonvital file missing
// Returns 2 for vital file missing
signed char CheckForUserStuff(){
	char _oneMissing = 0;
	#if PLATFORM == PLAT_VITA
		if (checkFileExist("app0:assets/LiberationSans-Regular.ttf")==0){
			//LazyMessage("app0:a/LiberationSans-Regular.ttf", "is missing. This should've been in the VPK.","Please download the VPK again.",NULL);
			CrossTexture* _nofonttext = SafeLoadPNG("app0:sce_sys/icon0.png");
	
			StartDrawing();
			DrawTexture(_nofonttext,32,32);
			EndDrawing();
			
			wait(3000);
			FreeTexture(_nofonttext);
			return 2;
		}
	#endif

	// Make data folder if it doesn't exist
	if (directoryExists(DATAFOLDER)==0){
		createDirectory(DATAFOLDER);
		_oneMissing=1;
	}

	// Check if StreamingAssets folder exists
	if (presetsAreInStreamingAssets==0){
		FixPath("StreamingAssets/",globalTempConcat,TYPE_DATA);
		if (directoryExists((const char*)globalTempConcat)==0){
			#if PLATFORM  == PLAT_WINDOWS
				char _tempResWidthString[20];
				char _tempResHeightString[20];
				itoa(screenWidth,_tempResWidthString,10);
				itoa(screenHeight,_tempResHeightString,10);
				LazyMessage("Your screen resolution is",_tempResWidthString,"by",_tempResHeightString);
			#endif
			LazyMessage((const char*)globalTempConcat,"does not exist. You must get StreamingAssets from a Higurashi","game, convert the files with my program, and then put the folder","in the correct place on the system. Refer to thread for tutorial.");
			#if USEUMA0==1
				LazyMessage("uma0:data/HIGURASHI/","doesn't exist either.",NULL,NULL);
			#endif
			_oneMissing=1;
		}
	}
	

	return _oneMissing;
}

void TryLoadMenuSoundEffect(){
	if (menuSound!=NULL){
		return;
	}
	char* tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, "/SE/","wa_038",".ogg");
	if (checkFileExist(tempstringconcat)){
		menuSoundLoaded=1;
		menuSound = LoadSound(tempstringconcat);
		SetSFXVolumeBefore(menuSound,FixSEVolume(256));
	}else{
		menuSoundLoaded=0;
	}
	free(tempstringconcat);
}

//===================

void FadeBustshot(int passedSlot,int _time,char _wait){
	if (isSkipping==1){
		_time=0;
	}

	//int passedSlot = lua_tonumber(passedState,1)-1;
	//Busts[passedSlot].bustStatus = BUST_STATUS_FADEOUT;
	//Busts[passedSlot].statusVariable = 
	Busts[passedSlot].alpha=0;
	Busts[passedSlot].bustStatus = BUST_STATUS_FADEOUT;
	if (_time!=0){
		Busts[passedSlot].alpha=255;
		//int _time = floor(lua_tonumber(passedState,7));
		int _totalFrames = floor(60*(_time/(double)1000));
		if (_totalFrames==0){
			_totalFrames=1;
		}
		int _alphaPerFrame=floor(255/_totalFrames);
		if (_alphaPerFrame==0){
			_alphaPerFrame=1;
		}
		Busts[passedSlot].statusVariable=_alphaPerFrame;
		Busts[passedSlot].bustStatus = BUST_STATUS_FADEOUT;

		if (_wait==1){
			while (Busts[passedSlot].isActive==1){
				FpsCapStart();
				ControlsStart();
				Update();
				Draw();
				if (WasJustPressed(SCE_CTRL_CROSS)){
					Busts[passedSlot].alpha = 1;
				}
				ControlsEnd();
				FpsCapWait();
			}
		}
	}
}

void FadeAllBustshots(int _time, char _wait){
	if (isSkipping==1){
		_time=0;
	}

	int i=0;
	for (i=0;i<MAXBUSTS;i++){
		if (Busts[i].isActive==1){
			FadeBustshot(i,_time,0);
		}
	}
	if (_wait==1){
		char _isDone=0;
		while (_isDone==0){
			_isDone=1;
			for (i=0;i<MAXBUSTS;i++){
				if (Busts[i].isActive==1){
					if (Busts[i].alpha>0){
						_isDone=0;
						break;
					}
				}
			}
			FpsCapStart();
			ControlsStart();
			Update();
			Draw();
			if (WasJustPressed(SCE_CTRL_CROSS)){
				for (i=0;i<MAXBUSTS;i++){
					if (Busts[i].isActive==1){
						Busts[i].alpha=1;
					}
				}
			}
			ControlsEnd();
			FpsCapWait();
		}
	}
}

void LocationStringFallback(char** tempstringconcat, const char* filename){
	if (checkFileExist(*tempstringconcat)==0){
		free(*tempstringconcat);
		if (graphicsLocation == LOCATION_CGALT){
			printf("Switching to cg\n");
			*tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, locationStrings[LOCATION_CG],filename,".png");
		}else if (graphicsLocation == LOCATION_CG){
			printf("Falling back on cgalt.\n");
			*tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, locationStrings[LOCATION_CGALT],filename,".png");
		}
	}
}

void DrawScene(const char* _filename, int time){
	if (isSkipping==1){
		time=0;
	}


	int _alphaPerFrame=255;
	//FadeAllBustshots(time,0);
	int i=0;
	//for (i=0;i<MAXBUSTS;i++){
	//	if (Busts[i].isActive==1 && Busts[i].lineCreatedOn != currentScriptLine-1){
	//		FadeBustshot(i,time,0);
	//	}
	//}

	signed short _backgroundAlpha=0;

	if (time!=0){
		int _time = time;
		int _totalFrames = floor(60*(_time/(double)1000));
		if (_totalFrames==0){
			_totalFrames=1;
		}
		_alphaPerFrame=floor(255/_totalFrames);
		if (_alphaPerFrame==0){
			_alphaPerFrame=1;
		}
	}

	char* tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, locationStrings[graphicsLocation],_filename,".png");
	LocationStringFallback(&tempstringconcat,_filename);
	CrossTexture* newBackground = SafeLoadPNG(tempstringconcat);
	free(tempstringconcat);

	while (_backgroundAlpha<255){
		FpsCapStart();

		Update();
		_backgroundAlpha+=_alphaPerFrame;
		if (_backgroundAlpha>255){
			_backgroundAlpha=255;
		}
		//int i;
		StartDrawing();
		
		if (currentBackground!=NULL){
			DrawBackground(currentBackground);
		}
		
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrder[i]!=255 && Busts[bustOrder[i]].isActive==1  && Busts[bustOrder[i]].lineCreatedOn != currentScriptLine-1){
				DrawBust(&(Busts[bustOrder[i]]));
			}
		}
		if (MessageBoxEnabled==1){
			DrawMessageBox();
		}
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrderOverBox[i]!=255 && Busts[bustOrderOverBox[i]].isActive==1 && Busts[bustOrderOverBox[i]].lineCreatedOn != currentScriptLine-1){
				DrawBust(&(Busts[bustOrderOverBox[i]]));
			}
		}

		
		DrawBackgroundAlpha(newBackground,_backgroundAlpha);
		
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrder[i]!=255 && Busts[bustOrder[i]].isActive==1  && Busts[bustOrder[i]].lineCreatedOn == currentScriptLine-1){
				DrawBust(&(Busts[bustOrder[i]]));
			}
		}

		if (filterActive==1){
			DrawCurrentFilter();
		}
		if (MessageBoxEnabled==1){
			DrawMessageText();
		}
		EndDrawing();

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS)){
			_backgroundAlpha=255;
		}
		ControlsEnd();

		FpsCapWait();
	}

	for (i=0;i<MAXBUSTS;i++){
		if (Busts[i].isActive==1 && Busts[i].lineCreatedOn != currentScriptLine-1){
			ResetBustStruct(&Busts[i], 1);
		}
	}

	if (currentBackground!=NULL){
		FreeTexture(currentBackground);
		currentBackground=NULL;
	}
	currentBackground=newBackground;
}

void DrawBustshot(unsigned char passedSlot, const char* _filename, int _xoffset, int _yoffset, int _layer, int _fadeintime, int _waitforfadein, int _isinvisible){
	if (isSkipping==1){
		_fadeintime=0;
		_waitforfadein=0;
	}
	Draw();
	int i;
	unsigned char skippedInitialWait=0;
	//WaitWithCodeStart(_fadeintime);

	ResetBustStruct(&(Busts[passedSlot]),1);

	char* tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, locationStrings[graphicsLocation],_filename,".png");
	LocationStringFallback(&tempstringconcat,_filename);
	Busts[passedSlot].image = SafeLoadPNG(tempstringconcat);
	free(tempstringconcat);
	if (Busts[passedSlot].image==NULL){
		ResetBustStruct(&(Busts[passedSlot]),1);
		return;
	}

	Busts[passedSlot].xOffset = _xoffset;
	Busts[passedSlot].yOffset = _yoffset;

	Busts[passedSlot].cacheXOffsetScale = GetXOffsetScale(Busts[passedSlot].image);
	Busts[passedSlot].cacheYOffsetScale = GetYOffsetScale(Busts[passedSlot].image);

	if (_isinvisible!=0){
		Busts[passedSlot].isInvisible=1;
	}else{
		Busts[passedSlot].isInvisible=0;
	}
	Busts[passedSlot].layer = _layer;

	// The lineCreatedOn variable is used to know if the bustshot should stay after a scene change. The bustshot can only stay after a scene change if it's created the line before the scene change AND it doesn't wait for fadein completion.
	if (_waitforfadein==0){
		Busts[passedSlot].lineCreatedOn = currentScriptLine;
	}else{
		Busts[passedSlot].lineCreatedOn = 0;
	}

	Busts[passedSlot].isActive=1;
	RecalculateBustOrder();
	if ((int)_fadeintime!=0){
		Busts[passedSlot].alpha=0;
		int _timeTotal = _fadeintime;
		int _time = floor(_fadeintime/2);
		int _totalFrames = floor(60*(_time/(double)1000));
		if (_totalFrames==0){
			_totalFrames=1;
		}
		int _alphaPerFrame=floor(255/_totalFrames);
		if (_alphaPerFrame==0){
			_alphaPerFrame=1;
		}
		Busts[passedSlot].statusVariable=_alphaPerFrame;
		Busts[passedSlot].statusVariable2 = _timeTotal -_time;
		Busts[passedSlot].bustStatus = BUST_STATUS_FADEIN;
	}

	i=1;
	if (_waitforfadein==1){
		while (Busts[passedSlot].alpha<255){
			FpsCapStart();
			ControlsStart();
			Update();
			if (Busts[passedSlot].alpha>255){
				Busts[passedSlot].alpha=255;
			}
			Draw();
			if (WasJustPressed(SCE_CTRL_CROSS) || skippedInitialWait==1){
				Busts[passedSlot].alpha = 255;
				Busts[passedSlot].bustStatus = BUST_STATUS_NORMAL;
				ControlsEnd();
				FpsCapWait();
				break;
			}
			ControlsEnd();
			i++;
			FpsCapWait();
		}
	}
}

// strcpy, but it won't copy from src to dest if the value is 1.
// You can use this to exclude certian spots
// I do not mean the ASCII character 1, which is 49.
void strcpyNO1(char* dest, const char* src){
	int i;
	int _destCopyOffset=0;
	int _srcStrlen = strlen(src);
	for (i=0;i<_srcStrlen;i++){
		if (src[i]!=1){
			dest[_destCopyOffset]=src[i];
			_destCopyOffset++;
		}
	}
}
// Same as strlen, but doesn't count any places with the value of 1 as a character.
int strlenNO1(char* src){
	int len=0;
	int i;
	for (i=0;;i++){
		if (src[i]=='\0'){
			break;
		}else if (src[i]!=1){
			len++;
		}
	}
	return len;
}

#if PLATFORM == PLAT_WINDOWS
	int _LagTestStart;
	void LagTestStart(){
		_LagTestStart = getTicks();
	}
	void LagTestEnd(){
		printf("lagometer: %d\n",getTicks()-_LagTestStart);
	}
#endif

// _dirRelativeToStreamingAssetsNoEndSlash should start AND END with a slash
// Example
// /SE/
// DO NOT FIX THE SE VOLUME BEFORE PASSING ARGUMENT
void GenericPlaySound(int passedSlot, const char* filename, int unfixedVolume, const char* _dirRelativeToStreamingAssetsNoEndSlash){
	if (strlen(filename)==0){
		printf("Sound effect filename empty.\n");
		return;
	}
	if (soundEffects[passedSlot]!=NULL){
		FreeSound(soundEffects[passedSlot]);
		soundEffects[passedSlot]=NULL;
	}
	char* tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, _dirRelativeToStreamingAssetsNoEndSlash,filename,".ogg");
	if (checkFileExist(tempstringconcat)==1){
		soundEffects[passedSlot] = LoadSound(tempstringconcat);
		//SetSFXVolume(soundEffects[passedSlot],FixSEVolume(unfixedVolume));
		CROSSPLAYHANDLE _tempHandle = PlaySound(soundEffects[passedSlot],1);
		SetSFXVolume(_tempHandle,FixSEVolume(unfixedVolume));
	}else{
		WriteToDebugFile("SE file not found");
		WriteToDebugFile(tempstringconcat);
	}
	free(tempstringconcat);
}

void OutputLine(const unsigned char* _tempMsg, char _endtypetemp, char _autoskip){
	// 1 when finished displaying the text
	char _isDone=0;
	if (isSkipping==1){
		_isDone=1;
	}
	MessageBoxEnabled=1;

	unsigned char message[strlen(_tempMsg)+1+strlen(currentMessages[currentLine])];
	// This will make the start of the message have whatever the start of the line says.
	// For example, if the line we're writing to already has "Keiichi is an idiot" written on it, that will be copied to the start of this message.
	strcpy(message,currentMessages[currentLine]);
	strcat(message,_tempMsg);
	int totalMessageLength=strlen(message);

	//printf("Total assembled: (START)%s(END)\n",message);

	if (totalMessageLength==0){
		endType = _endtypetemp;
		return;
	}

	// These are used when we're displaying the message to the user
	// Refer to the while loop near the end of this function.
	int _currentDrawLine = currentLine;
	int _currentDrawChar = GetNextCharOnLine(currentLine);
	int i, j;
	// This will loop through the entire message, looking for where I need to add new lines. When it finds a spot that
	// needs a new line, that spot in the message will become 0. So, when looking for the place to 
	int lastNewlinePosition=-1; // If this doesn't start at -1, the first character will be cut off. lastNewlinePosition+1 is always used, so a negative index won't be a problem.
	for (i = strlen(currentMessages[currentLine]); i < totalMessageLength; i++){
		if (message[i]==32){ // Only check when we meet a space. 32 is a space in ASCII
			message[i]='\0';
			// Check if the text has gone past the end of the screen OR we're out of array space for this line
			if (TextWidth(fontSize,&(message[lastNewlinePosition+1]))>=screenWidth-MESSAGETEXTXOFFSET || i-lastNewlinePosition>=SINGLELINEARRAYSIZE-1){
				char _didWork=0;
				for (j=i-1;j>lastNewlinePosition+1;j--){
					//printf("J:%d, M:%c\n",j,message[j]);
					if (message[j]==32){
						// j in message will now be the end point
						message[j]='\0';
						_didWork=1;
						// Fix the thing we messed up, i is no longer the end point
						message[i]=32;
						// This code would copy 
						strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
						lastNewlinePosition=j;
						currentLine++;
						break;
					}
				}
				if (_didWork==0){
					//oh well. That's fine. We'll just copy it anyway.
					strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
					lastNewlinePosition=i;
					currentLine++;
				}
				LastLineLazyFix(&currentLine);
			}else{
				message[i]=32;
			}
		}else{
			// Don't do special checks if it is a normal English ASCII character
			// Here we have special checks for stuff like image characters and new lines.
			if (message[i]<65 || message[i]>122){
				if (message[i]=='<'){
					int k;
					// Loop and look for the end
					for (k=i+1;k<i+100;k++){
						if (message[k]=='>'){
							break;
						}
					}
					if (k!=i+100){
						memset(&(message[i]),1,k-i+1); // Because this starts at i, k being 11 with i as 10 would just write 1 byte, therefor missing the end '>'. THe fix is to add one.
						i+=(k-i-1);
					}
				}else if (message[i]==226 && message[i+1]==128 && message[i+2]==148){ // Weird hyphen replace
					i=i+2;
					memset(&(message[i]),45,1); // Replace it with a normal hyphen
					memset(&(message[i+1]),1,2); // Replace these with value 1
				}else if (message[i]==226){ // COde for special image character
					unsigned char _imagechartype;
					if (message[i+1]==153 && message[i+2]==170){ // ♪
						_imagechartype = IMAGECHARNOTE;
					}else if (message[i+1]==152 && message[i+2]==134){ // ☆
						_imagechartype = IMAGECHARSTAR;
					}else{
						printf("Unknown image char! %d;%d\n",message[i+1],message[i+2]);
						_imagechartype = IMAGECHARUNKNOWN;
					}
					message[i]=0; // So we can use TextWidth
					for (j=0;j<MAXIMAGECHAR;j++){
						if (imageCharType[j]==-1){
							imageCharX[j] = TextWidth(fontSize,&(message[lastNewlinePosition+1]))+MESSAGETEXTXOFFSET;
							imageCharY[j] = TextHeight(fontSize)*currentLine+TextHeight(fontSize);
							imageCharLines[j] = currentLine;
							message[i]='\0';
							imageCharCharPositions[j] = strlenNO1(&(message[lastNewlinePosition+1]));
							imageCharType[j] = _imagechartype;
							//printf("Asssigned line %d and pos %d\n",imageCharLines[j],imageCharCharPositions[j]);
							break;
						}
					}
					memset(&(message[i]),32,3);
					i+=2;
				}else if (message[i]=='\n'){ // Interpret new line
					message[i]='\0';
					strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
					currentLine++;
					lastNewlinePosition=i;
				}
			}
		}

		LastLineLazyFix(&currentLine);
	}
	// This code will make a new line if there needs to be one because of the last word
	if (TextWidth(fontSize,&(message[lastNewlinePosition+1]))>=screenWidth-MESSAGETEXTXOFFSET){
		char _didWork=0;
		for (j=totalMessageLength-1;j>lastNewlinePosition+1;j--){
			if (message[j]==32){
				// WWWWWWWWWWWWWWWWWWWWWWWW MION
				message[j]='\0';
				// Copy stuff before the split, this would copy the W characters
				strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
				currentLine++;
				lastNewlinePosition=j;
				_didWork=1;
				// Copy stuff after the split, this would copy the MION
				strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
				break;
			}
		}
		// Were we able to find a place to put a new line? If not, do this.
		if (_didWork==0){
			printf("did not work.\n");
			// Just put as much as possible on one line.
			for (i=lastNewlinePosition+1;i<totalMessageLength;i++){
				char _tempCharCache = message[i];
				message[i]='\0';
				if (TextWidth(fontSize,&(message[lastNewlinePosition+1]))>screenWidth-MESSAGETEXTXOFFSET){
					// What this means is that when only the string UP TO the last character was small enough. Now we have to replicate the behavior of the previous loop to get the shorter string.
					char _tempCharCache2 = message[i-1];
					message[i-1]='\0';
					strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
					message[i-1]=_tempCharCache2;
					currentLine++;
					lastNewlinePosition=i-2;
				}else{
					//printf("%d;%s\n",TextWidth(fontSize,&(message[lastNewlinePosition+1])));
				}

				message[i] = _tempCharCache;
			}
			// Copy whatever remains
			strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
		}
	}else{
		// Copy whatever is left. In a
		// WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW(\n)NOOB
		// example, NOOB will be copied. This is seperate from the code above. NOOB by itself does not need a new line.
		strcpyNO1(currentMessages[currentLine],&(message[lastNewlinePosition+1]));
	}
	LastLineLazyFix(&currentLine);
	
	while(_isDone==0){
		#if PLATFORM != PLAT_VITA
			FpsCapStart();
		#endif

		// The first one that takes two bytes is U+0080, or 0xC2 0x80
		// If it is a two byte character, we don't want to try and draw when there's only one byte. Skip to include the next one.
		if (currentMessages[_currentDrawLine][_currentDrawChar]>=0xC2){
			_currentDrawChar++;
		}

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_CROSS) || capEnabled==0){
			_isDone=1;
		}
		ControlsEnd();
		
		StartDrawing();
		if (currentBackground!=NULL){
			DrawBackground(currentBackground);
		}
		
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrder[i]!=255 && Busts[bustOrder[i]].isActive==1){
				DrawBust(&(Busts[bustOrder[i]]));
			}
		}
		
		if (MessageBoxEnabled==1 && filterActive==0){
			DrawMessageBox();
		}
		
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrderOverBox[i]!=255 && Busts[bustOrderOverBox[i]].isActive==1){
				DrawBust(&(Busts[bustOrderOverBox[i]]));
			}
		}
		
		if (filterActive==1){
			DrawCurrentFilter();
		}
		if (MessageBoxEnabled==1){
			char _tempCharCache = currentMessages[_currentDrawLine][_currentDrawChar+1];
			currentMessages[_currentDrawLine][_currentDrawChar+1]='\0';
			for (i = 0; i <= _currentDrawLine; i++){
				GoodDrawText(MESSAGETEXTXOFFSET,currentTextHeight+i*(currentTextHeight),(char*)currentMessages[i],fontSize);
			}
			currentMessages[_currentDrawLine][_currentDrawChar+1]=_tempCharCache;
			for (i=0;i<MAXIMAGECHAR;i++){
				if (imageCharType[i]!=-1){
					if ((imageCharLines[i]<_currentDrawLine) || (imageCharLines[i]==_currentDrawLine && imageCharCharPositions[i]<=_currentDrawChar)){
						DrawTextureScale(imageCharImages[imageCharType[i]],imageCharX[i],imageCharY[i],((double)TextWidth(fontSize,"   ")/ GetTextureWidth(imageCharImages[imageCharType[i]])),((double)TextHeight(fontSize)/GetTextureHeight(imageCharImages[imageCharType[i]])));
					}
				}
			}
		}
		EndDrawing();
		
		if (_isDone==0){
			_currentDrawChar++;
			// If the next char we're about to display is the end of the line
			if (currentMessages[_currentDrawLine][_currentDrawChar]=='\0'){
				_currentDrawLine++;
				// If we just passed the line we'll be writing to next time then we're done
				if (_currentDrawLine==currentLine+1){
					_isDone=1; // We will no longer increment the current character
					_currentDrawLine-=1; // Fix this variable as we passed where we wanted to be
					_currentDrawChar=strlen(currentMessages[_currentDrawLine]); // The character we're displaying is at the end
				}else{ // Otherwise, start displaying at the start of the next line
					_currentDrawChar=0;
				}
			}
		}
		#if PLATFORM != PLAT_VITA
			FpsCapWait();
		#endif
	}

	// End of function
	endType = _endtypetemp;
}

void StopBGM(int _slot){
	StopMusic(currentMusic[_slot]);
	if (currentMusic[_slot]!=NULL){
		FreeMusic(currentMusic[_slot]);
		currentMusic[_slot]=NULL;
	}
}

void PlayBGM(const char* filename, int _volume, int _slot){
	if (currentMusic[_slot]!=NULL){
		StopMusic(currentMusic[_slot]);
		FreeMusic(currentMusic[_slot]);
		currentMusic[_slot]=NULL;
		currentMusicHandle[_slot]=0;
	}
	char* tempstringconcat = CombineStringsPLEASEFREE(STREAMINGASSETS, "/BGM/", filename, ".ogg");
	if (checkFileExist(tempstringconcat)==1){
		currentMusic[_slot] = LoadMusic(tempstringconcat);
		//SetMusicVolume(currentMusic[_slot],FixBGMVolume(_volume));
		currentMusicHandle[_slot] = PlayMusic(currentMusic[_slot]);
		SetMusicVolume(currentMusicHandle[_slot],FixBGMVolume(_volume));
		lastBGMVolume=_volume;
	}
	free(tempstringconcat);
	return;
}

// Settings file format:
// OPTIONSFILEFORMAT, 1 byte
// cpuOverclocked, 1 byte
// graphicsLocation, 1 byte
// autoModeWait, 4 bytes
// BGM volume, 1 byte, multiply it by 4 so it's a whole number when writing to save file
// SE volume, 1 byte, multiply it by 4 so it's a whole number when writing to save file
void SaveSettings(){
	FILE* fp;
	FixPath("settings.noob",globalTempConcat,TYPE_DATA);
	fp = fopen ((const char*)globalTempConcat, "w");
	//graphicsLocation

	unsigned char _bgmTemp = floor(bgmVolume*4);
	unsigned char _seTemp = floor(seVolume*4);

	unsigned char _tempOptionsFormat = OPTIONSFILEFORMAT;
	fwrite(&_tempOptionsFormat,1,1,fp);
	fwrite(&cpuOverclocked,1,1,fp);
	fwrite(&graphicsLocation,1,1,fp);
	fwrite(&autoModeWait,4,1,fp);

	fwrite(&_bgmTemp,1,1,fp);
	fwrite(&_seTemp,1,1,fp);

	fclose(fp);
	printf("SAved settings file.\n");
}

void LoadSettings(){
	FixPath("fontsize.noob",globalTempConcat,TYPE_DATA);
	if (checkFileExist((const char*)globalTempConcat)==0){
		SetDefaultFontSize();
	}else{
		LoadFontSizeFile();
	}

	FixPath("settings.noob",globalTempConcat,TYPE_DATA);
	if (checkFileExist((const char*)globalTempConcat)==1){
		FILE* fp;
		fp = fopen ((const char*)globalTempConcat, "r");
		unsigned char _tempOptionsFormat = 255;
		// This is the version of the format of the options file.
		fread(&_tempOptionsFormat,1,1,fp);
		if (_tempOptionsFormat>=1){
			fread(&cpuOverclocked,1,1,fp);
			fread(&graphicsLocation,1,1,fp);
			fread(&autoModeWait,4,1,fp);
		}
		if (_tempOptionsFormat>=2){
			unsigned char _bgmTemp;
			unsigned char _seTemp;

			fread(&_bgmTemp,1,1,fp);
			fread(&_seTemp,1,1,fp);
			bgmVolume = (float)_bgmTemp/4;
			seVolume = (float)_seTemp/4;
		}
		fclose(fp);

		if (cpuOverclocked==1){
			#if PLATFORM == PLAT_VITA
				scePowerSetArmClockFrequency(444);
			#endif
		}
		printf("Loaded settings file.\n");
	}
}

void DrawHistory(unsigned char _textStuffToDraw[][SINGLELINEARRAYSIZE]){
	ControlsEnd();
	int _noobHeight = TextHeight(fontSize);
	int _controlsStringWidth = TextWidth(fontSize,"UP and DOWN to scroll, "BACKBUTTONNAME" to return");
	int _scrollOffset=MAXMESSAGEHISTORY-HISTORYONONESCREEN;

	int i;
	while (1){
		FpsCapStart();

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_UP)){
			_scrollOffset--;
			if (_scrollOffset<0){
				_scrollOffset=0;
			}
		}
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_scrollOffset++;
			if (_scrollOffset>MAXMESSAGEHISTORY-HISTORYONONESCREEN){
				_scrollOffset=MAXMESSAGEHISTORY-HISTORYONONESCREEN;
			}
		}
		if (WasJustPressed(SCE_CTRL_CIRCLE) || WasJustPressed(SCE_CTRL_START)){
			ControlsEnd();
			break;
		}
		ControlsEnd();

		StartDrawing();

		if (currentBackground!=NULL){
			DrawBackground(currentBackground);
		}
	
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrder[i]!=255 && Busts[bustOrder[i]].isActive==1){
				DrawBust(&(Busts[bustOrder[i]]));
			}
		}
	
		for (i = MAXBUSTS-1; i != -1; i--){
			if (bustOrderOverBox[i]!=255 && Busts[bustOrderOverBox[i]].isActive==1){
				DrawBust(&(Busts[bustOrderOverBox[i]]));
			}
		}

		DrawRectangle(0,0,screenWidth,screenHeight,0,230,255,200);

		for (i = 0; i < HISTORYONONESCREEN; i++){
			//void GoodDrawTextColored(int x, int y, const char* text, float size, unsigned char r, unsigned char g, unsigned char b){
			GoodDrawTextColored(MESSAGETEXTXOFFSET,TextHeight(fontSize)+i*(TextHeight(fontSize)),(const char*)_textStuffToDraw[FixHistoryOldSub(i+_scrollOffset,oldestMessage)],fontSize,0,0,0);
		}

		GoodDrawTextColored(3,screenHeight-_noobHeight-5,"TEXTLOG",fontSize,0,0,0);
		GoodDrawTextColored(screenWidth-10-_controlsStringWidth,screenHeight-_noobHeight-5,"UP and DOWN to scroll, "BACKBUTTONNAME" to return",fontSize,0,0,0);

		DrawRectangle((screenWidth-5),0,5,screenHeight,0,0,0,255);
		DrawRectangle((screenWidth-5),floor(444*((double)_scrollOffset/(MAXMESSAGEHISTORY-HISTORYONONESCREEN))),5,100,255,0,0,255);

		EndDrawing();

		FpsCapWait();
	}
}

void ChangeEasyTouchMode(int _newControlValue){
	ControlsReset();
	easyTouchControlMode = _newControlValue;
}

#if PLATFORM == PLAT_WINDOWS

	void DrawTouchControlsHelp(){
		if (showControls==1){
			if (easyTouchControlMode!=TOUCHMODE_NONE){
				if (easyTouchControlMode==TOUCHMODE_MAINGAME){
					DrawTextureScaleSize(skipImage,0,0,screenWidth*.25,screenHeight*.20);
					DrawTextureScaleSize(autoImage,screenWidth*.25,0,screenWidth*.25,screenHeight*.20);
					DrawTextureScaleSize(textlogImage,screenWidth*.50,0,screenWidth*.25,screenHeight*.20);
					DrawTextureScaleSize(controlsMenuImage,screenWidth*.75,0,screenWidth*.25,screenHeight*.20);
				}else if (easyTouchControlMode==TOUCHMODE_MENU){
					DrawTextureScaleSize(controlsBackImage,0,0,screenWidth*.20,screenHeight);
					DrawTextureScaleSize(controlsSelectImage,screenWidth*.80,0,screenWidth*.20,screenHeight);
					DrawTextureScaleSize(controlsUpImage,screenWidth*.20,0,screenWidth*.60,screenHeight*.50);
					DrawTextureScaleSize(controlsDownImage,screenWidth*.20,screenHeight*.50,screenWidth*.60,screenHeight*.50);
				}else if (easyTouchControlMode == TOUCHMODE_LEFTRIGHTSELECT){

					DrawTextureScaleSize(controlsLeftImage,0,0,screenWidth*.20,screenHeight);
					DrawTextureScaleSize(controlsRightImage,screenWidth*.80,0,screenWidth*.20,screenHeight);
					DrawTextureScaleSize(controlsSelectImage,screenWidth*.20,0,screenWidth*.60,screenHeight*.50);
					DrawTextureScaleSize(controlsBackImage,screenWidth*.20,screenHeight*.50,screenWidth*.60,screenHeight*.50);
				
				}
			}
		}
	}

	// These are general control schemes that can be used in multiple areas
	// Change the variable easyTouchControlMode to one of the constants
	// Set to TOUCHMODE_NONE for touch controls that are special
	void CheckTouchControls(){

		if (WasJustPressedRegardless(SCE_ANDROID_BACK)){
			if (showControls==0){
				showControls=1;
			}else{
				showControls=0;
			}
		}

		if (easyTouchControlMode!=TOUCHMODE_NONE){
			if (easyTouchControlMode==TOUCHMODE_MAINGAME){
				if (WasJustPressed(SCE_TOUCH)){

					if (touchY<screenHeight*.20){
						if (touchX>screenWidth*.75){
							ChangeEasyTouchMode(TOUCHMODE_MENU);
							SettingsMenu();
							ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
						}else if (touchX>screenWidth*.50){
							//textlog
							ChangeEasyTouchMode(TOUCHMODE_MENU);
							DrawHistory(messageHistory);
							ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
						}else if (touchX>screenWidth*.25){
							//auto
							PlayMenuSound();
							if (autoModeOn==1){
								autoModeOn=0;
							}else{
								autoModeOn=1;
							}
						}else if (touchX>0){
							endType=Line_ContinueAfterTyping;
							isSkipping=1;
							PlayMenuSound();
						}
					}else{
						pad[SCE_CTRL_CROSS]=1;
					}

				}
				if (WasJustReleased(SCE_TOUCH)){
					pad[SCE_CTRL_CROSS]=0;
				}
			}else if (easyTouchControlMode==TOUCHMODE_MENU){
				if (WasJustPressed(SCE_TOUCH)){
					if (touchX>screenWidth*.80){
						pad[SCE_CTRL_CROSS]=1;
					}else if (touchX<screenWidth*.20){
						pad[SCE_CTRL_CIRCLE]=1;
					}else if (touchY>screenHeight*.5){
						pad[SCE_CTRL_DOWN]=1;
					}else{
						pad[SCE_CTRL_UP]=1;
					}
				}else if (WasJustReleased(SCE_TOUCH)){
					if (touchX>screenWidth*.80){
						pad[SCE_CTRL_CROSS]=0;
					}else if (touchX<screenWidth*.20){
						pad[SCE_CTRL_CIRCLE]=0;
					}else if (touchY>screenHeight*.5){
						pad[SCE_CTRL_DOWN]=0;
					}else{
						pad[SCE_CTRL_UP]=0;
					}
				}
			}else if (easyTouchControlMode == TOUCHMODE_LEFTRIGHTSELECT){
				if (WasJustPressed(SCE_TOUCH)){
					if (touchX<=screenWidth*.20){
						pad[SCE_CTRL_LEFT]=1;
					}else if (touchX>=screenWidth*.80){
						pad[SCE_CTRL_RIGHT]=1;
					}else if (touchY<screenHeight*.5){
						pad[SCE_CTRL_CROSS]=1;
					}else{
						pad[SCE_CTRL_CIRCLE]=1;
					}
				}else if (WasJustReleased(SCE_TOUCH)){
					if (touchX<=screenWidth*.20){
						pad[SCE_CTRL_LEFT]=0;
					}else if (touchX>=screenWidth*.80){
						pad[SCE_CTRL_RIGHT]=0;
					}else if (touchY<screenHeight*.5){
						pad[SCE_CTRL_CROSS]=0;
					}else{
						pad[SCE_CTRL_CIRCLE]=0;
					}
				}
			}
		}
	}
#endif

// FOLDER NAME SHOULD NOT END WITH SLASH
void GenerateStreamingAssetsPaths(char* _streamingAssetsFolderName){
	free(STREAMINGASSETS);
	free(PRESETFOLDER);
	free(SCRIPTFOLDER);
	free(SAVEFOLDER);
	
	STREAMINGASSETS = malloc(strlen(DATAFOLDER)+strlen(_streamingAssetsFolderName)+2);
	PRESETFOLDER = malloc(strlen(DATAFOLDER)+strlen(_streamingAssetsFolderName)+strlen("/Presets/")+1);
	SCRIPTFOLDER = malloc(strlen(DATAFOLDER)+strlen(_streamingAssetsFolderName)+strlen("/Scripts/")+1);
	SAVEFOLDER = malloc(strlen(DATAFOLDER)+strlen("Saves/")+1);
	strcpy(STREAMINGASSETS,DATAFOLDER);
	strcat(STREAMINGASSETS,_streamingAssetsFolderName);
	strcat(STREAMINGASSETS,"/");

	strcpy(PRESETFOLDER,DATAFOLDER);
	strcat(PRESETFOLDER,"Presets/");
	if (!directoryExists(PRESETFOLDER)){
		strcpy(PRESETFOLDER,DATAFOLDER);
		strcat(PRESETFOLDER,_streamingAssetsFolderName);
		strcat(PRESETFOLDER,"/Presets/");
		presetsAreInStreamingAssets=1;
	}else{
		presetsAreInStreamingAssets=0;
	}

	strcpy(SCRIPTFOLDER,DATAFOLDER);
	strcat(SCRIPTFOLDER,_streamingAssetsFolderName);
	strcat(SCRIPTFOLDER,"/Scripts/");

	strcpy(SAVEFOLDER,DATAFOLDER);
	strcat(SAVEFOLDER,"Saves/");
}

void UpdatePresetStreamingAssetsDir(char* filename){
	char _tempNewStreamingAssetsPathbuffer[256];
	strcpy(_tempNewStreamingAssetsPathbuffer,DATAFOLDER);
	strcat(_tempNewStreamingAssetsPathbuffer,"StreamingAssets_");
	strcat(_tempNewStreamingAssetsPathbuffer,filename);
	if (directoryExists(_tempNewStreamingAssetsPathbuffer)){
		// The directory does exist. Construct the string for the new StreamingAssets folder and regenerate the path strings
		strcpy(_tempNewStreamingAssetsPathbuffer,"StreamingAssets_");
		strcat(_tempNewStreamingAssetsPathbuffer,filename);
		GenerateStreamingAssetsPaths(_tempNewStreamingAssetsPathbuffer);
	}
}

/*
=================================================
*/

int L_DisplayWindow(lua_State* passedState){
	MessageBoxEnabled=0;
	return 0;
}

int L_ClearMessage(lua_State* passedState){
	//system("cls");
	currentLine=0;
	ClearMessageArray();
	return 0;
}

int L_OutputLine(lua_State* passedState){
	OutputLine((unsigned const char*)lua_tostring(passedState,4),lua_tonumber(passedState,5),0);
	return 0;
}

// Null, text, line type
int L_OutputLineAll(lua_State* passedState){
	OutputLine((unsigned const char*)lua_tostring(passedState,2),lua_tonumber(passedState,5),1);
	return 0;
}

//
int L_Wait(lua_State* passedState){
	if (isSkipping!=1 && capEnabled==1){
		wait(lua_tonumber(passedState,1));
	}
	return 0;
}

// filename, filter, unknown, unknown, time
int L_DrawSceneWithMask(lua_State* passedState){
	DrawScene(lua_tostring(passedState,1),lua_tonumber(passedState,5));
	return 0;
}

// filename
// fadein
int L_DrawScene(lua_State* passedState){
	DrawScene(lua_tostring(passedState,1),lua_tonumber(passedState,2));
	return 0;
}

int L_NotYet(lua_State* passedState){
	printf("An unimplemented Lua function was just executed!\n");
	return 0;
}

// Fist arg seems to be a channel arg.
	// Usually 1 for msys
	// Usually 2 for lsys
// Second arg is path in BGM folder without extention
// Third arg is volume. 128 seems to be average. I can hardly hear 8 with computer volume on 10.
// Fourth arg is unknown
int L_PlayBGM(lua_State* passedState){
	PlayBGM(lua_tostring(passedState,2),lua_tonumber(passedState,3),lua_tonumber(passedState,1));

	if (lua_tonumber(passedState,4)!=0){
		printf("*************** VERY IMPORTANT *******************\nThe last PlayBGM call didn't have 0 for the fourth argument! This is a good place to investigate!\n");
	}

	return 0;
}

// Some int argument
// Maybe music slot
int L_StopBGM(lua_State* passedState){
	StopBGM(lua_tonumber(passedState,1));
	return 0;
}

// Last arg, I was right. Bool for if wait for fadeoutr
// slot, time, (bool) should wait for finish
int L_FadeoutBGM(lua_State* passedState){
	if (currentMusicHandle[(int)lua_tonumber(passedState,1)]!=0){
		FadeoutMusic(currentMusicHandle[(int)lua_tonumber(passedState,1)],lua_tonumber(passedState,2));
		if (lua_toboolean(passedState,3)==1){
			wait(lua_tonumber(passedState,2));
		}
	}
	return 0;
}

// Bustshot slot? (Normally 1-3 used, 5 for black, 6 for cinema 7 for title)
	// Filename
	// Filter filename
	// ???
	// x offset (Character starts in the middle, screen is 640x480)
	// y offset (Character starts in the middle, screen is 640x480)
	// If the art starts in the middle of the screen and moves to its x and y offset
	// ???
	// ???
	// ???
	// ???
	// Some transparency argument? Nonzero makes the bust invisible
	// Layer. If one bust's layer is greater than another's, it's drawn above it. If it's the same layer number, more recent ones are on top
	// SPECIAL LAYERS -
		// Textbox's layer is 31. >31 layer shows a not darkened sprite.
	// Fadein time
	// (bool) wait for fadein? (15)
int L_DrawBustshotWithFiltering(lua_State* passedState){
	int i;
	for (i=8;i!=12;i++){
		if (lua_tonumber(passedState,i)!=0){
			printf("***********************IMPORTANT INFORMATION***************************\nAn argument I know nothing about was just used in DrawBustshotWithFiltering!\n***********************************************\n");
		}
	}

	//void DrawBustshot(unsigned char passedSlot, char* _filename, int _xoffset, int _yoffset, int _layer, int _fadeintime, int _waitforfadein, int _isinvisible){
	DrawBustshot(lua_tonumber(passedState,1), lua_tostring(passedState,2), lua_tonumber(passedState,5), lua_tonumber(passedState,6), lua_tonumber(passedState,13), lua_tonumber(passedState,14), lua_toboolean(passedState,15), lua_tonumber(passedState,12));

	////SDL_SetTextureAlphaMod(Busts[passedSlot].image, 255);
	return 0;
}

// Butshot slot
	// Filename
	// x offset (Character starts in the middle, screen is 640x480)
	// y offset (Character starts in the middle, screen is 640x480)
	// Some sort of scaling thing. 400 makes it invisible, 200 is half size. (scale applies to width and height) 1 is normal
	// If the art starts in the middle of the screen and moves to its x and y offset
	// ???
	// ???
	// ???
	// ???
	// ???
	// ???
	// Some transparency argument? Nonzero makes the bust invisible
	// Layer. If one bust's layer is greater than another's, it's drawn above it. If it's the same layer number, more recent ones are on top
	// 	// SPECIAL LAYERS -
	// 	// Textbox's layer is 31. >31 layer shows a not darkened sprite.
	// Fadein time
	// (bool) wait for fadein? (16)
	//
	// * The bustshot can only stay after a scene change if it's created the line before the scene change AND it doesn't wait for fadein completion.
int L_DrawBustshot(lua_State* passedState){
	Draw();

	int i;
	for (i=8;i!=12;i++){
		if (lua_tonumber(passedState,i)!=0){
			printf("***********************IMPORTANT INFORMATION***************************\nAn argument I know nothing about was just used in DrawBustshotWithFiltering!\n***********************************************\n");
		}
	}
	
	//void DrawBustshot(unsigned char passedSlot, char* _filename, int _xoffset, int _yoffset, int _layer, int _fadeintime, int _waitforfadein, int _isinvisible){
	DrawBustshot(lua_tonumber(passedState,1), lua_tostring(passedState,2), lua_tonumber(passedState,3), lua_tonumber(passedState,4), lua_tonumber(passedState,14), lua_tonumber(passedState,15), lua_toboolean(passedState,16), lua_tonumber(passedState,13));
	return 0;
}

int L_SetValidityOfInput(lua_State* passedState){
	if (lua_toboolean(passedState,1)==1){
		InputValidity=1;
	}else{
		InputValidity=0;
	}
	return 0;
}

// Fadeout time
// Wait for completely fadeout
int L_FadeAllBustshots(lua_State* passedState){
	FadeAllBustshots(lua_tonumber(passedState,1),lua_toboolean(passedState,2));
	//int i;
	//for (i=0;i<MAXBUSTS;i++){
	//	if (Busts[i].isActive==1){
	//		FreeTexture(Busts[i].image);
	//		ResetBustStruct(&(Busts[i]));
	//	}
	//}
	return 0;
}

int L_DisableWindow(lua_State* passedState){
	MessageBoxEnabled=0;
	return 0;
}

int L_FadeBustshotWithFiltering(lua_State* passedState){
	FadeBustshot(lua_tonumber(passedState,1),lua_tonumber(passedState,7),lua_toboolean(passedState,8));
	return 0;
}

//FadeBustshot( 2, FALSE, 0, 0, 0, 0, 0, TRUE );
//FadeBustshot( SLOT, MOVE, X, Y, UNKNOWNA, UNKNOWNB, FADETIME, WAIT );
int L_FadeBustshot(lua_State* passedState){
	FadeBustshot(lua_tonumber(passedState,1),lua_tonumber(passedState,7),lua_toboolean(passedState,8));
	return 0;
}

// Slot, file, volume
int L_PlaySE(lua_State* passedState){
	if (isSkipping==0 && seVolume>0){
		GenericPlaySound(lua_tonumber(passedState,1),lua_tostring(passedState,2),lua_tonumber(passedState,3),"/SE/");
	}
	return 0;
}

// PlayVoice(channel, filename, volume)
int L_PlayVoice(lua_State* passedState){
	if (isSkipping==0){
		GenericPlaySound(lua_tonumber(passedState,1),lua_tostring(passedState,2),lua_tonumber(passedState,3),"/voice/");
	}
	return 0;
}

// Loads a script file
int L_CallScript(lua_State* passedState){
	const char* filename = lua_tostring(passedState,1);

	char* tempstringconcat = CombineStringsPLEASEFREE(SCRIPTFOLDER, "",filename,".txt");
	char tempstring2[strlen(tempstringconcat)+1];
	strcpy(tempstring2,tempstringconcat);
	free(tempstringconcat);

	if (checkFileExist(tempstring2)==1){
		printf("Do script %s\n",tempstring2);
		RunScript("",tempstring2,0);
	}else{
		printf("Failed to find script\n");
	}
	return 0;
}

// "bg_166", 7, 200, 0
int L_ChangeScene(lua_State* passedState){
	DrawScene(lua_tostring(passedState,1),0);
	return 0;
}

// DrawSprite(slot, filename, ?, x, y, ?, ?, ?, ?, ?, ?, ?, ?, LAYER, FADEINTIME, WAITFORFADEIN)
// x is relative to -320
	// y is relative to -240???
	// DrawSprite(slot, filename, ?, x, y, ?, ?, ?, ?, ?, ?, ?, ?, LAYER, FADEINTIME, WAITFORFADEIN)
int L_DrawSprite(lua_State* passedState){
	//void DrawBustshot(unsigned char passedSlot, char* _filename, int _xoffset, int _yoffset, int _layer, int _fadeintime, int _waitforfadein, int _isinvisible){
	DrawBustshot(lua_tonumber(passedState,1),lua_tostring(passedState,2),320+lua_tonumber(passedState,4),240+lua_tonumber(passedState,5),lua_tonumber(passedState,14), lua_tonumber(passedState,15),lua_toboolean(passedState,16),0);
	//DrawBustshot(lua_tonumber(passedState,1)-1, lua_tostring(passedState,2), lua_tonumber(passedState,3), lua_tonumber(passedState,4), lua_tonumber(passedState,14), lua_tonumber(passedState,15), lua_toboolean(passedState,16), lua_tonumber(passedState,13));
	return 0;
}

//MoveSprite(slot, destinationx, destinationy, ?, ?, ?, ?, ?, timeittakes, waitforcompletion)
	// MoveSprite(5,-320,-4500,0,0,0,0,0,101400, TRUE)
int L_MoveSprite(lua_State* passedState){
	int _totalTime = lua_tonumber(passedState,9);
	int _passedSlot = lua_tonumber(passedState,1);
	// Number of x pixles the sprite has to move by the end

	printf("arg2:%d\n",(int)lua_tonumber(passedState,2));
	printf("x:%d\n",Busts[_passedSlot].xOffset);

	int _xTengoQue = lua_tonumber(passedState,2)-(Busts[_passedSlot].xOffset-320);
	int _yTengoQue = lua_tonumber(passedState,3)-(Busts[_passedSlot].yOffset-240);
	char _waitforcompletion = lua_toboolean(passedState,10);

	Busts[_passedSlot].statusVariable3 = lua_tonumber(passedState,2)+320;
	Busts[_passedSlot].statusVariable4 = lua_tonumber(passedState,3)+240;

	if (_totalTime!=0){
		unsigned int _totalFrames = floor(60*(_totalTime/(double)1000));
		int _xPerFrame = floor(_xTengoQue/_totalFrames);
		if (_xPerFrame==0){
			_xPerFrame=1;
		}
		printf("xtq: %d\n",_xTengoQue);
		printf("xprf: %d\n",_xPerFrame);
		printf("ytq: %d\n",_yTengoQue);
		printf("tf: %d\n",_totalFrames);
		int _yPerFrame = floor(_yTengoQue/(double)_totalFrames);
		if (_yPerFrame==0){
			_yPerFrame=1;
		}

		Busts[_passedSlot].statusVariable = _xPerFrame;
		printf("yperfraeme: %d\n",_yPerFrame);
		Busts[_passedSlot].statusVariable2 = _yPerFrame;
		Busts[_passedSlot].bustStatus = BUST_STATUS_SPRITE_MOVE;

	}else{
		Busts[_passedSlot].xOffset=lua_tonumber(passedState,2)+320;
		Busts[_passedSlot].yOffset=lua_tonumber(passedState,3)+240;
	}

	if (_waitforcompletion==1){
		while(Busts[_passedSlot].bustStatus!=BUST_STATUS_NORMAL){
			FpsCapStart();
			ControlsStart();
			if (WasJustPressed(SCE_CTRL_CROSS)){
				Busts[_passedSlot].xOffset=lua_tonumber(passedState,2)+320;
				Busts[_passedSlot].yOffset=lua_tonumber(passedState,3)+240;
			}
			ControlsEnd();
			Update();
			Draw();
			FpsCapWait();
		}
	}
	return 0;
}

//FadeSprite(slot, time, waitfrocompletion)
		// FadeSprite(5,700,FALSE)
int L_FadeSprite(lua_State* passedState){
	FadeBustshot(lua_tonumber(passedState,1),lua_tonumber(passedState,2),lua_toboolean(passedState,3));	
	return 0;
}

// Select(numoptions, arrayofstring)
//		Let's the user make a choice and have this not be a sound novel anymore. :/
//		First arg is the number of options and the second arg is a string of the names of the options
//		Result can be found in LoadValueFromLocalWork("SelectResult")
//			Choice result is zero based
//				First choice is zero, second is one
int L_Select(lua_State* passedState){
	ChangeEasyTouchMode(TOUCHMODE_MENU);
	int _totalOptions = lua_tonumber(passedState,1);
	char* noobOptions[_totalOptions];
	int i;
	for (i=0;i<_totalOptions;i++){
		lua_rawgeti(passedState,2,i+1);
		noobOptions[i] = (char*)calloc(1,strlen(lua_tostring(passedState,-1))+1);
		strcpy(noobOptions[i],lua_tostring(passedState,-1));
	}

	// This is the actual loop for choosing the choice
	signed char _choice=0;
	while (1){
		FpsCapStart();
		ControlsStart();

		_choice = MenuControls(_choice,0,_totalOptions-1);

		if (WasJustPressed(SCE_CTRL_CROSS)){
			lastSelectionAnswer = _choice;
			break;
		}
		ControlsEnd();
		StartDrawing();
		DrawBackground(currentBackground);
		if (MessageBoxEnabled==1){
			DrawMessageBox();
		}
		for (i=0;i<_totalOptions;i++){
			GoodDrawText(32,i*currentTextHeight,noobOptions[i],fontSize);
		}
		GoodDrawText(0,_choice*currentTextHeight,">",fontSize);

		EndDrawing();
		FpsCapWait();
	}

	// Free strings that were made with calloc earlier
	for (i=0;i<_totalOptions;i++){
		free(noobOptions[i]);
	}
	ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
	return 0;
}

// Loads a special variable
int L_LoadValueFromLocalWork(lua_State* passedState){
	const char* _wordWant = lua_tostring(passedState,1);
	printf("%s\n",_wordWant);
	if ( strcmp(_wordWant,"SelectResult")==0){
		lua_pushnumber(passedState,lastSelectionAnswer);
	}else{
		LazyMessage("Unknown LoadValueFromLocalWork!",_wordWant,"Please report to MyLegGuy!","thx");
	}
	return 1;
}

// Calls a function that was made in a script
int L_CallSection(lua_State* passedState){
	char buf[256];
	strcpy(buf, lua_tostring(passedState,1));
	strcat(buf,"()");
	printf("%s\n",buf);
	luaL_dostring(L,buf);
	return 0;
}

// I CAN DO THIS EZ-PZ WITH DRAWING RECTANGLES OVER THE SCREEN
// DrawFilm( 2,  0, 255, 0, 255, 0, 1000, TRUE );
// DrawFilm (slot?, r, g, b, filer's alpha, ?, fadein time, wait for fadein)
int L_DrawFilm(lua_State* passedState){
	filterR = lua_tonumber(passedState,2);
	filterG = lua_tonumber(passedState,3);
	filterB = lua_tonumber(passedState,4);
	filterA = lua_tonumber(passedState,5);
	filterActive=1;
	return 0;
}

int L_FadeFilm(lua_State* passedState){
	filterActive=0;
	return 1;
}

// This command is used so unoften that I didn't bother to make it look good.
// FadeBG( 3000, TRUE );
int L_FadeBG(lua_State* passedState){
	if (currentBackground!=NULL){
		FreeTexture(currentBackground);
		currentBackground=NULL;
	}
	return 0;
}

int L_DebugFile(lua_State* passedState){
	WriteToDebugFile(lua_tostring(passedState,1));
	return 0;
}

void MakeLuaUseful(){
	LUAREGISTER(L_OutputLine,"OutputLine")
	LUAREGISTER(L_ClearMessage,"ClearMessage")
	LUAREGISTER(L_OutputLineAll,"OutputLineAll")
	LUAREGISTER(L_Wait,"Wait")
	LUAREGISTER(L_DrawScene,"DrawScene")
	LUAREGISTER(L_DrawSceneWithMask,"DrawSceneWithMask")
	LUAREGISTER(L_PlayBGM,"PlayBGM")
	LUAREGISTER(L_FadeoutBGM,"FadeOutBGM")
	LUAREGISTER(L_DrawBustshotWithFiltering,"DrawBustshotWithFiltering");
	LUAREGISTER(L_SetValidityOfInput,"SetValidityOfInput")
	LUAREGISTER(L_FadeAllBustshots,"FadeAllBustshots")
	LUAREGISTER(L_DisableWindow,"DisableWindow")
	LUAREGISTER(L_DrawBustshot,"DrawBustshot")
	LUAREGISTER(L_FadeBustshotWithFiltering,"FadeBustshotWithFiltering")
	LUAREGISTER(L_FadeBustshot,"FadeBustshot")
	LUAREGISTER(L_DrawScene,"DrawBG")
	LUAREGISTER(L_PlaySE,"PlaySE")
	LUAREGISTER(L_StopBGM,"StopBGM")
	LUAREGISTER(L_CallScript,"CallScript") // Somehow, this works. No idea how.
	LUAREGISTER(L_Select,"Select") // That's right, I programmed selection for the one time in the question arcs where it's used
	LUAREGISTER(L_LoadValueFromLocalWork,"LoadValueFromLocalWork")
	LUAREGISTER(L_CallSection,"CallSection")
	LUAREGISTER(L_CallSection,"JumpSection") // TODO - Is this correct?
	LUAREGISTER(L_ChangeScene,"ChangeScene")
	LUAREGISTER(L_DrawSprite,"DrawSprite")
	LUAREGISTER(L_MoveSprite,"MoveSprite")
	LUAREGISTER(L_FadeSprite,"FadeSprite")
	LUAREGISTER(L_DrawFilm,"DrawFilm")
	LUAREGISTER(L_FadeFilm,"FadeFilm") // Used for fixing negative and removing films?
	LUAREGISTER(L_FadeBG,"FadeBG")
	LUAREGISTER(L_DebugFile,"Debugfile")
	LUAREGISTER(L_PlayVoice,"PlayVoice")
	LUAREGISTER(L_DisplayWindow,"DisplayWindow")

	// Functions that do nothing
	LUAREGISTER(L_NotYet,"SetFontId")
	LUAREGISTER(L_NotYet,"SetCharSpacing")
	LUAREGISTER(L_NotYet,"SetLineSpacing")
	LUAREGISTER(L_NotYet,"SetFontSize")
	LUAREGISTER(L_NotYet,"SetNameFormat")
	LUAREGISTER(L_NotYet,"SetScreenAspect")
	LUAREGISTER(L_NotYet,"SetWindowPos")
	LUAREGISTER(L_NotYet,"SetWindowSize")
	LUAREGISTER(L_NotYet,"SetWindowMargins")
	LUAREGISTER(L_NotYet,"SetGUIPosition")
	
	LUAREGISTER(L_NotYet,"SetValidityOfSaving")
	LUAREGISTER(L_NotYet,"SetValidityOfLoading")

	//  TEMP
	LUAREGISTER(L_NotYet,"SetSpeedOfMessage")
	LUAREGISTER(L_NotYet,"ShakeScreen")
	LUAREGISTER(L_NotYet,"ShakeScreenSx")
	LUAREGISTER(L_NotYet,"SetDrawingPointOfMessage")
	LUAREGISTER(L_NotYet,"SetStyleOfMessageSwinging")
	LUAREGISTER(L_NotYet,"SetValidityOfWindowDisablingWhenGraphicsControl")
	LUAREGISTER(L_NotYet,"EnableJumpingOfReturnIcon")
	LUAREGISTER(L_NotYet,"StopSE")

	LUAREGISTER(L_NotYet,"SetValidityOfTextFade")
	
	LUAREGISTER(L_NotYet,"SetValidityOfSkipping")
	LUAREGISTER(L_NotYet,"GetAchievement")
	LUAREGISTER(L_NotYet,"SetFontOfMessage")
	
	LUAREGISTER(L_NotYet,"ActivateScreenEffectForcedly")
	LUAREGISTER(L_NotYet,"SetValidityOfUserEffectSpeed")
	LUAREGISTER(L_NotYet,"Negative") // Command for color inversion
									// Negative( 1000, TRUE ); is inveted
									// FadeFilm( 200, TRUE ); fixes it??!
									// Name provably means to negate the colors, or replace the colors with their complementary ones on the other side of the color wheel
									// First arg is maybe time when it fades to inverted and argument is proably if it's inverted
	// Not investigated yet
		LUAREGISTER(L_NotYet,"BlurOffOn")
		LUAREGISTER(L_NotYet,"Break")
		LUAREGISTER(L_NotYet,"ChangeBustshot")
		LUAREGISTER(L_NotYet,"ChangeVolumeOfBGM")
		LUAREGISTER(L_NotYet,"ChapterPreview")
		LUAREGISTER(L_NotYet,"CheckTipsAchievements")
		LUAREGISTER(L_NotYet,"CloseGallery")
		LUAREGISTER(L_NotYet,"ControlMotionOfSprite")
		LUAREGISTER(L_NotYet,"DisableBlur")
		LUAREGISTER(L_NotYet,"DisableEffector")
		LUAREGISTER(L_NotYet,"DisableGradation")
		LUAREGISTER(L_NotYet,"DrawBGWithMask")
		LUAREGISTER(L_NotYet,"DrawBustFace")
		LUAREGISTER(L_NotYet,"DrawFace")
		LUAREGISTER(L_NotYet,"DrawSpriteWithFiltering")
		LUAREGISTER(L_NotYet,"DrawStandgraphic")
		LUAREGISTER(L_NotYet,"EnableBlur")
		LUAREGISTER(L_NotYet,"EnableHorizontalGradation")
		LUAREGISTER(L_NotYet,"EnlargeScreen")
		LUAREGISTER(L_NotYet,"ExecutePlannedControl")
		LUAREGISTER(L_NotYet,"FadeAllBustshots2")
		LUAREGISTER(L_NotYet,"FadeAllBustshots3")
		LUAREGISTER(L_NotYet,"FadeFace")
		LUAREGISTER(L_NotYet,"FadeOutMultiBGM")
		LUAREGISTER(L_NotYet,"FadeOutSE")
		LUAREGISTER(L_NotYet,"FadeScene")
		LUAREGISTER(L_NotYet,"FadeSceneWithMask")
		LUAREGISTER(L_NotYet,"FadeSpriteWithFiltering")
		LUAREGISTER(L_NotYet,"GetLocalFlag")
		LUAREGISTER(L_NotYet,"GetPositionOfSprite")
		LUAREGISTER(L_NotYet,"HideGallery")
		LUAREGISTER(L_NotYet,"JumpScript")
		LUAREGISTER(L_NotYet,"LanguagePrompt")
		LUAREGISTER(L_NotYet,"MoveBustshot")
		LUAREGISTER(L_NotYet,"MoveSpriteEx")
		LUAREGISTER(L_NotYet,"NullOp")
		LUAREGISTER(L_NotYet,"OpenGallery")
		LUAREGISTER(L_NotYet,"PlaceViewTip")
		LUAREGISTER(L_NotYet,"PlaceViewTip2")
		LUAREGISTER(L_NotYet,"PlusStandgraphic1")
		LUAREGISTER(L_NotYet,"PlusStandgraphic2")
		LUAREGISTER(L_NotYet,"PlusStandgraphic3")
		LUAREGISTER(L_NotYet,"PreloadBitmap")
		LUAREGISTER(L_NotYet,"Return")
		LUAREGISTER(L_NotYet,"RevealGallery")
		LUAREGISTER(L_NotYet,"SavePoint")
		LUAREGISTER(L_NotYet,"SetGuiPosition")
		LUAREGISTER(L_NotYet,"SetLocalFlag")
		LUAREGISTER(L_NotYet,"SetSkipAll")
		LUAREGISTER(L_NotYet,"SetTextFade")
		LUAREGISTER(L_NotYet,"SetValidityOfFilmToFace")
		LUAREGISTER(L_NotYet,"SetValidityOfInterface")
		LUAREGISTER(L_NotYet,"SpringText")
		LUAREGISTER(L_NotYet,"StartShakingOfAllObjects")
		LUAREGISTER(L_NotYet,"StartShakingOfBustshot")
		LUAREGISTER(L_NotYet,"StartShakingOfSprite")
		LUAREGISTER(L_NotYet,"StartShakingOfWindow")
		LUAREGISTER(L_NotYet,"StoreValueToLocalWork")
		LUAREGISTER(L_NotYet,"TerminateShakingOfAllObjects")
		LUAREGISTER(L_NotYet,"TerminateShakingOfBustshot")
		LUAREGISTER(L_NotYet,"TerminateShakingOfSprite")
		LUAREGISTER(L_NotYet,"TerminateShakingOfWindow")
		LUAREGISTER(L_NotYet,"TitleScreen")
		LUAREGISTER(L_NotYet,"ViewChapterScreen")
		LUAREGISTER(L_NotYet,"ViewExtras")
		LUAREGISTER(L_NotYet,"ViewTips")
		LUAREGISTER(L_NotYet,"WaitForInput")
		LUAREGISTER(L_NotYet,"WaitToFinishSEPlaying")
		LUAREGISTER(L_NotYet,"WaitToFinishVoicePlaying")
}

//======================================================

void Draw(){
	int i;
	//DrawTexture(testtex,32,32);

	StartDrawing();

	if (currentBackground!=NULL){
		DrawBackground(currentBackground);
	}

	for (i = MAXBUSTS-1; i != -1; i--){
		if (bustOrder[i]!=255 && Busts[bustOrder[i]].isActive==1){
			DrawBust(&(Busts[bustOrder[i]]));
		}
	}

	if (MessageBoxEnabled==1 && filterActive==0){
		// This is the message box
		DrawMessageBox();
	}

	for (i = MAXBUSTS-1; i != -1; i--){
		if (bustOrderOverBox[i]!=255 && Busts[bustOrderOverBox[i]].isActive==1){
			DrawBust(&(Busts[bustOrderOverBox[i]]));
		}
	}
	
	if (filterActive==1){
		DrawCurrentFilter();
	}
	if (MessageBoxEnabled==1){
		DrawMessageText();
	}
	EndDrawing();
}

// Returns what RunScript returns
char LuaThread(char* _torun){
	return RunScript(SCRIPTFOLDER, _torun,1);
}

// Returns 0 if normal
// Returns 1 if user quit
// Returns 2 if no files found
char FileSelector(char* directorylocation, char** _chosenfile, char* promptMessage){

	int i=0;
	int totalFiles=0;

	int _returnVal=0;

	// Can hold MAXFILES filenames
	// Each with no more than 50 characters (51 char block of memory. extra one for null char)
	char** filenameholder;
	filenameholder = (char**)calloc(MAXFILES,sizeof(char*));
	for (i=0;i<MAXFILES;i++){
		filenameholder[i]=(char*)calloc(1,MAXFILELENGTH);
	}

	CROSSDIR dir;
	CROSSDIRSTORAGE lastStorage;
	dir = openDirectory (directorylocation);

	if (dirOpenWorked(dir)==0){
		LazyMessage("Failed to open directory",directorylocation,NULL,NULL);
		// Free memori
		for (i=0;i<MAXFILES;i++){
			free(filenameholder[i]);
		}
		free(filenameholder);
		*_chosenfile = NULL;
		return 2;
	}

	for (i=0;i<MAXFILES;i++){
		if (directoryRead(&dir,&lastStorage) == 0){
			break;
		}
		memcpy((filenameholder[i]),getDirectoryResultName(&lastStorage),strlen(getDirectoryResultName(&lastStorage))+1);
	}
	directoryClose (dir);
	
	totalFiles = i;

	if (totalFiles==0){
		LazyMessage("No files found.",NULL,NULL,NULL);
		*_chosenfile=NULL;
		_returnVal=2;
	}else{
		int _choice=0;
		int _maxPerNoScroll=floor((screenHeight-5-currentTextHeight*2)/(currentTextHeight));
		if (totalFiles<_maxPerNoScroll){
			_maxPerNoScroll=totalFiles;
		}
		int _tmpoffset=0;
		while (currentGameStatus!=GAMESTATUS_QUIT){
			FpsCapStart();
			ControlsStart();
	
			if (WasJustPressed(SCE_CTRL_UP)){
				_choice--;
				if (_choice<0){
					_choice=totalFiles-1;
				}
			}
			if (WasJustPressed(SCE_CTRL_DOWN)){
				_choice++;
				if (_choice>=totalFiles){
					_choice=0;
				}
			}
			if (WasJustPressed(SCE_CTRL_RIGHT)){
				_choice+=5;
				if (_choice>=totalFiles){
					_choice=totalFiles-1;
				}
			}
			if (WasJustPressed(SCE_CTRL_LEFT)){
				_choice-=5;
				if (_choice<0){
					_choice=0;
				}
			}
			if (WasJustPressed(SCE_CTRL_CROSS)){
				(*_chosenfile) = (char*)calloc(1,strlen(filenameholder[_choice])+1);
				memcpy(*_chosenfile,filenameholder[_choice],strlen(filenameholder[_choice])+1);
				PlayMenuSound();
				break;		
			}
			if (WasJustPressed(SCE_CTRL_CIRCLE)){
				(*_chosenfile) = NULL;
				_returnVal=1;
				break;		
			}
	
			StartDrawing();
			//DrawText(20,20+TextHeight(fontSize)+i*(TextHeight(fontSize)),currentMessages[i],fontSize);
			if (promptMessage!=NULL){
				GoodDrawText(32,5,promptMessage,fontSize);
			}
			_tmpoffset=_choice+1-_maxPerNoScroll;
			if (_tmpoffset<0){
				_tmpoffset=0;
			}
			for (i=0;i<_maxPerNoScroll;i++){
				GoodDrawText(32,5+currentTextHeight*(i+2),filenameholder[i+_tmpoffset],fontSize);
			}
			GoodDrawTextColored(32,5+currentTextHeight*((_choice-_tmpoffset)+2),filenameholder[_choice],fontSize,0,255,0);
			GoodDrawText(5,5+currentTextHeight*((_choice-_tmpoffset)+2),">",fontSize);
			EndDrawing();
			ControlsEnd();
			FpsCapWait();
		}
	}

	// Free memori
	for (i=0;i<MAXFILES;i++){
		free(filenameholder[i]);
	}
	free(filenameholder);
	return _returnVal;
}

void FontSizeSetup(){
	ChangeEasyTouchMode(TOUCHMODE_MENU);
	char _choice=0;
	char _tempNumberString[10];
	itoa(fontSize,_tempNumberString,10);
	while (1){
		FpsCapStart();
		ControlsStart();
		_choice = MenuControls(_choice,0,2);

		if (WasJustPressed(SCE_CTRL_CROSS) || WasJustPressed(SCE_CTRL_RIGHT)){
			if (_choice==0){
				fontSize++;
				itoa(fontSize,_tempNumberString,10);
				#if PLATFORM == PLAT_VITA
					currentTextHeight = TextHeight(fontSize);
				#endif
			}else if (_choice==1){
				ReloadFont();
			}else if (_choice==2){
				ChangeEasyTouchMode(TOUCHMODE_MENU);
				ReloadFont();
				break;
			}
		}
		if (WasJustPressed(SCE_CTRL_CIRCLE) || WasJustPressed(SCE_CTRL_LEFT)){
			if (_choice==0){
				fontSize--;
				if (fontSize<=5){
					fontSize=6;
				}
				itoa(fontSize,_tempNumberString,10);
				#if PLATFORM == PLAT_VITA
					currentTextHeight = TextHeight(fontSize);
				#endif
			}
		}
		ControlsEnd();

		//void itoa(int _num, char* _buffer, int _uselessBase){

		StartDrawing();
		//void DrawText(int x, int y, const char* text, float size){
		GoodDrawText(32,currentTextHeight,"Font Size: ",fontSize);
			GoodDrawText(32+TextWidth(fontSize,"Font Size: "),currentTextHeight,_tempNumberString,fontSize);
		#if PLATFORM != PLAT_VITA
			GoodDrawText(32,currentTextHeight*2,"Test",fontSize);
			GoodDrawText(32,currentTextHeight*5,"While the text may look bad now, restarting ",fontSize);
			GoodDrawText(32,currentTextHeight*6,"after changing it will make it look good.",fontSize);
		#endif
		GoodDrawText(32,currentTextHeight*3,"Done",fontSize);

		#if PLATFORM != PLAT_VITA
			GoodDrawText(32,currentTextHeight*5,"You should be able to see this entire line. It shouldn't cut off.",fontSize);
	
			GoodDrawText(32,currentTextHeight*8,"Press the BACK button to see the controls. Green and red are used",fontSize);
			GoodDrawText(32,currentTextHeight*9,"to change the font size when you're on the first option.",fontSize);
	
			GoodDrawText(32,currentTextHeight*11,"You have to select \"Test\" to see the new size.",fontSize);
	
			GoodDrawText(32,currentTextHeight*13,"aeiouthnaeiouthnaeiouthnaeiouthnaeiouthnaeiouthnaeiouthnaeiouthn",fontSize);
		#endif
		

		GoodDrawText(5,currentTextHeight*(_choice+1),">",fontSize);
		EndDrawing();
		FpsCapWait();
	}
	SaveFontSizeFile();
}

void SettingsMenu(){
	PlayMenuSound();
	signed char _choice=0;
	char _tempAutoModeString[10] = {'\0'};
	int _tempStrWidth = TextWidth(fontSize,"Auto Mode Speed: ");
	itoa(autoModeWait,_tempAutoModeString,10);
	char _needResave=0;
	int _bustlocationcollinspacewidth = TextWidth(fontSize,"Bust location: ");
	char _canShowRena=0;
	// This variable is used to check if the player changed the bust location after exiting
	char _artBefore=graphicsLocation;
	CrossTexture* _renaImage=NULL;
	int _noobBGMVolumeWidth = TextWidth(fontSize,"BGM Volume  ");
	char _tempItoaHoldBGM[5] = {'\0'};
	char _tempItoaHoldSE[5] = {'\0'};

	// This checks if we have Rena busts in CG AND CGAlt
	char* _temppath = CombineStringsPLEASEFREE(STREAMINGASSETS,"CG/","re_se_de_a1.png","");
	if (checkFileExist(_temppath)==1){
		free(_temppath);
		_temppath = CombineStringsPLEASEFREE(STREAMINGASSETS,"CGAlt/","re_se_de_a1.png","");
		if (checkFileExist(_temppath)==1){
			_canShowRena=1;
		}
	}
	free(_temppath);
	
	// Loads Rena, if possible
	if (_canShowRena==1){
		_temppath = CombineStringsPLEASEFREE(STREAMINGASSETS,locationStrings[graphicsLocation],"re_se_de_a1.png","");
		_renaImage = SafeLoadPNG(_temppath);
		free(_temppath);
	}

	itoa(bgmVolume*4,_tempItoaHoldBGM,10);
	itoa(seVolume*4, _tempItoaHoldSE,10);

	while (1){
		FpsCapStart();
		ControlsStart();

		if (currentGameStatus!=GAMESTATUS_TITLE){
			_choice = MenuControls(_choice,0,9);
		}else{
			_choice = MenuControls(_choice,0,8);
		}

		if (WasJustPressed(SCE_CTRL_CIRCLE)){
			if (_choice==2){
				autoModeWait-=500;
				if (autoModeWait<=0){
					autoModeWait=500;
				}
				itoa(autoModeWait,_tempAutoModeString,10);
			}else{
				break;
			}
		}
		if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_choice==0){ // Resume
				PlayMenuSound();
				break;
			}else if (_choice==9){ // Quit
				endType = Line_ContinueAfterTyping;
				
				if (_choice==99){
					currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
				}else{
					currentGameStatus=GAMESTATUS_QUIT;
				}
				
				lua_getglobal(L,"quitxfunction");
				lua_call(L, 0, 0);
				break;
			}else if (_choice==1){ // OLD Restart BGM choice. Now an unused spot.
			}else if (_choice==2){ // Auto mode speed
				autoModeWait+=500;
				itoa(autoModeWait,_tempAutoModeString,10);
			}else if (_choice==4){ // CPU speed
				PlayMenuSound();
				if (cpuOverclocked==0){
					cpuOverclocked=1;
					#if PLATFORM == PLAT_VITA
						scePowerSetArmClockFrequency(444);
					#endif
				}else if (cpuOverclocked==1){
					cpuOverclocked=0;
					#if PLATFORM == PLAT_VITA
						scePowerSetArmClockFrequency(333);
					#endif
				}
			}else if (_choice==8){
				PlayMenuSound();
				if (LazyChoice("This will reset your settings.","Is this okay?",NULL,NULL)==1){
					autoModeWait=500;
					graphicsLocation = LOCATION_CGALT;
					cpuOverclocked=0; // We don't actually change the CPU speed. They'll never notice. ;)
					bgmVolume=.75;
					seVolume=1.0;
					// Some need to have their strings changed so the user can actually see the changes
					itoa(autoModeWait,_tempAutoModeString,10);
					itoa(bgmVolume*4,_tempItoaHoldBGM,10);
					itoa(seVolume*4, _tempItoaHoldSE,10);
					// Update music volume using new default setting
					SetAllMusicVolume(FixBGMVolume(lastBGMVolume));
				}
			}else if (_choice==7){
				FontSizeSetup();
				_bustlocationcollinspacewidth = TextWidth(fontSize,"Bust location: ");
				_noobBGMVolumeWidth = TextWidth(fontSize,"BGM Volume  ");
				_tempStrWidth = TextWidth(fontSize,"Auto Mode Speed: ");
				currentTextHeight = TextHeight(fontSize);
			}
		}

		if (WasJustPressed(SCE_CTRL_LEFT)){
			if (_choice==2){
				if (IsDown(SCE_CTRL_LTRIGGER)){
					autoModeWait-=200;
				}else{
					autoModeWait-=500;
				}
				if (autoModeWait<=0){
					autoModeWait=500;
				}
				itoa(autoModeWait,_tempAutoModeString,10);
				_needResave=1;
			}else if (_choice==5){
				if (bgmVolume==0){
					bgmVolume=1.25;
				}
				bgmVolume-=.25;
				itoa(bgmVolume*4,_tempItoaHoldBGM,10);
				SetAllMusicVolume(FixBGMVolume(lastBGMVolume));
			}else if (_choice==6){
				if (seVolume==0){
					seVolume=1.25;
				}
				seVolume-=.25;
				itoa(seVolume*4,_tempItoaHoldSE,10);
				if (menuSoundLoaded==1){
					SetSFXVolumeBefore(menuSound,FixSEVolume(256));
				}
				PlayMenuSound();
			}
		}
		if (WasJustPressed(SCE_CTRL_RIGHT)){
			if (_choice==2){
				if (IsDown(SCE_CTRL_LTRIGGER)){
					autoModeWait+=200;
				}else{
					autoModeWait+=500;
				}
				itoa(autoModeWait,_tempAutoModeString,10);
				_needResave=1;
			}
		}
		if (WasJustPressed(SCE_CTRL_CROSS) || WasJustPressed(SCE_CTRL_RIGHT) || WasJustPressed(SCE_CTRL_LEFT)){
			if (!WasJustPressed(SCE_CTRL_LEFT)){
				if (_choice==5){
					if (bgmVolume==1){
						bgmVolume=0;
					}else{
						bgmVolume+=.25;
					}
					itoa(bgmVolume*4,_tempItoaHoldBGM,10);
					SetAllMusicVolume(FixBGMVolume(lastBGMVolume));
				}else if (_choice==6){
					if (seVolume==1){
						seVolume=0;
					}else{
						seVolume+=.25;
					}
					itoa(seVolume*4,_tempItoaHoldSE,10);

					if (menuSoundLoaded==1){
						SetSFXVolumeBefore(menuSound,FixSEVolume(256));
					}
					PlayMenuSound();

				}
			}
			if (_choice==3){
				PlayMenuSound();
				if (graphicsLocation == LOCATION_CG){
					graphicsLocation = LOCATION_CGALT;
				}else if (graphicsLocation == LOCATION_CGALT){
					graphicsLocation = LOCATION_CG;
				}
				if (_canShowRena==1){
					FreeTexture(_renaImage);
					_temppath = CombineStringsPLEASEFREE(STREAMINGASSETS,locationStrings[graphicsLocation],"re_se_de_a1.png","");
					_renaImage = SafeLoadPNG(_temppath);
					free(_temppath);
				}
			}
		}

		ControlsEnd();
		StartDrawing();
		// Display sample Rena if changing bust location
		if (_choice==3){
			if (_canShowRena==1){
				DrawTexture(_renaImage,screenWidth-GetTextureWidth(_renaImage)-5,screenHeight-GetTextureHeight(_renaImage));
			}
		}

		if (currentGameStatus==GAMESTATUS_TITLE){
			GoodDrawText(32,5,"Back",fontSize);
		}else{
			GoodDrawText(32,5,"Resume",fontSize);
		}
		GoodDrawText(32,5+currentTextHeight,"===",fontSize);
		GoodDrawText(32,5+currentTextHeight*2,"Auto Mode Speed: ",fontSize);
			GoodDrawText(32+_tempStrWidth,5+currentTextHeight*2,_tempAutoModeString,fontSize);
		
		GoodDrawText(32,5+currentTextHeight*3,"Bust location: ",fontSize);
			if (graphicsLocation == LOCATION_CGALT){
				GoodDrawText(32+_bustlocationcollinspacewidth,5+currentTextHeight*3,"CGAlt",fontSize);
			}else if (graphicsLocation==LOCATION_CG){
				GoodDrawText(32+_bustlocationcollinspacewidth,5+currentTextHeight*3,"CG",fontSize);
			}

		// Display CPU overclock option
			#if PLATFORM == PLAT_VITA
				if (cpuOverclocked==1){
					GoodDrawTextColored(32,5+currentTextHeight*4,"Overclock CPU",fontSize,0,255,0);
				}else{
					GoodDrawText(32,5+currentTextHeight*4,"Overclock CPU",fontSize);
				}
			#else
				if (cpuOverclocked==1){
					GoodDrawTextColored(32,5+currentTextHeight*4,"Green Nothing",fontSize,0,255,0);
				}else{
					GoodDrawText(32,5+currentTextHeight*4,"Nothing",fontSize);
				}
			#endif
		GoodDrawText(32,5+currentTextHeight*5,"BGM Volume",fontSize);
			GoodDrawText(32+_noobBGMVolumeWidth,5+currentTextHeight*5,_tempItoaHoldBGM,fontSize);
		GoodDrawText(32,5+currentTextHeight*6,"SE Volume",fontSize);
			GoodDrawText(32+_noobBGMVolumeWidth,5+currentTextHeight*6,_tempItoaHoldSE,fontSize);

		GoodDrawText(32,5+currentTextHeight*7,"Font Size",fontSize);
		GoodDrawText(32,5+currentTextHeight*8,"Defaults",fontSize);
		if (currentGameStatus!=GAMESTATUS_TITLE){
			GoodDrawText(32,5+currentTextHeight*9,"Quit",fontSize);
		}
		GoodDrawText(0,5+_choice*currentTextHeight,">",fontSize);
		EndDrawing();
		FpsCapWait();
	}
	SaveSettings();
	if (_canShowRena==1){
		FreeTexture(_renaImage);
	}
	if (currentGameStatus!=GAMESTATUS_TITLE){
		if (_artBefore != graphicsLocation){
			LazyMessage("You changed the character art location.","The next time a character is loaded,","it will load from",locationStrings[graphicsLocation]);
		}
	}
}

void TitleScreen(){
	signed char _choice=0;
	
	signed char _titlePassword=0;

	int _versionStringWidth = TextWidth(fontSize,VERSIONSTRING);

	//SetClearColor(255,255,255,255);
	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();

		// Password right left down up square
			if (WasJustPressed(SCE_CTRL_RIGHT)){
				_titlePassword=1;
			}else if (WasJustPressed(SCE_CTRL_LEFT)){
				_titlePassword = Password(_titlePassword,1);
			}else if (WasJustPressed(SCE_CTRL_DOWN)){
				_titlePassword = Password(_titlePassword,2);
			}else if (WasJustPressed(SCE_CTRL_UP)){
				_titlePassword = Password(_titlePassword,3);
			}else if (WasJustPressed(SCE_CTRL_SQUARE)){
				_titlePassword = Password(_titlePassword,4);
				if (_titlePassword==5){
					if (LazyChoice("Would you like to activate top secret","speedy mode for MyLegGuy's testing?",NULL,NULL)==1){
						capEnabled=0;
						autoModeWait=50;
					}
				}
			}

		if (WasJustPressed(SCE_CTRL_DOWN)){
			_choice++;
			if (_choice>3){
				_choice=0;
			}
		}
		if (WasJustPressed(SCE_CTRL_UP)){
			_choice--;
			if (_choice<0){
				_choice=3;
			}
		}

		if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_choice==0){
				PlayMenuSound();
				if (currentPresetFilename==NULL){
					currentPresetChapter=0;
					ControlsEnd();
					currentGameStatus=GAMESTATUS_PRESETSELECTION;
				}
				break;
			}else if (_choice==1){
				PlayMenuSound();
				if (!directoryExists(SCRIPTFOLDER)){
					ControlsEnd();
					char* _tempChosenFile;
					if (FileSelector(PRESETFOLDER,&_tempChosenFile,(char*)"Select a preset to choose StreamingAssets folder")==2){
						LazyMessage(SCRIPTFOLDER,"does not exist and no files in",PRESETFOLDER,"Do you have any files?");
						continue;
					}else{
						if (_tempChosenFile==NULL){
							continue;
						}
						UpdatePresetStreamingAssetsDir(_tempChosenFile);
						free(_tempChosenFile);
					}
				}
				ControlsEnd();
				char* _tempManualFileSelectionResult;
				FileSelector(SCRIPTFOLDER,&_tempManualFileSelectionResult,(char*)"Select a script");
				if (_tempManualFileSelectionResult!=NULL){
					ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
					currentGameStatus=GAMESTATUS_MAINGAME;
					RunScript(SCRIPTFOLDER,_tempManualFileSelectionResult,0);
					free(_tempManualFileSelectionResult);
					currentGameStatus=GAMESTATUS_TITLE;
					ChangeEasyTouchMode(TOUCHMODE_MENU);
				}
				if (presetsAreInStreamingAssets==0){ // If the presets are not specific to a StreamingAssets folder, that means that the user could be using a different StreamingAssets folder. Reset paths just in case.
					GenerateStreamingAssetsPaths("StreamingAssets");
				}
			}else if (_choice==3){ // Quit button
				currentGameStatus=GAMESTATUS_QUIT;
				break;
			}else if (_choice==2){ // Go to setting menu
				ControlsEnd();
				SettingsMenu();
				ControlsEnd();
				break;
			}else{
				_choice=0;
			}
		}
		

		StartDrawing();

		GoodDrawText(32,5,"Main Menu",fontSize);

		GoodDrawText(32,5+currentTextHeight*(0+2),"Load preset and savefile",fontSize);
		GoodDrawText(32,5+currentTextHeight*(1+2),"Manual mode",fontSize);
		GoodDrawText(32,5+currentTextHeight*(2+2),"Settings",fontSize);
		GoodDrawText(32,5+currentTextHeight*(3+2),"Exit",fontSize);

		GoodDrawTextColored((screenWidth-5)-_versionStringWidth,screenHeight-5-currentTextHeight,VERSIONSTRING,fontSize,VERSIONCOLOR);
		GoodDrawText(5,screenHeight-5-currentTextHeight,SYSTEMSTRING,fontSize);

		GoodDrawText(5,5+currentTextHeight*(_choice+2),">",fontSize);
		EndDrawing();
		ControlsEnd();
		FpsCapWait();
	}
}

void TipMenu(){
	ClearMessageArray();
	if (currentPresetTipUnlockList.theArray[currentPresetChapter]==0){
		LazyMessage("No tips unlocked.",NULL,NULL,NULL);
		currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
		ControlsEnd();
		return;
	}
	// The number for the tip the user has selected. Starts at 1. Subtract 1 if using this for an array
	unsigned char _chosenTip=1;
	char _chosenTipString[4]={48,0,0,0};
	char _chosenTipStringMax[4]={48,0,0,0};
	char _totalSelectedString[256]={0};
	itoa(currentPresetTipUnlockList.theArray[currentPresetChapter],&(_chosenTipStringMax[0]),10);
	memset(_totalSelectedString,'\0',256);
	strcpy(_totalSelectedString,"EasyOutputLine(\"");
	if (tipNamesLoaded==1){
		strcat(_totalSelectedString,currentPresetTipNameList.theArray[0]);
		strcat(_totalSelectedString," (1/");
		strcat(_totalSelectedString,_chosenTipStringMax);
		strcat(_totalSelectedString,")");
	}else{
		strcat(_totalSelectedString,"1/");
		strcat(_totalSelectedString,_chosenTipStringMax);
	}
	strcat(_totalSelectedString,"\");");
	isSkipping=1;
	luaL_dostring(L,_totalSelectedString);
	isSkipping=0;
	int i;
	signed char _choice=0;

	ChangeEasyTouchMode(TOUCHMODE_LEFTRIGHTSELECT);

	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();

		if (WasJustPressed(SCE_CTRL_DOWN)){
			_choice++;
			if (_choice>1){
				_choice=0;
			}
		}
		if (WasJustPressed(SCE_CTRL_UP)){
			_choice--;
			if (_choice<0){
				_choice=1;
			}
		}

		if (WasJustPressed(SCE_CTRL_RIGHT)){
			_chosenTip++;
			if (_chosenTip>currentPresetTipUnlockList.theArray[currentPresetChapter]){
				_chosenTip=1;
			}
			// Update the string that is used to show what tip is selected
			ClearMessageArray();
			memset(_totalSelectedString,'\0',256);
			strcpy(_totalSelectedString,"EasyOutputLine(\"");
			if (tipNamesLoaded==1){
				itoa(_chosenTip,&(_chosenTipString[0]),10);
				strcat(_totalSelectedString,currentPresetTipNameList.theArray[_chosenTip-1]);
				strcat(_totalSelectedString," (");
				strcat(_totalSelectedString,_chosenTipString);
				strcat(_totalSelectedString,"/");
				strcat(_totalSelectedString,_chosenTipStringMax);
				strcat(_totalSelectedString,")");
			}else{
				itoa(_chosenTip,&(_chosenTipString[0]),10);
				strcat(_totalSelectedString,_chosenTipString);
				strcat(_totalSelectedString,"/");
				strcat(_totalSelectedString,_chosenTipStringMax);
			}
			strcat(_totalSelectedString,"\");");
			isSkipping=1;
			luaL_dostring(L,_totalSelectedString);
			isSkipping=0;
		}
		if (WasJustPressed(SCE_CTRL_LEFT) ){
			_chosenTip--;
			if (_chosenTip<1){
				_chosenTip=currentPresetTipUnlockList.theArray[currentPresetChapter];
			}
			// Update the string that is used to show what tip is selected
			ClearMessageArray();
			memset(_totalSelectedString,'\0',256);
			strcpy(_totalSelectedString,"EasyOutputLine(\"");
			if (tipNamesLoaded==1){
				itoa(_chosenTip,&(_chosenTipString[0]),10);
				strcat(_totalSelectedString,currentPresetTipNameList.theArray[_chosenTip-1]);
				strcat(_totalSelectedString," (");
				strcat(_totalSelectedString,_chosenTipString);
				strcat(_totalSelectedString,"/");
				strcat(_totalSelectedString,_chosenTipStringMax);
				strcat(_totalSelectedString,")");
			}else{
				itoa(_chosenTip,&(_chosenTipString[0]),10);
				strcat(_totalSelectedString,_chosenTipString);
				strcat(_totalSelectedString,"/");
				strcat(_totalSelectedString,_chosenTipStringMax);
			}
			strcat(_totalSelectedString,"\");");
			isSkipping=1;
			luaL_dostring(L,_totalSelectedString);
			isSkipping=0;
		}
		if (WasJustPressed(SCE_CTRL_CROSS)){
			ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
			ControlsEnd();
			// This will trick the in between lines functions into thinking that we're in normal script execution mode and not quit
			currentGameStatus=GAMESTATUS_MAINGAME;
			RunScript(SCRIPTFOLDER, currentPresetTipList.theArray[_chosenTip-1],1);
			ControlsEnd();
			ChangeEasyTouchMode(TOUCHMODE_MENU);
			currentGameStatus=GAMESTATUS_TIPMENU;
			break;
		}
		if (WasJustPressed(SCE_CTRL_CIRCLE)){
			ChangeEasyTouchMode(TOUCHMODE_MENU);
			ClearMessageArray();
			currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
			#if PLAYTIPMUSIC == 1
				StopBGM();
			#endif
			break;
		}



		ControlsEnd();
		StartDrawing();
		//GoodDrawText(32,5+currentTextHeight*(0+2),"Tip: ",fontSize);
		//GoodDrawText(32+_tipcollinwidth,5+currentTextHeight*(0+2),_totalSelectedString,fontSize);
		for (i = 0; i < 3; i++){
			//printf("%s\n",currentMessages[i]);
			GoodDrawText(32,currentTextHeight+i*(currentTextHeight),(char*)currentMessages[i],fontSize);
		}

		//GoodDrawText(32,currentTextHeight*5,"View TIP",fontSize);
		//GoodDrawText(32,currentTextHeight*6,"Back",fontSize);

		//GoodDrawText(5,(_choice+5)*currentTextHeight,">",fontSize);

		GoodDrawText(5,screenHeight-5-currentTextHeight*3,"Left and Right - Change TIP",fontSize);
		//GoodDrawText(5,screenHeight-5-currentTextHeight*3,"Up and Down - Select option",fontSize);
		GoodDrawText(5,screenHeight-5-currentTextHeight*2,BACKBUTTONNAME" - Back",fontSize);
		GoodDrawText(5,screenHeight-5-currentTextHeight,SELECTBUTTONNAME" - Select",fontSize);

		EndDrawing();
		FpsCapWait();
	}
}

void ChapterJump(){
	ChangeEasyTouchMode(TOUCHMODE_LEFTRIGHTSELECT);
	//currentGameStatus=3;
	//RunScript
	//currentGameStatus=4;
	int _chapterChoice=0;
	unsigned char _choice=0;
	char _tempNumberString[15];
	ControlsEnd();

	itoa(_chapterChoice,&(_tempNumberString[0]),10);
	strcpy((char*)globalTempConcat,currentPresetFileList.theArray[_chapterChoice]);
	strcat((char*)globalTempConcat," (");
	strcat((char*)globalTempConcat,_tempNumberString);
	strcat((char*)globalTempConcat,")");

	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();
		if (WasJustPressed(SCE_CTRL_RIGHT)){
			if (!IsDown(SCE_CTRL_RTRIGGER)){
				_chapterChoice++;
			}else{
				_chapterChoice+=5;
			}
			if (_chapterChoice>currentPresetChapter){
				_chapterChoice=0;
			}

			itoa(_chapterChoice,&(_tempNumberString[0]),10);
			strcpy((char*)globalTempConcat,currentPresetFileList.theArray[_chapterChoice]);
			strcat((char*)globalTempConcat," (");
			strcat((char*)globalTempConcat,_tempNumberString);
			strcat((char*)globalTempConcat,")");
		}
		if (WasJustPressed(SCE_CTRL_LEFT)){
			if (!IsDown(SCE_CTRL_RTRIGGER)){
				_chapterChoice--;
			}else{
				_chapterChoice-=5;
			}
			if (_chapterChoice<0){
				_chapterChoice=currentPresetChapter;
			}
			itoa(_chapterChoice,&(_tempNumberString[0]),10);
			strcpy((char*)globalTempConcat,currentPresetFileList.theArray[_chapterChoice]);
			strcat((char*)globalTempConcat," (");
			strcat((char*)globalTempConcat,_tempNumberString);
			strcat((char*)globalTempConcat,")");
		}
		if (WasJustPressed(SCE_CTRL_DOWN)){
			_choice++;
			if (_choice>1){
				_choice=0;
			}
		}
		if (WasJustPressed(SCE_CTRL_UP)){
			_choice--;
			if (_choice>=240){
				_choice=1;
			}
		}
		if (WasJustPressed(SCE_CTRL_CROSS)){
			ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
			if (_choice==0){
				ControlsEnd();
				currentGameStatus=GAMESTATUS_MAINGAME;
				RunScript(SCRIPTFOLDER, currentPresetFileList.theArray[_chapterChoice],1);
				ControlsEnd();
				ChangeEasyTouchMode(TOUCHMODE_MENU);
				currentGameStatus=GAMESTATUS_TIPMENU;
				break;
			}
			if (_choice==1){
				break;
			}
		}
		if (WasJustPressed(SCE_CTRL_CIRCLE)){
			ChangeEasyTouchMode(TOUCHMODE_MENU);
			break;
		}
		ControlsEnd();
		StartDrawing();
		
		if (chapterNamesLoaded==0){
			GoodDrawText(32,5+currentTextHeight*(0+2),(const char*)globalTempConcat,fontSize);
		}else{
			GoodDrawText(32,5+currentTextHeight*(0+2),currentPresetFileFriendlyList.theArray[_chapterChoice],fontSize);
		}

		GoodDrawText(32,5+currentTextHeight*(1+2),"Back",fontSize);
		GoodDrawText(5,5+currentTextHeight*(_choice+2),">",fontSize);

		GoodDrawText(5,screenHeight-5-currentTextHeight*4,"Left and Right - Change chapter",fontSize);
		GoodDrawText(5,screenHeight-5-currentTextHeight*3,"R and Left or Right - Change chapter quickly",fontSize);
		GoodDrawText(5,screenHeight-5-currentTextHeight*2,BACKBUTTONNAME" - Back",fontSize);
		GoodDrawText(5,screenHeight-5-currentTextHeight,SELECTBUTTONNAME" - Select",fontSize);
		EndDrawing();
		FpsCapWait();
	}
}

void SaveGameEditor(){
	ChangeEasyTouchMode(TOUCHMODE_LEFTRIGHTSELECT);
	char _endOfChapterString[10];
	itoa(currentPresetChapter,_endOfChapterString,10);
	ControlsEnd();
	while (1){
		FpsCapStart();

		ControlsStart();
		if (WasJustPressed(SCE_CTRL_RIGHT)){
			currentPresetChapter++;
			if (currentPresetChapter>currentPresetFileList.length-1){
				currentPresetChapter=0;
			}
			itoa(currentPresetChapter,_endOfChapterString,10);
		}
		if (WasJustPressed(SCE_CTRL_LEFT)){
			currentPresetChapter--;
			if (currentPresetChapter<0){
				currentPresetChapter=currentPresetFileList.length-1;
			}
			itoa(currentPresetChapter,_endOfChapterString,10);
		}
		if (WasJustPressed(SCE_CTRL_CROSS)){
			ChangeEasyTouchMode(TOUCHMODE_MENU);
			SaveGame();
			ControlsEnd();
			break;
		}
		ControlsEnd();
		StartDrawing();
		if (chapterNamesLoaded==0){
			GoodDrawText(32, currentTextHeight, _endOfChapterString, fontSize);
		}else{
			GoodDrawText(32, currentTextHeight, currentPresetFileFriendlyList.theArray[currentPresetChapter], fontSize);
		}

		
		GoodDrawText(32, screenHeight-currentTextHeight*3, "Welcome to the save file editor!", fontSize);
		GoodDrawText(32, screenHeight-currentTextHeight*2, SELECTBUTTONNAME" - Finish and save", fontSize);
		GoodDrawText(32, screenHeight-currentTextHeight, "Left and Right - Change last completed chapter", fontSize);
		EndDrawing();
		FpsCapWait();
	}
}

void NavigationMenu(){
	ChangeEasyTouchMode(TOUCHMODE_MENU);
	signed char _choice=0;
	int _endofscriptwidth = TextWidth(fontSize,(char*)"End of script: ");
	char _endOfChapterString[10];
	itoa(currentPresetChapter,_endOfChapterString,10);

	char _nextChapterExist=0;
	// Checks if there is another chapter left in the preset file. If so, set the variable accordingly
	if (!(currentPresetChapter+1>=currentPresetFileList.length)){
		_nextChapterExist=1;
	}else{
		// This is the default value. I just put this line of code here so I can remember
		_nextChapterExist=0;
		// Start on the first choice because "next" doesn't exist
		_choice=1;
	}

	unsigned char _codeProgress=0;

	while (currentGameStatus!=GAMESTATUS_QUIT){
		FpsCapStart();
		ControlsStart();

		// Editor secret code
			if (WasJustPressed(SCE_CTRL_UP)){
				_codeProgress = Password(_codeProgress,0);
			}
			if (WasJustPressed(SCE_CTRL_DOWN)){
				_codeProgress = Password(_codeProgress,1);
			}
			//int Password(int val, int _shouldHave){
			if (WasJustPressed(SCE_CTRL_LEFT)){
				_codeProgress = Password(_codeProgress,2);
			}
			if (WasJustPressed(SCE_CTRL_RIGHT)){
				_codeProgress = Password(_codeProgress,3);
				if (_codeProgress==4){
					SaveGameEditor();
					itoa(currentPresetChapter,_endOfChapterString,10);
					_nextChapterExist=1;
					_codeProgress=0;
				}
			}

		if (WasJustPressed(SCE_CTRL_DOWN)){
			_choice++;
			if (_choice>3){
				_choice=0;
				if (_nextChapterExist==0){
					_choice=1;
				}
			}
		}
		if (WasJustPressed(SCE_CTRL_UP)){
			_choice--;
			if (_choice<0){
				_choice=3;
			}
			if (_choice==0 && _nextChapterExist==0){
				_choice=3;
			}
		}
		if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_choice==0){
				printf("Go to next chapter\n");
				if (currentPresetChapter+1==currentPresetFileList.length){
					LazyMessage("There is no next chapter.", NULL, NULL, NULL);
				}else{
					easyTouchControlMode = TOUCHMODE_MAINGAME;
					ChangeEasyTouchMode(TOUCHMODE_MAINGAME);
					currentPresetChapter++;
					SetNextScriptName();
					currentGameStatus=GAMESTATUS_MAINGAME;
					break;
				}
			}else if (_choice==1){
				ChapterJump();
			}else if (_choice==2){
				printf("Viewing tips\n");
				currentGameStatus=GAMESTATUS_TIPMENU;
				ControlsEnd();
				#if PLAYTIPMUSIC == 1
					PlayBGM("lsys14",256);
				#endif

				break;
			}else if (_choice==3){
				printf("Exiting\n");
				currentGameStatus=GAMESTATUS_QUIT;
				break;
			}else{
				printf("INVALID SELECTION\n");
			}
		}
		ControlsEnd();
		StartDrawing();

		GoodDrawText(32,0,"End of script: ",fontSize);
		if (chapterNamesLoaded==0){
			GoodDrawText(_endofscriptwidth+32,0,_endOfChapterString,fontSize);
		}else{
			GoodDrawText(_endofscriptwidth+32,0,currentPresetFileFriendlyList.theArray[currentPresetChapter],fontSize);
		}
		
		

		if (_nextChapterExist==1){
			GoodDrawText(32,5+currentTextHeight*(0+2),"Next",fontSize);
		}
		GoodDrawText(32,5+currentTextHeight*(1+2),"Chapter Jump",fontSize);
		GoodDrawText(32,5+currentTextHeight*(2+2),"View Tips",fontSize);
		GoodDrawText(32,5+currentTextHeight*(3+2),"Exit",fontSize);
		GoodDrawText(5,5+currentTextHeight*(_choice+2),">",fontSize);
		EndDrawing();
		FpsCapWait();
	}
}

void NewGameMenu(){
	char _choice=0;
	ChangeEasyTouchMode(TOUCHMODE_MENU);
	while (1){
		FpsCapStart();

		ControlsStart();
		_choice = MenuControls(_choice,0,1);

		if (WasJustPressed(SCE_CTRL_CROSS)){
			if (_choice==0){
				break;
			}else{
				currentPresetChapter=0;
				ControlsEnd();
				SaveGameEditor();
				break;
			}
		}
		ControlsEnd();

		StartDrawing();
		GoodDrawText(32,currentTextHeight,"NEW GAME",fontSize);
		GoodDrawText(32,currentTextHeight*3,"Start from beginning",fontSize);
		GoodDrawText(32,currentTextHeight*4,"Savegame Editor",fontSize);
		GoodDrawText(5,currentTextHeight*(_choice+3),">",fontSize);
		EndDrawing();

		FpsCapWait();
	}
}

// =====================================================

// Returns 2 for missing or outdated happy.lua
// Returns 0 otherwise
char init_dohappylua(){
	// Happy.lua contains functions that both Higurashi script files use and my C code
	char _didLoadHappyLua;
	#if PLATFORM == PLAT_WINDOWS
		#if SUBPLATFORM == SUB_ANDROID
			_didLoadHappyLua = SafeLuaDoFile(L,"/sdcard/HIGURASHI/StreamingAssets/happy.lua",0);
		#else
			_didLoadHappyLua = SafeLuaDoFile(L,"./happy.lua",0);
		#endif
	#elif PLATFORM == PLAT_VITA
		_didLoadHappyLua = SafeLuaDoFile(L,"app0:assets/happy.lua",0);
	#endif
	lua_sethook(L, InBetweenLines, LUA_MASKLINE, 5);

	if (_didLoadHappyLua==0){
		#if PLATFORM == PLAT_VITA
			LazyMessage("Happy.lua is missing for some reason.","Redownload the VPK.","If that doesn't fix it,","report the problem to MyLegGuy.");
		#elif PLATFORM == PLAT_WINDOWS
			printf("Falling back on happybackup.lua");
			FixPath("StreamingAssets/happybackup.lua",globalTempConcat,TYPE_DATA);
			_didLoadHappyLua = SafeLuaDoFile(L,(char*)globalTempConcat,0);
			if (_didLoadHappyLua==0){
				LazyMessage("/sdcard/HIGURASHI/StreamingAssets/happy.lua","is missing. Did you convert the script files?",(const char*)globalTempConcat,"is also missing. It's from the script converter too.");
				return 2;
			}
		#endif
	}

	#if SUBPLATFORM == SUB_ANDROID
		if (_didLoadHappyLua==1){
			// This checks the version of happy.lua if the user is on Android
			// Android users need to put happy.lua on their SD card themselves
			// I need to make sure they have a compatible version.
			lua_getglobal(L,"GetHappyLuaVersion");
			lua_pcall(L,0,1,0);
			int _returnedHappyVersion = lua_tonumber(L,-1);
			printf("happy.lua version %d\n", _returnedHappyVersion);
			if (_returnedHappyVersion<MINHAPPYLUAVERSION){
				LazyMessage("Your happy.lua file is outdated. Please get it from","https://github.com/MyLegGuy/HigurashiVitaConverter/releases","and put happy.lua at","/sdcard/HIGURASHI/StreamingAssets/happy.lua");
				char _tempItoa[10];
				char _tempItoa2[10];
				itoa(_returnedHappyVersion,_tempItoa,10);
				itoa(MINHAPPYLUAVERSION,_tempItoa2,10);
				LazyMessage("By the way, you have happy.lua version",_tempItoa,"and you need happy.lua version",_tempItoa2);
				return 2;
			}
			if (_returnedHappyVersion>MAXHAPPYLUAVERSION){
				LazyMessage("Your happy.lua file is from the future!","Some things may not work, I don't know.","If you have problems, update the application.",NULL);
			}
		}
	#endif

	return 0;
}

// Please exit if this function returns 2
// Go ahead as normal if it returns 0
signed char init(){
	printf("====================================================\n===========================================================\n==================================================================\n");
	int i=0;
	#if RENDERER == REND_SDL
		SDL_Init( SDL_INIT_VIDEO );
		#if SUBPLATFORM == SUB_ANDROID
			// For knowing screen resolution and stuff.
			SDL_DisplayMode displayMode;
			if( SDL_GetCurrentDisplayMode( 0, &displayMode ) == 0 ){
				mainWindow = SDL_CreateWindow( "HappyWindo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, displayMode.w, displayMode.h, SDL_WINDOW_SHOWN );
				screenWidth=displayMode.w;
				screenHeight=displayMode.h;
			}else{
				printf("Failed to get display mode....\n");
			}
		#else
			mainWindow = SDL_CreateWindow( "HappyWindo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN );
			showErrorIfNull(mainWindow);
		#endif
		
		if (useVsync==0){
			mainWindowRenderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_ACCELERATED);
		}else{
			mainWindowRenderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_PRESENTVSYNC);
		}
		showErrorIfNull(mainWindowRenderer);
		// Check if this fails?
		IMG_Init( IMG_INIT_PNG );

		SDL_SetRenderDrawBlendMode(mainWindowRenderer,SDL_BLENDMODE_BLEND);
	#endif
	#if RENDERER == REND_VITA2D
		// Init vita2d and set its clear color.
		vita2d_init();

		// Zero a variable that should already be zero.
		memset(&pad, 0, sizeof(pad));

		//fontImageItalics = vita2d_load_font_file("app0:a/LiberationSans-Italic.ttf");
	#endif
	SetClearColor(0,0,0,255);

	// We need this for ReloadFont();
	FixPath("assets/Loading.png",globalTempConcat,TYPE_EMBEDDED);
	loadingImage = LoadPNG((char*)globalTempConcat);

	// Setup DATAFOLDER variable. Defaults to uma0 if it exists and it's unsafe build
	ResetDataDirectory();

	// This will also load the font size file and therefor must come before font loading
	// Will not crash if no settings found
	LoadSettings();

	#if TEXTRENDERER == TEXT_DEBUG
		FixPath("assets/Font.png",globalTempConcat,TYPE_EMBEDDED);
		fontImage=SafeLoadPNG((char*)globalTempConcat);
	#elif TEXTRENDERER == TEXT_FONTCACHE
		// Make sure it's null beforehand so the free function in the reload function does nothing
		fontImage = NULL;
		ReloadFont();
	#elif TEXTRENDERER == TEXT_VITA2D
		if (checkFileExist((char*)"app0:assets/LiberationSans-Regular.ttf")==1){
			fontImage = vita2d_load_font_file((char*)"app0:assets/LiberationSans-Regular.ttf");
		}
	#endif

	// We need this for any message display
	currentTextHeight = TextHeight(fontSize);

	#if PLATFORM == PLAT_WINDOWS
		controlsUpImage = LoadEmbeddedPNG("assets/controlsUpImage.png");
		controlsDownImage = LoadEmbeddedPNG("assets/controlsDownImage.png");
		controlsLeftImage = LoadEmbeddedPNG("assets/controlsLeftImage.png");
		controlsRightImage = LoadEmbeddedPNG("assets/controlsRightImage.png");
		controlsBackImage = LoadEmbeddedPNG("assets/controlsBackImage.png");
		controlsSelectImage = LoadEmbeddedPNG("assets/controlsSelectImage.png");
		controlsMenuImage = LoadEmbeddedPNG("assets/controlsMenuImage.png");
		autoImage = LoadEmbeddedPNG("assets/autoImage.png");
		skipImage = LoadEmbeddedPNG("assets/skipImage.png");
		textlogImage = LoadEmbeddedPNG("assets/textlogImage.png");

		SDL_SetTextureAlphaMod(controlsUpImage, 100);
		SDL_SetTextureAlphaMod(controlsDownImage, 100);
		SDL_SetTextureAlphaMod(controlsLeftImage, 100);
		SDL_SetTextureAlphaMod(controlsRightImage, 100);
		SDL_SetTextureAlphaMod(controlsBackImage, 100);
		SDL_SetTextureAlphaMod(controlsSelectImage, 100);
		SDL_SetTextureAlphaMod(controlsMenuImage, 100);
		SDL_SetTextureAlphaMod(autoImage, 100);
		SDL_SetTextureAlphaMod(skipImage, 100);
		SDL_SetTextureAlphaMod(textlogImage, 100);
	#endif

	// Checks if StreamingAssets and stuff exists.
	// Informs the user if they don't.
	char _tempCheckResult = CheckForUserStuff();
	if ((_tempCheckResult==1 && PLATFORM == PLAT_WINDOWS) || _tempCheckResult==2){
		return 2;
	}

	InitAudio();

	// These will soon be freed
	STREAMINGASSETS = malloc(1);
	PRESETFOLDER = malloc(1);
	SCRIPTFOLDER = malloc(1);
	SAVEFOLDER = malloc(1);
	// Make file paths with default StreamingAssets folder
	GenerateStreamingAssetsPaths("StreamingAssets");
	printf("%s\n",STREAMINGASSETS);
	printf("%s\n",SAVEFOLDER);
	printf("%s\n",PRESETFOLDER);
	printf("%s\n",SCRIPTFOLDER);

	// Load the menu sound effect if it's present
	TryLoadMenuSoundEffect();

	// Needed for any advanced message display
	imageCharImages[IMAGECHARUNKNOWN] = LoadEmbeddedPNG("assets/unknown.png");
	imageCharImages[IMAGECHARNOTE] = LoadEmbeddedPNG("assets/note.png");
	imageCharImages[IMAGECHARSTAR] = LoadEmbeddedPNG("assets/star.png");

	// Zero the image char arrray
	for (i=0;i<MAXIMAGECHAR;i++){
		imageCharType[i]=-1;
	}

	ClearDebugFile();

	// Fill with null char
	ClearMessageArray();

	// Initialize Lua
	L = luaL_newstate();
	luaL_openlibs(L);
	MakeLuaUseful();

	if (init_dohappylua()==2){
		return 2;
	}

	// Make the save files directory
	createDirectory(SAVEFOLDER);

	for (i=0;i<MAXBUSTS;i++){
		ResetBustStruct(&(Busts[i]),0);
	}

	// Let the user change the font size if the font size file wasn't saved
	// We only force this on Android. Not Vita because the size is already perfect.
	FixPath("fontsize.noob",globalTempConcat,TYPE_DATA);
	#if PLATFORM == PLAT_WINDOWS
		if (checkFileExist((const char*)globalTempConcat)==0){
			FontSizeSetup();
		}
	#endif
	return 0;
}

int main(int argc, char *argv[]){
	/* code */
	if (init()==2){
		currentGameStatus = GAMESTATUS_QUIT;
	}

	// Put stupid test stuff here
	while (currentGameStatus!=GAMESTATUS_QUIT){
		switch (currentGameStatus){
			case GAMESTATUS_TITLE:
				TitleScreen();
				break;
			case GAMESTATUS_LOADPRESET:
				// Create the string for the full path of the preset file and load it
				memset(&globalTempConcat,0,sizeof(globalTempConcat));
				strcpy((char*)globalTempConcat,PRESETFOLDER);
				strcat((char*)globalTempConcat,currentPresetFilename);
				LoadPreset((char*)globalTempConcat);
				// Next, we can try to switch the StreamingAssets directory to ux0:data/HIGURASHI/StreamingAssets_FILENAME/ if that directory exists
				UpdatePresetStreamingAssetsDir(currentPresetFilename);
				// This new directory may have the menu sound effect.
				TryLoadMenuSoundEffect();

				// Does not load the savefile, I promise.
				LoadGame();

				// If there is no save game, start a new one at chapter 0
				// Otherwise, go to the navigation menu
				if (currentPresetChapter==-1){
					ControlsEnd();
					NewGameMenu();
					ControlsEnd();
					if (currentPresetChapter==-1){
						ControlsEnd();
						currentPresetChapter=0;
						SetNextScriptName();
						currentGameStatus=GAMESTATUS_MAINGAME;
					}else{
						currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
					}
				}else{
					currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
				}
				
				break;
			case GAMESTATUS_PRESETSELECTION:
				if (FileSelector(PRESETFOLDER,&currentPresetFilename,(char*)"Select a preset")==2){
					ControlsEnd();
					printf("No files were found\n");
					StartDrawing();
					GoodDrawText(32,5,"No presets found.",fontSize);
					GoodDrawText(32,TextHeight(fontSize)+5,(const char*)"If you ran the converter, you should've gotten some.",fontSize);
					GoodDrawText(32,TextHeight(fontSize)*2+10,(const char*)"You can manually put presets in:",fontSize);
					GoodDrawText(32,TextHeight(fontSize)*3+15,(const char*)PRESETFOLDER,fontSize);
					GoodDrawText(32,200,"Press "SELECTBUTTONNAME" to return",fontSize);
					EndDrawing();
					
					while (currentGameStatus!=GAMESTATUS_QUIT){
						FpsCapStart();
						ControlsStart();
						if (WasJustPressed(SCE_CTRL_CROSS)){
							ControlsEnd();
							break;
						}
						ControlsEnd();
						FpsCapWait();
					}
				}
				ControlsEnd();
				if (currentPresetFilename==NULL){
					currentGameStatus=GAMESTATUS_TITLE;
				}else{
					currentGameStatus=GAMESTATUS_LOADPRESET;
				}
				break;
			case GAMESTATUS_MAINGAME:
				; // This blank statement is here to allow me to declare a variable. Variables can not be declared directly after a label.
				char _didWork = LuaThread((char*)nextScriptToLoad);
				if (currentPresetFileList.length!=0){
					if (_didWork==0){ // If the script didn't run, don't advance the game
						currentPresetChapter--; // Go back a script
						if (currentPresetChapter<0 || currentPresetChapter==255){ // o, no, we've gone back too far!
							LazyMessage("So... the first script failed to launch.","You now have the info on why, so go","try and fix it.","Pressing X will close the application.");
							currentGameStatus=GAMESTATUS_QUIT;
						}else{
							currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
						}
					}else{
						SaveGame();
					}
					if (currentGameStatus!=GAMESTATUS_QUIT){
						currentGameStatus=GAMESTATUS_NAVIGATIONMENU;
					}
				}else{
					currentGameStatus=GAMESTATUS_TITLE;
				}
				break;
			case GAMESTATUS_NAVIGATIONMENU:
				// Menu for chapter jump, tip selection, and going to the next chapter
				NavigationMenu();
				break;
			case GAMESTATUS_TIPMENU:
				// Menu for selecting tip to view
				TipMenu();
				break;
		}
	}
	printf("ENDGAME\n");
	//QuitApplication(L);
	return 0;
}
