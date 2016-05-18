#include <windows.h>		// Header File For Windows
#include <math.h>			// Header File For Math Library Routines


#include <string.h>  // ???????????? ???? ??? Windows
#include <time.h>    // ???????????? ???? ??? ??????????? ?????????? ?????/??????(?????)
#include <gl\gl.h>    // ???????????? ???? ??? ?????????? OpenGL32
#include <gl\glu.h>   // ???????????? ???? ??? ?????????? GLu32
//#include <gl\glaux.h> // ???????????? ???? ??? ?????????? GLaux
#include <olectl.h>          // ???????????? ???? ??? ??????????
#include <stdarg.h>   // Заголовочный файл для функций для работы с переменным
//#include <fstream.h>
#include <mmsystem.h>
#include <math.h>

#include <conio.h>
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <deque>

#include "samrrr_bibl.h"
#include "out.h"
#include "mur.h"
#include "STRUCTURES.h"

#include "NeHeGL.h"														// Header File For NeHeGL

#pragma comment( lib, "opengl32.lib" )									// Search For OpenGL32.lib While Linking
#pragma comment( lib, "glu32.lib" )										// Search For GLu32.lib While Linking

using namespace std;

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;
#ifndef CDS_FULLSCREEN													// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4												// Compilers. By Defining It This Way,
#endif																	// We Can Avoid Errors
int 
  razokx =/**/640/*/GetSystemMetrics(SM_CXSCREEN)/**/
, razoky =/**/480/*/GetSystemMetrics(SM_CYSCREEN)/**/;

GL_Window			window;

GLuint  base;      // База списка отображения для фонта

HDC HDc;

GLuint tex_mur;

int menu = 0;


//#define MAX_AST 30
#define WM_TOGGLEFULLSCREEN (WM_USER+1)								

typedef struct													// Create A Structure
{
	GLubyte	*imageData;											// Image Data (Up To 32 Bits)
	GLuint	bpp;												// Image Color Depth In Bits Per Pixel.
	GLuint	width;												// Image Width
	GLuint	height;												// Image Height
	GLuint	texID;												// Texture ID Used To Select A Texture
} TextureImage;													// Structure Name


MU_ALG mu_alg;

GDATA gen;

//TextureImage textureTGA[60];

GL_Window*	g_window;													// Window Structure
Keys*		g_keys;														// Keyboard

struct                    // Создание структуры для информации о таймере
{
	__int64       frequency;                 // Частота таймера
	float         resolution;                // Точность таймера
	unsigned long mm_timer_start;            // Стартовое значение мультимедийного таймера
	unsigned long mm_timer_elapsed;          // Прошедшее время мультимедийного таймера
	bool          performance_timer;         // Использовать высокоточный таймер?
	__int64       performance_timer_start;   // Стартовое значение высокоточного таймера
	__int64       performance_timer_elapsed; // Прошедшее время высокоточного таймера
} mytimer;

typedef struct
{
	float c1, c2, c3, x, y, z;
} ggcam;

#define GUI_LEN_SCROLL 15

struct GBUTTON
{
	int type;//1-button 2-scroll
	int x0, y0, x1, y1;
	char name[100];
	float scroll;
	float max_scroll, min_scroll;
	float on_mouse;
};

struct GMENU
{
	GBUTTON *button;
	int buttons;
};

class GUI
{
	GMENU *menu;
	int menus;
public:
	
};

GUI gameGUI;

ggcam cam;

MOUSE mo;



void circl(float x, float y, float z, float c);
void movveforw(float cam_c, float cam_c1, float cam_c2, float rass, float *cam_x, float *cam_y, float *cam_z);
void movve(float *cam_c, float *cam_c1, float *cam_c2, float x, float y);


GLvoid BuildFont(float heigf)								// Build Our Bitmap Font
{
	HFONT	font;										// Windows Font ID
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(96);								// Storage For 96 Characters

	font = CreateFont(heigf,							// Height Of Font
		0,								// Width Of Font
		0,								// Angle Of Escapement
		0,								// Orientation Angle
		FW_BOLD,						// Font Weight
		FALSE,							// Italic
		FALSE,							// Underline
		FALSE,							// Strikeout
		ANSI_CHARSET,					// Character Set Identifier
		OUT_TT_PRECIS,					// Output Precision
		CLIP_DEFAULT_PRECIS,			// Clipping Precision
		ANTIALIASED_QUALITY,			// Output Quality
		FF_DONTCARE | DEFAULT_PITCH,		// Family And Pitch
		L"Arial");					// Font Name

	oldfont = (HFONT)SelectObject(HDc, font);           // Selects The Font We Want
	wglUseFontBitmaps(HDc, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	SelectObject(HDc, oldfont);							// Selects The Font We Want
	DeleteObject(font);									// Delete The Font
}

GLvoid KillFont(GLvoid)									// Delete The Font List
{
	glDeleteLists(base, 96);							// Delete All 96 Characters
}

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

void TerminateApplication(GL_Window* window)							// Terminate The Application
{
	PostMessage(window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

BOOL ChangeScreenResolution(int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory(&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize = sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth = width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight = height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel = bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

void ReshapeGL(int width, int height)									// Reshape The Window When It's Moved Or Resized
{
	glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity();													// Reset The Projection Matrix
	gluPerspective(90.0f, (GLfloat)(width) / (GLfloat)(height),			// Calculate The Aspect Ratio Of The Window
		0.1f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();													// Reset The Modelview Matrix
	KillFont();
	BuildFont(-height / 40.0f);
	razokx = width;
	razoky = height;
}

int dvavx(int i)
{
	int r, o = 2;
	for (r = 1; r<i; r++)
		o *= 2;
	return o;
}

float min1(float i, float r)
{
	if (i>r)return r;
	return i;
}

float abs1(float i)
{
	if (i<0)return -i;
	return i;
}

float max1(float i, float r)
{
	if (i>r)return i;
	return r;
}

BOOL CreateWindowGL(GL_Window* window)									// This Code Creates Our OpenGL Window
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT windowRect = { 0, 0, window->init.width, window->init.height };	// Define Our Window Coordinates

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution(window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox(HWND_DESKTOP, L"Mode Switch Failed.\nRunning In Windowed Mode.", L"Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			//ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	// Create The OpenGL Window
	window->hWnd = CreateWindowEx(windowExtendedStyle,					// Extended Style
		(LPCWSTR)window->init.application->className,	// Class Name
		(LPCWSTR)window->init.title,					// Window Title
		windowStyle,							// Window Style
		0, 0,								// Window X,Y Position
		windowRect.right - windowRect.left,	// Window Width
		windowRect.bottom - windowRect.top,	// Window Height
		HWND_DESKTOP,						// Desktop Is Window's Parent
		0,									// No Menu
		window->init.application->hInstance, // Pass The Window Instance
		window);

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	window->hDC = GetDC(window->hWnd);									// Grab A Device Context For This Window
	if (window->hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	PixelFormat = ChoosePixelFormat(window->hDC, &pfd);				// Find A Compatible Pixel Format
	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	if (SetPixelFormat(window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	window->hRC = wglCreateContext(window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent(window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext(window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC(window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	ShowWindow(window->hWnd, SW_NORMAL);								// Make The Window Visible
	window->isVisible = TRUE;											// Set isVisible To True

	ReshapeGL(window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory(window->keys, sizeof (Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount();							// Get Tick Count




	HDc = window->hDC;


	return TRUE;														// Window Creating Was A Success

}

BOOL DestroyWindowGL(GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent(window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext(window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context
			}
			ReleaseDC(window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow(window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings(NULL, 0);									// Switch Back To Desktop Resolution
		ShowCursor(TRUE);												// Show The Cursor
	}
	return TRUE;														// Return True
}

bool LoadTGA(TextureImage *texture, char *filename, int fff)				// Loads A TGA File Into Memory
{
	GLubyte		TGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };		// Uncompressed TGA Header
	GLubyte		TGAcompare[12];									// Used To Compare TGA Header
	GLubyte		header[6];										// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;									// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;										// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;											// Temporary Variable
	GLuint		type = GL_RGBA;									// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");							// Open The TGA File

	if (file == NULL ||											// Does File Even Exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||	// Does The Header Match What We Want?
		fread(header, 1, sizeof(header), file) != sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)										// Did The File Even Exist? *Added Jim Strong*
			return FALSE;										// Return False
		else													// Otherwise
		{
			fclose(file);										// If Anything Failed, Close The File
			return FALSE;										// Return False
		}
	}

	texture->width = header[1] * 256 + header[0];				// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];				// Determine The TGA Height	(highbyte*256+lowbyte)

	if (texture->width <= 0 ||									// Is The Width Less Than Or Equal To Zero
		texture->height <= 0 ||									// Is The Height Less Than Or Equal To Zero
		(header[4] != 24 && header[4] != 32))						// Is The TGA 24 or 32 Bit?
	{
		fclose(file);											// If Anything Failed, Close The File
		return FALSE;											// Return False
	}

	texture->bpp = header[4];								// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel = texture->bpp / 8;							// Divide By 8 To Get The Bytes Per Pixel
	imageSize = (texture->width)*(texture->height)*bytesPerPixel;	// Calculate The Memory Required For The TGA Data
	//imageSize	=128*128*128;	
	texture->imageData = (GLubyte *)malloc(imageSize);			// Reserve Memory To Hold The TGA Data

	if (texture->imageData == NULL ||								// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file) != imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if (texture->imageData != NULL)							// Was Image Data Loaded
			free(texture->imageData);							// If So, Release The Image Data

		fclose(file);											// Close The File
		return FALSE;											// Return False
	}

	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)			// Loop Through The Image Data
	{															// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture->imageData[i];								// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];		// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;						// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose(file);												// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);						// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);				// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fff);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fff);	// Linear Filtered

	if (texture[0].bpp == 24)										// Was The TGA 24 Bits
	{
		type = GL_RGB;											// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;												// Texture Building Went Ok, Return True
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD				tickCount;										// Used For The Tick Counter

	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLong(hWnd, GWL_USERDATA));

	switch (uMsg)														// Evaluate Window Message
	{
	case WM_PAINT:													// Window Needs Updating
	{
																		tickCount = GetTickCount();								// Get The Tick Count
																		Update(tickCount - window->lastTickCount);					// Update The Counter
																		window->lastTickCount = tickCount;							// Set Last Count To Current Count
																		Draw();													// Draw Our Scene
																		SwapBuffers(window->hDC);									// Swap Buffers (Double Buffering)
	}
		return 0;														// Return
	case WM_MOUSEMOVE:
		mo.x = LOWORD(lParam);
		mo.y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		mo.l_press = 1;
		break;
	case WM_LBUTTONUP:
		mo.l_press = 0;
		break;
	case WM_RBUTTONDOWN:
		mo.r_press = 1;
		break;
	case WM_RBUTTONUP:
		mo.r_press = 0;
		break;

	case WM_SYSCOMMAND:												// Intercept System Commands
	{
																		switch (wParam)												// Check System Calls
																		{
																		case SC_SCREENSAVE:										// Screensaver Trying To Start?
																		case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
																			return 0;												// Prevent From Happening
																		}
																		break;														// Exit
	}
		return 0;														// Return

	case WM_CREATE:													// Window Creation
	{
																		CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
																		window = (GL_Window*)(creation->lpCreateParams);
																		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(window));



	}
		return 0;														// Return
	case WM_CLOSE:													// Closing The Window
		TerminateApplication(window);								// Terminate The Application
		return 0;														// Return

	case WM_SIZE:													// Size Action Has Taken Place
		switch (wParam)												// Evaluate Size Action
		{
		case SIZE_MINIMIZED:									// Was Window Minimized?
			window->isVisible = FALSE;							// Set isVisible To False
			return 0;												// Return

		case SIZE_MAXIMIZED:									// Was Window Maximized?
			window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return

		case SIZE_RESTORED:										// Was Window Restored?
			window->isVisible = TRUE;							// Set isVisible To True
			ReshapeGL(LOWORD(lParam), HIWORD(lParam));		// Reshape Window - LoWord=Width, HiWord=Height
			return 0;												// Return
		}
		break;															// Break


	case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = TRUE;					// Set The Selected Key (wParam) To True
			return 0;												// Return
		}
		break;															// Break

	case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
		if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
		{
			window->keys->keyDown[wParam] = FALSE;					// Set The Selected Key (wParam) To False
			return 0;												// Return
		}
		break;															// Break

	case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
		g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
		PostMessage(hWnd, WM_QUIT, 0, 0);
		break;															// Break
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass(Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory(&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize = sizeof (WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc = (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance = application->hInstance;				// Set The Instance
	windowClass.hbrBackground = (HBRUSH)COLOR_WINDOW + 1;				// Class Background Brush Color
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName = (LPCWSTR)application->className;				// Sets The Applications Classname
	if (RegisterClassEx(&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox(HWND_DESKTOP, L"RegisterClassEx Failed!", L"Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int i, r, o, j;

	gen.foods = 400;
	gen.food_pixel = 10;
	gen.mur_fooders = 400;
	gen.mur_sco = 100;
	gen.stones = 100;
	gen.stone_size = 10;

	cam.x = 400;
	cam.y = 400;
	cam.z = -400;

	out.set(0, "O - zoom+");
	out.set(1, "P - zoom-");
	out.set(2, "WASD - move");
	out.set(3, "R - speed up");
	out.set(4, "T - speed down");

	Application			application;									// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure

	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

	// Fill Out Window
	ZeroMemory(&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys = &keys;								// Window Key Structure
	window.init.application = &application;							// Window Application

	// Window Title
	window.init.title = "T\0I\0T\0L\0E\0 \0W\0I\0N\0D\0O\0W\0";		// Tutorial Title


	window.init.width = razokx;									// Window Width
	window.init.height = razoky;									// Window Height
	window.init.bitsPerPixel = 32;									// Bits Per Pixel
	window.init.isFullScreen = TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory(&keys, sizeof (Keys));									// Zero keys Structure

	// Ask The User If They Want To Start In FullScreen Mode? (Remove These 4 Lines If You Want To Force Fullscreen)
	if (MessageBox(HWND_DESKTOP, L"Would You Like To Run In Fullscreen Mode?", L"Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass(&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox(HWND_DESKTOP, L"Error Registering Window Class!", L"Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default
	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL(&window) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (Initialize(&window, &keys) == FALSE)				
			{
				// Failure
				TerminateApplication(&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage(&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage(&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage();								// Application Is Minimized Wait For A Message
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If ( (...

			// Application Is Finished
			Deinitialize();											// User Defined DeInitialization

			DestroyWindowGL(&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox(HWND_DESKTOP, L"Error Creating OpenGL Window", L"Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	UnregisterClass((LPCWSTR)application.className, application.hInstance);		// UnRegister Window Class
	return 0;
}																		// End Of WinMain()

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage(window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

void putknopk(float x, float y, float x1, float y1, char* textinkn)
{
	int i, r, o, j, q, w, a, b, c, d;


	glDisable(GL_TEXTURE_2D);
	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(x - 1, 1 - (y), -1);
	glVertex3f(x - 1, 1 - (y1), -1);
	glVertex3f(x1 - 1, 1 - (y1), -1);
	glVertex3f(x1 - 1, 1 - (y), -1);
	glEnd();

	glColor3f(1, 1, 1);
	glBegin(GL_POLYGON);
	glVertex3f(x + 0.01 - 1, 1 - (y + 0.01), -1);
	glVertex3f(x + 0.01 - 1, 1 - (y1 - 0.01), -1);
	glVertex3f(x1 - 0.01 - 1, 1 - (y1 - 0.01), -1);
	glVertex3f(x1 - 0.01 - 1, 1 - (y + 0.01), -1);
	glEnd();

	glColor3f(0, 0, 0);
	glRasterPos3f(x + 0.015 - 1, 1 - (y1 + y) / 2 - 0.014, -1);
	glPrint(textinkn);

}

int BuildTexture(char *szPathName, GLuint &texid)						// Load Image And Convert To A Texture
{
	HDC			hdcTemp;												// The DC To Hold Our Bitmap
	HBITMAP		hbmpTemp;												// Holds The Bitmap Temporarily
	IPicture	*pPicture;												// IPicture Interface
	OLECHAR		wszPath[MAX_PATH + 1];									// Full Path To Picture (WCHAR)
	char		szPath[MAX_PATH + 1];	// Full Path To Picture
	TCHAR		szPathDD[MAX_PATH + 1];
	long		lWidth;													// Width In Logical Units
	long		lHeight;												// Height In Logical Units
	long		lWidthPixels;											// Width In Pixels
	long		lHeightPixels;											// Height In Pixels
	GLint		glMaxTexDim;											// Holds Maximum Texture Size

	if (strstr(szPathName, "http://"))									// If PathName Contains http:// Then...
	{
		strcpy(szPath, szPathName);										// Append The PathName To szPath
	}
	else																// Otherwise... We Are Loading From A File
	{
		//GetCurrentDirectory(MAX_PATH, szPathDD);							// Get Our Working Directory
		CharToOem(szPathDD, szPath);
		strcat(szPath, "\\");											// Append "\" After The Working Directory
		strcat(szPath, szPathName);										// Append The PathName
	}

	MultiByteToWideChar(CP_ACP, 0, szPath, -1, wszPath, MAX_PATH);		// Convert From ASCII To Unicode
	HRESULT hr = OleLoadPicturePath(wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);

	if (FAILED(hr))														// If Loading Failed
		return FALSE;													// Return False

	hdcTemp = CreateCompatibleDC(GetDC(0));								// Create The Windows Compatible Device Context
	if (!hdcTemp)														// Did Creation Fail?
	{
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);					// Get Maximum Texture Size Supported

	pPicture->get_Width(&lWidth);										// Get IPicture Width (Convert To Pixels)
	lWidthPixels = MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);										// Get IPicture Height (Convert To Pixels)
	lHeightPixels = MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	// Resize Image To Closest Power Of Two
	if (lWidthPixels <= glMaxTexDim) // Is Image Width Less Than Or Equal To Cards Limit
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels) / log(2.0f)) + 0.5f);
	else  // Otherwise  Set Width To "Max Power Of Two" That The Card Can Handle
		lWidthPixels = glMaxTexDim;

	if (lHeightPixels <= glMaxTexDim) // Is Image Height Greater Than Cards Limit
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels) / log(2.0f)) + 0.5f);
	else  // Otherwise  Set Height To "Max Power Of Two" That The Card Can Handle
		lHeightPixels = glMaxTexDim;

	//	Create A Temporary Bitmap
	BITMAPINFO	bi = { 0 };												// The Type Of Bitmap We Request
	DWORD		*pBits = 0;												// Pointer To The Bitmap Bits

	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);				// Set Structure Size
	bi.bmiHeader.biBitCount = 32;									// 32 Bit
	bi.bmiHeader.biWidth = lWidthPixels;							// Power Of Two Width
	bi.bmiHeader.biHeight = lHeightPixels;						// Make Image Top Up (Positive Y-Axis)
	bi.bmiHeader.biCompression = BI_RGB;								// RGB Encoding
	bi.bmiHeader.biPlanes = 1;									// 1 Bitplane

	//	Creating A Bitmap This Way Allows Us To Specify Color Depth And Gives Us Imediate Access To The Bits
	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);

	if (!hbmpTemp)														// Did Creation Fail?
	{
		DeleteDC(hdcTemp);												// Delete The Device Context
		pPicture->Release();											// Decrements IPicture Reference Count
		return FALSE;													// Return False (Failure)
	}

	SelectObject(hdcTemp, hbmpTemp);									// Select Handle To Our Temp DC And Our Temp Bitmap Object

	// Render The IPicture On To The Bitmap
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	// Convert From BGR To RGB Format And Add An Alpha Value Of 255
	for (long i = 0; i < lWidthPixels * lHeightPixels; i++)				// Loop Through All Of The Pixels
	{
		BYTE* pPixel = (BYTE*)(&pBits[i]);							// Grab The Current Pixel
		BYTE  temp = pPixel[0];									// Store 1st Color In Temp Variable (Blue)
		pPixel[0] = pPixel[2];									// Move Red Value To Correct Position (1st)
		pPixel[2] = temp;											// Move Temp Value To Correct Blue Position (3rd)
		pPixel[3] = 255;											// Set The Alpha Value To 255
	}

	glGenTextures(1, &texid);											// Create The Texture

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);		// (Modify This For The Type Of Filtering You Want)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);     // (Modify This For The Type Of Filtering You Want)

	// (Modify This If You Want Mipmaps)
	glTexImage2D(GL_TEXTURE_2D, 0, 3, lWidthPixels, lHeightPixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBits);

	DeleteObject(hbmpTemp);												// Delete The Object
	DeleteDC(hdcTemp);													// Delete The Device Context

	pPicture->Release();												// Decrements IPicture Reference Count
	//}else{return FALSE;};
	return TRUE;														// Return True (All Good)
}

BOOL Initialize(GL_Window* window, Keys* keys)							// Any GL Init Code & User Initialiazation Goes Here
{
	char s[100];
	char destination[65];

	int i, r, o, j, q, w;
	g_window = window;												// Window Values
	g_keys = keys;													// Key Values
	
	//TimerInit();

	glGenTextures(1, &tex_mur);

	/*
	LoadTGA(&blockinfo[4].textureTGA, "Data/bmp/block4icon.tga", GL_LINEAR);
	*/

	glEnable(GL_TEXTURE_2D);											// Enable Texture Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);								// Black Background
	glClearDepth(1.0f);												// Depth Buffer Setup
	glDepthFunc(GL_LEQUAL);											// The Type Of Depth Testing
	glEnable(GL_DEPTH_TEST);											// Enable Depth Testing
	glShadeModel(GL_SMOOTH);											// Select Smooth Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);					// Set Perspective Calculations To Most Accurate


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0.99f);
	glDisable(GL_ALPHA_TEST);

	BuildFont(-razoky / 40.0f);


	return TRUE;
}

void Deinitialize(void)
{
}

void Update(DWORD milliseconds)
{
	static bool keys[256];
	static bool old_keys[256];

	int i, r, o, j, q, w, a, b, c, d;

	for (i = 0; i < 256; i++)
	{
		old_keys[i] = keys[i];
		keys[i] = g_keys->keyDown[i];
	}
	if (1==2)
	{
		TerminateApplication(g_window);
	}
	/*
	if (g_keys->keyDown[VK_ESCAPE])
	{
		if (gamemen == 1)
		{
			baseship = ship[myshipid];
		}
		gamemen = 3;
		//exitgame=1;
	}
	*/

	if (g_keys->keyDown['M'])
		menu = (menu + 1) % 2;

	if (menu == 0)
	{
		if (g_keys->keyDown['S'])
			cam.y -= cam.z / 10;
		if (g_keys->keyDown['W'])
			cam.y += cam.z / 10;
		if (g_keys->keyDown['A'])
			cam.x += cam.z / 10;
		if (g_keys->keyDown['D'])
			cam.x -= cam.z / 10;
		if (g_keys->keyDown['O'])
			cam.z *= 0.95;
		if (g_keys->keyDown['P'])
			cam.z /= 0.95;
		if (g_keys->keyDown['F'])
			mu_alg.addfood(gen);

		out.set(0, "CONTROLLS HELP");
		out.set(1, "F - add food");
		out.set(2, "O - zoom+");
		out.set(3, "P - zoom-");
		out.set(4, "WASD - move");
		//out.set(5, "R - speed up");
		//out.set(6, "T - speed down");
	}
	if (menu == 1)
	{
		out.set(0, "GENER MENU");
		out.set(1, "stones('Q'+    'W'-):" + i_to_s(gen.stones));
		out.set(2, "stone size('A'+    'S'-):" + i_to_s(gen.stone_size));
		out.set(3, "foods('Z'+    'X'-):" + i_to_s(gen.foods));
		out.set(4, "food per pixel('R'+    'T'-):" + i_to_s(gen.food_pixel));
		out.set(5, "ant scouts('F'+    'G'-):" + i_to_s(gen.mur_sco));
		out.set(6, "ant fooders('V'+    'B'-):" + i_to_s(gen.mur_fooders));
		out.set(7, "GEN - 'K'");

		if (g_keys->keyDown['R'])
			gen.food_pixel += 10;
		if (g_keys->keyDown['T'] && gen.food_pixel > 0)
			gen.food_pixel -= 10;
		if (g_keys->keyDown['F'])
			gen.mur_sco += 10;
		if (g_keys->keyDown['G'] && gen.mur_sco > 0)
			gen.mur_sco -= 10;
		if (g_keys->keyDown['V'])
			gen.mur_fooders += 10;
		if (g_keys->keyDown['B'] && gen.mur_fooders > 0)
			gen.mur_fooders -= 10;


		if (g_keys->keyDown['Q'])
			gen.stones+=10;
		if (g_keys->keyDown['W'] && gen.stones > 0)
			gen.stones-=10;
		if (g_keys->keyDown['A'])
			gen.stone_size++;
		if (g_keys->keyDown['S'] && gen.stone_size > 1)
			gen.stone_size--;
		if (g_keys->keyDown['Z'])
			gen.foods+=10;
		if (g_keys->keyDown['X'] && gen.foods > 1)
			gen.foods-=10;
		if (g_keys->keyDown['K'])
			mu_alg.init(800, 800, gen);
	}

	if (g_keys->keyDown[VK_F1])
	{
		if (!g_createFullScreen)
		{
			window.init.width = GetSystemMetrics(SM_CXSCREEN);
			window.init.height = GetSystemMetrics(SM_CYSCREEN);
		}
		else{
			window.init.width = 150;
			window.init.height = 150;
		}

		ToggleFullscreen(g_window);
	}

}

void circl(float x, float y, float z, float c)
{
	int i, r, o, j, q, w, a, b, d;
	float xx, yy, x1, y1, x2, y2, x3, y3, u, v;
	glDisable(GL_TEXTURE_2D);


	glBegin(GL_POLYGON);

	o = 10;
	for (i = 0; i<o; i++)
	{
		xx = ddx(i * 360 / o, c);
		yy = ddy(i * 360 / o, c);
		glVertex3f(x + xx, y + yy, z);
		//		glVertex3f(x+rand()%10,y+rand()%10,z+rand()%10);
	}

	glEnd();


}

float movec(float c, float cnu, float gr)
{

	int i, r, o, j, q, w;
	float u, v, xx, yy;
	xx = c;

	q = 0;
	if (c >= 360) c -= 360;
	if (c  <   0) c += 360;
	if (cnu >= 360) cnu -= 360;
	if (cnu<   0) cnu += 360;

	if (min1(min1(min1(abs1(cnu - c - 360), abs1(c - cnu - 360)),
		min1(abs1(cnu - c), abs1(c - cnu))),
		min1(abs1(cnu - c + 360), abs1(c - cnu + 360))


		)<abs1(gr))
	{
		q = 1; xx = cnu;
	}

	if (q == 0)
	{
		if (abs(cnu - c)<180)
		{
			if (c - cnu>0){ xx = c - gr; }
			else{ xx = c + gr; }
		}
		else{
			if (c  >180){ xx = c + gr; }
			else{ xx = c - gr; }
		}
	}

	if (xx >= 360) xx -= 360;
	if (xx  <   0) xx += 360;
	return xx;

}



class MTEX{
public:
	char* data;
	int x, y;
	MTEX()
	{
		data = (char*)malloc(1024 * 4 * 1024 * 4 * 3);
	}
}mur_tex;


void Draw()
{
	if (!(mu_alg.IS_init()))
	{
		mu_alg.init(800, 800, gen);	
		cam.x = 400;
		cam.y = 400;
		cam.z = -400;

	}

	int desem, i, r, o, j, q, w, a, b, c, d, tt1, tt2, tt3, tt4;
	float xx, yy, zz, x1, y1, z1, x2, y2, z2, x3, y3, z3, u, v;

	glDisable(GL_TEXTURE_2D);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glClearColor(0.0, 0.0, 0.0, 1);

	/*
	mxz = razokx / 2;
	myz = razoky / 2;
	if (((mx - mxz) != 0) || ((my - myz) != 0))
	{
		movve(&cam.c2, &cam.c1, &cam.c3, -(mx - mxz), (my - myz));
		//SetCursorPos(razokx / 2, razoky / 2);
	}
	*/

	glPushMatrix();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	cam.c3 = 0;
	cam.c2 = 0;
	cam.c1 = 180;
	//glTranslatef(0,0,-5 );  
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glRotatef(cam.c3, 0, 0, 1);
	glRotatef(cam.c1, 1, 0, 0);
	glRotatef(cam.c2, 0, 1, 0);
	glTranslatef(-cam.x, -cam.y, -cam.z);

	mu_alg.upd();




	/**/
	{
		
		int x, y;
		mu_alg.tex_data_full(mur_tex.data,4096,4096);

		//

		int i, r, o, j;

		/**/
		glDeleteTextures(1, &tex_mur);
		glGenTextures(1, &tex_mur);

		glBindTexture(GL_TEXTURE_2D, tex_mur);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4096, 4096, 0, GL_RGB, GL_UNSIGNED_BYTE, mur_tex.data);
		/**/

		/**/
		glColor4f(1, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, tex_mur);
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0 - 0.5, 0, 0);

		glTexCoord2f(0, 1);
		glVertex3f(0 - 0.5, 4096, 0);

		glTexCoord2f(1, 1);
		glVertex3f(2048 - 0.5, 4096, 0);

		glTexCoord2f(1, 0);
		glVertex3f(2048 - 0.5, 0, 0);
		glEnd();
	}
	/**/

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glColor4f(1, 1, 1, 1);
	for (i = 0; i < 10000; i++)
		if (mu_alg.mur[i].t != 0)
		{
			glPushMatrix();
			glTranslatef(mu_alg.mur[i].x, mu_alg.mur[i].y, 0);
			glRotatef(mu_alg.mur[i].c+90, 0, 0, 1);

			if (strcmp(mu_alg.mur[i].role, "base") == 0)
				glColor4f(1, 1, 1, 1);
			if (strcmp(mu_alg.mur[i].role, "food") == 0)
				glColor4f(1, 1, 0, 1);
			if (strcmp(mu_alg.mur[i].role, "home") == 0)
				glColor4f(1, 0, 1, 1);

			circl((0       ),
				  (0       ), 0, 0.05 );
			circl((0 - 0.09),
				  (0       ), 0, 0.05 );
			circl((0 + 0.14),
				(0), 0, 0.1);
			glPopMatrix();
		}

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glColor4f(1, 1, 1, 1);
	for(i = 0; i < 40; i++)
	{
		glRasterPos3f(-razokx / (float)razoky, 0.95 - 0.05 * i, -1);
		glPrint(out.s[i].data());
	}

}


void getvec3d(float cam_c, float cam_c1, float cam_c2, float *x, float *y, float *z)
{
	float u, v;

	Vec raycam;
	Vec raycamleft;
	Vec raycamup;

	raycam.x = 1;
	raycam.y = 0;
	raycam.z = 0;

	raycamleft.x = 0;
	raycamleft.y = 1;
	raycamleft.z = 0;

	raycamup.x = 0;
	raycamup.y = 0;
	raycamup.z = -1;

	roate(&raycam.y, &raycam.z, 0, 0, cam_c2);
	roate(&raycamleft.y, &raycamleft.z, 0, 0, cam_c2);
	roate(&raycamup.y, &raycamup.z, 0, 0, cam_c2);

	roate(&raycam.x, &raycam.z, 0, 0, cam_c1);
	roate(&raycamleft.x, &raycamleft.z, 0, 0, cam_c1);
	roate(&raycamup.x, &raycamup.z, 0, 0, cam_c1);

	roate(&raycam.x, &raycam.y, 0, 0, cam_c);
	roate(&raycamleft.x, &raycamleft.y, 0, 0, cam_c);
	roate(&raycamup.x, &raycamup.y, 0, 0, cam_c);

	z[0] = -raycam.x;
	x[0] = raycam.y;
	y[0] = -raycam.z;

}

void movve(float *cam_c, float *cam_c1, float *cam_c2, float x, float y)
{
	float u, v;

	Vec raycam;
	Vec raycamleft;
	Vec raycamup;

	raycam.x = 1;
	raycam.y = 0;
	raycam.z = 0;

	raycamleft.x = 0;
	raycamleft.y = 1;
	raycamleft.z = 0;

	raycamup.x = 0;
	raycamup.y = 0;
	raycamup.z = -1;

	roate(&raycam.x, &raycam.z, 0, 0, y);
	roate(&raycamleft.x, &raycamleft.z, 0, 0, y);
	roate(&raycamup.x, &raycamup.z, 0, 0, y);

	roate(&raycam.x, &raycam.y, 0, 0, -x);
	roate(&raycamleft.x, &raycamleft.y, 0, 0, -x);
	roate(&raycamup.x, &raycamup.y, 0, 0, -x);
	//sdelal samrrr@mail.ru
	roate(&raycam.y, &raycam.z, 0, 0, cam_c2[0]);
	roate(&raycamleft.y, &raycamleft.z, 0, 0, cam_c2[0]);
	roate(&raycamup.y, &raycamup.z, 0, 0, cam_c2[0]);

	roate(&raycam.x, &raycam.z, 0, 0, cam_c1[0]);
	roate(&raycamleft.x, &raycamleft.z, 0, 0, cam_c1[0]);
	roate(&raycamup.x, &raycamup.z, 0, 0, cam_c1[0]);

	roate(&raycam.x, &raycam.y, 0, 0, cam_c[0]);
	roate(&raycamleft.x, &raycamleft.y, 0, 0, cam_c[0]);
	roate(&raycamup.x, &raycamup.y, 0, 0, cam_c[0]);

	//	camv.x=raycam.y;
	//	camv.y=-raycam.z;
	//	camv.z=-raycam.x;

	/*
	//xyz
	//xzy
	//yxz
	//yzx
	//zxy
	//zyx
	*/

	u = ss(0, 0, raycam.y, raycam.x);
	u = -u;
	cam_c[0] = u;

	roate(&raycam.x, &raycam.y, 0, 0, -u);
	roate(&raycamleft.x, &raycamleft.y, 0, 0, -u);
	roate(&raycamup.x, &raycamup.y, 0, 0, -u);

	u = ss(0, 0, raycam.x, raycam.z);
	u = u + 90 + 180;
	cam_c1[0] = u;

	roate(&raycam.x, &raycam.z, 0, 0, -u);
	roate(&raycamleft.x, &raycamleft.z, 0, 0, -u);
	roate(&raycamup.x, &raycamup.z, 0, 0, -u);

	u = ss(0, 0, raycamup.y, raycamup.z);

	cam_c2[0] = u;

	//roate(&raycam    .y,&raycam    .z,0,0,-u);
	//roate(&raycamleft.y,&raycamleft.z,0,0,-u);
	//roate(&raycamup  .y,&raycamup  .z,0,0,-u);


}

void movveforw(float cam_c, float cam_c1, float cam_c2, float rass, float *cam_x, float *cam_y, float *cam_z)
{
	float u, v;

	Vec raycam;
	Vec raycamleft;
	Vec raycamup;

	raycam.x = 1;
	raycam.y = 0;
	raycam.z = 0;

	raycamleft.x = 0;
	raycamleft.y = 1;
	raycamleft.z = 0;

	raycamup.x = 0;
	raycamup.y = 0;
	raycamup.z = -1;

	roate(&raycam.y, &raycam.z, 0, 0, cam_c2);
	roate(&raycamleft.y, &raycamleft.z, 0, 0, cam_c2);
	roate(&raycamup.y, &raycamup.z, 0, 0, cam_c2);

	roate(&raycam.x, &raycam.z, 0, 0, cam_c1);
	roate(&raycamleft.x, &raycamleft.z, 0, 0, cam_c1);
	roate(&raycamup.x, &raycamup.z, 0, 0, cam_c1);

	roate(&raycam.x, &raycam.y, 0, 0, cam_c);
	roate(&raycamleft.x, &raycamleft.y, 0, 0, cam_c);
	roate(&raycamup.x, &raycamup.y, 0, 0, cam_c);

	cam_z[0] -= raycam.x*rass;
	cam_x[0] += raycam.y*rass;
	cam_y[0] -= raycam.z*rass;



}

