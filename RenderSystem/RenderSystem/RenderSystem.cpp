

#include "RenderSystem.h"
#include "RenderCore.h"
#include "TextureInfo.h"
//#include "Framework.h"

const static int MAX_VERTICE_NODE_PER_COMPUTE_COUNT = 4096;

//void CreateRenderSystem(CRenderSystem** ppAbstract)
//{
//	CRenderSystem* p = new CRenderSystem;
//	*ppAbstract = p;
//}
//
//void DestroyRenderSystem(CRenderSystem* pAbstract)
//{
//	CRenderSystem* p = static_cast<CRenderSystem*>(pAbstract);
//	SAFE_DELETE(p);
//	pAbstract = NULL;
//}

CRenderSystem* CRenderSystem::_instance = NULL;

CRenderSystem* CRenderSystem::GetInstance()
{
	if (_instance == NULL)
	{
		_instance = new CRenderSystem;;
	}
	return _instance;
}

CRenderSystem::CRenderSystem(void)
{
	m_GLParam.m_pContext = NULL;
	m_GLParam.m_Width = 0;
	m_GLParam.m_Height = 0;

	m_IsCliped = false;
	m_ClipX = 0;
	m_ClipY = 0;
	m_ClipWidth = 0;
	m_ClipHeight = 0;

	m_defaultLineWidth = 1;

	m_texenviMode = TEXENVI_MODULATE;
	m_OldtexenviMode = TEXENVI_MODULATE;

	m_RenderCore = NULL;

	m_bIsRenderStyle = false;

	m_bFixArtifacts = true;

	memset(m_matrix, 0, sizeof(float) * 16);

	m_stFinalPoint = new ST_POINTF[MAX_VERTICE_NODE_PER_COMPUTE_COUNT];
	m_nFillRectTextureID = 0;
}

CRenderSystem::~CRenderSystem(void)
{
	SAFE_DELETE(m_RenderCore);

	SAFE_DELETE_ARRAY(m_stFinalPoint);
}

void CRenderSystem::Initialize(void* pContextParam)
{

	//------------------------------------------------------------------
	stContextParam* pParam = static_cast<stContextParam*>(pContextParam);
	m_GLParam.m_pContext = pParam->m_pContext;
	m_GLParam.m_Width = pParam->m_Width;
	m_GLParam.m_Height = pParam->m_Height;
	//m_ClipWidth = CFramework::GetInstance()->GetScreenWidth();
	//m_ClipHeight = CFramework::GetInstance()->GetScreenHeight();
	m_ClipWidth = 800;
	m_ClipHeight = 600;

	m_RenderCore = new CRenderCore(m_GLParam.m_Width, m_GLParam.m_Height);
}

void CRenderSystem::LastInitialize()
{
	CreateWhiteBufferTexture();
}

void CRenderSystem::ReleaseWhiteTextureBuffer()
{
	if (m_nFillRectTextureID > 0)
	{
		DeleteTexture(m_nFillRectTextureID);
		m_nFillRectTextureID = 0;
	}
}

void CRenderSystem::CreateWhiteBufferTexture()
{
	if (m_nFillRectTextureID > 0)
	{
		return;
	}

	int bufferW = 0;
	int bufferH = 0;

	stImage whiteImg;
	whiteImg.w = 64;
	whiteImg.h = 64;
	whiteImg.tex_w = 64;
	whiteImg.tex_h = 64;
	whiteImg.pitch = 64 * 4;
	whiteImg.attrib = imageAttrib8888;
	whiteImg.data = (unsigned char*)malloc(64 * 64 * 4);
	memset(whiteImg.data, 0xFF, 64 * 64 * 4 * sizeof(unsigned char));

	m_nFillRectTextureID = CreateTexture(&whiteImg);

	m_vFillRectTransform.m_TextureWidth = bufferW;
	m_vFillRectTransform.m_TextureHeight = bufferH;
	m_vFillRectTransform.m_TexCoordWidth = bufferW;
	m_vFillRectTransform.m_TexCoordHeight = bufferH;
	m_vFillRectTransform.m_TexCoordX = 0;
	m_vFillRectTransform.m_TexCoordY = 0;
}

void CRenderSystem::Dispose()
{
	ReleaseWhiteTextureBuffer();
}

void CRenderSystem::SetFixArtifacts(bool bVar)
{
	m_bFixArtifacts = bVar;
}

void CRenderSystem::DrawTexture(CTextureInfo* textureInfo, stDisplayTransform* pTransform)
{
	if (textureInfo == NULL)
		return;

	UINT textureId = textureInfo->getTextureID();

	if (textureId > 0
		&& textureInfo->IsResReady()
		)
	{
		DrawTexture(textureId, pTransform, textureInfo->IsReadInAPK());
	}
}

void CRenderSystem::DrawTexture(UINT textrueID, stDisplayTransform* pTransform1, bool bReadInAPK)
{
	if (textrueID <= 0)
	{
		return;
	}

	//memcpy(&m_tempDT, pTransform1, sizeof(stDisplayTransform));
	stDisplayTransform* pTransform = pTransform1;
	UINT texID = (UINT)textrueID;

	//float scaleValue = CFramework::GetInstance()->GetScreenScale();
	float scaleValue = 1;

	if (scaleValue != 1.0f
		|| (int)m_vMatrixTransform.size() > 0
		)
	{
		pTransform->m_useLinearBind = true;
	}

	m_RenderCore->BindTexture(texID, pTransform->m_useLinearBind);

	float coordX = (float)(pTransform->m_TexCoordX);
	float coordY = (float)(pTransform->m_TexCoordY);
	float coordW = (float)(pTransform->m_TexCoordWidth);
	float coordH = (float)(pTransform->m_TexCoordHeight);
	float TextureW = (float)(pTransform->m_TextureWidth);
	float TextureH = (float)(pTransform->m_TextureHeight);

#ifdef FIX_ARTIFACTS_BY_STRECHING_TEXEL

	float TexCoordX1 = (2 * coordX + 1) / (2 * TextureW);
	float TexCoordY1 = (2 * coordY + 1) / (2 * TextureH);
	float TexCoordX2 = TexCoordX1 + (coordW * 2 - 2) / (2 * TextureW);
	float TexCoordY2 = TexCoordY1 + (coordH * 2 - 2) / (2 * TextureH);

	if (m_bFixArtifacts)
	{
		if (scaleValue != 1.0f
			|| (int)m_vMatrixTransform.size() > 0)
		{
			TexCoordX1 = (2 * coordX + 2) / (2 * TextureW);
			TexCoordY1 = (2 * coordY + 2) / (2 * TextureH);
			TexCoordX2 = TexCoordX1 + (coordW * 2 - 4) / (2 * TextureW);
			TexCoordY2 = TexCoordY1 + (coordH * 2 - 4) / (2 * TextureH);
		}
	}
#else
	float TexCoordX1 = coordX / TextureW;
	float TexCoordY1 = coordY / TextureH;
	float TexCoordX2 = (coordX + coordW) / TextureW;
	float TexCoordY2 = (coordY + coordH) / TextureH;
#endif

	// 变形
	pTransform->m_TLVertexX = pTransform->m_TLVertexX + m_stTLGradient.x;
	pTransform->m_TLVertexY = pTransform->m_TLVertexY + m_stTLGradient.y;

	pTransform->m_TRVertexX = pTransform->m_TRVertexX + m_stTRGradient.x;
	pTransform->m_TRVertexY = pTransform->m_TRVertexY + m_stTRGradient.y;

	pTransform->m_BLVertexX = pTransform->m_BLVertexX + m_stBLGradient.x;
	pTransform->m_BLVertexY = pTransform->m_BLVertexY + m_stBLGradient.y;

	pTransform->m_BRVertexX = pTransform->m_BRVertexX + m_stBRGradient.x;
	pTransform->m_BRVertexY = pTransform->m_BRVertexY + m_stBRGradient.y;

	float VertexX1 = pTransform->m_TLVertexX * scaleValue;
	float VertexY1 = pTransform->m_TLVertexY * scaleValue;

	float VertexX2 = pTransform->m_TRVertexX * scaleValue;
	float VertexY2 = pTransform->m_TRVertexY * scaleValue;

	float VertexX3 = pTransform->m_BLVertexX * scaleValue;
	float VertexY3 = pTransform->m_BLVertexY * scaleValue;

	float VertexX4 = pTransform->m_BRVertexX * scaleValue;
	float VertexY4 = pTransform->m_BRVertexY * scaleValue;

	switch (pTransform->m_Flag)
	{
	case TRANS_MIRROR:
	{
		m_stFinalPoint[0].x = VertexX1;
		m_stFinalPoint[0].y = VertexY1;
		m_stFinalPoint[1].x = VertexX2;
		m_stFinalPoint[1].y = VertexY2;
		m_stFinalPoint[2].x = VertexX4;
		m_stFinalPoint[2].y = VertexY4;
		m_stFinalPoint[3].x = VertexX3;
		m_stFinalPoint[3].y = VertexY3;
	}
	break;

	case TRANS_MIRROR_ROT180:
	case TRANS_ROT180:
	{
		m_stFinalPoint[0].x = VertexX4;
		m_stFinalPoint[0].y = VertexY4;
		m_stFinalPoint[1].x = VertexX3;
		m_stFinalPoint[1].y = VertexY3;
		m_stFinalPoint[2].x = VertexX1;
		m_stFinalPoint[2].y = VertexY1;
		m_stFinalPoint[3].x = VertexX2;
		m_stFinalPoint[3].y = VertexY2;
	}
	break;

	case TRANS_ROT90:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_stFinalPoint[0].x = VertexX1 + vertexHeight;
		m_stFinalPoint[0].y = VertexY1;
		m_stFinalPoint[1].x = VertexX1 + vertexHeight;
		m_stFinalPoint[1].y = VertexY1 + vertexWidth;
		m_stFinalPoint[2].x = VertexX1;
		m_stFinalPoint[2].y = VertexY1 + vertexWidth;
		m_stFinalPoint[3].x = VertexX1;
		m_stFinalPoint[3].y = VertexY1;
	}
	break;

	case TRANS_MIRROR_ROT90:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_stFinalPoint[0].x = VertexX1 + vertexHeight;
		m_stFinalPoint[0].y = VertexY1 + vertexWidth;
		m_stFinalPoint[1].x = VertexX1 + vertexHeight;
		m_stFinalPoint[1].y = VertexY1;
		m_stFinalPoint[2].x = VertexX1;
		m_stFinalPoint[2].y = VertexY1;
		m_stFinalPoint[3].x = VertexX1;
		m_stFinalPoint[3].y = VertexY1 + vertexWidth;
	}
	break;

	case TRANS_MIRROR_ROT270:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_stFinalPoint[0].x = VertexX1;
		m_stFinalPoint[0].y = VertexY1;
		m_stFinalPoint[1].x = VertexX1;
		m_stFinalPoint[1].y = VertexY1 + vertexWidth;
		m_stFinalPoint[2].x = VertexX1 + vertexHeight;
		m_stFinalPoint[2].y = VertexY1 + vertexWidth;
		m_stFinalPoint[3].x = VertexX1 + vertexHeight;
		m_stFinalPoint[3].y = VertexY1;
	}
	break;

	case TRANS_ROT270:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_stFinalPoint[0].x = VertexX1;
		m_stFinalPoint[0].y = VertexY1 + vertexWidth;
		m_stFinalPoint[1].x = VertexX1;
		m_stFinalPoint[1].y = VertexY1;
		m_stFinalPoint[2].x = VertexX1 + vertexHeight;
		m_stFinalPoint[2].y = VertexY1;
		m_stFinalPoint[3].x = VertexX1 + vertexHeight;
		m_stFinalPoint[3].y = VertexY1 + vertexWidth;
	}
	break;

	default:
	{
		m_stFinalPoint[0].x = VertexX1;
		m_stFinalPoint[0].y = VertexY1;
		m_stFinalPoint[1].x = VertexX2;
		m_stFinalPoint[1].y = VertexY2;
		m_stFinalPoint[2].x = VertexX4;
		m_stFinalPoint[2].y = VertexY4;
		m_stFinalPoint[3].x = VertexX3;
		m_stFinalPoint[3].y = VertexY3;
	}
	}

	ExcuteGlobalMatrixTrans();

	//if (g_colorOverlay != 0xffffffff)
	//{
	//	float r = (g_colorOverlay >> 24 & 0xff) / 255.0f;
	//	float g = (g_colorOverlay >> 16 & 0xff) / 255.0f;
	//	float b = (g_colorOverlay >> 8 & 0xff) / 255.0f;
	//	float a = (g_colorOverlay & 0xff) / 255.0f;
	//	if (GetTexEnvidMode() == TEXENVI_ADD)
	//	{
	//		for (int i = 0; i < 4; i++)
	//		{
	//			pTransform->m_Color[i].red = r;
	//			pTransform->m_Color[i].green = g;
	//			pTransform->m_Color[i].blue = b;
	//			pTransform->m_Color[i].alpha *= a;
	//		}
	//	}
	//	else
	//	{
	//		for (int i = 0; i < 4; i++)
	//		{
	//			pTransform->m_Color[i].red *= r;
	//			pTransform->m_Color[i].green *= g;
	//			pTransform->m_Color[i].blue *= b;
	//			pTransform->m_Color[i].alpha *= a;
	//		}
	//	}
	//}

	switch (pTransform->m_Flag)
	{
		//FlipX
	case TRANS_MIRROR:
	{
		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//FlipY
	case TRANS_MIRROR_ROT180:
	{
		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//FlipXY
	case TRANS_ROT180:
	{
		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//ROT90
	case TRANS_ROT90:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//TRANS_MIRROR_ROT90
	case TRANS_MIRROR_ROT90:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//TRANS_MIRROR_ROT270
	case TRANS_MIRROR_ROT270:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//TRANS_ROT270
	case TRANS_ROT270:
	{
		float vertexWidth = pTransform->m_TRVertexX - pTransform->m_TLVertexX;
		float vertexHeight = pTransform->m_BLVertexY - pTransform->m_TLVertexY;

		vertexWidth *= scaleValue;
		vertexHeight *= scaleValue;

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	//No Flip
	default:
	{
		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[0]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY1);
		m_RenderCore->SetColor4f(pTransform->m_Color[1]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);

		m_RenderCore->SetTexCoord2f(TexCoordX2, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[3]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);

		m_RenderCore->SetTexCoord2f(TexCoordX1, TexCoordY2);
		m_RenderCore->SetColor4f(pTransform->m_Color[2]);
		m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);
	}
	break;
	}
}

UINT CRenderSystem::CreateTexture(stImage* image)
{
	return m_RenderCore->CreateTexture(image);
}

void CRenderSystem::DeleteTexture(UINT textureID)
{
	m_RenderCore->DeleteTexture(textureID);
}

void CRenderSystem::SetClip(int x, int y, int w, int h)
{
	if (w < 0 || h < 0)
	{
		return;
	}

	if (x != m_ClipX || y != m_ClipY || w != m_ClipWidth || h != m_ClipHeight)
	{
		m_RenderCore->RenderFlush();

		m_RenderCore->SetGLScissorEnable(true);

		float posX = (float)x;
		float posY = (float)y;
		float width = (float)w;
		float height = (float)h;

		float scaleValue = 1.0f;
		if (scaleValue != 1.0f)
		{
			posX = posX * scaleValue;
			posY = posY * scaleValue;

			width = width * scaleValue;
			height = height * scaleValue;
		}

		if (posX < 0)
		{
			width += posX;
			posX = 0;
		}

		if (posY < 0)
		{
			height += posY;
			posY = 0;
		}

		if (posX + width > m_GLParam.m_Width)
		{
			width = m_GLParam.m_Width - posX;
		}

		if (posY + height > m_GLParam.m_Height)
		{
			height = m_GLParam.m_Height - posY;
		}

		m_RenderCore->SetGLScissorArea(posX, m_GLParam.m_Height - (posY + height), width, height);

		m_IsCliped = true;
		m_ClipX = x;
		m_ClipY = y;
		m_ClipWidth = w;
		m_ClipHeight = h;
	}
}

void CRenderSystem::ClipRect(int x, int y, int w, int h)
{
	int clipDestX1 = m_ClipX;
	int clipDestY1 = m_ClipY;
	int clipDestX2 = m_ClipX + m_ClipWidth;
	int clipDestY2 = m_ClipY + m_ClipHeight;

	if (m_ClipX < x)
	{
		clipDestX1 = x;
	}
	if (m_ClipY < y)
	{
		clipDestY1 = y;
	}
	if (m_ClipX + m_ClipWidth > x + w)
	{
		clipDestX2 = x + w;
	}
	if (m_ClipY + m_ClipHeight > y + h)
	{
		clipDestY2 = y + h;
	}

	int destW = clipDestX2 - clipDestX1;
	int destH = clipDestY2 - clipDestY1;

	if (destW > 0 && destH > 0)
	{
		SetClip(clipDestX1, clipDestY1, destW, destH);
	}
	else
	{
		SetClip(clipDestX1, clipDestY1, 0, 0);
	}
}

void CRenderSystem::SetClipFullScreen()
{
	if (m_IsCliped)
	{
		m_RenderCore->RenderFlush();
		m_RenderCore->SetGLScissorEnable(false);
		m_IsCliped = false;
	}

	m_ClipX = 0;
	m_ClipY = 0;
	m_ClipWidth = 800;
	m_ClipHeight = 600;
}

void CRenderSystem::GetClipArea(int &clipX, int &clipY, int &clipWidth, int &clipHeight)
{
	clipX = m_ClipX;
	clipY = m_ClipY;
	clipWidth = m_ClipWidth;
	clipHeight = m_ClipHeight;
}

void CRenderSystem::SetMatrixOperationStart(stMatrixOperation* pMatrixOperation)
{
	if (pMatrixOperation)
	{
		stMatrixOperation matrixOper = *(pMatrixOperation);
		m_vMatrixTransform.push_back(matrixOper);
	}
}

void CRenderSystem::ExcuteGlobalMatrixTrans(int verticeCount)
{
	int matrixCount = (int)m_vMatrixTransform.size();
	if (matrixCount <= 0)
	{
		return;
	}

	//g_colorOverlay = m_vMatrixTransform[matrixCount - 1].m_colorOverlay;

	for (int i = 0; i < verticeCount; i++)
	{
		for (int j = matrixCount - 1; j >= 0; j--)
		{
			ExcuteMatrixTrans(m_stFinalPoint[i], m_vMatrixTransform[j]);
		}
	}
}
void CRenderSystem::ExcuteMatrixTrans(ST_POINTF& stPoint, stMatrixOperation matrixOper)
{
	float scaleValue = 1.0f;

	if (matrixOper.m_hasTranslate)
	{
		stPoint.x += matrixOper.m_translateX * scaleValue;
		stPoint.y += matrixOper.m_translateY * scaleValue;
	}

	stPoint.x -= matrixOper.m_x * scaleValue;
	stPoint.y -= matrixOper.m_y * scaleValue;

	if (matrixOper.m_hasScale)
	{
		stPoint.x *= matrixOper.m_scaleX;
		stPoint.y *= matrixOper.m_scaleY;
	}

	if (matrixOper.m_hasRotate)
	{
		// 这里因为是2D，所以m_rotateX和m_rotateY都是0，只有m_rotateZ有意义
		if (matrixOper.m_rotateZ != 0)
		{
			float rotateVal = GetDegrees2Radians(matrixOper.m_rotateAngle);
			//MathRotate(stPoint, rotateVal);
		}
	}

	stPoint.x += matrixOper.m_x * scaleValue;
	stPoint.y += matrixOper.m_y * scaleValue;
}

void CRenderSystem::SetMatrixOperationOver()
{
	m_vMatrixTransform.pop_back();

	/*int size = (int)m_vMatrixTransform.size();
	if (size <= 0)
	{
		g_colorOverlay = 0xffffffff;
	}
	else
	{
		g_colorOverlay = m_vMatrixTransform[size - 1].m_colorOverlay;
	}*/
}

void CRenderSystem::FillRectAlpha(int x, int y, int w, int h, int color)
{
	if (IsOutOfRectArea(x, y, w, h))
		return;

	m_vFillRectTransform.m_TLVertexX = (float)x;
	m_vFillRectTransform.m_TLVertexY = (float)y;
	m_vFillRectTransform.m_TRVertexX = (float)(x + w);
	m_vFillRectTransform.m_TRVertexY = (float)y;
	m_vFillRectTransform.m_BLVertexX = (float)x;
	m_vFillRectTransform.m_BLVertexY = (float)(y + h);
	m_vFillRectTransform.m_BRVertexX = (float)(x + w);
	m_vFillRectTransform.m_BRVertexY = (float)(y + h);

	for (int i = 0; i < 4; i++)
	{
		m_vFillRectTransform.m_Color[i].red = (float)((color >> 24) & 0xff) / 255;
		m_vFillRectTransform.m_Color[i].green = (float)((color >> 16) & 0xff) / 255;
		m_vFillRectTransform.m_Color[i].blue = (float)((color >> 8) & 0xff) / 255;
		m_vFillRectTransform.m_Color[i].alpha = (float)((color) & 0xff) / 255;
	}

	DrawTexture(m_nFillRectTextureID, &m_vFillRectTransform);

	//float posX = x;
	//float posY = y;
	//float width = w;
	//float height = h;

	//float scaleValue = CFramework::GetInstance()->GetScreenScale();
	//if (scaleValue != 1.0f)
	//{
	//	posX = posX * scaleValue;
	//	posY = posY * scaleValue;

	//	width = width * scaleValue;
	//	height = height * scaleValue;
	//}

	//m_stFinalPoint[0].x = posX;
	//m_stFinalPoint[0].y = posY;
	//m_stFinalPoint[1].x = posX + width;
	//m_stFinalPoint[1].y = posY;
	//m_stFinalPoint[2].x = posX + width;
	//m_stFinalPoint[2].y = posY + height;
	//m_stFinalPoint[3].x = posX;
	//m_stFinalPoint[3].y = posY + height;

	//int matrixCount = (int)m_vMatrixTransform.size();
	//for (int i = 0; i < 4; i++)
	//{
	//	for (int j = matrixCount - 1; j >= 0; j--)
	//	{
	//		ExcuteMatrixTrans(m_stFinalPoint[i], m_vMatrixTransform[j]);
	//	}
	//}

	//m_RenderCore->RenderFlush();
	////纹理贴图是默认打开的，填充矩形时，需要将其先关闭，画完了再打开
	//m_RenderCore->SetGLTexture2DEnable(false);
	//m_RenderCore->SetGLTexCoordArrayEnable(false);
	//m_RenderCore->SetGLColorArrayEnable(false);

	//m_RenderCore->SetRenderType(GL_RENDER_TYPE_QUADS);
	//m_RenderCore->SetVertex2f(m_stFinalPoint[0].x, m_stFinalPoint[0].y);
	//m_RenderCore->SetVertex2f(m_stFinalPoint[1].x, m_stFinalPoint[1].y);
	//m_RenderCore->SetVertex2f(m_stFinalPoint[2].x, m_stFinalPoint[2].y);
	//m_RenderCore->SetVertex2f(m_stFinalPoint[3].x, m_stFinalPoint[3].y);

	//float rr = (float)((color >> 24) & 0xff) / 255;
	//float gg = (float)((color >> 16) & 0xff) / 255;
	//float bb = (float)((color >> 8) & 0xff) / 255;
	//float aa = (float)((color) & 0xff) / 255;
	//m_RenderCore->GLColor4f(rr, gg, bb, aa);

	//m_RenderCore->RenderFlush();

	//m_RenderCore->GLColor4f(1, 1, 1, 1);
	//m_RenderCore->SetGLTexture2DEnable(true);
	//m_RenderCore->SetGLTexCoordArrayEnable(true);
	//m_RenderCore->SetGLColorArrayEnable(true);
}

void CRenderSystem::DrawRect(int x, int y, int w, int h, int color)
{
	if (IsOutOfRectArea(x, y, w, h))
		return;

	float posX = (float)x;
	float posY = (float)y;
	float width = (float)w;
	float height = (float)h;

	float scaleValue = 1.0f;
	if (scaleValue != 1.0f)
	{
		posX = posX * scaleValue;
		posY = posY * scaleValue;

		width = width * scaleValue;
		height = height * scaleValue;
	}

	m_RenderCore->RenderFlush();

	//纹理贴图是默认打开的，画矩形时，需要将其先关闭，画完了再打开
	m_RenderCore->SetGLTexture2DEnable(false);
	m_RenderCore->SetGLTexCoordArrayEnable(false);
	m_RenderCore->SetGLColorArrayEnable(false);
	m_RenderCore->SetGLLineSmoothEnable(true);

	m_RenderCore->SetRenderType(GL_RENDER_TYPE_LINE_LOOP);
	m_RenderCore->SetVertex2f(posX, posY);
	m_RenderCore->SetVertex2f(posX + width, posY);
	m_RenderCore->SetVertex2f(posX + width, posY + height);
	m_RenderCore->SetVertex2f(posX, posY + height);
	float rr = (float)((color >> 24) & 0xff) / 255;
	float gg = (float)((color >> 16) & 0xff) / 255;
	float bb = (float)((color >> 8) & 0xff) / 255;
	float aa = (float)((color) & 0xff) / 255;
	m_RenderCore->GLColor4f(rr, gg, bb, aa);

	m_RenderCore->RenderFlush();
	m_RenderCore->GLColor4f(1, 1, 1, 1);
	m_RenderCore->SetGLTexture2DEnable(true);
	m_RenderCore->SetGLTexCoordArrayEnable(true);
	m_RenderCore->SetGLColorArrayEnable(true);
	m_RenderCore->SetGLLineSmoothEnable(false);
}

void CRenderSystem::DrawLine(int x1, int y1, int x2, int y2, int linewidth, int color)
{
	m_RenderCore->RenderFlush();

	//纹理贴图是默认打开的，画矩形时，需要将其先关闭，画完了再打开
	m_RenderCore->SetGLTexture2DEnable(false);
	m_RenderCore->SetGLTexCoordArrayEnable(false);
	m_RenderCore->SetGLColorArrayEnable(false);
	m_RenderCore->SetGLLineSmoothEnable(true);

	m_RenderCore->SetRenderType(GL_RENDER_TYPE_LINE);
	if (linewidth != m_defaultLineWidth)
		m_RenderCore->SetLineWidth(linewidth);

	float posX1 = (float)x1;
	float posY1 = (float)y1;
	float posX2 = (float)x2;
	float posY2 = (float)y2;

	float scaleValue = 1.0f;
	if (scaleValue != 1.0f)
	{
		posX1 = posX1 * scaleValue;
		posY1 = posY1 * scaleValue;

		posX2 = posX2 * scaleValue;
		posY2 = posY2 * scaleValue;
	}

	float rr = (float)((color >> 24) & 0xff) / 255;
	float gg = (float)((color >> 16) & 0xff) / 255;
	float bb = (float)((color >> 8) & 0xff) / 255;
	float aa = (float)((color) & 0xff) / 255;
	m_RenderCore->SetVertex2f(posX1, posY1);
	m_RenderCore->SetVertex2f(posX2, posY2);
	m_RenderCore->GLColor4f(rr, gg, bb, aa);

	m_RenderCore->RenderFlush();
	m_RenderCore->GLColor4f(1, 1, 1, 1);
	m_RenderCore->SetGLTexture2DEnable(true);
	m_RenderCore->SetGLTexCoordArrayEnable(true);
	m_RenderCore->SetGLColorArrayEnable(true);
	m_RenderCore->SetGLLineSmoothEnable(false);

	if (linewidth != m_defaultLineWidth)
	{
		m_RenderCore->SetLineWidth(m_defaultLineWidth);
	}
}

void CRenderSystem::DrawFan(int posX, int posY, int startangle, int endangle, int radius, int color)
{
	if (startangle < 0 || endangle < 0 || startangle > 360 || endangle > 360)
	{
		return;
	}

	if (startangle >= endangle)
	{
		return;
	}

	float scaleValue = 1.0f;
	m_RenderCore->DrawFan(posX, posY, startangle, endangle, radius, color, scaleValue);
}

void CRenderSystem::EndDisplay()
{
	m_RenderCore->RenderFlush();
	m_RenderCore->ResetCurTextureID();
}

void CRenderSystem::SetAlphaBlend(bool blend)
{
	m_RenderCore->SetGLBlendEnable(blend);
}

void CRenderSystem::SetBlendFunc(enBlendType blendType)
{
	m_RenderCore->SetGLBlendFunc(blendType);
}

void CRenderSystem::ReSetBlendFunc()
{
	m_RenderCore->ResetGLBlendFunc();
}

void CRenderSystem::SubImageToTexture(UINT textureID, int xoffset, int yoffset, int width, int height, bool isgrey, const void* pixels)
{
	m_RenderCore->SubImageToTexture(textureID, xoffset, yoffset, width, height, isgrey, pixels);
}

bool CRenderSystem::IsOutOfRectArea(int x, int y, int w, int h)
{
	if ((x < m_ClipX && x + w < m_ClipX)
		|| (y < m_ClipY && y + h < m_ClipY)
		|| (x > m_ClipX + m_ClipWidth)
		|| (y > m_ClipY + m_ClipHeight)
		)
		return true;
	return false;
}

void CRenderSystem::SetTexEnviMode(TEXENVI_MODE mode)
{
	if (m_texenviMode != mode)
	{
		m_RenderCore->RenderFlush();
		m_RenderCore->SetTexEnviMode(mode);
		m_texenviMode = mode;
	}
}

TEXENVI_MODE CRenderSystem::GetTexEnvidMode()
{
	return m_texenviMode;
}

void CRenderSystem::SetGlobalRenderParam(TEXENVI_MODE texMode, stMatrixOperation* pMatrixOperation)
{
	m_OldtexenviMode = m_texenviMode;

	m_texenviMode = texMode;

	if (m_texenviMode != m_OldtexenviMode)
	{
		m_RenderCore->RenderFlush();
		m_RenderCore->SetTexEnviMode(m_texenviMode);
	}

	SetMatrixOperationStart(pMatrixOperation);
}

void CRenderSystem::EndGlobalRenderParam()
{
	if (m_texenviMode != m_OldtexenviMode)
	{
		m_RenderCore->RenderFlush();
		m_RenderCore->SetTexEnviMode(m_OldtexenviMode);
		m_texenviMode = m_OldtexenviMode;
	}

	SetMatrixOperationOver();
}

void CRenderSystem::ResetGL(int screenWidth, int screenHeight)
{
	if (NULL != m_RenderCore)
	{
		m_RenderCore->Initialize(screenWidth, screenHeight);
	}
}

void CRenderSystem::IsRenderStyle(bool value)
{
	m_bIsRenderStyle = value;
}

void CRenderSystem::SetGradient(ST_POINTF stTLGradient, ST_POINTF stTRGradient, ST_POINTF stBLGradient, ST_POINTF stBRGradient)
{
	m_stTLGradient = stTLGradient;
	m_stTRGradient = stTRGradient;
	m_stBLGradient = stBLGradient;
	m_stBRGradient = stBRGradient;
}

void CRenderSystem::ClearGradinet()
{
	m_stTLGradient.x = 0;
	m_stTLGradient.y = 0;

	m_stTRGradient.x = 0;
	m_stTRGradient.y = 0;

	m_stBLGradient.x = 0;
	m_stBLGradient.y = 0;

	m_stBRGradient.x = 0;
	m_stBRGradient.y = 0;
}

void CRenderSystem::DrawTriangleVertex(UINT textureID, const float* addVertice, const float* uvs, int addVerticesCount, const int* addTriangles, int addTrianglesCount, const Color4f& color)
{
	if (textureID <= 0
		|| NULL == addVertice
		|| NULL == uvs
		|| NULL == addTriangles)
	{
		return;
	}

	float scale = 1.0f;

	m_RenderCore->SetRenderType(GL_RENDER_TYPE_TRIANGLES);
	m_RenderCore->BindTexture(textureID, true);

	if (addVerticesCount / 2 > MAX_VERTICE_NODE_PER_COMPUTE_COUNT)
	{
		addVerticesCount = MAX_VERTICE_NODE_PER_COMPUTE_COUNT * 2;
	}

	memcpy(m_stFinalPoint, addVertice, sizeof(ST_POINTF)* (addVerticesCount / 2));

	if (1.0f != scale)
	{
		for (int i = 0; i < addVerticesCount / 2; i++)
		{
			m_stFinalPoint[i].x *= scale;
			m_stFinalPoint[i].y *= scale;
		}
	}

	ExcuteGlobalMatrixTrans(addVerticesCount / 2);

	for (int i = 0; i < addTrianglesCount; i++)
	{
		m_RenderCore->SetTriangleIndex(addTriangles[i]);
	}

	for (int i = 0; i < addVerticesCount; i += 2)
	{
		m_RenderCore->SetTexCoord2f(uvs[i], uvs[i + 1]);
		m_RenderCore->SetColor4f(color);
		m_RenderCore->SetVertex2f(m_stFinalPoint[i / 2].x, m_stFinalPoint[i / 2].y);
	}
}