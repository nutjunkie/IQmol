#include <windows.h>

#include "string.h"
#include "glmath.h"

#include <gl/glew.h> // http://glew.sourceforge.net/
#include <gl/wglew.h>
#include <gl/wglew.h>

#include <gl/glut.h> // http://user.xmission.com/~nate/glut.html

#include <FreeImage.h> // http://freeimage.sourceforge.net/

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

#pragma comment(lib, "FreeImage.lib")

// ----------------------------------------------------------------------------------------------------------------------------

class CTexture
{
protected:
	GLuint TextureID;

public:
	CTexture();
	~CTexture();

	operator GLuint ();

	void Delete();
	bool LoadTexture2D(char *Texture2DFileName);
};

// ----------------------------------------------------------------------------------------------------------------------------

class CShaderProgram
{
public:
	GLuint *UniformLocations;

protected:
	GLuint VertexShader, FragmentShader, Program;

public:
	CShaderProgram();
	~CShaderProgram();

	operator GLuint ();

	void Delete();
	bool Load(char *VertexShaderFileName, char *FragmentShaderFileName);

protected:
	GLuint LoadShader(GLenum Type, char *ShaderFileName);
	void SetDefaults();
};

// ----------------------------------------------------------------------------------------------------------------------------

class CCamera
{
protected:
	mat4x4 *View;

public:
	vec3 X, Y, Z, Reference, Position;

	CCamera();
	~CCamera();

	void CalculateViewMatrix();
	void LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference = false);
	void Move(vec3 Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(float zDelta);
	void SetViewMatrixPointer(float *View);
};

// ----------------------------------------------------------------------------------------------------------------------------

class CQuad
{
public:
	vec3 a, b, c, d, normal;
};

// ----------------------------------------------------------------------------------------------------------------------------

class COpenGLRenderer
{
protected:
	int Width, Height;
	mat4x4 Model, View, Projection, ProjectionBiasInverse;

	GLuint RotationTexture, NormalBuffer, DepthBuffer, SSAOTexture, BlurTexture, VBO[2], FBO;
	CShaderProgram Preprocess, SSAO, SSAOFilterH, SSAOFilterV;

	int QuadsCount;
	
public:
	bool RenderGLUTObjects, Blur, ShowNormalBuffer;

public:
	COpenGLRenderer();
	~COpenGLRenderer();

	bool Init();
	void Render(float FrameTime);
	void Resize(int Width, int Height);
	void Destroy();

protected:
	bool InitScene();
	void RenderScene();
};

// ----------------------------------------------------------------------------------------------------------------------------

class CWnd
{
protected:
	HWND hWnd;
	HDC hDC;
	HGLRC hGLRC;
	char *WindowName;
	int Width, Height, Samples;
	POINT LastCurPos;

public:
	CWnd();
	~CWnd();

	bool Create(HINSTANCE hInstance, char *WindowName, int Width, int Height, int Samples = 4, bool CreateForwardCompatibleContext = false, bool DisableVerticalSynchronization  = true);
	void Show(bool Maximized = false);
	void MessageLoop();
	void Destroy();

	void OnKeyDown(UINT Key);
	void OnMouseMove(int cx, int cy);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int cx, int cy);
	void OnSize(int Width, int Height);
};

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow);
