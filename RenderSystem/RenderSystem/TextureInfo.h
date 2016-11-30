#ifndef ____TEXTURE_INFO_H____
#define ____TEXTURE_INFO_H____

#include "common.h"
#include <Windows.h>

#define  MAX_IMAGE_NAME_LEN  64

struct stTexturePalInfo
{
	stTexturePalInfo() :m_palIndex(-1), m_palDataLength(-1), m_palData(NULL) {}
	~stTexturePalInfo() { m_palData = NULL; }

	int m_palIndex;
	short m_palDataLength;
	char* m_palData;
};

class CTextureInfo
{
	enum
	{
		EIMAGETYPE_PNG,
		EIMAGETYPE_TGA
	};

public:
	CTextureInfo(void);
	virtual ~CTextureInfo(void);
	void LoadImageRes(const TCHAR* pfileName, const TCHAR* pResName);

	void ReloadImageAsynchronous();
	void ReloadImageSynchronous();

	virtual void ReleaseMemoryData();

	// ������Դ��С
	void SetResSize(UINT size)
	{
		m_resSize = size;
	}
	// ��ȡ��Դ��С
	UINT GetResSize()
	{
		return m_resSize;
	}

	/*
	@ brief ж����Դ�������ڴ��еĺ��Կ�����˵�
	*/
	void unLoadImageRes();
	/*
	@ brief ж����Դ��ֻж���ڴ����ݣ��Կ��˵�������Ȼ����
	*/
	void unLoadImageMemory();
	void getImageSize(short& shWidth, short& shHeight);
	void getTextureSize(short& shTexWidth, short& shTexHeight);
	stImage* getImageData();
	UINT   getTextureID() { return m_texID; }

	virtual int GetTextrueWidth();
	virtual int GetTextrueHeight();

	virtual bool IsResReady();
	virtual void SetResIsReady();
	virtual void SetResIsNotReady();

	bool SetImageData(stImage* pImageData);

	void BindTexture();

	virtual void EraseResData();

	void SetTexturePalInfo(stTexturePalInfo* texturePalInfo);

	int GetTexturePalIndex();

	char* GetTexturePalData() const;

	short GetTexturePalLenth();

	TCHAR* GetImageName() { return m_szImageName; };
	TCHAR* GetResName() { return m_szResName; };

	void SetNeedReleaseImgMem() { m_bIsNeedReleaseImgMem = true; };

	virtual void ResumeData();

	bool IsCanAskRes();

	virtual bool IsHasImageData() { return m_pImageData != NULL; };

	void SetResName(const TCHAR* pfileName, const TCHAR* pResName);

	virtual bool IsReadInAPK() { return m_bIsReadInAPK; };

	virtual void SetReadInAPK() { m_bIsReadInAPK = true; };

private:
	short m_wWidth;
	short m_wHeight;
	short m_wTextWidth;
	short m_wTextHeight;
	UINT    m_texID;
	//IImageRes* m_pImageRes;
	stImage* m_pImageData;
	TCHAR  m_szImageName[MAX_IMAGE_NAME_LEN];
	TCHAR  m_szResName[MAX_IMAGE_NAME_LEN];

	UINT m_resSize;
	bool m_isForcRemoved; // �Ƿ�ǿ��ɾ����

	int m_palIndex;
	short m_palLength;
	char* m_palData;

	bool m_bIsNeedReleaseImgMem;
	int m_nAskResCoolDownTime;

	bool m_bIsReadInAPK;
};

#endif