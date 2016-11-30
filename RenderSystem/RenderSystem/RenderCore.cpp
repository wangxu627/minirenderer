#include "RenderCore.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

//#include "GameMath.h"
#ifdef __USE_SHADER__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "Shader/ShaderProgram.h"

#include "Shader/ShaderCommon.h"
#include "Shader/ShaderPointSprite.h"
#include "Shader/ShaderStroke.h"
#else
#include "OpenGLES/GLES/gl.h"
#include "OpenGLES/GLES/glext.h"
#endif

//#include "Framework.h"

CRenderCore::CRenderCore(int screenWidth, int screenHeight)
{
	Initialize(screenWidth, screenHeight);
}

void CRenderCore::Initialize(int screenWidth, int screenHeight)
{
	InitOpenGLES(screenWidth, screenHeight);
	memset(m_IVABuff, 0, sizeof(TextureColoredVertex)*MAX_GL_BUFFER_ARRAY_SIZE);
	m_CanUseVBO = false;
	InitRenderCore();
	m_RenderType = GL_QUADS;
	glGenBuffers(1, &m_verticesID);
	m_length = 0;

	memset(m_fanvertice, 0, sizeof(m_fanvertice));

	m_TriangleIndex = new unsigned short[MAX_GL_BUFFER_ARRAY_SIZE];
}

void CRenderCore::InitOpenGLES(int screenwidth, int screenheight)
{
#ifndef __USE_SHADER__
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	glOrthof(0, (float)screenwidth, (float)screenheight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_ALPHA_TEST);
#endif

	glEnable(GL_TEXTURE_2D);
	glViewport(0, 0, screenwidth, screenheight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef __USE_SHADER__
	initShaders();
	loadOrthoMatrix(0, (float)screenwidth, (float)screenheight, 0, -1, 1);
#endif
}

CRenderCore::~CRenderCore()
{
	glDeleteBuffers(1, &m_verticesID);

	SAFE_DELETE_ARRAY(m_TriangleIndex);
}

bool CRenderCore::IsExtensionSupported(char* szTargetExtension)
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *)strchr(szTargetExtension, ' ');
	if (pszWhere || *szTargetExtension == '\0')
		return false;

	// Get Extensions String
	pszExtensions = glGetString(GL_EXTENSIONS);

	//LOG("gl extension is %s", pszExtensions);

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for (;;)
	{
		pszWhere = (unsigned char *)strstr((const char *)pszStart, szTargetExtension);
		if (!pszWhere)
			break;
		pszTerminator = pszWhere + strlen(szTargetExtension);
		if (pszWhere == pszStart || *(pszWhere - 1) == ' ')
			if (*pszTerminator == ' ' || *pszTerminator == '\0')
				return true;
		pszStart = pszTerminator;
	}
	return false;
}

void CRenderCore::InitRenderCore()
{
	for (int i = 0; i < MAX_GL_BUFFER_ARRAY_SIZE * 3 / 2; i += 6)
	{
		int q = i / 6 * 4;

		m_QuadIndexes[i + 0] = q + 0;
		m_QuadIndexes[i + 1] = q + 1;
		m_QuadIndexes[i + 2] = q + 2;

		m_QuadIndexes[i + 3] = q + 0;
		m_QuadIndexes[i + 4] = q + 2;
		m_QuadIndexes[i + 5] = q + 3;
	}
	SetImmediateModeGLVertexArrays();
}

void CRenderCore::SetImmediateModeGLVertexArrays()
{
	m_CurrentTextureID = -1;
	m_CurrentVertex = 0;
	m_CurrentTriangleID = 0;

	if (m_CanUseVBO)
	{
		glGenBuffers(1, &m_QuadIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_GL_BUFFER_ARRAY_SIZE * 3 / 2 * sizeof(short), m_QuadIndexes, GL_STATIC_DRAW);
	}

#ifdef __USE_SHADER__
	m_currentShader = m_commonShader->use();

	m_currentShader->VertexAttribPointer(ARRAY_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(TextureColoredVertex), &m_IVABuff[0].geometryVertex);
	m_currentShader->EnableVertexAttribArray(ARRAY_POSITION, true);
	m_currentShader->VertexAttribPointer(ARRAY_TEXTURECOORD, 2, GL_FLOAT, GL_FALSE, sizeof(TextureColoredVertex), &m_IVABuff[0].textureVertex);
	m_currentShader->EnableVertexAttribArray(ARRAY_TEXTURECOORD, true);
	m_currentShader->VertexAttribPointer(ARRAY_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(TextureColoredVertex), &m_IVABuff[0].vertexColor);
	m_currentShader->EnableVertexAttribArray(ARRAY_COLOR, false);
	m_currentShader->SetRenderTexture(true);
	m_currentShader->SetRenderColor(false);
	m_currentShader->SetRenderMode();
#else
	glVertexPointer(2, GL_FLOAT, sizeof(TextureColoredVertex), &m_IVABuff[0].geometryVertex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TextureColoredVertex), &m_IVABuff[0].textureVertex);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(TextureColoredVertex), &m_IVABuff[0].vertexColor);
	glEnableClientState(GL_COLOR_ARRAY);
#endif
}

UINT CRenderCore::CreateTexture(stImage* image)
{
	GLuint texId = 0;
	glGenTextures(1, &texId);
	BindTexture(texId, false);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//处理灰度图与RGBA图像
	if (image->attrib == imageAttribGrey)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, image->tex_w, image->tex_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image->data);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->tex_w, image->tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	}

#ifdef __USE_TEXTURE_INFO__
	pTEXTURE_INFO pTextureInfo = new stTextureInfo;
	pTextureInfo->m_textureWidth = (float)image->tex_w;
	pTextureInfo->m_textureHeight = (float)image->tex_h;
	m_textureInfomation[texId] = pTextureInfo;
#endif

	return texId;
}

void CRenderCore::DeleteTexture(UINT textureID)
{
	GLuint textID = textureID;

	if (textID > 0)
	{
		glDeleteTextures(1, &textID);

		glFlush();
	}
#ifdef __USE_TEXTURE_INFO__
	map<UINT, pTEXTURE_INFO>::iterator it = m_textureInfomation.find(textID);
	if (it == m_textureInfomation.end())
		return;

	pTEXTURE_INFO pData = (*it).second;
	SAFE_DELETE(pData);
	m_textureInfomation.erase(textID);
#endif
}

bool CRenderCore::IsTextrue(UINT textureID)
{
	return glIsTexture(textureID);
}

void CRenderCore::SetGLScissorEnable(bool enable)
{
	if (enable)
	{
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

void CRenderCore::SetGLScissorArea(int x, int y, int w, int h)
{
	if (w < 0
		|| h < 0
		)
	{
		return;
	}

	glScissor(x, y, w, h);
}

void CRenderCore::PushMatrix()
{
#ifndef __USE_SHADER__
	glPushMatrix();
#endif
}

void CRenderCore::PopMatrix()
{
#ifndef __USE_SHADER__
	glPopMatrix();
#endif
}

void CRenderCore::Translatef(float x, float y, float z)
{
#ifndef __USE_SHADER__
	glTranslatef(x, y, z);
#endif
}

void CRenderCore::Scalef(float x, float y, float z)
{
#ifndef __USE_SHADER__
	glScalef(x, y, z);
#endif
}

void CRenderCore::Rotatef(float angle, float x, float y, float z)
{
#ifndef __USE_SHADER__
	glRotatef(angle, x, y, z);
#endif
}

void CRenderCore::ClearColorf(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void CRenderCore::ClearColorBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void CRenderCore::SetGLTexture2DEnable(bool enable)
{
	if (enable)
	{
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
}

void CRenderCore::SetGLTexCoordArrayEnable(bool enable)
{
	if (enable)
	{
#ifdef __USE_SHADER__
		m_currentShader->EnableVertexAttribArray(ARRAY_TEXTURECOORD, true);
		m_currentShader->SetRenderTexture(true);
		m_currentShader->SetRenderMode();
#else
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	}
	else
	{
#ifdef __USE_SHADER__
		m_currentShader->EnableVertexAttribArray(ARRAY_TEXTURECOORD, false);
		m_currentShader->SetRenderTexture(false);
		m_currentShader->SetRenderMode();
#else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	}
}

void CRenderCore::GetModelViewMatrix(float* pMatrix)
{
	glGetFloatv(GL_MODELVIEW_MATRIX, pMatrix);
}

void CRenderCore::SetGLColorArrayEnable(bool enable)
{
	if (enable)
	{
#ifdef __USE_SHADER__
		m_currentShader->EnableVertexAttribArray(ARRAY_COLOR, true);
		m_currentShader->SetRenderColor(true);
		m_currentShader->SetRenderMode();
#else
		glEnableClientState(GL_COLOR_ARRAY);
#endif
	}
	else
	{
#ifdef __USE_SHADER__
		m_currentShader->EnableVertexAttribArray(ARRAY_COLOR, false);
		m_currentShader->SetRenderColor(false);
		m_currentShader->SetRenderMode();
#else
		glDisableClientState(GL_COLOR_ARRAY);
#endif
	}
}

void CRenderCore::SetGLLineSmoothEnable(bool enable)
{
	if (enable)
	{
#ifndef __USE_SHADER__
		glEnable(GL_LINE_SMOOTH);
#endif
	}
	else
	{
#ifndef __USE_SHADER__
		glDisable(GL_LINE_SMOOTH);
#endif
	}
}

void CRenderCore::GLColor4f(float r, float g, float b, float a)
{
#ifdef __USE_SHADER__
	m_currentShader->SetSourceColor(r, g, b, a);
	if (r == 1.0 && g == 1.0 && b == 1.0 && a == 1.0)
	{
		m_currentShader->SetRenderColor(false);
	}
	else
	{
		m_currentShader->SetRenderColor(true);
	}
	m_currentShader->SetRenderMode();
#else
	glColor4f(r, g, b, a);
#endif
}

void CRenderCore::SetGLBlendEnable(bool enable)
{
	if (enable)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void CRenderCore::SetGLBlendFunc(enBlendType blendType)
{
	switch (blendType)
	{
	case enNormalBlend:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case enBrightBlend:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	default:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void CRenderCore::ResetGLBlendFunc()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CRenderCore::SubImageToTexture(UINT textureID, int xoffset, int yoffset, int width, int height, bool isgrey, const void* pixels)
{
	BindTexture(textureID, false);
	if (isgrey)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}
	else
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}
}

void CRenderCore::DrawFan(int posX, int posY, int startangle, int endangle, int radius, int color, float scale)
{
	RenderFlush();

	SetGLTexture2DEnable(false);
	SetGLTexCoordArrayEnable(false);
	SetGLColorArrayEnable(false);

	float xx = (float)posX;
	float yy = (float)posY;
	float ra = (float)radius;

	if (scale != 1.0f)
	{
		xx *= scale;
		yy *= scale;
		ra *= scale;
	}

	m_fanvertice[0] = xx;
	m_fanvertice[1] = yy;

	for (int i = startangle; i < endangle; i++)
	{
		float x = (ra * cosf(GetDegrees2Radians((float)(i - 90))));
		float y = (ra * sinf(GetDegrees2Radians((float)(i - 90))));

		m_fanvertice[(i - startangle) * 2 + 2 + 0] = xx + x;
		m_fanvertice[(i - startangle) * 2 + 2 + 1] = yy + y;
	}

#ifdef __USE_SHADER__
	m_currentShader->VertexAttribPointer(ARRAY_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT), m_fanvertice);
#else
	glVertexPointer(2, GL_FLOAT, 0, m_fanvertice);
#endif

	float rr = (float)((color >> 24) & 0xff) / 255;
	float gg = (float)((color >> 16) & 0xff) / 255;
	float bb = (float)((color >> 8) & 0xff) / 255;
	float aa = (float)((color) & 0xff) / 255;
	GLColor4f(rr, gg, bb, aa);

	glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(endangle - startangle + 1));

	SetGLTexture2DEnable(true);

	SetImmediateModeGLVertexArrays();

	GLColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void CRenderCore::SetVertex2f(GLfloat x, GLfloat y)
{
	if (m_CurrentVertex >= MAX_GL_BUFFER_ARRAY_SIZE)
	{
		return;
	}

	m_IVABuff[m_CurrentVertex].geometryVertex.x = x;
	m_IVABuff[m_CurrentVertex].geometryVertex.y = y;
	m_CurrentVertex++;
}

void CRenderCore::SetTexCoord2f(GLfloat x, GLfloat y)
{
	if (m_CurrentVertex >= MAX_GL_BUFFER_ARRAY_SIZE)
	{
		return;
	}

	m_IVABuff[m_CurrentVertex].textureVertex.x = x;
	m_IVABuff[m_CurrentVertex].textureVertex.y = y;
}

void CRenderCore::SetColor4f(Color4f color)
{
	if (m_CurrentVertex >= MAX_GL_BUFFER_ARRAY_SIZE)
	{
		return;
	}

	m_IVABuff[m_CurrentVertex].vertexColor = color;
}

void CRenderCore::SetRenderType(GL_RENDER_TYPE prim)
{
	if (prim != m_RenderType)
	{
		RenderFlush();
	}
	switch (prim)
	{
	case GL_RENDER_TYPE_QUADS:
	{
		m_RenderType = GL_QUADS;
	}
	break;
	case GL_RENDER_TYPE_LINE_LOOP:
	{
		m_RenderType = GL_LINE_LOOP;
	}
	break;
	case GL_RENDER_TYPE_LINE:
	{
		m_RenderType = GL_LINES;
	}
	break;
	case GL_RENDER_TYPE_TRIANGLES:
	{
		m_RenderType = GL_RENDER_TYPE_TRIANGLES;
	}
	break;
	default:
	{
		m_RenderType = GL_QUADS;
	}
	}
}

void CRenderCore::SetLineWidth(int linewidth)
{
	glLineWidth(linewidth);
}

void CRenderCore::RenderBegin()
{
	if (m_CurrentVertex > 0)
	{
		RenderFlush();
	}
	m_CurrentVertex = 0;
}

void CRenderCore::RenderFlush()
{
	if (m_CurrentVertex > 0)
	{
		if (m_RenderType == GL_QUADS)
		{
			if (m_CanUseVBO && m_QuadIndexBufferID != 0)
			{
				glDrawElements(GL_TRIANGLES, m_CurrentVertex / 4 * 6, GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				glDrawElements(GL_TRIANGLES, m_CurrentVertex / 4 * 6, GL_UNSIGNED_SHORT, m_QuadIndexes);
			}
		}
		else if (m_RenderType == GL_RENDER_TYPE_TRIANGLES)
		{
			glDrawElements(GL_TRIANGLES, m_CurrentTriangleID, GL_UNSIGNED_SHORT, m_TriangleIndex);
		}
		else
		{
			glDrawArrays(m_RenderType, 0, m_CurrentVertex);
		}
	}

	m_CurrentVertex = 0;
	m_CurrentTriangleID = 0;
	m_RenderType = GL_QUADS;
}

void CRenderCore::BindTexture(GLuint texture, bool isLinearFilter)
{
	if (m_CurrentTextureID != texture)
	{
		RenderFlush();
#ifdef __USE_SHADER__
		glActiveTexture(GL_TEXTURE0);
#endif
		glBindTexture(GL_TEXTURE_2D, texture);
#ifdef __USE_SHADER__
		m_currentShader->SetTextureUniform(0);
#endif
		m_CurrentTextureID = texture;

		if (isLinearFilter)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}
}

void CRenderCore::SetTexEnviMode(int mode)
{
	switch (mode)
	{
	case TEXENVI_MODULATE:
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
	break;
	case TEXENVI_ADD:
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	}
	break;
	}
}

void CRenderCore::ResetCurTextureID()
{
	m_CurrentTextureID = -1;
}

void CRenderCore::SetTriangleIndex(unsigned short triangleIdx)
{
	if (m_CurrentTriangleID >= MAX_GL_BUFFER_ARRAY_SIZE)
	{
		return;
	}

	m_TriangleIndex[m_CurrentTriangleID] = triangleIdx + m_CurrentVertex;
	m_CurrentTriangleID++;
}

/***********************************************************************************************
GLES2.0相关代码
***********************************************************************************************/
#ifdef __USE_SHADER__

void CRenderCore::loadOrthoMatrix(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
	GLfloat r_l = right - left;
	GLfloat t_b = top - bottom;
	GLfloat f_n = far - near;
	GLfloat tx = -(right + left) / (right - left);
	GLfloat ty = -(top + bottom) / (top - bottom);
	GLfloat tz = -(far + near) / (far - near);

	orthographicMatrix[0] = 2.0f / r_l;
	orthographicMatrix[1] = 0.0f;
	orthographicMatrix[2] = 0.0f;
	orthographicMatrix[3] = tx;

	orthographicMatrix[4] = 0.0f;
	orthographicMatrix[5] = 2.0f / t_b;
	orthographicMatrix[6] = 0.0f;
	orthographicMatrix[7] = ty;

	orthographicMatrix[8] = 0.0f;
	orthographicMatrix[9] = 0.0f;
	orthographicMatrix[10] = 2.0f / f_n;
	orthographicMatrix[11] = tz;

	orthographicMatrix[12] = 0.0f;
	orthographicMatrix[13] = 0.0f;
	orthographicMatrix[14] = 0.0f;
	orthographicMatrix[15] = 1.0f;

	m_currentShader = m_commonShader->use();
	glUniformMatrix4fv(m_currentShader->GetProjectUniform(), 1, 0, orthographicMatrix);
	m_currentShader = m_pointSpriteShader->use();
	glUniformMatrix4fv(m_currentShader->GetProjectUniform(), 1, 0, orthographicMatrix);
}

void CRenderCore::initShaders()
{
	m_currentShader = NULL;
	m_commonShader = new CShaderCommon();
	m_pointSpriteShader = new CShaderPointSprite();
	//m_strokeShader = new CShaderStroke();
}

#endif