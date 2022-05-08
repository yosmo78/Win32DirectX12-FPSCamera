/*
#ifndef COBJMACROS
#define COBJMACROS
#endif
*/

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/*
#ifndef _NO_CRT_STDIO_INLINE
#define _NO_CRT_STDIO_INLINE
#endif
*/

//windows
#include <windows.h>

#if !_M_X64
#error 64-BIT platform required!
#endif

//direct x
#include <d3d12.h>
#include <dxgi1_6.h> //#include <dxgi1_4.h> what is difference? https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi?redirectedfrom=MSDN
//https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-intro

//3 ways, runtime compile shaders, precompile into a cso or prcompile into a header
//time in release which is faster (cso or header?)
//runtime compile like:
/*
	ID3DBlob* errorBuff; //a direct 3D pointer pointing to a null terminated string containing an error msg
	ID3DBlob* pixelShaderBlob;
	if(FAILED(D3DCompileFromFile(L"PixelShader.hlsl", NULL, NULL, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelShaderBlob, &errorBuff)))
    {
    	MessageBoxA(NULL, (const char*)errorBuff->GetBufferPointer(), "Error", MB_OK | MB_ICONERROR);
		return -1;
    }
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	pixelShaderBytecode.BytecodeLength = pixelShaderBlob->GetBufferSize();

	//alternatively you can load file into a string, then call D3DCompile/D3DCompile2
*/

//for CSO files use D3DReadFileToBlob like:
/*
 	ID3DBlob *vertexShaderBlob;
    if(FAILED(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob)))
    {
		return -1;
    }
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	vertexShaderBytecode.BytecodeLength = vertexShaderBlob->GetBufferSize();
*/

//for header file
/*
#include "vertShader.h"
D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
vertexShaderBytecode.pShaderBytecode = VertexShader; //whatever the byte array is named
vertexShaderBytecode.BytecodeLength = sizeof(VertexShader);
*/

#if MAIN_DEBUG
#include <dxgidebug.h>
#	if RUNTIME_DEBUG_COMPILE
#	include <D3Dcompiler.h> 
#	else
#		if !COMPILED_DEBUG_CSO
#		include "vertShaderDebug.h"
#		include "pixelShaderDebug.h"
#		endif
#	endif
#else
#include "vertShader.h"
#include "pixelShader.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#define DEFAULT_SCREEN_WIDTH 1280
#define DEFAULT_SCREEN_HEIGHT 720

#define MINIMUM_SCREEN_WIDTH 300
#define MINIMUM_SCREEN_HEIGHT 300

#define PI_F 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679f
#define PI_D 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32; //floating 32
typedef double   f64; //floating 64


typedef struct Mat3f
{
	union
	{
		f32 m[3][3];
	};
} Mat3f;

typedef struct Mat4f
{
	union
	{
		f32 m[4][4];
	};
} Mat4f;

typedef struct Mat3x4f
{
	union
	{
		f32 m[3][4];
	};
} Mat3x4f;

typedef struct Vec2f
{
	union
	{
		f32 v[2];
		struct
		{
			f32 x;
			f32 y;
		};
	};
} Vec2f;

typedef struct Vec3f
{
	union
	{
		f32 v[3];
		struct
		{
			f32 x;
			f32 y;
			f32 z;
		};
	};
} Vec3f;

typedef struct Vec4f
{
	union
	{
		f32 v[4];
		struct
		{
			f32 x;
			f32 y;
			f32 z;
			f32 w;
		};
	};
} Vec4f;

typedef struct Quatf
{
	union
	{
		f32 q[4];
		struct
		{
			f32 w; //real
			f32 x;
			f32 y;
			f32 z;
		};
		struct
		{
			f32 real; //real;
			Vec3f v;
		};
	};
} Quatf;


typedef struct vertexShaderCB
{
	Mat4f mvpMat;
	Mat3x4f nMat; //there is 3 floats of padding for 16 byte alignment;
} vertexShaderCB;

typedef struct pixelShaderCB
{
	Vec4f vLightColor;
	Vec3f vInvLightDir; //there is 3 floats of padding for 16 byte alignment;
} pixelShaderCB;

//Game state
u8 Running;
u8 isPaused;

//ScreenGraphics State
u8 isFullscreen;
u8 isMinimized;
u8 isUsingVSync;
u8 isScreenTearingSupported;
u8 forceSoftwareRasterize;

//Player Movement
f32 speed = 10.0f;           //movement speed
f32 mouseSensitivity = .05f;  //mouse sensitivity

bool movingForward = false;
bool movingLeft = false;
bool movingRight = false;
bool movingBackwards = false;
bool movingUp = false;
bool movingDown = false;

//Camera
Vec3f position;
f32 rotHor;
f32 rotVert;
const f32 hFov = 60.0f;
const f32 vFov = 60.0f;

//Win32 Screen Variables
HWND MainWindowHandle;
RECT winRect;           //Current Window Rect
RECT nonFullScreenRect; //Rect Not In Full Screen Position (used to restore window to not full screen position when coming out of fullscreen) 
RECT snapPosRect;       //Rect to snap cursor in the middle of
u32 midX;				//Middle of Screen X //TODO change to a 2D vector and wrap with midY
u32 midY;               //Middle of Screen Y
const char *WindowName = "FPS Camera Basic";

u32 screen_width = DEFAULT_SCREEN_WIDTH;
u32 screen_height = DEFAULT_SCREEN_HEIGHT;

const u32 NUM_FRAMES = 3;
u32 currentFrameIdx;

ID3D12Device2* device;
ID3D12CommandQueue* commandQueue;
ID3D12CommandAllocator* commandAllocator[NUM_FRAMES];
ID3D12GraphicsCommandList* commandList;
IDXGISwapChain4* swapChain; //should this be a IDXGISwapChain3 instead?
ID3D12Resource* backBufferRenderTargets[NUM_FRAMES];
u64 rtvDescriptorSize;
ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial

D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.
D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

//GPU/CPU syncronization
ID3D12Fence* fences[NUM_FRAMES]; //can do it with just one fence and currFenceValue
u64 currFenceValue = 0;
u64 fenceValue[NUM_FRAMES];
HANDLE fenceEvent;

//All views
//Remeber views are required so the gpu can see and understand a resourse (only exception is root constants), so if anything is to be used by a shader it must have a view
//(for any I/O needs a view, backbuffers(swapchain and depth and stencil buffer), uniforms, vertex/index inputs, textures)
D3D12_VERTEX_BUFFER_VIEW planeVertexBufferView;
D3D12_INDEX_BUFFER_VIEW planeIndexBufferView;
u32 planeIndexCount;
D3D12_VERTEX_BUFFER_VIEW cubeVertexBufferView;
D3D12_INDEX_BUFFER_VIEW cubeIndexBufferView;
u32 cubeIndexCount;
//the render target view and depth stencil view is stored in a descriptor heap! (views in descriptor heaps are the descriptors)
ID3D12DescriptorHeap* rtvDescriptorHeap;
ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

//pipeline info
ID3D12RootSignature* rootSignature; // root signature defines data shaders will access
ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state

#if MAIN_DEBUG
ID3D12Debug *debugInterface;
ID3D12InfoQueue *pIQueue; 
#endif

vertexShaderCB vertexConstantBuffer;
pixelShaderCB pixelConstantBuffer;

inline
void InitMat3f( Mat3f *a_pMat )
{
	a_pMat->m[0][0] = 1; a_pMat->m[0][1] = 0; a_pMat->m[0][2] = 0;
	a_pMat->m[1][0] = 0; a_pMat->m[1][1] = 1; a_pMat->m[1][2] = 0;
	a_pMat->m[2][0] = 0; a_pMat->m[2][1] = 0; a_pMat->m[2][2] = 1;
}

inline
void InitMat4f( Mat4f *a_pMat )
{
	a_pMat->m[0][0] = 1; a_pMat->m[0][1] = 0; a_pMat->m[0][2] = 0; a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0; a_pMat->m[1][1] = 1; a_pMat->m[1][2] = 0; a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0; a_pMat->m[2][1] = 0; a_pMat->m[2][2] = 1; a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = 0; a_pMat->m[3][1] = 0; a_pMat->m[3][2] = 0; a_pMat->m[3][3] = 1;
}

inline
void InitTransMat4f( Mat4f *a_pMat, f32 x, f32 y, f32 z )
{
	a_pMat->m[0][0] = 1; a_pMat->m[0][1] = 0; a_pMat->m[0][2] = 0; a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0; a_pMat->m[1][1] = 1; a_pMat->m[1][2] = 0; a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0; a_pMat->m[2][1] = 0; a_pMat->m[2][2] = 1; a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = x; a_pMat->m[3][1] = y; a_pMat->m[3][2] = z; a_pMat->m[3][3] = 1;
}

inline
void InitTransMat4f( Mat4f *a_pMat, Vec3f *a_pTrans )
{
	a_pMat->m[0][0] = 1;           a_pMat->m[0][1] = 0;           a_pMat->m[0][2] = 0;           a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0;           a_pMat->m[1][1] = 1;           a_pMat->m[1][2] = 0;           a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0;           a_pMat->m[2][1] = 0;           a_pMat->m[2][2] = 1;           a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = a_pTrans->x; a_pMat->m[3][1] = a_pTrans->y; a_pMat->m[3][2] = a_pTrans->z; a_pMat->m[3][3] = 1;
}

/*
inline
void InitRotXMat4f( Mat4f *a_pMat, f32 angle )
{
	a_pMat->m[0][0] = 1; a_pMat->m[0][1] = 0;                        a_pMat->m[0][2] = 0;                       a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0; a_pMat->m[1][1] = cosf(angle*PI_F/180.0f);  a_pMat->m[1][2] = sinf(angle*PI_F/180.0f); a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0; a_pMat->m[2][1] = -sinf(angle*PI_F/180.0f); a_pMat->m[2][2] = cosf(angle*PI_F/180.0f); a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = 0; a_pMat->m[3][1] = 0;                        a_pMat->m[3][2] = 0;                       a_pMat->m[3][3] = 1;
}

inline
void InitRotYMat4f( Mat4f *a_pMat, f32 angle )
{
	a_pMat->m[0][0] = cosf(angle*PI_F/180.0f);  a_pMat->m[0][1] = 0; a_pMat->m[0][2] = -sinf(angle*PI_F/180.0f); a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0;                        a_pMat->m[1][1] = 1; a_pMat->m[1][2] = 0;                        a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = sinf(angle*PI_F/180.0f);  a_pMat->m[2][1] = 0; a_pMat->m[2][2] = cosf(angle*PI_F/180.0f);  a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = 0;                        a_pMat->m[3][1] = 0; a_pMat->m[3][2] = 0;                        a_pMat->m[3][3] = 1;
}

inline
void InitRotZMat4f( Mat4f *a_pMat, f32 angle )
{
	a_pMat->m[0][0] = cosf(angle*PI_F/180.0f);  a_pMat->m[0][1] = sinf(angle*PI_F/180.0f); a_pMat->m[0][2] = 0; a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = -sinf(angle*PI_F/180.0f); a_pMat->m[1][1] = cosf(angle*PI_F/180.0f); a_pMat->m[1][2] = 0; a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0;                        a_pMat->m[2][1] = 0; 					   a_pMat->m[2][2] = 1; a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = 0;                        a_pMat->m[3][1] = 0;                       a_pMat->m[3][2] = 0; a_pMat->m[3][3] = 1;
}
*/

inline
void InitRotArbAxisMat4f( Mat4f *a_pMat, Vec3f *a_pAxis, f32 angle )
{
	f32 c = cosf(angle*PI_F/180.0f);
	f32 mC = 1.0f-c;
	f32 s = sinf(angle*PI_F/180.0f);
	a_pMat->m[0][0] = c                          + (a_pAxis->x*a_pAxis->x*mC); a_pMat->m[0][1] = (a_pAxis->y*a_pAxis->x*mC) + (a_pAxis->z*s);             a_pMat->m[0][2] = (a_pAxis->z*a_pAxis->x*mC) - (a_pAxis->y*s);             a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = (a_pAxis->x*a_pAxis->y*mC) - (a_pAxis->z*s);             a_pMat->m[1][1] = c                          + (a_pAxis->y*a_pAxis->y*mC); a_pMat->m[1][2] = (a_pAxis->z*a_pAxis->y*mC) + (a_pAxis->x*s);             a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = (a_pAxis->x*a_pAxis->z*mC) + (a_pAxis->y*s);             a_pMat->m[2][1] = (a_pAxis->y*a_pAxis->z*mC) - (a_pAxis->x*s);             a_pMat->m[2][2] = c                          + (a_pAxis->z*a_pAxis->z*mC); a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = 0;                                                       a_pMat->m[3][1] = 0;                                                       a_pMat->m[3][2] = 0;                                                       a_pMat->m[3][3] = 1;
}

//can't use near and far cause of stupid msvc https://stackoverflow.com/questions/3869830/near-and-far-pointers/3869852
//Following is OpenGL, but i need to verify that it is what the OpenGL standard gives!
inline
void InitPerspectiveProjectionMat4fOpenGL( Mat4f *a_pMat, u64 width, u64 height, f32 a_hFOV, f32 a_vFOV, f32 nearPlane, f32 farPlane )
{
	f32 thFOV = tanf(a_hFOV*PI_F/360);
	f32 tvFOV = tanf(a_vFOV*PI_F/360);
	f32 nMinF = (nearPlane-farPlane);
	f32 xmax = nearPlane * thFOV;
	f32 ymax = nearPlane * tvFOV;
  	f32 ymin = -ymax;
  	f32 xmin = -xmax;
  	f32 w = xmax - xmin;
  	f32 h = ymax - ymin;
  	f32 aspect = height / (f32)width;
	a_pMat->m[0][0] = 2.0f*nearPlane*aspect/(w*thFOV); a_pMat->m[0][1] = 0;                        a_pMat->m[0][2] = 0;                               a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0;                               a_pMat->m[1][1] = 2.0f*nearPlane/(h*tvFOV); a_pMat->m[1][2] = 0;                               a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0;                               a_pMat->m[2][1] = 0;                        a_pMat->m[2][2] = (farPlane+nearPlane)/nMinF;      a_pMat->m[2][3] = -1.0f;
	a_pMat->m[3][0] = 0;                               a_pMat->m[3][1] = 0;                        a_pMat->m[3][2] = 2.0f*(farPlane*nearPlane)/nMinF; a_pMat->m[3][3] = 0;
}

//Following are DirectX Matrices
inline
void InitPerspectiveProjectionMat4fDirectXRH( Mat4f *a_pMat, u64 width, u64 height, f32 a_hFOV, f32 a_vFOV, f32 nearPlane, f32 farPlane )
{
	f32 thFOV = tanf(a_hFOV*PI_F/360);
	f32 tvFOV = tanf(a_vFOV*PI_F/360);
	f32 nMinF = farPlane/(nearPlane-farPlane);
  	f32 aspect = height / (f32)width;
	a_pMat->m[0][0] = aspect/(thFOV); a_pMat->m[0][1] = 0;            a_pMat->m[0][2] = 0;               a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0;              a_pMat->m[1][1] = 1.0f/(tvFOV); a_pMat->m[1][2] = 0;               a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0;              a_pMat->m[2][1] = 0;            a_pMat->m[2][2] = nMinF;           a_pMat->m[2][3] = -1.0f;
	a_pMat->m[3][0] = 0;              a_pMat->m[3][1] = 0;            a_pMat->m[3][2] = nearPlane*nMinF; a_pMat->m[3][3] = 0;
}

inline
void InitPerspectiveProjectionMat4fDirectXLH( Mat4f *a_pMat, u64 width, u64 height, f32 a_hFOV, f32 a_vFOV, f32 nearPlane, f32 farPlane )
{
	f32 thFOV = tanf(a_hFOV*PI_F/360);
	f32 tvFOV = tanf(a_vFOV*PI_F/360);
	f32 nMinF = farPlane/(nearPlane-farPlane);
  	f32 aspect = height / (f32)width;
	a_pMat->m[0][0] = aspect/(thFOV); a_pMat->m[0][1] = 0;            a_pMat->m[0][2] = 0;               a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 0;              a_pMat->m[1][1] = 1.0f/(tvFOV); a_pMat->m[1][2] = 0;               a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 0;              a_pMat->m[2][1] = 0;            a_pMat->m[2][2] = -nMinF;          a_pMat->m[2][3] = 1.0f;
	a_pMat->m[3][0] = 0;              a_pMat->m[3][1] = 0;            a_pMat->m[3][2] = nearPlane*nMinF; a_pMat->m[3][3] = 0;
}


inline
f32 DeterminantUpper3x3Mat4f( Mat4f *a_pMat )
{
	return (a_pMat->m[0][0] * ((a_pMat->m[1][1]*a_pMat->m[2][2]) - (a_pMat->m[1][2]*a_pMat->m[2][1]))) + 
		   (a_pMat->m[0][1] * ((a_pMat->m[2][0]*a_pMat->m[1][2]) - (a_pMat->m[1][0]*a_pMat->m[2][2]))) + 
		   (a_pMat->m[0][2] * ((a_pMat->m[1][0]*a_pMat->m[2][1]) - (a_pMat->m[2][0]*a_pMat->m[1][1])));
}

inline
void InverseUpper3x3Mat4f( Mat4f *__restrict a_pMat, Mat4f *__restrict out )
{
	f32 fDet = DeterminantUpper3x3Mat4f( a_pMat );
	assert( fDet != 0.f );
	f32 fInvDet = 1.0f / fDet;
	out->m[0][0] = fInvDet * ((a_pMat->m[1][1]*a_pMat->m[2][2]) - (a_pMat->m[1][2]*a_pMat->m[2][1]));
	out->m[0][1] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[2][1]) - (a_pMat->m[0][1]*a_pMat->m[2][2]));
	out->m[0][2] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[1][2]) - (a_pMat->m[0][2]*a_pMat->m[1][1]));
	out->m[0][3] = 0.0f;

	out->m[1][0] = fInvDet * ((a_pMat->m[2][0]*a_pMat->m[1][2]) - (a_pMat->m[2][2]*a_pMat->m[1][0]));
	out->m[1][1] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[2][2]) - (a_pMat->m[0][2]*a_pMat->m[2][0])); 
	out->m[1][2] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[1][0]) - (a_pMat->m[1][2]*a_pMat->m[0][0]));
	out->m[1][3] = 0.0f;

	out->m[2][0] = fInvDet * ((a_pMat->m[1][0]*a_pMat->m[2][1]) - (a_pMat->m[1][1]*a_pMat->m[2][0]));
	out->m[2][1] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[2][0]) - (a_pMat->m[0][0]*a_pMat->m[2][1]));
	out->m[2][2] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[1][1]) - (a_pMat->m[1][0]*a_pMat->m[0][1]));
	out->m[2][3] = 0.0f;

	out->m[3][0] = 0.0f;
	out->m[3][1] = 0.0f;
	out->m[3][2] = 0.0f;
	out->m[3][3] = 1.0f;
}

inline
void InverseTransposeUpper3x3Mat4f( Mat4f *__restrict a_pMat, Mat4f *__restrict out )
{
	f32 fDet = DeterminantUpper3x3Mat4f( a_pMat );
	assert( fDet != 0.f );
	f32 fInvDet = 1.0f / fDet;
	out->m[0][0] = fInvDet * ((a_pMat->m[1][1]*a_pMat->m[2][2]) - (a_pMat->m[1][2]*a_pMat->m[2][1]));
	out->m[0][1] = fInvDet * ((a_pMat->m[2][0]*a_pMat->m[1][2]) - (a_pMat->m[2][2]*a_pMat->m[1][0]));
	out->m[0][2] = fInvDet * ((a_pMat->m[1][0]*a_pMat->m[2][1]) - (a_pMat->m[1][1]*a_pMat->m[2][0]));
	out->m[0][3] = 0.0f;

	out->m[1][0] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[2][1]) - (a_pMat->m[0][1]*a_pMat->m[2][2]));
	out->m[1][1] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[2][2]) - (a_pMat->m[0][2]*a_pMat->m[2][0])); 
	out->m[1][2] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[2][0]) - (a_pMat->m[0][0]*a_pMat->m[2][1]));
	out->m[1][3] = 0.0f;

	out->m[2][0] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[1][2]) - (a_pMat->m[0][2]*a_pMat->m[1][1]));
	out->m[2][1] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[1][0]) - (a_pMat->m[1][2]*a_pMat->m[0][0]));
	out->m[2][2] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[1][1]) - (a_pMat->m[1][0]*a_pMat->m[0][1]));
	out->m[2][3] = 0.0f;

	out->m[3][0] = 0.0f;
	out->m[3][1] = 0.0f;
	out->m[3][2] = 0.0f;
	out->m[3][3] = 1.0f;
}

inline
void InverseTransposeUpper3x3Mat4f( Mat4f *__restrict a_pMat, Mat3x4f *__restrict out )
{
	f32 fDet = DeterminantUpper3x3Mat4f( a_pMat );
	assert( fDet != 0.f );
	f32 fInvDet = 1.0f / fDet;
	out->m[0][0] = fInvDet * ((a_pMat->m[1][1]*a_pMat->m[2][2]) - (a_pMat->m[1][2]*a_pMat->m[2][1]));
	out->m[0][1] = fInvDet * ((a_pMat->m[2][0]*a_pMat->m[1][2]) - (a_pMat->m[2][2]*a_pMat->m[1][0]));
	out->m[0][2] = fInvDet * ((a_pMat->m[1][0]*a_pMat->m[2][1]) - (a_pMat->m[1][1]*a_pMat->m[2][0]));
	out->m[0][3] = 0.0f;

	out->m[1][0] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[2][1]) - (a_pMat->m[0][1]*a_pMat->m[2][2]));
	out->m[1][1] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[2][2]) - (a_pMat->m[0][2]*a_pMat->m[2][0])); 
	out->m[1][2] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[2][0]) - (a_pMat->m[0][0]*a_pMat->m[2][1]));
	out->m[1][3] = 0.0f;

	out->m[2][0] = fInvDet * ((a_pMat->m[0][1]*a_pMat->m[1][2]) - (a_pMat->m[0][2]*a_pMat->m[1][1]));
	out->m[2][1] = fInvDet * ((a_pMat->m[0][2]*a_pMat->m[1][0]) - (a_pMat->m[1][2]*a_pMat->m[0][0]));
	out->m[2][2] = fInvDet * ((a_pMat->m[0][0]*a_pMat->m[1][1]) - (a_pMat->m[1][0]*a_pMat->m[0][1]));
	out->m[2][3] = 0.0f;
}


inline
void Mat4fMult( Mat4f *__restrict a, Mat4f *__restrict b, Mat4f *__restrict out)
{
	out->m[0][0] = a->m[0][0]*b->m[0][0] + a->m[0][1]*b->m[1][0] + a->m[0][2]*b->m[2][0] + a->m[0][3]*b->m[3][0];
	out->m[0][1] = a->m[0][0]*b->m[0][1] + a->m[0][1]*b->m[1][1] + a->m[0][2]*b->m[2][1] + a->m[0][3]*b->m[3][1];
	out->m[0][2] = a->m[0][0]*b->m[0][2] + a->m[0][1]*b->m[1][2] + a->m[0][2]*b->m[2][2] + a->m[0][3]*b->m[3][2];
	out->m[0][3] = a->m[0][0]*b->m[0][3] + a->m[0][1]*b->m[1][3] + a->m[0][2]*b->m[2][3] + a->m[0][3]*b->m[3][3];

	out->m[1][0] = a->m[1][0]*b->m[0][0] + a->m[1][1]*b->m[1][0] + a->m[1][2]*b->m[2][0] + a->m[1][3]*b->m[3][0];
	out->m[1][1] = a->m[1][0]*b->m[0][1] + a->m[1][1]*b->m[1][1] + a->m[1][2]*b->m[2][1] + a->m[1][3]*b->m[3][1];
	out->m[1][2] = a->m[1][0]*b->m[0][2] + a->m[1][1]*b->m[1][2] + a->m[1][2]*b->m[2][2] + a->m[1][3]*b->m[3][2];
	out->m[1][3] = a->m[1][0]*b->m[0][3] + a->m[1][1]*b->m[1][3] + a->m[1][2]*b->m[2][3] + a->m[1][3]*b->m[3][3];

	out->m[2][0] = a->m[2][0]*b->m[0][0] + a->m[2][1]*b->m[1][0] + a->m[2][2]*b->m[2][0] + a->m[2][3]*b->m[3][0];
	out->m[2][1] = a->m[2][0]*b->m[0][1] + a->m[2][1]*b->m[1][1] + a->m[2][2]*b->m[2][1] + a->m[2][3]*b->m[3][1];
	out->m[2][2] = a->m[2][0]*b->m[0][2] + a->m[2][1]*b->m[1][2] + a->m[2][2]*b->m[2][2] + a->m[2][3]*b->m[3][2];
	out->m[2][3] = a->m[2][0]*b->m[0][3] + a->m[2][1]*b->m[1][3] + a->m[2][2]*b->m[2][3] + a->m[2][3]*b->m[3][3];

	out->m[3][0] = a->m[3][0]*b->m[0][0] + a->m[3][1]*b->m[1][0] + a->m[3][2]*b->m[2][0] + a->m[3][3]*b->m[3][0];
	out->m[3][1] = a->m[3][0]*b->m[0][1] + a->m[3][1]*b->m[1][1] + a->m[3][2]*b->m[2][1] + a->m[3][3]*b->m[3][1];
	out->m[3][2] = a->m[3][0]*b->m[0][2] + a->m[3][1]*b->m[1][2] + a->m[3][2]*b->m[2][2] + a->m[3][3]*b->m[3][2];
	out->m[3][3] = a->m[3][0]*b->m[0][3] + a->m[3][1]*b->m[1][3] + a->m[3][2]*b->m[2][3] + a->m[3][3]*b->m[3][3];
}

inline
void Vec3fAdd( Vec3f *a, Vec3f *b, Vec3f *out )
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}

inline
void Vec3fSub( Vec3f *a, Vec3f *b, Vec3f *out )
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}

inline
void Vec3fMult( Vec3f *a, Vec3f *b, Vec3f *out )
{
	out->x = a->x * b->x;
	out->y = a->y * b->y;
	out->z = a->z * b->z;
}

inline
void Vec3fCross( Vec3f *a, Vec3f *b, Vec3f *out )
{
	out->x = (a->y * b->z) - (a->z * b->y);
	out->y = (a->z * b->x) - (a->x * b->z);
	out->z = (a->x * b->y) - (a->y * b->x);
}

inline
void Vec3fScale( Vec3f *a, f32 scale, Vec3f *out )
{
	out->x = a->x * scale;
	out->y = a->y * scale;
	out->z = a->z * scale;
}

inline
f32 Vec3fDot( Vec3f *a, Vec3f *b )
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

inline
void Vec3fNormalize( Vec3f *a, Vec3f *out )
{

	f32 mag = sqrtf((a->x*a->x) + (a->y*a->y) + (a->z*a->z));
	if(mag == 0)
	{
		out->x = 0;
		out->y = 0;
		out->z = 0;
	}
	else
	{
		out->x = a->x/mag;
		out->y = a->y/mag;
		out->z = a->z/mag;
	}
}

inline
void InitUnitQuatf( Quatf *q, f32 angle, Vec3f *axis )
{
	f32 s = sinf(angle*PI_F/360.0f);
	q->w = cosf(angle*PI_F/360.0f);
	q->x = axis->x * s;
	q->y = axis->y * s;
	q->z = axis->z * s;
}

inline
void QuatfMult( Quatf *__restrict a, Quatf *__restrict b, Quatf *__restrict out )
{
	out->w = (a->w * b->w) - (a->x* b->x) - (a->y* b->y) - (a->z* b->z);
	out->x = (a->w * b->x) + (a->x* b->w) + (a->y* b->z) - (a->z* b->y);
	out->y = (a->w * b->y) + (a->y* b->w) + (a->z* b->x) - (a->x* b->z);
	out->z = (a->w * b->z) + (a->z* b->w) + (a->x* b->y) - (a->y* b->x);
}

//todo simplify to reduce floating point error
inline
void InitViewMat4ByQuatf( Mat4f *a_pMat, f32 horizontalAngle, f32 verticalAngle, Vec3f *a_pPos )
{
	Quatf qHor, qVert;
	Vec3f vertAxis = {cosf(horizontalAngle*PI_F/180.0f),0,-sinf(horizontalAngle*PI_F/180.0f)};
	Vec3f horAxis = {0,1,0};
	InitUnitQuatf( &qVert, verticalAngle, &vertAxis );
	InitUnitQuatf( &qHor, horizontalAngle, &horAxis );

	Quatf qRot;
	QuatfMult( &qVert, &qHor, &qRot);

	a_pMat->m[0][0] = 1.0f - 2.0f*(qRot.y*qRot.y + qRot.z*qRot.z);                                        a_pMat->m[0][1] = 2.0f*(qRot.x*qRot.y - qRot.w*qRot.z);                                               a_pMat->m[0][2] = 2.0f*(qRot.x*qRot.z + qRot.w*qRot.y);        		                                  a_pMat->m[0][3] = 0;
	a_pMat->m[1][0] = 2.0f*(qRot.x*qRot.y + qRot.w*qRot.z);                                               a_pMat->m[1][1] = 1.0f - 2.0f*(qRot.x*qRot.x + qRot.z*qRot.z);                                        a_pMat->m[1][2] = 2.0f*(qRot.y*qRot.z - qRot.w*qRot.x);        		                                  a_pMat->m[1][3] = 0;
	a_pMat->m[2][0] = 2.0f*(qRot.x*qRot.z - qRot.w*qRot.y);                                               a_pMat->m[2][1] = 2.0f*(qRot.y*qRot.z + qRot.w*qRot.x);                                               a_pMat->m[2][2] = 1.0f - 2.0f*(qRot.x*qRot.x + qRot.y*qRot.y); 		                                  a_pMat->m[2][3] = 0;
	a_pMat->m[3][0] = -a_pPos->x*a_pMat->m[0][0] - a_pPos->y*a_pMat->m[1][0] - a_pPos->z*a_pMat->m[2][0]; a_pMat->m[3][1] = -a_pPos->x*a_pMat->m[0][1] - a_pPos->y*a_pMat->m[1][1] - a_pPos->z*a_pMat->m[2][1]; a_pMat->m[3][2] = -a_pPos->x*a_pMat->m[0][2] - a_pPos->y*a_pMat->m[1][2] - a_pPos->z*a_pMat->m[2][2]; a_pMat->m[3][3] = 1;
}

void PrintMat4f( Mat4f *a_pMat )
{
	for( u32 dwIdx = 0; dwIdx < 4; ++dwIdx )
	{
		for( u32 dwJdx = 0; dwJdx < 4; ++dwJdx )
		{
			printf("%f ", a_pMat->m[dwIdx][dwJdx] );
		}
		printf("\n");
	}
}

f32 clamp(f32 d, f32 min, f32 max) {
  const f32 t = d < min ? min : d;
  return t > max ? max : t;
}

u32 max( u32 a, u32 b )
{
	return a > b ? a : b;
}

void CloseProgram()
{
	Running = 0;
}

//TODO CHANGE TO A POPUP AND FILE LOG
int logWindowsError(const char* msg)
{
#if MAIN_DEBUG
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&lpMsgBuf, 0, NULL );
    OutputDebugStringA( msg );
    OutputDebugStringA( (LPCTSTR)lpMsgBuf );

    LocalFree( lpMsgBuf );
    //MessageBoxW(NULL, L"Error creating DXGI factory", L"Error", MB_OK | MB_ICONERROR);
#endif
    return -1;
}

int logError(const char* msg)
{
#if MAIN_DEBUG
	MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONERROR);
#endif
    return -1;
}


inline
void UpdateCurrentFrame()
{
	currentFrameIdx = swapChain->GetCurrentBackBufferIndex();
}

inline
bool SignalCurrentFrame()
{
	++fenceValue[currentFrameIdx];
    if( FAILED( commandQueue->Signal( fences[currentFrameIdx], fenceValue[currentFrameIdx] ) ) )
    {
    	CloseProgram();
    	logError( "Error signalling fence!\n" ); 
    	return false;
    }
    return true;
}

inline
bool WaitCurrentFrame()
{
	if( fences[currentFrameIdx]->GetCompletedValue() < fenceValue[currentFrameIdx] )
    {
    	if( FAILED( fences[currentFrameIdx]->SetEventOnCompletion( fenceValue[currentFrameIdx], fenceEvent ) ) )
		{
			CloseProgram();
			logError( "Failed to set fence event!\n" );
			return false;
		}
		WaitForSingleObject( fenceEvent, INFINITE );
    }
    return true;
}

//append a signal to the end of the command queue and wait for it
inline
bool FlushCommandQueue()
{
	UpdateCurrentFrame();
   	if( !SignalCurrentFrame() )
   	{
   		return false;
   	}
   	if( !WaitCurrentFrame() )
   	{
		return false;
   	}
   	return true;
}


void drawScene( f32 deltaTime )
{
	commandAllocator[currentFrameIdx]->Reset();
	commandList->Reset( commandAllocator[currentFrameIdx], pipelineStateObject );
	//commandList->Reset( commandAllocator[currentFrameIdx], NULL );
	//commandList->SetPipelineState( pipelineStateObject ); //does this always need to be set?
	
	//transition swapchain backbuffer from present state to render state
	D3D12_RESOURCE_BARRIER presentToRenderBarrier;
    presentToRenderBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    presentToRenderBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    presentToRenderBarrier.Transition.pResource = backBufferRenderTargets[currentFrameIdx];
   	presentToRenderBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    presentToRenderBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    presentToRenderBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ResourceBarrier( 1, &presentToRenderBarrier );

    //get render target view
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr = (u64)rtvHandle.ptr + ( rtvDescriptorSize * currentFrameIdx );

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    const float clearColor[] = { 0.5294f, 0.8078f, 0.9216f, 1.0f };
    commandList->ClearRenderTargetView( rtvHandle, clearColor, 0, NULL );
    commandList->ClearDepthStencilView( dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
    
    commandList->SetGraphicsRootSignature( rootSignature ); //does this always need to be set?
	commandList->SetGraphicsRoot32BitConstants( 1, 4 + 3, &pixelConstantBuffer ,0);

    commandList->RSSetViewports( 1, &viewport ); //does this always need to be set?
    commandList->RSSetScissorRects( 1, &scissorRect ); //does this always need to be set?
    
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); 
    

	Mat4f mView;
    InitViewMat4ByQuatf( &mView, rotHor, rotVert, &position );

	Mat4f mProj;
    InitPerspectiveProjectionMat4fDirectXLH( &mProj, screen_width, screen_height, hFov,vFov, 0.2f, 100.0f );

    Mat4f mVP;
    Mat4fMult( &mView, &mProj, &mVP);


	Mat4f mPlaneModel;
	InitMat4f( &mPlaneModel );
    Mat4fMult(&mPlaneModel,&mVP, &vertexConstantBuffer.mvpMat);
    InverseTransposeUpper3x3Mat4f( &mPlaneModel, &vertexConstantBuffer.nMat );

    commandList->SetGraphicsRoot32BitConstants( 0, ( 4 * 4 ) + ( ( ( 4 * 2 ) + 3 ) ), &vertexConstantBuffer ,0);
    commandList->IASetVertexBuffers( 0, 1, &planeVertexBufferView );
    commandList->IASetIndexBuffer( &planeIndexBufferView );
    commandList->DrawIndexedInstanced( planeIndexCount, 1, 0, 0, 0 );

    Mat4f mTrans;
    InitTransMat4f( &mTrans, 0, 0, 5);
    Mat4f mRot;
    Vec3f rotAxis = {0.57735026919f,0.57735026919f,0.57735026919f};
    static f32 cubeRotAngle = 0;
    cubeRotAngle += 50.0f*deltaTime;
    InitRotArbAxisMat4f( &mRot, &rotAxis, cubeRotAngle );
    Mat4f mCubeModel;
    Mat4fMult(&mRot, &mTrans, &mCubeModel);
    Mat4fMult(&mCubeModel,&mVP, &vertexConstantBuffer.mvpMat);
    InverseTransposeUpper3x3Mat4f( &mCubeModel, &vertexConstantBuffer.nMat );

    commandList->SetGraphicsRoot32BitConstants( 0, ( 4 * 4 ) + ( ( ( 4 * 2 ) + 3 ) ), &vertexConstantBuffer ,0);
    commandList->IASetVertexBuffers( 0, 1, &cubeVertexBufferView );
    commandList->IASetIndexBuffer( &cubeIndexBufferView );
    commandList->DrawIndexedInstanced( cubeIndexCount, 1, 0, 0, 0 );

    //transition backbuffer from render state to present state
    D3D12_RESOURCE_BARRIER renderToPresentBarrier;
    renderToPresentBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    renderToPresentBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    renderToPresentBarrier.Transition.pResource = backBufferRenderTargets[currentFrameIdx];
   	renderToPresentBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    renderToPresentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    renderToPresentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList->ResourceBarrier( 1, &renderToPresentBarrier );

    if( FAILED( commandList->Close() ) )
    {
    	CloseProgram();
    	logError( "Error command list failed to close!\n" ); 
    	return;
    }

    ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

	//append signal command to end of command queue
    if( !SignalCurrentFrame() )
    {
    	return;
    }

    //present here
    u32 syncInterval;
    u32 presentFlags;
    if( isFullscreen )
    {
    	//TODO do a better interval and present option for fullscreen
    	syncInterval = 1;
    	presentFlags = 0;
    }
    else
    {
    	syncInterval = isUsingVSync ? 1 : 0;
    	presentFlags = ( isScreenTearingSupported  && !isUsingVSync ) ? DXGI_PRESENT_ALLOW_TEARING  : 0;
	}
	if( FAILED( swapChain->Present( syncInterval, presentFlags ) ) )
    {
    	CloseProgram();
    	logError( "Error presenting swap chain buffer!\n" ); 
    	return;
    }
}

inline
bool CreateDepthStencilBuffer()
{
	/*
	frame time->
	|--- prev ---|--- curr ---|--- next ---|

	we only need 1 depth buffer currently, since the cpu is not mutating it while it is actively rendering (unlike uniforms). 
	and it is reset within the command lists and only actively used within the current frame rendering. (it is synchronous within the command list, this is not always the case, but here it is) (unlike swap chain buffers which cannot be mutated until after the screen is done with them)

	only 1 frame will be using it as a time, since one frame doesn't start rendering until the last one is finished executing its command lists (maybe in future programs, this won't be true)
	
	hence we only need 1 depth buffer here.

	"Since we are only using a single command queue for rendering, then all of the rendering commands are executed serially in the queue so the depth buffer will not be accessed by multiple queues at the same time. The reason why we need multiple color buffers is because we can’t write the next frame until the Windows Driver Display Model (WDDM) is finished presenting the frame on the screen. Stalling the CPU thread until the frame is finished being presented is not efficient so to avoid the stall, we simply render to a different (front) buffer. Since the depth buffer is not used by WDDM, we don’t need to create multiple depth buffers!"

	*/

	D3D12_HEAP_PROPERTIES depthBuffHeapBufferDesc; //describes heap type
	depthBuffHeapBufferDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthBuffHeapBufferDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthBuffHeapBufferDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	depthBuffHeapBufferDesc.CreationNodeMask = 1;
	depthBuffHeapBufferDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depthBufferDesc; //describes what is placed in heap
  	depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  	depthBufferDesc.Alignment = 0;
  	depthBufferDesc.Width = screen_width;
  	depthBufferDesc.Height = screen_height;
  	depthBufferDesc.DepthOrArraySize = 1;
  	depthBufferDesc.MipLevels = 0;
  	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
  	depthBufferDesc.SampleDesc.Count = 1;
  	depthBufferDesc.SampleDesc.Quality = 0;
  	depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  	depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	if( FAILED( device->CreateCommittedResource( &depthBuffHeapBufferDesc, D3D12_HEAP_FLAG_NONE, &depthBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS( &depthStencilBuffer ) ) ) )
	{
		logError( "Failed to allocate depth buffer!\n" );
		CloseProgram();
		return false;
	}
#if !MAIN_DEBUG
	depthStencilBuffer->SetName(L"Depth/Stencil Deault Heap");
#endif

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView( depthStencilBuffer, &depthStencilViewDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
	return true;
}

inline
void CenterCursor( HWND Window )
{
    GetWindowRect( Window, &winRect );
    midX = winRect.left + ( screen_width / 2 );
    midY = winRect.top + ( screen_height / 2 );
    snapPosRect.left = (s32)midX;
    snapPosRect.right = (s32)midX;
    snapPosRect.top = (s32)midY;
    snapPosRect.bottom = (s32)midY;
    if( !isPaused )
    {
        ClipCursor( &snapPosRect );
    }	
}


void Pause()
{
	if( !isPaused )
	{
		isPaused = 1;
		ClipCursor( NULL );
		ShowCursor( TRUE );
	}
}

void TogglePause()
{
    isPaused = isPaused ^ 1;
    if( isPaused )
    {
    	ClipCursor( NULL );
    	ShowCursor( TRUE );
    }
    else
    {
    	ClipCursor( &snapPosRect );
    	ShowCursor( FALSE );
    }
}

void ToggleVSync()
{
    isUsingVSync = isUsingVSync^1;
}

//there appears to be ghosting in fullscreen mode... how to fix that
void Fullscreen( HWND WindowHandle )
{
	isFullscreen = isFullscreen ^ 1;
	if( !FlushCommandQueue() )
   	{
   		CloseProgram();
   		return;
   	}
	swapChain->SetFullscreenState(isFullscreen, NULL);
}

u8 isFullScreenSwapHappening = 0;

LRESULT CALLBACK
Win32MainWindowCallback(
    HWND Window,   //HWND a handle to a window
    UINT Message,  //System defined message, Window will call us and ask us to respond it
    WPARAM WParam, //
    LPARAM LParam) //
{
    LRESULT Result = 0;
    switch (Message)
    {
    // The default window procedure will play a system notification sound 
    // when pressing the Alt+Enter keyboard combination if this message is 
    // not handled.
    case WM_SYSCHAR:
    break;

    //look into this https://stackoverflow.com/questions/63096226/directx-resize-shows-win32-background-at-edges
    //https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Display.cpp#L195
    //TODO fix white space appearing when resizing the window too fast
    case WM_SIZE:
    {
    	//TODO probably do something smart like not render when minimized...
    	u32 temp_screen_width = LOWORD( LParam );
    	u32 temp_screen_height = HIWORD( LParam );
    	if( WParam == SIZE_MINIMIZED || temp_screen_width == 0 || temp_screen_height == 0 )
    	{
    		isMinimized = 1;
    		Pause();
    	}
    	else
    	{
    		isMinimized = 0;
    	}
    	screen_width =  max( 1, temp_screen_width );
    	screen_height = max( 1, temp_screen_height );

    	viewport.Width = (f32)screen_width;
    	viewport.Height = (f32)screen_height;

    	scissorRect.right = screen_width;
    	scissorRect.bottom = screen_height;

    	//allow current frame to generate cmd list (don't execute them yet) but also finish executing all currently executing commands

    	//flush command queue
    	if( swapChain ) //TODO how to get rid of swap chain null check, BUT add a check to only resize buffers if at least one dimension has changed
    	{

			BOOL fullscreenState;
    		swapChain->GetFullscreenState( &fullscreenState, NULL );
    		if( fullscreenState != isFullscreen )
    		{
    			isFullscreen = isFullscreen ^ 1;
    		}


   			if( !FlushCommandQueue() )
   			{
   				CloseProgram();
   				return 0;
   			}

        	for( u32 dwFrame = 0; dwFrame < NUM_FRAMES; ++dwFrame )
        	{
        		backBufferRenderTargets[dwFrame]->Release();
        		fenceValue[dwFrame] = 0;
        	}
        	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        	if( FAILED( swapChain->GetDesc( &swapChainDesc ) ) )
        	{
				CloseProgram();
				logError( "Failed to get swap chain desc!\n" );
				return 0;
        	}
        	//look into ResizeTarget  as well
        	if( FAILED( swapChain->ResizeBuffers(NUM_FRAMES, screen_width, screen_height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags) ) )
        	{
				CloseProgram();
				logError( "Failed to resize swap chain!\n" );
        	}
	
        	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
			//iterate over descriptor heap, storing mem address of each backbuffer in the swap chain into a descriptor
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			for( u32 dwIdx = 0; dwIdx < NUM_FRAMES; ++dwIdx )
			{
				if( FAILED( swapChain->GetBuffer( dwIdx, IID_PPV_ARGS( &backBufferRenderTargets[dwIdx] ) ) ) )
				{
					CloseProgram();
					logError( "Failed to get rtv handle!\n" ); 
					return 0;
				}
				device->CreateRenderTargetView( backBufferRenderTargets[dwIdx], nullptr, rtvHandle );
				rtvHandle.ptr = (u64)rtvHandle.ptr + rtvDescriptorSize;
			}

			depthStencilBuffer->Release();

			//this reallocates DSBuffer overwrites descriptor heap to it
			if( !CreateDepthStencilBuffer() )
			{
				return 0;
			}

			UpdateCurrentFrame();
			if( !isMinimized )
			{
				drawScene( 0.0f ); //TODO calculate time since last draw
			}
			UpdateCurrentFrame();
		}
		
        CenterCursor( Window );
    }break;

    case WM_MOVE:
    {
    	CenterCursor( Window );
    }break;

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)LParam;
        lpMMI->ptMinTrackSize.x = MINIMUM_SCREEN_WIDTH;
        lpMMI->ptMinTrackSize.y = MINIMUM_SCREEN_HEIGHT;
    }break;

    case WM_CLOSE: //when user clicks on the X button on the window
    {
        CloseProgram();
    } break;

    case WM_PAINT: //to allow us to update the window
    {
    	//TODO get current back buffer index and draw scene? or gen a render list
        //drawScene( 0.0f ); //calls present, so current back buffer index will need to be updated and will need to sync next frame using fenceEvent
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint( Window, &Paint ); //will fill out struct with information on where to paint
	//TODO
        EndPaint( Window, &Paint ); //closes paint area
    } break;

    case WM_ACTIVATE:
    {
    	switch(WParam)
    	{
    		//WM_MOUSEACTIVATE 
    		case WA_ACTIVE:
    		case WA_CLICKACTIVE:
    		{
    			//printf("Activate\n");
    		} break;
    		case WA_INACTIVE:
    		{
    			//printf("Inactivate\n");
    			//maybe can handle fullscreen Alt+Tabbing away here of minimizing window... but what about if you have multiple monitors and just ckick in the other monitor...
    			Pause();
    		} break;
    		default:
    		{
    			//error
    		}
    	}
    } break;
    /*
    case WM_SYSCOMMAND:
    {
    	printf("Got a sys command!\n");
    } break;
	*/
    default:
        Result = DefWindowProc( Window, Message, WParam, LParam ); //call windows to handle default behavior of things we don't handle
    }

    return Result;
}

inline
#if MAIN_DEBUG
HWND InitWin32Window()
#else
HWND InitWin32Window( HINSTANCE Instance )
#endif
{
    WNDCLASSEX WindowClass;
    WindowClass.cbSize = sizeof( WNDCLASSEX );
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; //https://devblogs.microsoft.com/oldnewthing/20060601-06/?p=31003
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
#if MAIN_DEBUG
    WindowClass.hInstance = GetModuleHandle( NULL );
#else
    WindowClass.hInstance = Instance;
#endif
    WindowClass.hIcon = LoadIcon( 0, IDI_APPLICATION ); //IDI_APPLICATION: Default application icon, 0 means use a default Icon
    WindowClass.hCursor = LoadCursor( 0, IDC_ARROW ); //IDC_ARROW: Standard arrow, 0 means used a predefined Cursor
    WindowClass.hbrBackground = NULL; 
    WindowClass.lpszMenuName = NULL;	// No menu 
    WindowClass.lpszClassName = "WindowTestClass"; //name our class
    WindowClass.hIconSm = NULL; //can also do default Icon here? will NULL be default automatically?
    if ( !RegisterClassEx( &WindowClass ) )
    {
    	logWindowsError( "Failed to Register Window Class:\n" );
    	return NULL;
    }
    HWND WindowHandle = CreateWindowEx( 0, WindowClass.lpszClassName, WindowName,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,  CW_USEDEFAULT, CW_USEDEFAULT, screen_width, screen_height, //if fullscreen get monitor width and height
        0, 0, WindowClass.hInstance, NULL );

    if ( !WindowHandle )
    {
    	logWindowsError( "Failed to Instantiate Window Class:\n" );  
        return NULL;   
    }

    return WindowHandle;
}

inline
void InitCursorScreenLock( HWND WindowHandle )
{
	CenterCursor( WindowHandle );
    //https://stackoverflow.com/questions/14134076/winapi-hide-cursor-inside-window-client-area
    ShowCursor( FALSE );
}

inline
void InitRawInput( HWND WindowHandle )
{
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = (USHORT) 0x01; 
	Rid[0].usUsage = (USHORT) 0x02; 
	Rid[0].dwFlags = RIDEV_INPUTSINK;   
	Rid[0].hwndTarget = WindowHandle;
	RegisterRawInputDevices( Rid, 1, sizeof( Rid[0] ) );
}

inline
void InitStartingGameState()
{
	Running = 1;
    isPaused = 0;
}

inline
void InitScreenGraphicsState()
{
	isFullscreen = 0;
	isUsingVSync = 1;
	isScreenTearingSupported = 0;
	forceSoftwareRasterize = 0;

	pixelConstantBuffer.vLightColor = {0.83137f,0.62745f,0.09020f,1.0f};
	pixelConstantBuffer.vInvLightDir = {0.57735026919f,0.57735026919f,0.57735026919f};
}

inline
void InitStartingCamera()
{
	position  = { 0, 0, 0 };
	rotHor = 0;
	rotVert = 0;
}


#if MAIN_DEBUG
inline
bool EnableDebugLayer()
{
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    if ( FAILED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) ) )
    {
    	logError( "Error creating DirectX12 Debug layer\n" );  
        return false;
    }
    debugInterface->EnableDebugLayer();
    return true;
}
#endif

inline
ID3D12CommandQueue *InitDirectCommandQueue( ID3D12Device2* dxd3Device )
{
	ID3D12CommandQueue *cq;
	D3D12_COMMAND_QUEUE_DESC cqDesc;
    cqDesc.Type =     D3D12_COMMAND_LIST_TYPE_DIRECT;
    cqDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqDesc.Flags =    D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.NodeMask = 0;

    if( FAILED( dxd3Device->CreateCommandQueue( &cqDesc, IID_PPV_ARGS( &cq ) ) ) )
	{
		return NULL;
	}
	return cq;
}

ID3D12Resource* planeVertexBuffer; //a default committed heap
ID3D12Resource* planeIndexBuffer; //a default committed heap
ID3D12Resource* cubeVertexBuffer; //a default committed heap
ID3D12Resource* cubeIndexBuffer; //a default committed heap

//TODO these can be released after Waiting for command list to be done executing the copy from upload to default heap
ID3D12Resource* planeVertexUploadBuffer; //a tmp upload committed heap
ID3D12Resource* planeIndexUploadBuffer; //a tmp upload committed heap
ID3D12Resource* cubeVertexUploadBuffer; //a tmp upload committed heap
ID3D12Resource* cubeIndexUploadBuffer; //a tmp upload committed heap


inline
void UploadModels()
{
	//can we combine vertices and indices into 1 array, do 1 upload, then just have separate views into the default heap?
	f32 planeVertices[] =
	{
		// positions              // vertex norms    // vertex colors
		 1000.0f,  -1.0f,  1000.0f, 0.0f,  1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		 1000.0f,  -1.0f, -1000.0f, 0.0f,  1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		-1000.0f,  -1.0f,  1000.0f, 0.0f,  1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		-1000.0f,  -1.0f, -1000.0f, 0.0f,  1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		 1000.0f,  -1.0f,  1000.0f, 0.0f, -1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		 1000.0f,  -1.0f, -1000.0f, 0.0f, -1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		-1000.0f,  -1.0f,  1000.0f, 0.0f, -1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f,
		-1000.0f,  -1.0f, -1000.0f, 0.0f, -1.0f, 0.0f, 0.5882f, 0.2941f, 0.0f, 1.0f
	};

	u32 planeIndices[] = 
	{
		0, 1, 2,
		2, 1, 3,
		4, 6, 5,
		6, 7, 5
	};

	planeIndexCount = 12;

	//change to placed resource to combine all buffers? (calculate total memory need, and then streaming in buffers as they load in from disk)
	D3D12_HEAP_PROPERTIES heapBufferDesc; //describes heap type
	heapBufferDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapBufferDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapBufferDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapBufferDesc.CreationNodeMask = 1;
	heapBufferDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceBufferDesc; //describes what is placed in heap
  	resourceBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  	resourceBufferDesc.Alignment = 0;
  	resourceBufferDesc.Width = sizeof(planeVertices);
  	resourceBufferDesc.Height = 1;
  	resourceBufferDesc.DepthOrArraySize = 1;
  	resourceBufferDesc.MipLevels = 1;
  	resourceBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
  	resourceBufferDesc.SampleDesc.Count = 1;
  	resourceBufferDesc.SampleDesc.Quality = 0;
  	resourceBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  	resourceBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  	//can leave optimized clear value to nullptr, cause we are going to immediately upload data to it
	device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&planeVertexBuffer)  );
#if MAIN_DEBUG
	planeVertexBuffer->SetName( L"Plane Vertex Buffer Resource Heap" );
#endif

	heapBufferDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&planeVertexUploadBuffer)  );
#if MAIN_DEBUG
	planeVertexUploadBuffer->SetName( L"Plane Vertex Buffer Resource Upload Heap" );
#endif

	u8* pUploadBufferData;
    if( FAILED( planeVertexUploadBuffer->Map( 0, nullptr, (void**) &pUploadBufferData ) ) )
    {
        return;
    }

    //upload to upload heap
    memcpy(pUploadBufferData,planeVertices,sizeof(planeVertices));

    planeVertexUploadBuffer->Unmap( 0, nullptr );

    commandList->CopyBufferRegion( planeVertexBuffer, 0, planeVertexUploadBuffer, 0, sizeof(planeVertices) );


	D3D12_RESOURCE_BARRIER defaultHeapUploadToReadBarrier;
    defaultHeapUploadToReadBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    defaultHeapUploadToReadBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    defaultHeapUploadToReadBarrier.Transition.pResource = planeVertexBuffer;
   	defaultHeapUploadToReadBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    defaultHeapUploadToReadBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    defaultHeapUploadToReadBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    commandList->ResourceBarrier( 1, &defaultHeapUploadToReadBarrier );

    planeVertexBufferView.BufferLocation = planeVertexBuffer->GetGPUVirtualAddress();
    planeVertexBufferView.StrideInBytes = 3*sizeof(f32) + 3*sizeof(f32) + 4*sizeof(f32); //size of s single vertex
    planeVertexBufferView.SizeInBytes = sizeof(planeVertices);

    heapBufferDesc.Type = D3D12_HEAP_TYPE_DEFAULT;

    resourceBufferDesc.Width = sizeof(planeIndices);

    device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&planeIndexBuffer)  );
#if MAIN_DEBUG
	planeIndexBuffer->SetName( L"Plane Index Buffer Resource Heap" );
#endif

	heapBufferDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&planeIndexUploadBuffer)  );
#if MAIN_DEBUG
	planeIndexUploadBuffer->SetName( L"Plane Index Buffer Resource Upload Heap" );
#endif


    u8* pIndexUploadBufferData;
    if( FAILED( planeIndexUploadBuffer->Map( 0, nullptr, (void**) &pIndexUploadBufferData ) ) )
    {
        return;
    }

    //upload to upload heap
    memcpy(pIndexUploadBufferData,planeIndices,sizeof(planeIndices));

    planeIndexUploadBuffer->Unmap( 0, nullptr );

    commandList->CopyBufferRegion( planeIndexBuffer, 0, planeIndexUploadBuffer, 0, sizeof(planeIndices) );

    defaultHeapUploadToReadBarrier.Transition.pResource = planeIndexBuffer;
    commandList->ResourceBarrier( 1, &defaultHeapUploadToReadBarrier );

	planeIndexBufferView.BufferLocation = planeIndexBuffer->GetGPUVirtualAddress();
    planeIndexBufferView.SizeInBytes = sizeof(planeIndices);
    planeIndexBufferView.Format = DXGI_FORMAT_R32_UINT; 

    f32 cubeVertices[] = {
    	// positions          // vertex norms     //vertex colors
       	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,

       	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,

       	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

       	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

       	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

       	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
       	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f
    };
    u32 cubeIndicies[] =
    {
    	 0,  1,  2, //back
    	 3,  4,  5,

    	 6,  7,  8,
    	 9, 10, 11,

    	12, 13, 14,
    	15, 16, 17,

    	18, 20, 19,
    	21, 23, 22,

    	24, 25, 26, //bottom
    	27, 28, 29,

    	30, 31, 32, //top
    	33, 34, 35
    };

	heapBufferDesc.Type = D3D12_HEAP_TYPE_DEFAULT;

    resourceBufferDesc.Width = sizeof(cubeVertices);

    device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&cubeVertexBuffer)  );
#if MAIN_DEBUG
	cubeVertexBuffer->SetName( L"Cube Vertex Buffer Resource Heap" );
#endif

	heapBufferDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cubeVertexUploadBuffer)  );
#if MAIN_DEBUG
	cubeVertexUploadBuffer->SetName( L"Cube Vertex Buffer Resource Upload Heap" );
#endif


    u8* pVertexUploadBufferData;
    if( FAILED( cubeVertexUploadBuffer->Map( 0, nullptr, (void**) &pVertexUploadBufferData ) ) )
    {
        return;
    }

    //upload to upload heap
    memcpy(pVertexUploadBufferData,cubeVertices,sizeof(cubeVertices));

    cubeVertexUploadBuffer->Unmap( 0, nullptr );

    commandList->CopyBufferRegion( cubeVertexBuffer, 0, cubeVertexUploadBuffer, 0, sizeof(cubeVertices) );

    defaultHeapUploadToReadBarrier.Transition.pResource = cubeVertexBuffer;
    commandList->ResourceBarrier( 1, &defaultHeapUploadToReadBarrier );

    cubeVertexBufferView.BufferLocation = cubeVertexBuffer->GetGPUVirtualAddress();
    cubeVertexBufferView.StrideInBytes = 3*sizeof(f32) + 3*sizeof(f32) + 4*sizeof(f32); //size of s single vertex
    cubeVertexBufferView.SizeInBytes = sizeof(cubeVertices);


	heapBufferDesc.Type = D3D12_HEAP_TYPE_DEFAULT;

    resourceBufferDesc.Width = sizeof(cubeIndicies);

    device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&cubeIndexBuffer)  );
#if MAIN_DEBUG
	cubeIndexBuffer->SetName( L"Cube Index Buffer Resource Heap" );
#endif

	heapBufferDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	device->CreateCommittedResource( &heapBufferDesc, D3D12_HEAP_FLAG_NONE, &resourceBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cubeIndexUploadBuffer)  );
#if MAIN_DEBUG
	cubeIndexUploadBuffer->SetName( L"Cube Index Buffer Resource Upload Heap" );
#endif


    if( FAILED( cubeIndexUploadBuffer->Map( 0, nullptr, (void**) &pIndexUploadBufferData ) ) )
    {
        return;
    }

    //upload to upload heap
    memcpy(pIndexUploadBufferData,cubeIndicies,sizeof(cubeIndicies));

    cubeIndexUploadBuffer->Unmap( 0, nullptr );

    commandList->CopyBufferRegion( cubeIndexBuffer, 0, cubeIndexUploadBuffer, 0, sizeof(cubeIndicies) );

    defaultHeapUploadToReadBarrier.Transition.pResource = cubeIndexBuffer;
    commandList->ResourceBarrier( 1, &defaultHeapUploadToReadBarrier );

	cubeIndexBufferView.BufferLocation = cubeIndexBuffer->GetGPUVirtualAddress();
    cubeIndexBufferView.SizeInBytes = sizeof(cubeIndicies);
    cubeIndexBufferView.Format = DXGI_FORMAT_R32_UINT; 


    cubeIndexCount = 36;
}


//TODO get alt+tab working in all modes! (doesn't seem to ever go to the background) https://stackoverflow.com/questions/972299/best-practices-for-alt-tab-support-in-a-directx-app
//TODO handle device lost
inline
bool InitDirectX12( HWND WindowHandle )
{

	//TODO we should define a minimum version of DirectX12 we support, then check if the user has that version, if not program quits.
	//Then we should make all directx12 objects at that minimum version. Not less and not higher. For example IDXGIFactory7 might be a lower version like IDXGIFactory6 or whatever
	//just needs to be exactly at our minimum lowest version requirement.
	//That way we don't need to do queryinterface calls, can just directly type case.
	//shader compiler version too, make sure! (can use device->CheckFeatureSupport(...) maybe, but then how about check before creating the factory?). 
	//nothing should fail then if it is correctly called.

	IDXGIFactory7* dxgiFactory;
#if MAIN_DEBUG
    u32 createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
    u32 createFactoryFlags = 0;
#endif
	if( FAILED( CreateDXGIFactory2( createFactoryFlags, IID_PPV_ARGS( &dxgiFactory ) ) ) ) 
	{
		logError( "Error creating DXGI factory\n" );  
		return false;
	}

	IDXGIAdapter1* adapter1 = nullptr;
	IDXGIAdapter4* adapter4 = nullptr;

	//TODO look into EnumAdapterByGpuPreference
 	//The point here is that the IDXGIFactory4::EnumWarpAdapter method was introduced in DXGI 1.4 but the IDXGIAdapter4 that we need wasn’t added until DXGI 1.6. 
	//instead of getting first adapter that works, can keep enumerating until you find the most memory one
	// or highest clock rate one, or one with highest D3D capabilities, or user preference one
	//would be cool to allow the user to switch the device mid application, but could be super complex
	u64 amountOfVideoMemory = 0;
	if( forceSoftwareRasterize )
	{
		if( SUCCEEDED( dxgiFactory->EnumWarpAdapter( IID_PPV_ARGS( &adapter1 ) ) ) )
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter1->GetDesc1( &desc );

#if MAIN_DEBUG
			if( SUCCEEDED( adapter1->QueryInterface( IID_PPV_ARGS( &adapter4 ) ) ) )
			{
				assert( adapter1 == adapter4 );
				amountOfVideoMemory = desc.DedicatedVideoMemory;
				adapter1->Release();
			}
			else
			{
				logError( "Error creatings software adapter!\n" );  
				return false;
			}
#else
			amountOfVideoMemory = desc.DedicatedVideoMemory;
			adapter4 = ( IDXGIAdapter4* )adapter1;
#endif
		}
	}
	else
	{
		for( u32 i = 0; dxgiFactory->EnumAdapters1( i, &adapter1 ) != DXGI_ERROR_NOT_FOUND; ++i )
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter1->GetDesc1( &desc );

			if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
			{
				adapter1->Release();
				continue;
			}
			if( SUCCEEDED( D3D12CreateDevice( adapter1, D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ) )
			{
#if MAIN_DEBUG
				if( SUCCEEDED( adapter1->QueryInterface( IID_PPV_ARGS( &adapter4 ) ) ) )
				{
					assert( adapter1 == adapter4 );
					amountOfVideoMemory = desc.DedicatedVideoMemory;
					adapter1->Release();
					break;
				}
#else
				amountOfVideoMemory = desc.DedicatedVideoMemory;
				adapter4 = ( IDXGIAdapter4* )adapter1;
				break;
#endif
			}
			adapter1->Release(); //is this needed?
		}
	}
	if( !adapter4 )
	{
		logError( "Error could not find a DirectX Adapter!\n" );  
		return false;
	}
#if MAIN_DEBUG
	printf( "Amount of Video Memory on selected D3D12 device: %lld\n", amountOfVideoMemory );
#endif
	//actually retrieve the device interface to the adapter
	if( FAILED( D3D12CreateDevice( adapter4, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &device ) ) ) )
	{
		logError( "Error could not open directx 12 supporting GPU!\n" );  
		return false;
	}
	adapter4->Release();

#if MAIN_DEBUG
	//for getting errors from directx when debugging
	if( SUCCEEDED( device->QueryInterface( IID_PPV_ARGS( &pIQueue ) ) ) )
	{
		pIQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pIQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pIQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };
 
        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };
 
        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;
 
        if( FAILED( pIQueue->PushStorageFilter( &NewFilter ) ) )
        {
        	logError( "Detected device creation problem!\n" );  
        	return false;
        }
	}
#endif

	commandQueue = InitDirectCommandQueue( device );
	if( !commandQueue )
	{
		logError( "Failed to create command queue!\n" ); 
		return false;
	}

	//tearing requires DXGI 1.5, which would be at a IDXGIFactory5 level
	u32 tmpFeatureSupport;
	if( FAILED( dxgiFactory->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tmpFeatureSupport, sizeof( u32 ) ) ) )
	{
		isScreenTearingSupported = 0;
	}
	else
	{
		isScreenTearingSupported = (u8)tmpFeatureSupport;
	}

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = screen_width;
	swapChainDesc.Height = screen_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = 0;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = NUM_FRAMES;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = isScreenTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	//look into DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH


	/*
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc;
	swapChainFullscreenDesc.RefreshRate = ;
	swapChainFullscreenDesc.ScanlineOrdering = ;
	swapChainFullscreenDesc.Scaling = ;
	swapChainFullscreenDesc.Windowed = ;
	*/

	IDXGISwapChain1* tempSwapChain = NULL;
	if( FAILED( dxgiFactory->CreateSwapChainForHwnd( commandQueue, WindowHandle, &swapChainDesc, NULL, NULL, &tempSwapChain  ) ) )
	{
		logError( "Failed to create swap chain!\n" ); 
		return false;
	}

	/*
		how doe the above compare with:
		IDXGISwapChain* tempSwapChain; 			
		dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain);
		in terms of features, backwards compat, performance, and memory size
	*/

	//gets rid of default ALT+Enter
	if( FAILED( dxgiFactory->MakeWindowAssociation( WindowHandle, DXGI_MWA_NO_ALT_ENTER ) ) )
	{
		logError( "Failed to make window association!\n" ); 
		return false;
	}

#if MAIN_DEBUG
	if( FAILED( tempSwapChain->QueryInterface( IID_PPV_ARGS( &swapChain ) ) ) )
	{
		logError( "Failed to query swap chain interface!\n" ); 
		return false;
	}
	assert( tempSwapChain == swapChain );
	tempSwapChain->Release();
#else
	swapChain = (IDXGISwapChain4*) tempSwapChain;
#endif	

	UpdateCurrentFrame();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = NUM_FRAMES;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	if( FAILED( device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &rtvDescriptorHeap ) ) ) )
	{
		logError( "Failed to create render target descriptor heap!\n" ); 
		return false;
	}

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	//iterate over descriptor heap, storing mem address of each backbuffer in the swap chain into a descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for( u32 dwIdx = 0; dwIdx < NUM_FRAMES; ++dwIdx )
	{
		if( FAILED( swapChain->GetBuffer( dwIdx, IID_PPV_ARGS( &backBufferRenderTargets[dwIdx] ) ) ) )
		{
			logError( "Failed to get rtv handle!\n" ); 
			return false;
		}
		device->CreateRenderTargetView( backBufferRenderTargets[dwIdx], nullptr, rtvHandle );
		rtvHandle.ptr = (u64)rtvHandle.ptr + rtvDescriptorSize;

		if( FAILED( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &commandAllocator[dwIdx] ) ) ) )
		{
			logError( "Failed to create command allocator!\n" );
			return false;
		}

		//TODO https://www.braynzarsoft.net/viewtutorial/q16390-03-initializing-directx-12 creates a fence for each back buffer
		// while https://www.3dgep.com/learning-directx-12-1/ creates a singular fence.
		// which way is better?
		if( FAILED( device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &fences[dwIdx] ) ) ) )
		{
			logError( "Failed to create GPU/CPU fence!\n" );
			return false;
		}
		fenceValue[dwIdx] = 0;
	}

	//stores view for depth buffer
	//this heap can be reused when resizing the depth buffer, is it optimal to combine this heap with the swap chain descriptor heap?
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	if( FAILED( device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS( &dsDescriptorHeap ) ) ) )
	{
		logError( "Failed to create depth buffer descriptor heap!\n" );
		return false;
	}

	if( !CreateDepthStencilBuffer() )
	{
		return false;
	}

	if( FAILED( device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[currentFrameIdx], NULL, IID_PPV_ARGS( &commandList ) ) ) )
	{
		logError( "Failed to create Command list (it will change which allocator it allocates commands into every frame)!\n" );
		return false;
	}

	fenceEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( !fenceEvent )
	{
		logError( "Failed to create fence event!\n" );
		return false;
	}

	UploadModels();

	if( FAILED( commandList->Close() ) )
	{
		logError( "Command list failed to close, go through debug layer to see what command failed!\n" );
		return false;
	}
	
	ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

    if( !SignalCurrentFrame() )
    {
    	return false;
    }

	//https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits?redirectedfrom=MSDN
	//show example of all three ways of setting constant buffer in root signature

	//D3D12_ROOT_PARAMETER1 is for D3D_ROOT_SIGNATURE_VERSION_1_1 and D3D12_ROOT_PARAMETER is for D3D_ROOT_SIGNATURE_VERSION_1_0
	// so remember to CheckFeatureSupport, for D3D12_FEATURE_ROOT_SIGNATURE, at beginning and use the lowest version for required minimum spec


	//i don't think we need a CBV for root constants, as the CBV acts like the pointer stored in the descriptor

	//https://stackoverflow.com/questions/18374565/hlsl-cbuffer-uniforms-size-and-offset
	/*
	cbuffer uniformsCB : register(b0,space0)
	{
	    float4x4 mvpMat;
		float3x3 nMat; // x y z, pad, x, y, z, pad, x ,y, z (pad if there is another element following this in the struct)
	};
	*/
	//vertex shader constants
	D3D12_ROOT_CONSTANTS cbVertDesc;
	cbVertDesc.ShaderRegister = 0;
	cbVertDesc.RegisterSpace = 0;
	                            //float4x4  //float3x3
	cbVertDesc.Num32BitValues = ( 4 * 4 ) + ( ( ( 4 * 2 ) + 3 ) );

	D3D12_ROOT_CONSTANTS cbPixelDesc;
	cbPixelDesc.ShaderRegister = 1;
	cbPixelDesc.RegisterSpace = 0;
	                            //float4 and float3
	cbPixelDesc.Num32BitValues = 4 + 3;

	D3D12_ROOT_PARAMETER rootParams[2];
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[0].Constants = cbVertDesc;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParams[1].Constants = cbPixelDesc;
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//D3D12_VERSIONED_ROOT_SIGNATURE_DESC
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.pParameters = rootParams;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT  | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS  | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS; //D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS

	ID3DBlob* serializedRootSignature;
	if( FAILED( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &serializedRootSignature, nullptr ) ) )
	{
		logError( "Failed to serialize root signature!\n" );
		return false;
	}

	if( FAILED( device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(), IID_PPV_ARGS( &rootSignature ) ) ) )
	{
		logError( "Failed to create root signature!\n" );
		return false;
	}
	//todo can i free serializedRootSignature here?

	//this describes the vertex inut layout (if we were doing instancing this can change to allow per instance data)

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputLayout;
	inputLayoutDesc.NumElements = 3;

	D3D12_SHADER_BYTECODE vertexShaderBytecode;
	vertexShaderBytecode.pShaderBytecode = vertexShaderBlob;
	vertexShaderBytecode.BytecodeLength = sizeof(vertexShaderBlob);

	D3D12_SHADER_BYTECODE pixelShaderBytecode;
	pixelShaderBytecode.pShaderBytecode = pixelShaderBlob;
	pixelShaderBytecode.BytecodeLength = sizeof(pixelShaderBlob);

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
	renderTargetBlendDesc.BlendEnable = 0;
	renderTargetBlendDesc.LogicOpEnable = 0;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC pipelineBlendState;
	pipelineBlendState.AlphaToCoverageEnable = 0;
	pipelineBlendState.IndependentBlendEnable = 0;
	for( u32 dwRenderTarget = 0; dwRenderTarget < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++dwRenderTarget )
	{
		pipelineBlendState.RenderTarget[ dwRenderTarget ] = renderTargetBlendDesc;
	}

	D3D12_RASTERIZER_DESC pipelineRasterizationSettings;
	pipelineRasterizationSettings.FillMode = D3D12_FILL_MODE_SOLID;
	pipelineRasterizationSettings.CullMode = D3D12_CULL_MODE_BACK;
	pipelineRasterizationSettings.FrontCounterClockwise = 0;
	pipelineRasterizationSettings.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pipelineRasterizationSettings.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pipelineRasterizationSettings.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pipelineRasterizationSettings.DepthClipEnable = 1;
	pipelineRasterizationSettings.MultisampleEnable = 0;
	pipelineRasterizationSettings.AntialiasedLineEnable = 0;
	pipelineRasterizationSettings.ForcedSampleCount = 0;
	pipelineRasterizationSettings.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_DEPTH_STENCILOP_DESC frontFaceDesc;
	frontFaceDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	frontFaceDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	frontFaceDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_DEPTH_STENCILOP_DESC backFaceDesc;
	backFaceDesc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	backFaceDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	backFaceDesc.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	backFaceDesc.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_DEPTH_STENCIL_DESC pipelineDepthStencilState;
	pipelineDepthStencilState.DepthEnable = 1;
	pipelineDepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipelineDepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pipelineDepthStencilState.StencilEnable = 0;
	pipelineDepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	pipelineDepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	pipelineDepthStencilState.FrontFace = frontFaceDesc;
	pipelineDepthStencilState.BackFace = backFaceDesc;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc;
	pipelineDesc.pRootSignature = rootSignature; //why is this even here if we are going to set it in the command list?
	pipelineDesc.VS = vertexShaderBytecode;
	pipelineDesc.PS = pixelShaderBytecode;
	pipelineDesc.DS = {};
	pipelineDesc.HS = {};
	pipelineDesc.GS = {};
	pipelineDesc.StreamOutput = {};
	pipelineDesc.BlendState = pipelineBlendState;
	pipelineDesc.SampleMask = 0xffffffff;
	pipelineDesc.RasterizerState = pipelineRasterizationSettings;
	pipelineDesc.DepthStencilState = pipelineDepthStencilState;
	pipelineDesc.InputLayout = inputLayoutDesc;
	pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED ;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	for( u32 dwRenderTargetFormat = 1; dwRenderTargetFormat < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++dwRenderTargetFormat )
	{
		pipelineDesc.RTVFormats[dwRenderTargetFormat] = DXGI_FORMAT_UNKNOWN;
	}
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineDesc.SampleDesc = sampleDesc;
	pipelineDesc.NodeMask = 0;
	pipelineDesc.CachedPSO = {};
	pipelineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE; //set in debug mode for embedded graphics

	if( FAILED( device->CreateGraphicsPipelineState( &pipelineDesc, IID_PPV_ARGS( &pipelineStateObject ) ) ) )
	{
		logError( "Failed to create pipeline state object!\n" );
		return false;
	}

	dxgiFactory->Release();

	viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (f32)screen_width;
    viewport.Height = (f32)screen_height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = screen_width;
    scissorRect.bottom = screen_height;

	return true;
}

#if MAIN_DEBUG  
void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}
#endif

//TODO: casey did this with refterm https://github.com/cmuratori/refterm/blob/main/refterm.c
//typedef BOOL WINAPI set_process_dpi_aware(void);
//typedef BOOL WINAPI set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT);

//maybe future do void WinMainCRTStartup()
#if MAIN_DEBUG
int main()
#else
int APIENTRY WinMain(
    _In_ HINSTANCE Instance,
    _In_opt_ HINSTANCE PrevInstance,
    _In_ LPSTR CommandLine,
    _In_ int ShowCode )
#endif
{

	/*
 	HMODULE WinUser = LoadLibraryW(L"user32.dll"); //when would the user32 dll every be missing though...
    set_process_dpi_awareness_context *SetThreadDpiAwarenessContext = (set_process_dpi_awareness_context *)GetProcAddress(WinUser, "SetThreadDpiAwarenessContext");
    if(SetThreadDpiAwarenessContext)
    {
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    }
    else
    {
        set_process_dpi_aware *SetProcessDPIAware = (set_process_dpi_aware *)GetProcAddress(WinUser, "SetProcessDPIAware");
        if(SetProcessDPIAware)
        {
            SetProcessDPIAware();
        }
    }
	*/

	SetThreadDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

#if MAIN_DEBUG
	MainWindowHandle = InitWin32Window();
#else
	MainWindowHandle = InitWin32Window( Instance );
#endif
	if( !MainWindowHandle )
	{
		return -1;
	}
#if MAIN_DEBUG
	if( !EnableDebugLayer() )
	{
		return -1;
	}
#endif

	InitScreenGraphicsState();
	if( !InitDirectX12( MainWindowHandle ) )
	{
		return -1;
	}

	InitCursorScreenLock( MainWindowHandle );
    InitRawInput( MainWindowHandle );

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency( &PerfCountFrequencyResult );
    int64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    LARGE_INTEGER LastCounter;
    QueryPerformanceCounter( &LastCounter );


/*
    // Set the working directory to the path of the executable. // so that it isn't in the directory the .exe was launched from
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if ( GetModuleFileNameW(hModule, path, MAX_PATH) > 0 )
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

*/

    InitStartingGameState();
    InitStartingCamera();

    while( Running )
    {
    	//synchronization
        UpdateCurrentFrame();
        if( !WaitCurrentFrame() )
        {
			continue;
        }

    	uint64_t EndCycleCount = __rdtsc();
    
    	LARGE_INTEGER EndCounter;
    	QueryPerformanceCounter(&EndCounter);
    
    	//Display the value here
    	s64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
    	f32 deltaTime = CounterElapsed / (f32)PerfCountFrequency ;
    	f64 MSPerFrame = (f64) ( ( 1000.0f * CounterElapsed ) / (f64)PerfCountFrequency );
    	f64 FPS = PerfCountFrequency / (f64)CounterElapsed;
    	LastCounter = EndCounter;

#if MAIN_DEBUG
    	char buf[64];
    	sprintf_s( &buf[0], 64, "%s: fps %f", WindowName, FPS );
    	SetWindowTextA( MainWindowHandle, &buf[0] );
#endif
		Vec2f frameRot = { 0, 0 };


        MSG Message;
        while( PeekMessage( &Message, 0, 0, 0, PM_REMOVE ) )
        {
        	//switch to raw input https://docs.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement?redirectedfrom=MSDN
        	switch( Message.message )
        	{
                case WM_QUIT:
                {
                	CloseProgram();
                	break;
                }
                /*
                case WM_SYSCOMMAND: //maybe has to be handled in the windo
                {
                	printf("Got a sys command!\n");
                	break;
                }
                */
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYDOWN:
                case WM_KEYUP:
                {
                   uint32_t VKCode = (uint32_t) Message.wParam;
                   bool WasDown = ( Message.lParam & ( 1 << 30 ) ) != 0;
                   bool IsDown = ( Message.lParam & ( 1 << 31 ) ) == 0;
                   bool AltKeyWasDown = ( Message.lParam & ( 1 << 29 ) );
                   switch( VKCode )
                   {
                        case VK_UP:
                        case 'W':
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingForward = IsDown;
                        	}
                        	break;
                        }
                        case VK_DOWN:
                        case 'S':
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingBackwards = IsDown;
                        	}
                        	break;
                        }
                        case VK_LEFT:
                        case 'A':
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingLeft = IsDown;
                        	}
                        	break;
                        }
                        case VK_RIGHT:
                        case 'D':
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingRight = IsDown;
                        	}
                        	break;
                        }
                        case 'F':
                        {
                        	if( WasDown != 1 && WasDown != IsDown )
                        	{
                        		Fullscreen( MainWindowHandle );
                        	}
                        	break;
                        }
                        case 'V':
                        {
                        	if( WasDown != IsDown && WasDown != 1)
                        	{
                        		ToggleVSync();
                        	}
                        	break;
                        }
                        /*
                        case VK_MENU:
                        {
                        	//alt-key
                        	break;
                        }
                        case VK_TAB:
                        {
                        	//tab key
                        	break;
                        }
                        */
                        case VK_SPACE:
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingUp = IsDown;
                        	}
                        	break;
                        }
                        case VK_SHIFT:
                        {
                        	if( WasDown != IsDown )
                        	{
                        		movingDown = IsDown;
                        	}
                        	break;
                        }
                        case VK_ESCAPE:
                        {
                        	if( WasDown != 1 && WasDown != IsDown )
                        	{
                        		TogglePause();
                        	}
                        	break;
                        }
                        case VK_F4:
                        {
                        	if( AltKeyWasDown )
                        	{
                        		CloseProgram();
                        	}
                        	break;
                        }
                        default:
                        {
                        	break;
                        }
                    }
                    break;
                }
                case WM_INPUT:
                {
                	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse
                	UINT dwSize = sizeof( RAWINPUT );
    				static BYTE lpb[sizeof( RAWINPUT )];

    				GetRawInputData( (HRAWINPUT)Message.lParam, RID_INPUT, lpb, &dwSize, sizeof( RAWINPUTHEADER ) );
				
    				RAWINPUT* raw = (RAWINPUT*)lpb;
				
    				if (raw->header.dwType == RIM_TYPEMOUSE) 
    				{
    				    frameRot.x += raw->data.mouse.lLastX;
    				    frameRot.y += raw->data.mouse.lLastY;
    				}
    				else
    				{
    					TranslateMessage( &Message );
                		DispatchMessage( &Message );
    				}
    				/*
    				else if(raw->header.dwType == RIM_TYPEKEYBOARD )
    				{
    					switch(data.keyboard.VKey)
    					{
    						default:
    						break;
    					}
    				}
    				*/
                	break;
                }
                default:
                {
                	TranslateMessage( &Message );
                	DispatchMessage( &Message );
                	break;
                }
            }
        }

        if( !isMinimized )
		{
        	if( !isPaused )
        	{
        		Vec2f forwardOrientation = { -sinf(rotHor*PI_F/180.0f), cosf(rotHor*PI_F/180.0f) };
        	    Vec2f rightOrientation = { -sinf((rotHor+90)*PI_F/180.0f), cosf((rotHor+90)*PI_F/180.0f) };
        	    Vec3f positionChange = {0.0f,0.0f,0.0f};
        	    if(movingForward)
        	    {
        	    	positionChange.x -= forwardOrientation.x;
        	      	positionChange.z += forwardOrientation.y;
        	    }
        	    if(movingLeft)
        	    {
        	    	positionChange.x += rightOrientation.x;
        	      	positionChange.z -= rightOrientation.y;
        	    }
        	    if(movingRight)
        	    {
        	      	positionChange.x -= rightOrientation.x;
        	      	positionChange.z += rightOrientation.y;
        	    }
        	    if(movingBackwards)
        	    {
        	    	positionChange.x += forwardOrientation.x;
        	      	positionChange.z -= forwardOrientation.y;
        	    }
        	    if(movingUp)
        	    {
        	    	positionChange.y += 1;
        	    }
        	    if(movingDown)
        	    {
        	    	positionChange.y -= 1;
        	    }
	
        	    Vec3f oldPos;
        	    oldPos.x = position.x;
        	    oldPos.y = position.y;
        	    oldPos.z = position.z;
        	    Vec3f val;
        	    
        	    Vec3fNormalize(&positionChange, &val);
        	    Vec3fScale(&val,speed*deltaTime,&positionChange);
        	    Vec3fAdd( &oldPos, &positionChange, &position );
	
        	    //should this be before position change?
        	    //this does not take into account delta time because it is measured on a per-pixel screen difference in the
        	    //windows msg queue
        		rotHor  = fmodf(rotHor  + mouseSensitivity*frameRot.x, 360.0f );
        	    rotVert = clamp(rotVert + mouseSensitivity*frameRot.y, -90.0f, 90.0f);
        	}
	
        	drawScene( ( 1 - isPaused ) * deltaTime );
    	}
    	else
    	{
    		//render empty frame while minimized to make power usage go down to very low while minimized (and vsync it)
    		if( !SignalCurrentFrame() )
    		{
    			return -1;
    		}
			if( FAILED( swapChain->Present( 1, 0 ) ) )
    		{
    			CloseProgram();
    			logError( "Error presenting swap chain buffer!\n" ); 
    			return -1;
    		}
    	}
    }

  
 //going to clean up only in Debug mode (so we know exactly what is allocated on close), so we aren't wasting the user's time in actual release on close 
#if MAIN_DEBUG
	FlushCommandQueue();
	rtvDescriptorHeap->Release();
	dsDescriptorHeap->Release();
	planeVertexBuffer->Release();
	planeIndexBuffer->Release(); 
	cubeVertexBuffer->Release(); 
	cubeIndexBuffer->Release();
	commandList->Release();
	for( u32 dwFrame = 0; dwFrame < NUM_FRAMES; ++dwFrame )
    {
    	backBufferRenderTargets[dwFrame]->Release();
    	commandAllocator[dwFrame]->Release();
    	fences[dwFrame]->Release();
    }
    depthStencilBuffer->Release();
    commandQueue->Release();
	//TODO remove these when we can release them ealier
	planeVertexUploadBuffer->Release();
	planeIndexUploadBuffer->Release();
	cubeVertexUploadBuffer->Release();
	cubeIndexUploadBuffer->Release(); 

	pipelineStateObject->Release();
	rootSignature->Release();
    CloseHandle(fenceEvent);
    swapChain->Release();

    //Is this ok todo with debug enabled by these?
	debugInterface->Release(); 
	pIQueue->Release(); 

    device->Release();

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = (USHORT) 0x01; 
	Rid[0].usUsage = (USHORT) 0x02; 
	Rid[0].dwFlags = RIDEV_REMOVE;   
	Rid[0].hwndTarget = MainWindowHandle;
	RegisterRawInputDevices( Rid, 1, sizeof( Rid[0] ) );

	//CoUninitialize();
	DestroyWindow( MainWindowHandle );
    atexit(&ReportLiveObjects);
#endif
    return 0;
}
