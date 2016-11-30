#ifndef ____RENDER_SYSTEM_H____
#define ____RENDER_SYSTEM_H____

#include "common.h"
#include <vector>

class CRenderCore;
class CTextureInfo;

class CRenderSystem
{
public:
	static CRenderSystem* GetInstance();
	virtual void Initialize(void* pContextParam);
	virtual void Dispose();

	virtual void IsRenderStyle(bool value);
	virtual void EndDisplay();
	virtual void DrawTexture(CTextureInfo* textureInfo, stDisplayTransform* pTransform);
	virtual void DrawTexture(UINT textrueID, stDisplayTransform* pTransform, bool bIsReadInAPK = false);
	virtual void DrawTriangleVertex(UINT textureID, const float* addVertice, const float* uvs, int addVerticesCount, const int* addTriangles, int addTrianglesCount, const Color4f& color);
	virtual UINT CreateTexture(stImage* stimage);
	virtual void DeleteTexture(UINT textureID);
	virtual void FillRectAlpha(int x, int y, int w, int h, int color);
	virtual void DrawRect(int x, int y, int w, int h, int color);
	virtual void DrawFan(int posX, int posY, int startangle, int endangle, int radius, int color);
	virtual void DrawLine(int x1, int y1, int x2, int y2, int linewidth, int color);
	virtual void SetClip(int x, int y, int w, int h);
	virtual void ClipRect(int x, int y, int w, int h);
	virtual void SetClipFullScreen();
	virtual void GetClipArea(int &clipX, int &clipY, int &clipWidth, int &clipHeight);
	virtual void SetMatrixOperationStart(stMatrixOperation* pMatrixOperation);
	virtual void SetMatrixOperationOver();
	void SetAlphaBlend(bool blend);
	virtual void SetBlendFunc(enBlendType blendType);
	virtual void ReSetBlendFunc();
	virtual void SubImageToTexture(UINT textureID, int xoffset, int yoffset, int width, int height, bool isgrey, const void* pixels);

	virtual void SetTexEnviMode(TEXENVI_MODE mode);
	virtual TEXENVI_MODE GetTexEnvidMode();

	virtual void SetGlobalRenderParam(TEXENVI_MODE texMode, stMatrixOperation* pMatrixOperation);
	virtual void EndGlobalRenderParam();

	virtual void SetFixArtifacts(bool bVar);
	virtual void LastInitialize();
	virtual void ReleaseWhiteTextureBuffer();



	//     BEGIN_INTERFACE_MAP(QueryInterface)
	//         INTERFACE_MAP(IID_IABSTRACT,IAbstract);
	//         INTERFACE_MAP(IID_IRenderSystem,IRenderSystem);
	//     END_INTERFACE_MAP()

	virtual void ResetGL(int screenWidth, int screenHeight);

	virtual void SetGradient(ST_POINTF stTLGradient, ST_POINTF stTRGradient, ST_POINTF stBLGradient, ST_POINTF stBRGradient);
	virtual void ClearGradinet();

	void CreateWhiteBufferTexture();

private:
	CRenderSystem(void);
	virtual ~CRenderSystem(void);

	bool IsOutOfRectArea(int x, int y, int w, int h);

	void ExcuteGlobalMatrixTrans(int verticeCount = 4);

	void ExcuteMatrixTrans(ST_POINTF& stPoint, stMatrixOperation matrixOper);

private:
	stContextParam m_GLParam;
	CRenderCore* m_RenderCore;

	bool m_IsCliped;
	int m_ClipX;
	int m_ClipY;
	int m_ClipWidth;
	int m_ClipHeight;

	int m_defaultLineWidth;

	//纹理着色模式
	TEXENVI_MODE m_texenviMode;
	TEXENVI_MODE m_OldtexenviMode;

	stDisplayTransform m_tempDT;

	bool m_bIsRenderStyle;

	float m_matrix[16];

	// 顶点偏移量
	ST_POINTF m_stTLGradient;
	ST_POINTF m_stTRGradient;
	ST_POINTF m_stBLGradient;
	ST_POINTF m_stBRGradient;

	// 最终顶点
	ST_POINTF* m_stFinalPoint;

	// 矩阵变换的全局参数
	std::vector<stMatrixOperation> m_vMatrixTransform;

	// 是否取纹理UV值时进行校正
	bool m_bFixArtifacts;

	// 用于fillRect的材质ID
	UINT m_nFillRectTextureID;
	stDisplayTransform m_vFillRectTransform;

	static CRenderSystem* _instance;
};

#endif


