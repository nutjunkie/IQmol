#include "opengl_tutorials_win32_framework.h"

// ----------------------------------------------------------------------------------------------------------------------------

CString ModuleDirectory, ErrorLog;

bool wgl_context_forward_compatible = false;

int gl_version = 0, gl_max_texture_size = 0, gl_max_texture_max_anisotropy_ext = 0;

// ----------------------------------------------------------------------------------------------------------------------------

CTexture::CTexture()
{
	TextureID = 0;
}

CTexture::~CTexture()
{
}

CTexture::operator GLuint ()
{
	return TextureID;
}

void CTexture::Delete()
{
	glDeleteTextures(1, &TextureID);
	TextureID = 0;
}

bool CTexture::LoadTexture2D(char *Texture2DFileName)
{
	CString FileName = ModuleDirectory + Texture2DFileName;
	CString ErrorText = "Error loading file " + FileName + "! -> ";

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(FileName);

	if(fif == FIF_UNKNOWN)
	{
		fif = FreeImage_GetFIFFromFilename(FileName);
	}

	if(fif == FIF_UNKNOWN)
	{
		ErrorLog.Append(ErrorText + "fif is FIF_UNKNOWN" + "\r\n");
		return false;
	}

	FIBITMAP *dib = NULL;

	if(FreeImage_FIFSupportsReading(fif))
	{
		dib = FreeImage_Load(fif, FileName);
	}

	if(dib == NULL)
	{
		ErrorLog.Append(ErrorText + "dib is NULL" + "\r\n");
		return false;
	}

	int Width = FreeImage_GetWidth(dib), oWidth = Width;
	int Height = FreeImage_GetHeight(dib), oHeight = Height;
	int Pitch = FreeImage_GetPitch(dib);
	int BPP = FreeImage_GetBPP(dib);

	if(Width == 0 || Height == 0)
	{
		ErrorLog.Append(ErrorText + "Width or Height is 0" + "\r\n");
		return false;
	}

	if(Width > gl_max_texture_size) Width = gl_max_texture_size;
	if(Height > gl_max_texture_size) Height = gl_max_texture_size;

	if(!GLEW_ARB_texture_non_power_of_two)
	{
		Width = 1 << (int)floor((log((float)Width) / log(2.0f)) + 0.5f); 
		Height = 1 << (int)floor((log((float)Height) / log(2.0f)) + 0.5f);
	}

	if(Width != oWidth || Height != oHeight)
	{
		FIBITMAP *rdib = FreeImage_Rescale(dib, Width, Height, FILTER_BICUBIC);

		FreeImage_Unload(dib);

		if((dib = rdib) == NULL)
		{
			ErrorLog.Append(ErrorText + "rdib is NULL" + "\r\n");
			return false;
		}

		Pitch = FreeImage_GetPitch(dib);
	}

	BYTE *Data = FreeImage_GetBits(dib);

	if(Data == NULL)
	{
		ErrorLog.Append(ErrorText + "Data is NULL" + "\r\n");
		return false;
	}

	GLenum Format = 0;

	if(BPP == 32) Format = GL_BGRA;
	if(BPP == 24) Format = GL_BGR;

	if(Format == 0)
	{
		FreeImage_Unload(dib);
		ErrorLog.Append(ErrorText + "Format is 0" + "\r\n");
		return false;
	}

	if(gl_version < 12)
	{
		if(Format == GL_BGRA) Format = GL_RGBA;
		if(Format == GL_BGR) Format = GL_RGB;

		int bpp = BPP / 8;

		BYTE *line = Data;

		for(int y = 0; y < Height; y++)
		{
			BYTE *pixel = line;

			for(int x = 0; x < Width; x++)
			{
				BYTE Temp = pixel[0];
				pixel[0] = pixel[2];
				pixel[2] = Temp;

				pixel += bpp;
			}

			line += Pitch;
		}
	}

	glGenTextures(1, &TextureID);

	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_version >= 14 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, gl_max_texture_max_anisotropy_ext);
	}

	if(gl_version >= 14 && gl_version <= 21)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);

	if(gl_version >= 30)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FreeImage_Unload(dib);

	return true;
}

// ----------------------------------------------------------------------------------------------------------------------------

CShaderProgram::CShaderProgram()
{
	SetDefaults();
}

CShaderProgram::~CShaderProgram()
{
}

CShaderProgram::operator GLuint ()
{
	return Program;
}

void CShaderProgram::Delete()
{
	delete [] UniformLocations;

	glDetachShader(Program, VertexShader);
	glDetachShader(Program, FragmentShader);

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	glDeleteProgram(Program);

	SetDefaults();
}

bool CShaderProgram::Load(char *VertexShaderFileName, char *FragmentShaderFileName)
{
	if(UniformLocations || VertexShader || FragmentShader || Program)
	{
		Delete();
	}

	bool Error = false;

	Error |= ((VertexShader = LoadShader(GL_VERTEX_SHADER, VertexShaderFileName)) == 0);

	Error |= ((FragmentShader = LoadShader(GL_FRAGMENT_SHADER, FragmentShaderFileName)) == 0);

	if(Error)
	{
		Delete();
		return false;
	}

	Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	int Param = 0;
	glGetProgramiv(Program, GL_LINK_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		ErrorLog.Append("Error linking program (%s, %s)!\r\n", VertexShaderFileName, FragmentShaderFileName);

		int InfoLogLength = 0;
		glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetProgramInfoLog(Program, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		Delete();

		return false;
	}

	return true;
}

GLuint CShaderProgram::LoadShader(GLenum Type, char *ShaderFileName)
{
	CString FileName = ModuleDirectory + ShaderFileName;

	FILE *File;

	if(fopen_s(&File, FileName, "rb") != 0)
	{
		ErrorLog.Append("Error loading file " + FileName + "!\r\n");
		return 0;
	}

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	char *Source = new char[Size + 1];
	fread(Source, 1, Size, File);
	fclose(File);
	Source[Size] = 0;

	GLuint Shader;

	Shader = glCreateShader(Type);
	glShaderSource(Shader, 1, (const char**)&Source, NULL);
	delete [] Source;
	glCompileShader(Shader);

	int Param = 0;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		ErrorLog.Append("Error compiling shader %s!\r\n", ShaderFileName);

		int InfoLogLength = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char *InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetShaderInfoLog(Shader, InfoLogLength, &CharsWritten, InfoLog);
			ErrorLog.Append(InfoLog);
			delete [] InfoLog;
		}

		glDeleteShader(Shader);

		return 0;
	}

	return Shader;
}

void CShaderProgram::SetDefaults()
{
	UniformLocations = NULL;
	VertexShader = 0;
	FragmentShader = 0;
	Program = 0;
}

// ----------------------------------------------------------------------------------------------------------------------------

CCamera::CCamera()
{
	View = NULL;

	Reference = vec3(0.0f, 0.0f, 0.0f);
	Position = vec3(0.0f, 0.0f, 5.0f);

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);
}

CCamera::~CCamera()
{
}

void CCamera::CalculateViewMatrix()
{
	if(View)
	{
		*View = ViewMatrix(X, Y, Z, Position);
	}
}

void CCamera::LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference)
{
	this->Reference = Reference;
	this->Position = Position;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

void CCamera::Move(vec3 Movement)
{
	Reference += Movement;
	Position += Movement;

	CalculateViewMatrix();
}

vec3 CCamera::OnKeys(BYTE Keys, float FrameTime)
{
	float Speed = 5.0f;

	if(Keys & 0x40) // SHIFT
	{
		Speed *= 2.0f;
	}

	float Distance = Speed * FrameTime;

	vec3 Up(0.0f, 1.0f, 0.0f);
	vec3 Right = X;
	vec3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	vec3 Movement;

	if(Keys & 0x01) // W
	{
		Movement += Forward;
	}

	if(Keys & 0x02) // S
	{
		Movement -= Forward;
	}

	if(Keys & 0x04) // A
	{
		Movement -= Right;
	}

	if(Keys & 0x08) // D
	{
		Movement += Right;
	}

	if(Keys & 0x10) // R
	{
		Movement += Up;
	}

	if(Keys & 0x20) // F
	{
		Movement -= Up;
	}

	return Movement;
}

void CCamera::OnMouseMove(int dx, int dy)
{
	float sensitivity = 0.25f;

	float hangle = (float)dx * sensitivity;
	float vangle = (float)dy * sensitivity;

	Position -= Reference;

	Y = rotate(Y, vangle, X);
	Z = rotate(Z, vangle, X);

	if(Y.y < 0.0f)
	{
		Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		Y = cross(Z, X);
	}

	X = rotate(X, hangle, vec3(0.0f, 1.0f, 0.0f));
	Y = rotate(Y, hangle, vec3(0.0f, 1.0f, 0.0f));
	Z = rotate(Z, hangle, vec3(0.0f, 1.0f, 0.0f));

	Position = Reference + Z * length(Position);

	CalculateViewMatrix();
}

void CCamera::OnMouseWheel(float zDelta)
{
	Position -= Reference;

	if(zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if(zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;

	CalculateViewMatrix();
}

void CCamera::SetViewMatrixPointer(float *View)
{
	this->View = (mat4x4*)View;

	CalculateViewMatrix();
}

CCamera Camera;

// ----------------------------------------------------------------------------------------------------------------------------

COpenGLRenderer::COpenGLRenderer()
{
	RenderGLUTObjects = true;
	Blur = true;
	ShowNormalBuffer = false;

	Camera.SetViewMatrixPointer(&View);
}

COpenGLRenderer::~COpenGLRenderer()
{
}

bool COpenGLRenderer::Init()
{
	if(gl_version < 21)
	{
		ErrorLog.Set("OpenGL 2.1 not supported!");
		return false;
	}

	bool Error = false;

	if(!GLEW_ARB_texture_non_power_of_two)
	{
		ErrorLog.Append("GL_ARB_texture_non_power_of_two not supported!\r\n");
		Error = true;
	}

	if(!GLEW_ARB_depth_texture)
	{
		ErrorLog.Append("GL_ARB_depth_texture not supported!\r\n");
		Error = true;
	}

	if(!GLEW_EXT_framebuffer_object)
	{
		ErrorLog.Append("GL_EXT_framebuffer_object not supported!\r\n");
		Error = true;
	}

	Error |= !Preprocess.Load("preprocess.vs", "preprocess.fs");
	Error |= !SSAO.Load("ssao.vs", "ssao.fs");
	Error |= !SSAOFilterH.Load("ssaofilter.vs", "ssaofilterh.fs");
	Error |= !SSAOFilterV.Load("ssaofilter.vs", "ssaofilterv.fs");

	Error |= !InitScene();

	if(Error)
	{
		return false;
	}

	SSAO.UniformLocations = new GLuint[2];
	SSAO.UniformLocations[0] = glGetUniformLocation(SSAO, "sxy");
	SSAO.UniformLocations[1] = glGetUniformLocation(SSAO, "ProjectionBiasInverse");

	SSAOFilterH.UniformLocations = new GLuint[1];
	SSAOFilterH.UniformLocations[0] = glGetUniformLocation(SSAOFilterH, "sx");

	SSAOFilterV.UniformLocations = new GLuint[1];
	SSAOFilterV.UniformLocations[0] = glGetUniformLocation(SSAOFilterV, "sy");

	glUseProgram(SSAO);
	glUniform1i(glGetUniformLocation(SSAO, "NormalBuffer"), 0);
	glUniform1i(glGetUniformLocation(SSAO, "DepthBuffer"), 1);
	glUniform1i(glGetUniformLocation(SSAO, "RotationTexture"), 2);
	glUseProgram(0);

	srand(GetTickCount());

	// generate 16 2D vectors used for sampling the depth buffer --------------------------------------------------------------

	vec2 *Samples = new vec2[16];
	float RandomAngle = (float)M_PI_4, Radius = 0.415f;

	for(int i = 0; i < 16; i++)
	{
		Samples[i].x = cos(RandomAngle) * (float)(i + 1) / 16.0f * Radius;
		Samples[i].y = sin(RandomAngle) * (float)(i + 1) / 16.0f * Radius;

		RandomAngle += (float)M_PI_2;

		if(((i + 1) % 4) == 0) RandomAngle += (float)M_PI_4;
	}

	glUseProgram(SSAO);
	glUniform2fv(glGetUniformLocation(SSAO, "Samples"), 16, (float*)Samples);
	glUseProgram(0);

	delete [] Samples;

	// generate 64x64 rotation texture used for rotating the sampling 2D vectors ----------------------------------------------

	vec4 *RotationTextureData = new vec4[64 * 64];

	RandomAngle = (float)rand() / (float)RAND_MAX * (float)M_PI * 2.0f;
	
	for(int i = 0; i < 64 * 64; i++)
	{
		RotationTextureData[i].x = cos(RandomAngle) * 0.5f + 0.5f;
		RotationTextureData[i].y = sin(RandomAngle) * 0.5f + 0.5f;
		RotationTextureData[i].z = -sin(RandomAngle) * 0.5f + 0.5f;
		RotationTextureData[i].w = cos(RandomAngle) * 0.5f + 0.5f;

		RandomAngle += (float)rand() / (float)RAND_MAX * (float)M_PI * 2.0f;
	}

	glGenTextures(1, &RotationTexture);
	glBindTexture(GL_TEXTURE_2D, RotationTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_FLOAT, (float*)RotationTextureData);

	delete [] RotationTextureData;

	// ------------------------------------------------------------------------------------------------------------------------

	glGenTextures(1, &NormalBuffer);
	glGenTextures(1, &DepthBuffer);
	glGenTextures(1, &SSAOTexture);
	glGenTextures(1, &BlurTexture);

	glGenFramebuffersEXT(1, &FBO);

	Camera.LookAt(vec3(0.0f, 1.5, 0.0f), vec3(-1.0f, 1.5, 1.0f));

	return true;
}

void COpenGLRenderer::Render(float FrameTime)
{
	// render scene -----------------------------------------------------------------------------------------------------------

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&Projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&View);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, NormalBuffer, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, DepthBuffer, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(Preprocess);

	RenderScene();

	glUseProgram(0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// calculate ssao ---------------------------------------------------------------------------------------------------------

	if(!ShowNormalBuffer)
	{
		if(Blur)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, SSAOTexture, 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
		}

		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, NormalBuffer);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, DepthBuffer);
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, RotationTexture);
		glUseProgram(SSAO);
		glBegin(GL_QUADS);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(1.0f, 0.0f);
			glVertex2f(1.0f, 1.0f);
			glVertex2f(0.0f, 1.0f);
		glEnd();
		glUseProgram(0);
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);

		if(Blur)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, BlurTexture, 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);

			// horizontal blur ------------------------------------------------------------------------------------------------

			glBindTexture(GL_TEXTURE_2D, SSAOTexture);
			glUseProgram(SSAOFilterH);
			glBegin(GL_QUADS);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(1.0f, 0.0f);
				glVertex2f(1.0f, 1.0f);
				glVertex2f(0.0f, 1.0f);
			glEnd();
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

			// vertical blur --------------------------------------------------------------------------------------------------

			glBindTexture(GL_TEXTURE_2D, BlurTexture);
			glUseProgram(SSAOFilterV);
			glBegin(GL_QUADS);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(1.0f, 0.0f);
				glVertex2f(1.0f, 1.0f);
				glVertex2f(0.0f, 1.0f);
			glEnd();
			glUseProgram(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	else // display normal buffer ---------------------------------------------------------------------------------------------
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NormalBuffer);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

void COpenGLRenderer::Resize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	glViewport(0, 0, Width, Height);

	Projection = PerspectiveProjectionMatrix(45.0f, (float)Width, (float)Height, 0.125f, 512.0f);
	ProjectionBiasInverse = PerspectiveProjectionMatrixInverse(Projection) * BiasMatrixInverse();

	glBindTexture(GL_TEXTURE_2D, NormalBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, DepthBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, SSAOTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, BlurTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glUseProgram(SSAO);
	glUniform2f(SSAO.UniformLocations[0], (float)Width / 64.0f, (float)Height / 64.0f);
	glUniformMatrix4fv(SSAO.UniformLocations[1], 1, GL_FALSE, &ProjectionBiasInverse);
	glUseProgram(0);

	glUseProgram(SSAOFilterH);
	glUniform1f(SSAOFilterH.UniformLocations[0], 1.0f / (float)Width);
	glUseProgram(SSAOFilterV);
	glUniform1f(SSAOFilterV.UniformLocations[0], 1.0f / (float)Height);
	glUseProgram(0);
}

void COpenGLRenderer::Destroy()
{
	if(gl_version >= 21)
	{
		Preprocess.Delete();
		SSAO.Delete();
		SSAOFilterH.Delete();
		SSAOFilterV.Delete();

		glDeleteBuffers(2, VBO);
	}

	glDeleteTextures(1, &RotationTexture);

	glDeleteTextures(1, &NormalBuffer);
	glDeleteTextures(1, &DepthBuffer);
	glDeleteTextures(1, &SSAOTexture);
	glDeleteTextures(1, &BlurTexture);

	if(GLEW_EXT_framebuffer_object)
	{
		glDeleteFramebuffersEXT(1, &FBO);
	}
}

bool COpenGLRenderer::InitScene()
{
	CString RoomFileName = ModuleDirectory + "room.xyz";

	FILE *File;

	if(fopen_s(&File, RoomFileName, "rb") != 0)
	{
		ErrorLog.Append("Error loading file %s!\r\n", RoomFileName);
		return false;
	}

	fread(&QuadsCount, sizeof(int), 1, File);

	CQuad *Quads = new CQuad[QuadsCount + 18];

	fread(Quads, sizeof(CQuad) * QuadsCount, 1, File);

	fclose(File);

	vec3 offset = vec3(0.0f, 0.5f, 3.375f);

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f, -0.5f,  0.5f) + offset;
		
	QuadsCount++;

	offset = vec3(1.5f, 0.5f, 3.375f);

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f, -0.5f,  0.5f) + offset;
		
	QuadsCount++;

	offset = vec3(0.75f, 1.5f, 3.375f);

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3( 0.5f,  0.5f,  0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3(-0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f,  0.5f,  0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f,  0.5f, -0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f,  0.5f, -0.5f) + offset;

	QuadsCount++;

	Quads[QuadsCount].a = vec3(-0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].b = vec3( 0.5f, -0.5f, -0.5f) + offset;
	Quads[QuadsCount].c = vec3( 0.5f, -0.5f,  0.5f) + offset;
	Quads[QuadsCount].d = vec3(-0.5f, -0.5f,  0.5f) + offset;
		
	QuadsCount++;

	vec3 *Vertices = new vec3[QuadsCount * 4];
	vec3* Normals = new vec3[QuadsCount * 4];

	for(int i = 0; i < QuadsCount; i++)
	{
		vec3 t = normalize(Quads[i].b - Quads[i].a);
		vec3 b = normalize(Quads[i].c - Quads[i].a);
		vec3 n = normalize(cross(t, b));

		Vertices[i * 4 + 0] = Quads[i].a;
		Normals[i * 4 + 0] = n;

		Vertices[i * 4 + 1] = Quads[i].b;
		Normals[i * 4 + 1] = n;

		Vertices[i * 4 + 2] = Quads[i].c;
		Normals[i * 4 + 2] = n;

		Vertices[i * 4 + 3] = Quads[i].d;
		Normals[i * 4 + 3] = n;
	}

	glGenBuffers(2, VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, QuadsCount * 4 * 3 * 4, Vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, QuadsCount * 4 * 3 * 4, Normals, GL_STATIC_DRAW);

	delete [] Quads;
	delete [] Vertices;
	delete [] Normals;

	return true;
}

void COpenGLRenderer::RenderScene()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glNormalPointer(GL_FLOAT, 0, NULL);
	glDrawArrays(GL_QUADS, 0, QuadsCount * 4);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_CULL_FACE);

	if(RenderGLUTObjects)
	{
		glLoadMatrixf(&View);
		glTranslatef(2.5f, 1.185f, -2.0f);
		glRotatef(33.0f, 0.0f, 1.0f, 0.0f);
		glutSolidTeapot(0.25f);

		glLoadMatrixf(&View);
		glTranslatef(2.5f, 1.185f, -2.5f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		glutSolidTeapot(0.25f);

		glLoadMatrixf(&View);
		glTranslatef(2.5f, 1.185f, -3.0f);
		glRotatef(-33.0f, 0.0f, 1.0f, 0.0f);
		glutSolidTeapot(0.25f);

		glLoadMatrixf(&View);
		glTranslatef(-2.5f, 0.25f, -1.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glutSolidTorus(0.25f, 0.5f, 64, 64);

		glLoadMatrixf(&View);
		glTranslatef(-2.5f, 0.25f, 0.0f);
		glutSolidSphere(0.25f, 32, 32);

		glLoadMatrixf(&View);
		glTranslatef(-2.5f, 0.125f, 0.365f);
		glutSolidSphere(0.125f, 32, 32);
	}

	glDisable(GL_DEPTH_TEST);
}

COpenGLRenderer OpenGLRenderer;

// ----------------------------------------------------------------------------------------------------------------------------

CWnd::CWnd()
{
	char *moduledirectory = new char[256];
	GetModuleFileName(GetModuleHandle(NULL), moduledirectory, 256);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
	delete [] moduledirectory;
}

CWnd::~CWnd()
{
}

bool CWnd::Create(HINSTANCE hInstance, char *WindowName, int Width, int Height, int Samples, bool CreateForwardCompatibleContext, bool DisableVerticalSynchronization)
{
	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = "Win32OpenGLWindowClass";

	if(RegisterClassEx(&WndClassEx) == 0)
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	this->WindowName = WindowName;

	this->Width = Width;
	this->Height = Height;

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if((hWnd = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, WindowName, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		ErrorLog.Set("CreateWindowEx failed!");
		return false;
	}

	if((hDC = GetDC(hWnd)) == NULL)
	{
		ErrorLog.Set("GetDC failed!");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int PixelFormat;

	if((PixelFormat = ChoosePixelFormat(hDC, &pfd)) == 0)
	{
		ErrorLog.Set("ChoosePixelFormat failed!");
		return false;
	}

	static int MSAAPixelFormat = 0;

	if(SetPixelFormat(hDC, MSAAPixelFormat == 0 ? PixelFormat : MSAAPixelFormat, &pfd) == FALSE)
	{
		ErrorLog.Set("SetPixelFormat failed!");
		return false;
	}

	if((hGLRC = wglCreateContext(hDC)) == NULL)
	{
		ErrorLog.Set("wglCreateContext failed!");
		return false;
	}

	if(wglMakeCurrent(hDC, hGLRC) == FALSE)
	{
		ErrorLog.Set("wglMakeCurrent failed!");
		return false;
	}

	if(glewInit() != GLEW_OK)
	{
		ErrorLog.Set("glewInit failed!");
		return false;
	}

	if(MSAAPixelFormat == 0 && Samples > 0)
	{
		if(GLEW_ARB_multisample && WGLEW_ARB_pixel_format)
		{
			while(Samples > 0)
			{
				UINT NumFormats = 0;

				int iAttributes[] =
				{
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
					WGL_COLOR_BITS_ARB, 32,
					WGL_DEPTH_BITS_ARB, 24,
					WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
					WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
					WGL_SAMPLES_ARB, Samples,
					0
				};

				if(wglChoosePixelFormatARB(hDC, iAttributes, NULL, 1, &MSAAPixelFormat, &NumFormats) == TRUE && NumFormats > 0) break;

				Samples--;
			}

			wglDeleteContext(hGLRC);

			DestroyWindow(hWnd);

			UnregisterClass(WndClassEx.lpszClassName, hInstance);

			return Create(hInstance, WindowName, Width, Height, Samples, CreateForwardCompatibleContext, DisableVerticalSynchronization);
		}
		else
		{
			Samples = 0;
		}
	}

	this->Samples = Samples;

	int major, minor;

	sscanf_s((char*)glGetString(GL_VERSION), "%d.%d", &major, &minor);

	gl_version = major * 10 + minor;

	if(CreateForwardCompatibleContext && gl_version >= 30 && WGLEW_ARB_create_context)
	{
		wglDeleteContext(hGLRC);

		int GLFCRCAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, major,
			WGL_CONTEXT_MINOR_VERSION_ARB, minor,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		if((hGLRC = wglCreateContextAttribsARB(hDC, 0, GLFCRCAttribs)) == NULL)
		{
			ErrorLog.Set("wglCreateContextAttribsARB failed!");
			return false;
		}

		if(wglMakeCurrent(hDC, hGLRC) == FALSE)
		{
			ErrorLog.Set("wglMakeCurrent failed!");
			return false;
		}

		wgl_context_forward_compatible = true;
	}
	else
	{
		wgl_context_forward_compatible = false;
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_texture_max_anisotropy_ext);
	}

	if(DisableVerticalSynchronization && WGLEW_EXT_swap_control)
	{
		wglSwapIntervalEXT(0);
	}

	return OpenGLRenderer.Init();
}

void CWnd::Show(bool Maximized)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(hWnd, &wRect);
	GetClientRect(hWnd, &cRect);

	wRect.right += Width - cRect.right;
	wRect.bottom += Height - cRect.bottom;

	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;

	wRect.left = dRect.right / 2 - wRect.right / 2;
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);

	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
}

void CWnd::MessageLoop()
{
	MSG Msg;

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void CWnd::Destroy()
{
	OpenGLRenderer.Destroy();

	wglDeleteContext(hGLRC);

	DestroyWindow(hWnd);
}

void CWnd::OnKeyDown(UINT Key)
{
	switch(Key)
	{
		case VK_F1:
			OpenGLRenderer.RenderGLUTObjects = !OpenGLRenderer.RenderGLUTObjects;
			break;

		case VK_F2:
			OpenGLRenderer.Blur = !OpenGLRenderer.Blur;
			break;

		case VK_F3:
			OpenGLRenderer.ShowNormalBuffer = !OpenGLRenderer.ShowNormalBuffer;
			break;
	}
}

void CWnd::OnMouseMove(int cx, int cy)
{
	if(GetKeyState(VK_RBUTTON) & 0x80)
	{
		Camera.OnMouseMove(LastCurPos.x - cx, LastCurPos.y - cy);

		LastCurPos.x = cx;
		LastCurPos.y = cy;
	}
}

void CWnd::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);
}

void CWnd::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);

	static DWORD LastFPSTime = GetTickCount(), LastFrameTime = LastFPSTime;
	static int FPS = 0;

	DWORD Time = GetTickCount();

	float FrameTime = (Time - LastFrameTime) * 0.001f;

	LastFrameTime = Time;

	if(Time - LastFPSTime > 1000)
	{
		CString Text = WindowName;

		Text.Append(" - %dx%d", Width, Height);
		Text.Append(", ATF %dx", gl_max_texture_max_anisotropy_ext);
		Text.Append(", MSAA %dx", Samples);
		Text.Append(", FPS: %d", FPS);
		Text.Append(" - OpenGL %d.%d", gl_version / 10, gl_version % 10);
		if(gl_version >= 30) if(wgl_context_forward_compatible) Text.Append(" Forward compatible"); else Text.Append(" Compatibility profile");
		Text.Append(" - %s", (char*)glGetString(GL_RENDERER));
		
		SetWindowText(hWnd, Text);

		LastFPSTime = Time;
		FPS = 0;
	}
	else
	{
		FPS++;
	}

	BYTE Keys = 0x00;

	if(GetKeyState('W') & 0x80) Keys |= 0x01;
	if(GetKeyState('S') & 0x80) Keys |= 0x02;
	if(GetKeyState('A') & 0x80) Keys |= 0x04;
	if(GetKeyState('D') & 0x80) Keys |= 0x08;
	if(GetKeyState('R') & 0x80) Keys |= 0x10;
	if(GetKeyState('F') & 0x80) Keys |= 0x20;

	if(GetKeyState(VK_SHIFT) & 0x80) Keys |= 0x40;

	if(Keys & 0x3F)
	{
		vec3 Movement = Camera.OnKeys(Keys, FrameTime);
		Camera.Move(Movement);
	}

	OpenGLRenderer.Render(FrameTime);

	SwapBuffers(hDC);

	EndPaint(hWnd, &ps);

	InvalidateRect(hWnd, NULL, FALSE);
}

void CWnd::OnRButtonDown(int cx, int cy)
{
	LastCurPos.x = cx;
	LastCurPos.y = cy;
}

void CWnd::OnSize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	OpenGLRenderer.Resize(Width, Height);
}

CWnd Wnd;

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;

		case WM_MOUSEMOVE:
			Wnd.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
			break;

		case 0x020A: // WM_MOUSWHEEL
			Wnd.OnMouseWheel(HIWORD(wParam));
			break;

		case WM_KEYDOWN:
			Wnd.OnKeyDown((UINT)wParam);
			break;

		case WM_PAINT:
			Wnd.OnPaint();
			break;

		case WM_RBUTTONDOWN:
			Wnd.OnRButtonDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_SIZE:
			Wnd.OnSize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	if(Wnd.Create(hInstance, "Screen space ambient occlusion", 800, 600, 0))
	{
		Wnd.Show();
		Wnd.MessageLoop();
	}
	else
	{
		MessageBox(NULL, ErrorLog, "Error", MB_OK | MB_ICONERROR);
	}

	Wnd.Destroy();

	return 0;
}
