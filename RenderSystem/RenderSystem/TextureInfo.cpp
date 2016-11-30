#include "TextureInfo.h"
#include "RenderSystem.h"

#include <assert.h>

CTextureInfo::CTextureInfo(void)
{
	m_texID = 0;
	m_wWidth = 0;
	m_wHeight = 0;
	m_wTextWidth = 0;
	m_wTextHeight = 0;
	m_resSize = 0;

	m_pImageData = NULL;

	memset(m_szImageName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));
	memset(m_szResName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));

	m_isForcRemoved = false;

	m_palIndex = -1;
	m_palLength = -1;
	m_palData = NULL;

	m_bIsNeedReleaseImgMem = false;
	m_nAskResCoolDownTime = 0;

	m_bIsReadInAPK = false;
}


CTextureInfo::~CTextureInfo(void)
{
	unLoadImageRes();

	SAFE_DELETE(m_palData);
#ifdef TRACK_TEXTURE
	g_TextureInfoMap.erase((int)this);
#endif
}

void CTextureInfo::unLoadImageRes()
{
	CRenderSystem* pDisplay = CRenderSystem::GetInstance();

	if (m_texID > 0
		&& pDisplay != NULL)
	{
		pDisplay->DeleteTexture(m_texID);
		m_texID = 0;
#ifdef TRACK_TEXTURE 
		SAFE_DELETE(m_pImageData);
#endif
	}

#ifndef TRACK_TEXTURE 
	SAFE_DELETE(m_pImageData);
#endif

#ifdef TRACK_TEXTURE
	g_TextureInfoMap.erase((int)this);
#endif
}

void CTextureInfo::unLoadImageMemory()
{
#ifndef TRACK_TEXTURE
	// 	if (m_pImageData != NULL)
	// 	{
	// 		free(m_pImageData);
	// 		m_pImageData = NULL;
	// 	}
	SAFE_DELETE(m_pImageData);
#endif
}

void CTextureInfo::getImageSize(short& shWidth, short& shHeight)
{
	shWidth = m_wWidth;
	shHeight = m_wHeight;
}

void CTextureInfo::getTextureSize(short& shTexWidth, short& shTexHeight)
{
	shTexWidth = m_wTextWidth;
	shTexHeight = m_wTextHeight;
}

stImage* CTextureInfo::getImageData()
{
	return m_pImageData;
}

void CTextureInfo::SetResName(const TCHAR* pfileName, const TCHAR* pResName)
{
	memset(m_szImageName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));
	memset(m_szResName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));

	strcpy(m_szImageName, pfileName);
	strcpy(m_szResName, pResName);
}

void CTextureInfo::LoadImageRes(const TCHAR* pfileName, const TCHAR* pResName)
{
	//CFramework* pFramework = CFramework::GetInstance();

	memset(m_szImageName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));
	memset(m_szResName, 0, MAX_IMAGE_NAME_LEN * sizeof(TCHAR));

	strcpy(m_szImageName, pfileName);
	strcpy(m_szResName, pResName);

	/*if (pFramework->IsGamePaused())
	{
		return;
	}*/

	//ImageManager->AddImageRes(m_szImageName, m_szResName, GetResUID(), m_resSize, m_palIndex);
}

int CTextureInfo::GetTexturePalIndex()
{
	return m_palIndex;
}

char* CTextureInfo::GetTexturePalData() const
{
	return m_palData;
}

short CTextureInfo::GetTexturePalLenth()
{
	return m_palLength;
}

void CTextureInfo::ReloadImageAsynchronous()
{
	//assert(_tcslen(m_szImageName) > 0);
	//assert(_tcslen(m_szResName) > 0);

	//if (_tcslen(m_szImageName) > 0
	//	&& _tcslen(m_szResName) > 0)
	//{
	//	ImageManager->AddImageRes(m_szImageName, m_szResName, GetResUID(), m_resSize, m_palIndex);
	//}
}

void CTextureInfo::ReloadImageSynchronous()
{
	/*if (_tcslen(m_szImageName) > 0
		&& _tcslen(m_szResName) > 0
		)
	{
		LOG("CTextureInfo::ReloadImageSynchronous ... Name : %s", m_szImageName);

		CFramework* pFramework = CFramework::GetInstance();

		if (NULL != pFramework
			&& pFramework->IsGamePaused()
			)
		{
			return;
		}

		if (!IsResReady())
		{
			IArchive* pImageArchive = NULL;
			TCHAR PathName[MAX_PATH] = { 0 };

			GETARCHIVEPTR_NOSAVE(pFramework, pImageArchive, m_szImageName);

			if (pImageArchive != NULL)
			{
				pImageArchive->SeekTo(0, SEEK_SET);

				LoadImageRes(pImageArchive, 1, m_szImageName, m_szResName);

				SetNeedReleaseImgMem();
			}

			DestroyArchiveObject(pImageArchive);
		}
	}*/
}

bool CTextureInfo::SetImageData(stImage* pImageData)
{
	unLoadImageRes();

	if (pImageData != NULL)
	{
		m_pImageData = pImageData;

		m_wWidth = m_pImageData->w;
		m_wHeight = m_pImageData->h;
		m_wTextWidth = m_pImageData->tex_w;
		m_wTextHeight = m_pImageData->tex_h;

		return true;
	}

	return false;
}

int CTextureInfo::GetTextrueWidth()
{
	return m_wTextWidth;
}

int CTextureInfo::GetTextrueHeight()
{
	return m_wTextHeight;
}

void CTextureInfo::BindTexture()
{
	if (IsResReady())
	{
		if (m_texID == 0
			&& m_pImageData != NULL
			)
		{
			m_texID = CRenderSystem::GetInstance()->CreateTexture(m_pImageData);
			//unLoadImageMemory();
			m_bIsNeedReleaseImgMem = true;

#ifdef TRACK_TEXTURE
			if (g_TextureInfoMap.find((int)this) == g_TextureInfoMap.end())
			{
				g_TextureInfoMap[(int)this] = this;
			}
#endif
		}
	}
}

void CTextureInfo::EraseResData()
{
	unLoadImageRes();
}

void CTextureInfo::SetTexturePalInfo(stTexturePalInfo* texturePalInfo)
{
	if (texturePalInfo == NULL)
	{
		m_palIndex = -1;
		m_palLength = -1;
		SAFE_DELETE(m_palData);
		return;
	}

	m_palIndex = texturePalInfo->m_palIndex;
	m_palLength = texturePalInfo->m_palDataLength;

	if (m_palData != NULL)
	{
		SAFE_DELETE(m_palData);
	}

	m_palData = new char[m_palLength];
	memcpy(m_palData, texturePalInfo->m_palData, m_palLength);
}

void CTextureInfo::ReleaseMemoryData()
{
	if (m_bIsNeedReleaseImgMem
		//&& m_texID > 0	
		)
	{
		unLoadImageMemory();
		m_bIsNeedReleaseImgMem = false;
	}
}

void CTextureInfo::ResumeData()
{
	// 及时加载资源
	ReloadImageSynchronous();
}

bool CTextureInfo::IsCanAskRes()
{
	//if (m_nAskResCoolDownTime <= 0)
	//{
	//	m_nAskResCoolDownTime = RES_ASK_COOLDOWN_TIME;
	//	return true;
	//}

	//m_nAskResCoolDownTime--;
	//return false;

	return true;
}