#pragma once

#include <stddef.h>
#include <math.h>
#include "FLEffect.h"
#include "Battlefront2Lod.h"
#include "LensFlare.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class LightBeam;
class pcRedShader;

// VTABLE: SWBF2 0x007a3b54
class LightBeamEffect : public FLEffect
{
public:

    LightBeamEffect()
        : FLEffect(0xE8651328u, 3)
    {
        m_pShader = 0;
    }

    virtual ~LightBeamEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static LightBeamEffect* s_pInstance;

    static LightBeamEffect* Instance() { return s_pInstance; }

    void RenderLightBeam(const LightBeam* pBeam, unsigned int uiFlags) const;

    pcRedShader* m_pShader;

private:

    void _PlatformInit();

    void _PlatformCleanup();
};
 
 
 // VTABLE: SWBF2 0x0079b7f0 Thread
 // VTABLE: SWBF2 0x0079b910 PblHandled
 // VTABLE: SWBF2 0x0079b918
 class LightBeam : public FLEffectClass, public FLEffectObject
{
public:
    LightBeam();
    LightBeam(const LightBeam& that);
    virtual ~LightBeam();
    virtual FLEffectObject* BuildEffect();
    virtual void SetHeadPosition(const PblVector3& vPos);
    virtual void SetDirection(const PblVector3& vDir);
    virtual void SetLength(float fLength);
    virtual void SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail);
    virtual void SetPositionOmni(const PblVector3& vPos);

    // FUNCTION: SWBF2 0x004be070
    virtual void SetInitialWidth(float fWidth)
    {
        _fHeadSizeX = _fHeadSizeZ = fWidth * 0.5f;
        _Flare._fSize = fWidth;
    }

    // FUNCTION: SWBF2 0x004be0a0
    virtual void SetInitialWidth(float fWidthX, float fWidthZ)
    {
        _fHeadSizeX = fWidthX * 0.5f;
        _fHeadSizeZ = fWidthZ * 0.5f;
        _Flare._fSize = _fHeadSizeX + _fHeadSizeZ;
    }

    virtual void SetFinalWidth(float fWidth);
    virtual void SetFinalWidth(float fWidthX, float fWidthZ);
    virtual void SetFadeLength(float fLength);
    virtual void Unhide();
    virtual void HideInView(unsigned int uiView, bool bHide);
    virtual bool IsHiddenInView(unsigned int uiView);
    virtual void AddToRenderList();
    virtual void RemoveFromRenderList();

    void SetColor(const RedColor& color);

    void Render(unsigned int uiFlags) const { LightBeamEffect::Instance()->RenderLightBeam(this, uiFlags); }
    void SetDrawDistance(float fDistance)
    {
        _fSquaredDrawDistance = fDistance * fDistance;
        _Flare.SetDrawDistance(fDistance);
    }
    void SetFadeFactor(float fFactor) { _fFadeAmount = fFactor; }
    void SetFlareIntensity(float fIntensity)
    {
        _fFlareIntensity = fIntensity;
        unsigned char ucAlpha = (unsigned char)(int)(_color.a * _fFlareIntensity);
        RedColor flareColor = _color;
        flareColor.a = ucAlpha;
        _Flare.SetColor(flareColor);
    }

    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
protected:
    virtual void UpdatedWorldMatrix();
public:

    void GetLocalBoundingSphere(PblVector3& vCenter, float& fRadius) const;

    void SetBeamIntensity(float fIntensity) { _fBeamIntensity = fIntensity; }

    LensFlare _Flare;
    PblVector3 _vHeadPosition;
    PblVector3 _vDirection;
    RedColor _color;
    float _fLength;
    float _fHeadSizeX;
    float _fHeadSizeZ;
    float _fTailSizeX;
    float _fTailSizeZ;
    float _fFadeLength;
    float _fFadeAmount;
    float _fSquaredDrawDistance;
    float _fFlareIntensity;
    float _fBeamIntensity;
    float _fFlareCosAngle;
};
 
 
 
 
 // VTABLE: SWBF2 0x0079c010 Thread
 // VTABLE: SWBF2 0x0079c040 LightBeam
 // VTABLE: SWBF2 0x0079c0b8 RedSceneObject
 // VTABLE: SWBF2 0x0079c15c PblHandled
 class ManagedLightBeam : public RedSceneObject, public LightBeam
{
public:

    ManagedLightBeam();

    ManagedLightBeam(const LightBeam& beam);
    // SYNTHETIC: SWBF2 0x004cf48c
    // ??_EManagedLightBeam@@WGA@AEPAXI@Z
    // SYNTHETIC: SWBF2 0x004cf494
    // ??_EManagedLightBeam@@WII@AEPAXI@Z
    // SYNTHETIC: SWBF2 0x004cf4a7
    // ??_EManagedLightBeam@@WHA@AEPAXI@Z
    // SYNTHETIC: SWBF2 0x004cc9b0
    // ManagedLightBeam::`scalar deleting destructor'
    // SYNTHETIC: SWBF2 0x004cca10
    // ManagedLightBeam::~ManagedLightBeam
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual void SetHeadPosition(const PblVector3& vPos);
    virtual void SetDirection(const PblVector3& vDir);
    virtual void SetLength(float fLength);
    virtual void SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail);
    virtual void SetPositionOmni(const PblVector3& vPos);
    virtual void SetInitialWidth(float fWidth);
    virtual void SetInitialWidth(float fWidthX, float fWidthZ);
    virtual void SetFinalWidth(float fWidth);
    virtual void SetFinalWidth(float fWidthX, float fWidthZ);
    virtual void SetFadeLength(float fLength);
    virtual void Unhide();
    virtual void HideInView(unsigned int uiView, bool bHide);
    virtual bool IsHiddenInView(unsigned int uiView);
    virtual void AddToRenderList();
    virtual void RemoveFromRenderList();
protected:
    virtual void UpdatedWorldMatrix();
private:

    inline void _UpdateBoundingSphere();
public:
    unsigned int _uiHideInView;
};
 
 

// FUNCTION: SWBF2 0x004cc4c0
inline void ManagedLightBeam::SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail)
{
    LightBeam::SetHeadTailPosition(vHead, vTail);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004bdec0
inline void LightBeam::SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail)
{
    PblVector3 vDelta = vTail - vHead;
    SetHeadPosition(vHead);
    SetLength(PblMath::Sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z));
    if (_fLength > 0.0f)
    {
        float fInv = 1.0f / _fLength;
        SetDirection((vDelta * fInv));
    }
    else
    {
        SetDirection(PblVector3(0.0f, 0.0f, 1.0f));
    }
}

// FUNCTION: SWBF2 0x004cc340
inline void ManagedLightBeam::SetHeadPosition(const PblVector3& vPos)
{
    LightBeam::SetHeadPosition(vPos);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc530
inline void ManagedLightBeam::SetPositionOmni(const PblVector3& vPos)
{
    LightBeam::SetPositionOmni(vPos);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc940
inline void ManagedLightBeam::AddToRenderList()
{
    if (!_bActive)
    {
        _UpdateBoundingSphere();
        Activate();
    }
}

// FUNCTION: SWBF2 0x004cc3d0
inline void ManagedLightBeam::SetDirection(const PblVector3& vDir)
{
    LightBeam::SetDirection(vDir);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc7e0
inline void ManagedLightBeam::SetFadeLength(float fLength)
{
    LightBeam::SetFadeLength(fLength);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc8e0
inline void ManagedLightBeam::UpdatedWorldMatrix()
{
    LightBeam::UpdatedWorldMatrix();
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc870
inline void ManagedLightBeam::HideInView(unsigned int uiView, bool bHide)
{
    const unsigned int mask = 1u << uiView;
    if (bHide)
    {
        _uiHideInView |= mask;
        _Flare._uiHideInView |= mask;
    }
    else
    {
        _uiHideInView &= ~mask;
        _Flare._uiHideInView &= ~mask;
    }
}

// FUNCTION: SWBF2 0x004cc450
inline void ManagedLightBeam::SetLength(float fLength)
{
    LightBeam::SetLength(fLength);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004bdd90
inline LightBeam::~LightBeam()
{
    DeactivateEffect();
}



// SYNTHETIC: SWBF2 0x0056faa0
// LightBeamEffect::`scalar deleting destructor'
inline LightBeamEffect::~LightBeamEffect()
{
}

// FUNCTION: SWBF2 0x004be010
inline void LightBeam::SetColor(const RedColor& color)
{
    _color = color;
    RedColor flareColor = _color;
    flareColor.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
    _Flare.SetColor(flareColor);
}

// FUNCTION: SWBF2 0x004bde50
inline void LightBeam::SetHeadPosition(const PblVector3& vPos)
{
    _vHeadPosition = vPos;
    PblVector3 vWorld;
    D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(&vWorld), reinterpret_cast<const D3DXVECTOR3*>(&_vHeadPosition), reinterpret_cast<const D3DXMATRIX*>(&m_mWorldMatrix));
    _Flare.SetPosition(vWorld);
}

// FUNCTION: SWBF2 0x004bdea0
inline void LightBeam::SetDirection(const PblVector3& vDir)
{
    _vDirection = vDir;
}

// FUNCTION: SWBF2 0x004bdfc0
inline void LightBeam::SetPositionOmni(const PblVector3& vPos)
{
    SetPosition(vPos);
    SetLength(0.0f);
    SetDirection(PblVector3(0.0f, 1.0f, 0.0f));
}

// FUNCTION: SWBF2 0x004be0e0
inline void LightBeam::SetFadeLength(float fLength)
{
    _fFadeLength = fLength;
}

// FUNCTION: SWBF2 0x004be100
inline void LightBeam::Unhide()
{
    _Flare._uiHideInView = 0;
}

// FUNCTION: SWBF2 0x004be110
inline void LightBeam::HideInView(unsigned int uiView, bool bHide)
{
    if (bHide)
    {
        _Flare._uiHideInView |= 1 << uiView;
    }
    else
    {
        _Flare._uiHideInView &= ~(1 << uiView);
    }
}

// FUNCTION: SWBF2 0x004be140
inline bool LightBeam::IsHiddenInView(unsigned int uiView)
{
    return (_Flare._uiHideInView & (1 << uiView)) != 0;
}

inline ManagedLightBeam::ManagedLightBeam()
    : RedSceneObject(&largeEffectLodData)
    , LightBeam()
    , _uiHideInView(0)
{
}
