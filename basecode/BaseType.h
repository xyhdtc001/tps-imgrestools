#ifndef _BASE_TYPE_H_
#define _BASE_TYPE_H_
#ifdef WIN32
#include <intrin.h>
#endif

#include <string.h>

typedef unsigned long DWORD;
typedef unsigned long long EID;
typedef unsigned long OBJID;
typedef unsigned int UINT;

typedef unsigned char uint8;
typedef unsigned char uchar;

typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef uint32 uint;
typedef int int32;
typedef	unsigned long	ulong;
typedef unsigned long long uint64;
typedef long long int64;

typedef float float32;
typedef double float64;

typedef char UTF8;
typedef unsigned short UTF16;
typedef unsigned int UTF32;


//static const uint32 UINT32_MAX = uint32(0xffffffff);

#ifndef NULL
#define NULL 0
#endif

#ifndef WINDOWS
#ifndef __stdcall
#define __stdcall pascal
#endif
#endif

#ifdef WINDOWS
typedef int socklen_t;
#endif

typedef unsigned int CharCode;


static const char SP[] = " ";
static const char EMPTY[] = "";
static const char CRLF[] = "\r\n";
static const char COMMA[] = ",";
static const char SEMICOLON[] = ";";
static const char EQUAL[] = "=";
static const char COLON[] = ":";

static const char CR[] = "\r";
static const char LF[] = "\n";
static const char TCP[] = "TCP";
static const char CRLF2[] = "\r\n\r\n";
static const char CRLF3[] = "\n\r\n";
static const char CRLF4[] = "\r\r\n";
static const char CRLF5[] = "\n\n";
static const char QUOTE[] = "\"";
static const char MC[] = "-";


#ifndef MTU
#define MTU 1500		//以太网标准MTU
#endif

#ifndef WINDOWS
 #ifndef UINT_PTR
 typedef unsigned int UINT_PTR;
 #endif
 #ifndef SOCEKT
 typedef UINT_PTR SOCKET;
 #endif
#endif

//#ifndef max
//#define max(a,b)    (((a) > (b)) ? (a) : (b))
//#endif

#ifndef __max
#define __max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

//#ifndef min
//#define min(a,b)    (((a) < (b)) ? (a) : (b))
//#endif

#ifndef __min
#define __min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef WINDOWS
inline uint32 min(uint32 a, uint32 b)
{
    return __min(a,b);
}
#endif

const EID EID_NONE = 0;

enum SYNC_TYPE
{
	TYPE_SYNC = 0,
	TYPE_ASYNC,
};

enum TRANS_TYPE
{
	TRANS_LOCALFILE = 0,
	TRANS_HTTP,
	TRANS_FTP,
};

enum DATA_TYPE
{
	DATA_TYPE_UNKNOWN = 0,
	DATA_TYPE_INT,
	DATA_TYPE_FLOAT,
	DATA_TYPE_STR,
	DATA_TYPE_DOUBLE,
	DATA_TYPE_LONGSTR,
};


struct DATA_STRU
{
	DATA_STRU()
	{
		type = DATA_TYPE_UNKNOWN;
		memset(strVal, 0, 32);
	}
	DATA_TYPE type;
	union
	{
		char strVal[32];
		int iVal;
		float fVal;
		double dbVal;
		char* pStr;
	};
};


struct CPos2D
{
	float x, y;
};

struct CPos3D
{
	float x, y, z;
};

struct CPos4D
{
	float x;
	float y;
	float z;
	float w;
};


struct CMyPos
{
	int	x, y;
	CMyPos(){x = y = 0;}
	CMyPos(int _x, int _y):x(_x),y(_y){}
};


struct CMy3DPos
{
	CMy3DPos(){x = 0.0f; y = 0.0f; z = 0.0f;};
	CMy3DPos(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	float x, y, z;
};


struct CMy3DDir
{
	float fHorizontal;
	float fVertical;
};

struct CMyRect
{
	int t, l, r, b;
};

struct CMyRectF 
{
	float left;
	float top;
	float right;
	float bottom;
};

struct CMySize
{
	int	iWidth, iHeight;
};


#define _UINT64 UINT64
#define _INT64 INT64

const int _MAX_STRING		= 256;
const OBJID	ID_NONE			= 0;

struct LINE_VERTEX
{
	float		x, y, z;		// 坐标
	float		rhw;
	DWORD		color;
	float		u, v;

	LINE_VERTEX():x(0.0f),y(0.0f),z(0.0f),rhw(1.0f), color(0xffffffff), u(0.0f), v(0.0f){}
};


struct MatrixData{
	float m[4][4];

	MatrixData(){
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
				m[i][j] = 0.0;
		}
	};

	MatrixData(float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44)
	{
		m[0][0] = _11;	m[0][1] = _12;	m[0][2] = _13;	m[0][3] = _14;
		m[1][0] = _21;	m[1][1] = _22;	m[1][2] = _23;	m[1][3] = _24;
		m[2][0] = _31;	m[2][1] = _32;	m[2][2] = _33;	m[2][3] = _34;
		m[3][0] = _41;	m[3][1] = _42;	m[3][2] = _43;	m[3][3] = _44;
	};
};

// 拾取检测结果数据体
struct PROBE_RESULT
{
	short wResultID; //表示探测结果对应的标识符
	int nProbeID; //表示探测到的物体的ProbeID
	CPos3D vHitPos; //表示探测到的点位置

	CPos3D vRayFrom; //表示探测所用的射线
	CPos3D vRayTo; //表示探测所用的射线
	float fProbeRange; //表示射线探测的最远距离
};

struct MouseButton
{
	enum Enum
	{
		Left = 0,
		Right,
		Middle,

		Button0 = 0,
		Button1,
		Button2,
		Button3,
		Button4,
		Button5,
		Button6,
		Button7,

		None = -1
	};

	MouseButton(Enum _value = None) : value(_value) { }
	friend bool operator == (MouseButton const& a, MouseButton const& b) { return a.value == b.value; }
	friend bool operator != (MouseButton const& a, MouseButton const& b) { return a.value != b.value; }
	int toValue() const { return (int)value; }
private:
	Enum value;
};

struct KeyCode
{
	enum Enum
	{
		None          = 0x00,
		Escape          = 0x01,
		One             = 0x02,
		Two             = 0x03,
		Three           = 0x04,
		Four            = 0x05,
		Five            = 0x06,
		Six             = 0x07,
		Seven           = 0x08,
		Eight           = 0x09,
		Nine            = 0x0A,
		Zero            = 0x0B,
		Minus           = 0x0C,    /* - on main keyboard */
		Equals			= 0x0D,
		Backspace		= 0x0E,    /* Backspace */
		Tab				= 0x0F,
		Q               = 0x10,
		W               = 0x11,
		E               = 0x12,
		R               = 0x13,
		T               = 0x14,
		Y               = 0x15,
		U               = 0x16,
		I               = 0x17,
		O               = 0x18,
		P               = 0x19,
		LeftBracket     = 0x1A,
		RightBracket    = 0x1B,
		Return			= 0x1C,    /* Enter on main keyboard */
		LeftControl		= 0x1D,
		A               = 0x1E,
		S               = 0x1F,
		D               = 0x20,
		F               = 0x21,
		G               = 0x22,
		H               = 0x23,
		J               = 0x24,
		K               = 0x25,
		L               = 0x26,
		Semicolon       = 0x27,
		Apostrophe		= 0x28,
		Grave           = 0x29,    /* Accent grave ( ~ ) */
		LeftShift       = 0x2A,
		Backslash       = 0x2B,
		Z               = 0x2C,
		X               = 0x2D,
		C               = 0x2E,
		V               = 0x2F,
		B               = 0x30,
		N               = 0x31,
		M               = 0x32,
		Comma           = 0x33,
		Period          = 0x34,    /* . on main keyboard */
		Slash           = 0x35,    /* '/' on main keyboard */
		RightShift      = 0x36,
		Multiply        = 0x37,    /* * on numeric keypad */
		LeftAlt        = 0x38,    /* Left Alt */
		Space           = 0x39,
		Capital         = 0x3A,
		F1              = 0x3B,
		F2              = 0x3C,
		F3              = 0x3D,
		F4              = 0x3E,
		F5              = 0x3F,
		F6              = 0x40,
		F7              = 0x41,
		F8              = 0x42,
		F9              = 0x43,
		F10             = 0x44,
		NumLock         = 0x45,
		ScrollLock      = 0x46,    /* Scroll Lock */
		Numpad7         = 0x47,
		Numpad8         = 0x48,
		Numpad9         = 0x49,
		Subtract        = 0x4A,    /* - on numeric keypad */
		Numpad4         = 0x4B,
		Numpad5         = 0x4C,
		Numpad6         = 0x4D,
		Add				= 0x4E,    /* + on numeric keypad */
		Numpad1         = 0x4F,
		Numpad2         = 0x50,
		Numpad3         = 0x51,
		Numpad0         = 0x52,
		Decimal			= 0x53,    /* . on numeric keypad */
		OEM_102         = 0x56,    /* < > | on UK/Germany keyboards */
		F11             = 0x57,
		F12             = 0x58,
		F13             = 0x64,    /*                     (NEC PC98) */
		F14             = 0x65,    /*                     (NEC PC98) */
		F15             = 0x66,    /*                     (NEC PC98) */
		Kana            = 0x70,    /* (Japanese keyboard)            */
		ABNT_C1         = 0x73,    /* / ? on Portugese (Brazilian) keyboards */
		Convert         = 0x79,    /* (Japanese keyboard)            */
		NoConvert       = 0x7B,    /* (Japanese keyboard)            */
		Yen             = 0x7D,    /* (Japanese keyboard)            */
		ABNT_C2         = 0x7E,    /* Numpad . on Portugese (Brazilian) keyboards */
		NumpadEquals    = 0x8D,    /* = on numeric keypad (NEC PC98) */
		PrevTrack       = 0x90,    /* Previous Track (KC_CIRCUMFLEX on Japanese keyboard) */
		At              = 0x91,    /*                     (NEC PC98) */
		Colon           = 0x92,    /*                     (NEC PC98) */
		Underline       = 0x93,    /*                     (NEC PC98) */
		Kanji           = 0x94,    /* (Japanese keyboard)            */
		Stop            = 0x95,    /*                     (NEC PC98) */
		AX              = 0x96,    /*                     (Japan AX) */
		Unlabeled       = 0x97,    /*                        (J3100) */
		NextTrack       = 0x99,    /* Next Track */
		NumpadEnter     = 0x9C,    /* Enter on numeric keypad */
		RightControl    = 0x9D,
		Mute            = 0xA0,
		Calculator      = 0xA1,
		PlayPause       = 0xA2,    /* Play / Pause */
		MediaStop       = 0xA4,    /* Media Stop */
		VolumeDown      = 0xAE,    /* Volume - */
		VolumeUp        = 0xB0,    /* Volume + */
		WebHome         = 0xB2,    /* Web home */
		NumpadComma     = 0xB3,    /* , on numeric keypad (NEC PC98) */
		Divide          = 0xB5,    /* / on numeric keypad */
		SysRq           = 0xB7,
		RightAlt        = 0xB8,    /* Right Alt */
		Pause           = 0xC5,
		Home            = 0xC7,    /* Home on arrow keypad */
		ArrowUp         = 0xC8,    /* UpArrow on arrow keypad */
		PageUp          = 0xC9,    /* PgUp on arrow keypad */
		ArrowLeft       = 0xCB,    /* LeftArrow on arrow keypad */
		ArrowRight      = 0xCD,    /* RightArrow on arrow keypad */
		End             = 0xCF,    /* End on arrow keypad */
		ArrowDown       = 0xD0,    /* DownArrow on arrow keypad */
		PageDown		= 0xD1,    /* PgDn on arrow keypad */
		Insert          = 0xD2,    /* Insert on arrow keypad */
		Delete          = 0xD3,    /* Delete on arrow keypad */
		LeftWindows     = 0xDB,    /* Left Windows key */
		RightWindow     = 0xDC,    /* Right Windows key */
		RightWindows    = 0xDC,    /* Right Windows key - Correct spelling :) */
		AppMenu         = 0xDD,    /* AppMenu key */
		Power           = 0xDE,    /* System Power */
		Sleep           = 0xDF,    /* System Sleep */
		Wake			= 0xE3,    /* System Wake */
		WebSearch		= 0xE5,
		WebFavorites	= 0xE6,
		WebRefresh		= 0xE7,
		WebStop			= 0xE8,
		WebForward		= 0xE9,
		WebBack			= 0xEA,
		MyComputer		= 0xEB,
		Mail			= 0xEC,
		MediaSelect		= 0xED
	};

	KeyCode(Enum _value = None) : value(_value) { }
	friend bool operator == (KeyCode const& a, KeyCode const& b) { return a.value == b.value; }
	friend bool operator != (KeyCode const& a, KeyCode const& b) { return a.value != b.value; }
	int toValue() const { return (int)value; }
private:
	Enum value;
};

enum Modifier
{
	Shift = 0x0000001,
	Ctrl  = 0x0000010,
	Alt   = 0x0000100
};

struct Render_Data_Report
{
	UINT uNumVisibleModels;
	UINT uNumVisibleOmniLights;
	UINT uNumVisibleSpotLights;
	UINT uNumDrawedModels;
	UINT uNumDrawedFaces;
	UINT uNumDrawedVertices;
	UINT uNumDrawedLines;
	UINT uNumDrawCalls;
	UINT uNumCombVer_Mesh;
	UINT uNumCombVer_Effect;
	UINT uNumCombVer_Terrain;
	UINT uNumCombVer_Skin;
};

#endif