#ifndef ____COMMON_H____
#define ____COMMON_H____

#include <stdio.h>
#include <stdlib.h>

#define SAFE_DELETE(x) \
	if(x!=NULL)\
		delete x;\
    x = NULL;
#define SAFE_DELETE_ARRAY(x)\
	if(x!=NULL)\
		delete [] x;\
	x = NULL;

typedef unsigned int UINT;
typedef signed char                      SCHR;
typedef unsigned char                    UCHR;
typedef signed char                      SSTR;
typedef unsigned char                    USTR;
typedef signed short int                 SHINT;
typedef unsigned short int               UHINT;
typedef signed long int                  SLONG;
#ifdef WIN32
typedef signed __int64                   SHYPER;
typedef unsigned __int64                 UHYPER;
#endif
typedef unsigned char                    BYTE;
typedef unsigned long                    DWORD;
typedef unsigned long                    ULONG;
typedef signed short					 SHORT;
typedef unsigned short                   USHORT;
typedef unsigned int                     UINT;
typedef unsigned short                   WORD;
typedef BYTE* LPBYTE;

typedef struct
{
	float x;
	float y;
} Vector2f;

typedef	enum _ImageAttrib
{
	imageAttribBegin = 0,
	imageAttrib1555 = 0,	// a1r5g5b5
	imageAttrib4444 = 1,	// a4r4g4b4
	imageAttrib8888 = 2,	// a8r8g8b8 add by qjb [2004-11-1]
	imageAttribGrey = 3,
	imageAttribEnd = imageAttribGrey,
}ImageAttrib;

enum Flip_Flags
{
	TRANS_NONE = 0,				// 0 0 0
	TRANS_MIRROR = 1,				// 0 0 1
	TRANS_MIRROR_ROT180 = 2,				// 0 1 0
	TRANS_ROT180 = 3,				// 0 1 1
	TRANS_ROT90 = 4,				// 1 0 0
	TRANS_MIRROR_ROT90 = 5,				// 1 0 1
	TRANS_MIRROR_ROT270 = 6,				// 1 1 0
	TRANS_ROT270 = 7,				// 1 1 1
};

enum enBlendType
{
	enNormalBlend = 0,
	enBrightBlend
};

enum TEXENVI_MODE
{
	TEXENVI_MODULATE = 0,
	TEXENVI_ADD,
};

struct Color4f
{
	Color4f() :red(1.0f), green(1.0f), blue(1.0f), alpha(1.0f) {}
	float red;
	float green;
	float blue;
	float alpha;
};

typedef struct ST_POINTF
{
	ST_POINTF() : x(0), y(0) {}
	ST_POINTF(float X, float Y) : x(X), y(Y) {}
	float x;
	float y;
}POINT_MF, *PPOINT_MF;

struct stImage
{
	stImage() : w(0), h(0), tex_w(0), tex_h(0), pitch(0), attrib(0), data(NULL) {};

	~stImage()
	{
		// �ͷ�ͼƬ��Դ
		if (NULL != data)
		{
			free(data);
			data = NULL;
		}
	};

	int  w;			        //ͼ����
	int  h;			        //ͼ��߶�
	int  tex_w;				//������
	int  tex_h;				//����߶�
	int  pitch;		        //ÿ��ͼ����Ҫ�������ֽ���
	int  attrib;            //ͼ������
							//unsigned char data[4];	//ͼ������
	unsigned char* data;	//ͼ������
};

struct stDisplayTransform
{
	stDisplayTransform() :m_TextureWidth(0), m_TextureHeight(0),
		m_TLVertexX(0), m_TLVertexY(0),
		m_TRVertexX(0), m_TRVertexY(0),
		m_BLVertexX(0), m_BLVertexY(0),
		m_BRVertexX(0), m_BRVertexY(0),
		m_TexCoordX(0), m_TexCoordY(0),
		m_TexCoordWidth(0), m_TexCoordHeight(0),
		m_useLinearBind(false), m_Flag(0)
	{}
	//����ߴ�
	int m_TextureWidth;
	int m_TextureHeight;

	//�������꣬�������Ͻ�
	float m_TLVertexX;
	float m_TLVertexY;

	//�������꣬�������Ͻ�
	float m_TRVertexX;
	float m_TRVertexY;

	//�������꣬�������½�
	float m_BLVertexX;
	float m_BLVertexY;

	//�������꣬�������½�
	float m_BRVertexX;
	float m_BRVertexY;

	//����Coord���꣬coord���Ͻ�
	int m_TexCoordX;
	int m_TexCoordY;

	//����Coord�ߴ�
	int m_TexCoordWidth;
	int m_TexCoordHeight;

	//������ɫ��Ϣ����Ҫ����������ɫ��ʹ�ã�
	//���ĸ�����ֱ�������ɫ
	Color4f m_Color[4];

	//�Ƿ�ʹ��Linear
	bool m_useLinearBind;

	//��ת��Ϣ
	int m_Flag;
};

struct stMatrixOperation
{
	stMatrixOperation() :m_x(0), m_y(0),
		m_hasTranslate(false), m_translateX(0), m_translateY(0),
		m_hasScale(false), m_scaleX(1.0f), m_scaleY(1.0f),
		m_hasRotate(false), m_rotateAngle(0), m_rotateX(0), m_rotateY(0), m_rotateZ(0),
		m_colorOverlay(0xffffffff)
	{}

	float m_x;
	float m_y;

	bool m_hasTranslate;
	float m_translateX;
	float m_translateY;

	bool m_hasScale;
	float m_scaleX;
	float m_scaleY;

	bool m_hasRotate;
	float m_rotateAngle;
	float m_rotateX;
	float m_rotateY;
	float m_rotateZ;

	int m_colorOverlay;
};

struct stContextParam
{
	void* m_pContext;
	void* m_pFramework;
	int     m_Width;
	int     m_Height;

	const char* m_gameapk_path;
	const char* m_packageName;
	const char* m_localfile_path;
	const char* m_sdCard_path;
	const char* m_downloadFile_path;
	const char* m_extensionFileName;

	bool m_bIsDownloadVersion;
};

static float GetDegrees2Radians(float angle)
{
	return ((angle) * 0.01745329252f);	//PI/180
}

#endif
