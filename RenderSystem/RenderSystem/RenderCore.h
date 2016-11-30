#ifndef ____RENDER_CORE_H____
#define ____RENDER_CORE_H____

//#define __USE_SHADER__
//#define __USE_TEXTURE_INFO__
#ifdef __USE_TEXTURE_INFO__
#include <map>
using namespace std;
#endif

#include "common.h"

#define MAX_GL_BUFFER_ARRAY_SIZE	10240
#define GL_QUADS 888
struct TextureColoredVertex
{
	Vector2f geometryVertex;
	Vector2f textureVertex;
	Color4f vertexColor;
};

typedef struct stTextureInfo
{
	float m_textureWidth;
	float m_textureHeight;
	bool m_uselinearFilter;
} TEXTURE_INFO, *pTEXTURE_INFO;

enum GL_RENDER_TYPE
{
	GL_RENDER_TYPE_QUADS,
	GL_RENDER_TYPE_TRIANGLES,
	GL_RENDER_TYPE_LINE_LOOP,
	GL_RENDER_TYPE_LINE
};
#ifdef __USE_SHADER__
class CShaderProgram;
class CShaderCommon;
class CShaderPointSprite;
class CShaderStroke;
#endif

class CRenderCore
{
public:
	CRenderCore(int screenWidth, int screenHeight);
	~CRenderCore(void);

	void Initialize(int screenWidth, int screenHeight);

	bool IsExtensionSupported(char* szTargetExtension);

	void InitRenderCore();
	void SetImmediateModeGLVertexArrays();

	UINT CreateTexture(stImage* stimage);
	void DeleteTexture(UINT textureID);
	bool IsTextrue(UINT textureID);
	void SetGLScissorEnable(bool enable);
	void SetGLScissorArea(int x, int y, int w, int h);
	void PushMatrix();
	void PopMatrix();
	void Translatef(float x, float y, float z);
	void Scalef(float x, float y, float z);
	void Rotatef(float angle, float x, float y, float z);
	void ClearColorf(float r, float g, float b, float a);
	void ClearColorBuffer();
	void SetGLTexture2DEnable(bool enable);
	void SetGLTexCoordArrayEnable(bool enable);
	void SetGLColorArrayEnable(bool enable);
	void GetModelViewMatrix(float* pMatrix);
	void SetGLLineSmoothEnable(bool enable);
	void GLColor4f(float r, float g, float b, float a);
	void SetGLBlendEnable(bool enable);
	void SetGLBlendFunc(enBlendType blendType);
	void ResetGLBlendFunc();
	void SubImageToTexture(UINT textureID, int xoffset, int yoffset, int width, int height, bool isgrey, const void* pixels);
	void DrawFan(int posX, int posY, int startangle, int endangle, int radius, int color, float scale);

	void SetVertex2f(float x, float y);
	void SetTexCoord2f(float x, float y);
	void SetColor4f(Color4f color);
	void SetRenderType(GL_RENDER_TYPE prim);
	void SetLineWidth(int linewidth);
	void RenderBegin();
	void RenderFlush();

	void BindTexture(UINT texture, bool isLinearFilter);
	void SetTexEnviMode(int mode);

	///----------------------------------------------------------------------
	/// 重置当前记录的材质ID
	///----------------------------------------------------------------------
	void ResetCurTextureID();

	void SetTriangleIndex(unsigned short triangleIdx);

protected:
	void InitOpenGLES(int screenwidth, int screenheight);

	/***********************************************************************************************
	GLES2.0相关代码
	***********************************************************************************************/
#ifdef __USE_SHADER__
	void loadOrthoMatrix(float left, float right, float bottom, float top, float near, float far);
	void initShaders();
#endif

private:
	UINT m_CurrentTextureID;
	int m_CurrentVertex;

	bool m_CanUseVBO;
	short m_QuadIndexes[MAX_GL_BUFFER_ARRAY_SIZE * 3 / 2];
	UINT m_QuadIndexBufferID;

	TextureColoredVertex m_IVABuff[MAX_GL_BUFFER_ARRAY_SIZE];
	UINT m_RenderType;
	UINT                m_verticesID;
	int                   m_length;

	unsigned short* m_TriangleIndex;
	int m_CurrentTriangleID;
	/**********************************************************
	@ vars for draw fan
	**********************************************************/
	float m_fanvertice[2 * 361];

#ifdef __USE_SHADER__

	float orthographicMatrix[16];

	CShaderProgram* m_currentShader;

	CShaderCommon* m_commonShader;
	CShaderPointSprite* m_pointSpriteShader;
	CShaderStroke* m_strokeShader;
#endif

#ifdef __USE_TEXTURE_INFO__
	map<UINT, pTEXTURE_INFO> m_textureInfomation;
#endif

};

#endif
