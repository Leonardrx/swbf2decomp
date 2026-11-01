#include "pcDustEffect.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcParticleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "pcSunFlare.h"

// GLOBAL: SWBF2 0x007eafe8
// sBox
static PblBox sBox(-400.0f, -400.0f, -400.0f, 400.0f, 400.0f, 400.0f);



struct DustVertexCompressed
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
struct DustVertex
{
    PblVector3 pos;
    RedColor color;
    float u, v;
};

// FUNCTION: SWBF2 0x00410c20
DustEffectRenderData::DustEffectRenderData(const DustEffectClass* pClass)
    : Renderable(2)
{
    m_pClass = pClass;
    m_pPrimitive = 0;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pShader = (pcParticleShader*)pcRedShader::Create(0x8A8743BF /* "Particle" */, 0x9282,
                                                            &m_pClass->mTextureHash, 1, 0x10004,
                                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        m_pShader = (pcParticleShader*)pcRedShader::Create(0x8A8743BF /* "Particle" */, 0x282,
                                                            &m_pClass->mTextureHash, 1, 0x10004,
                                                            RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    m_pShader->NearFadeEnable(true);
    mHeap = new PblHeap<int, float>(m_pClass->mNumParticles, 1000000.0f, 0);
    m_pVertices = 0;
    Activate(0);
}

// FUNCTION: SWBF2 0x00410d40
DustEffectRenderData::~DustEffectRenderData()
{
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }
    if (m_pShader)
    {
        m_pShader->Release();
    }
    delete mHeap;
    Deactivate();
}

// SYNTHETIC: SWBF2 0x00411640
// DustEffectRenderData::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00410e20
void DustEffectRenderData::Render(int value)
{
    unsigned int uFlags = (unsigned int)value;
    if (uFlags & 0x200000)
    {
        if (m_pPrimitive)
        {
            m_pPrimitive->Release();
            m_pPrimitive = 0;
        }
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float* vForward = pCamera->_Matrix.m[2];
    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        const PblVector3& vPos = m_pClass->mParticle[i].mPos;
        float fKey = -(vPos.y * vForward[1] + vPos.x * vForward[0] + vPos.z * vForward[2]);
        mHeap->Push(i, fKey);
    }

    Begin();
    while (!mHeap->IsEmpty())
    {
        const DustEffectClass::DustParticle& particle = m_pClass->mParticle[mHeap->Get(0)];
        RedColor color;
        color.b = (unsigned char)(int)(particle.mColor.b * 255.0f);
        color.g = (unsigned char)(int)(particle.mColor.g * 255.0f);
        color.r = (unsigned char)(int)(particle.mColor.r * 255.0f);
        color.a = (unsigned char)(int)(particle.mColor.a * 255.0f);
        AddParticle(particle.mPos, particle.mSize, color, 0.0f);
        mHeap->Pop();
    }
    End();
}

// FUNCTION: SWBF2 0x00411020
void DustEffectRenderData::Begin() const
{
    if (m_pPrimitive)
    {
        m_pPrimitive->Release();
    }

    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED)
    {
        m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x9282, 16,
                                              m_pClass->mNumParticles * 4, m_pClass->mNumParticles * 6, false);
        CompressedPos::SetBoundingBox(sBox);
    }
    else
    {
        m_pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST, 0x282, 24,
                                              m_pClass->mNumParticles * 4, m_pClass->mNumParticles * 6, false);
    }
    m_pPrimitive->m_bbox = sBox;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblMatrix& mat = pCamera->_Matrix;
    m_vUp = mat.GetYAxis();
    m_vRight = mat.GetXAxis();
    m_pVertices = m_pPrimitive->BeginVertexData();
}

// FUNCTION: SWBF2 0x004111c0
void DustEffectRenderData::AddParticle(const PblVector3& vPos, float fSize, const RedColor& color,
                                       float fOffset) const
{
    PblVector3 up = fSize * m_vUp;
    PblVector3 right = fSize * m_vRight;
    PblVector3 pos = (vPos - up * 0.5f) + right * (fOffset - 0.5f);

    if (RedRenderer::pcGetGeometryMode())
    {
        DustVertexCompressed*& v = (DustVertexCompressed*&)m_pVertices;
        v->pos.SetPos(pos);
        v->color = color;
        v->uv.SetUV(1.0f, 0.0f);
        ++v;

        v->pos.SetPos(pos + right);
        v->color = color;
        v->uv.SetUV(1.0f, 1.0f);
        ++v;

        v->pos.SetPos((pos + up) + right);
        v->color = color;
        v->uv.SetUV(0.0f, 1.0f);
        ++v;

        v->pos.SetPos(pos + up);
        v->color = color;
        v->uv.SetUV(0.0f, 0.0f);
        ++v;
    }
    else
    {
        DustVertex* pVertex;

        ((DustVertex*)m_pVertices)->pos = pos;
        ((DustVertex*)m_pVertices)->color = color;
        pVertex = (DustVertex*)m_pVertices;
        pVertex->u = 1.0f;
        pVertex->v = 0.0f;
        m_pVertices = pVertex + 1;

        ((DustVertex*)m_pVertices)->pos = pos + right;
        ((DustVertex*)m_pVertices)->color = color;
        pVertex = (DustVertex*)m_pVertices;
        pVertex->u = 1.0f;
        pVertex->v = 1.0f;
        m_pVertices = pVertex + 1;

        ((DustVertex*)m_pVertices)->pos = (pos + up) + right;
        ((DustVertex*)m_pVertices)->color = color;
        pVertex = (DustVertex*)m_pVertices;
        pVertex->u = 0.0f;
        pVertex->v = 1.0f;
        m_pVertices = pVertex + 1;

        ((DustVertex*)m_pVertices)->pos = pos + up;
        ((DustVertex*)m_pVertices)->color = color;
        pVertex = (DustVertex*)m_pVertices;
        pVertex->u = 0.0f;
        pVertex->v = 0.0f;
        m_pVertices = pVertex + 1;
    }
}

// FUNCTION: SWBF2 0x004115d0
void DustEffectRenderData::End() const
{
    m_pPrimitive->EndVertexData();
    unsigned short* pIndices = m_pPrimitive->BeginIndexData();
    for (int i = 0; i < m_pClass->mNumParticles; ++i)
    {
        unsigned short uBase = (unsigned short)(i * 4);
        *pIndices++ = uBase;
        *pIndices++ = uBase + 1;
        *pIndices++ = uBase + 2;
        *pIndices++ = uBase;
        *pIndices++ = uBase + 2;
        *pIndices++ = uBase + 3;
    }
    m_pPrimitive->EndIndexData();
    m_pVertices = 0;
}

// FUNCTION: SWBF2 0x00411670
void DustEffectClass::Render(const PblMatrix& matrix, float fAlpha, unsigned int uiFlags) const
{
    RedColor color;
    color.b = RedColor::WHITE.b;
    color.g = RedColor::WHITE.g;
    color.r = RedColor::WHITE.r;
    color.a = (unsigned char)(int)(fAlpha * 255.0f);
    if (mRenderData->m_pPrimitive)
    {
        RedRenderer::pcRenderPrimitive(mRenderData->m_pPrimitive,
                                       mRenderData->m_pShader,
                                       matrix,
                                       color, 0, 0, 0, 0);
    }
}

// FUNCTION: SWBF2 0x004116d0
void DustEffectClass::CreateRenderData()
{
    mRenderData = new DustEffectRenderData(this);
}

// FUNCTION: SWBF2 0x00411740
void DustEffectClass::DestroyRenderData()
{
    if (mRenderData)
    {
        delete mRenderData;
        mRenderData = 0;
    }
}

// TEMPLATE: SWBF2 0x00411860
// PblHeap<int,float>::Pop
template void PblHeap<int, float>::Pop();

// TEMPLATE: SWBF2 0x00411760
// PblHeap<int,float>::PblHeap<int,float>
template PblHeap<int, float>::PblHeap(int, float, void*);
// TEMPLATE: SWBF2 0x004117c0
// PblHeap<int,float>::~PblHeap<int,float>
template PblHeap<int, float>::~PblHeap();
// TEMPLATE: SWBF2 0x00411800
// PblHeap<int,float>::Push
template void PblHeap<int, float>::Push(int&, float);

// TEMPLATE: SWBF2 0x004117d0
// PblHeap<int,float>::IsEmpty
template bool PblHeap<int, float>::IsEmpty();
// TEMPLATE: SWBF2 0x004117e0
// PblHeap<int,float>::Get
template int PblHeap<int, float>::Get(int);
