#include <d3dx9math.h>
#include "Portal.h"

#include "RedEngineFL/Util/RedCommandConsole.h"
#include "RedEngineFL/Util/ext/SubAllocator.h"
#include "RedEngineFL/Util/ext/types.h"
#include "RedEngineFL/geometry/basics.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <stdlib.h>


// FUNCTION: SWBF2 0x006df220
geo::ExtrudedPoly3D& geo::ExtrudedPoly3D::operator=(const geo::ExtrudedPoly3D& rOther)
{
    if (points_.Reserve_(rOther.points_._numElements, true))
    {
        points_._numElements = rOther.points_._numElements;
        for (int i = 0; i < points_._numElements; ++i)
        {
            points_._elements[i] = rOther.points_._elements[i];
        }
    }
    min_ = rOther.min_;
    max_ = rOther.max_;
    xaxis_ = rOther.xaxis_;
    yaxis_ = rOther.yaxis_;
    zaxis_ = rOther.zaxis_;
    return *this;
}

// GLOBAL: SWBF2 0x009a082c
int _cframe;

// GLOBAL: SWBF2 0x009a0830
pvs::IManager* _man;
namespace pvs
{

// SYNTHETIC: SWBF2 0x00408870
// pvs::`dynamic initializer for 'displayableSuballoc_''
// SYNTHETIC: SWBF2 0x00769a50
// pvs::`dynamic atexit destructor for 'displayableSuballoc_''

// GLOBAL: SWBF2 0x007df8c0
// displayableSuballoc_
util::StaticSubAlloc<util::SLNode<pvs::IDisplayable*> > displayableSuballoc_;
}

// GLOBAL: SWBF2 0x007df864
static bool gEnable = true;

// SYNTHETIC: SWBF2 0x00408840
// `dynamic initializer for 'g_Enable''
static RedCommandConsole g_Enable("vis.Enable", MultiVar(&gEnable));

extern bool gSaveCam;
extern bool gBackFaceCull;
extern bool gProximityTest;


// FUNCTION: SWBF2 0x006de890
geo::Frustum::Frustum(const Vector3<float>& rCenter, const Vector3<float>& rUp, const Vector3<float>& rFwd,
                      float fWidth, float fHeight, float fNear, float fFar)
    : center_(rCenter), up_(rUp), fwd_(rFwd), width_(fWidth), height_(fHeight), near_(fNear), far_(fFar),
      planes_(), initdone_(false)
{
}

// FUNCTION: SWBF2 0x006de9c0
geo::Frustum::Frustum()
    : center_(), up_(), fwd_(), width_(0.0f), height_(0.0f), near_(0.0f), far_(0.0f), planes_(), initdone_(false)
{
}

namespace pvs
{

// FUNCTION: SWBF2 0x006e0900
int GetCurrentFrame_()
{
    return _cframe;
}
}

// SYNTHETIC: SWBF2 0x006cb0f0
// pvs::BaseCamera::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006cbc80
void pvs::BaseCamera::GetAABound(geo::Box&) const
{
}

// TEMPLATE: SWBF2 0x006de6e0
// util::Max_<unsigned int>
template const unsigned int& util::Max_<unsigned int>(const unsigned int&, const unsigned int&);

// TEMPLATE: SWBF2 0x006de700
// geo::Max_<float>
template const float& geo::Max_<float>(const float&, const float&);

// TEMPLATE: SWBF2 0x006de720
// geo::Min_<float>
template const float& geo::Min_<float>(const float&, const float&);

// TEMPLATE: SWBF2 0x006de740
// util::List<pvs::IDisplayable *,util::SLNode<pvs::IDisplayable *> >::List<pvs::IDisplayable *,util::SLNode<pvs::IDisplayable *> >
template util::List<pvs::IDisplayable*, util::SLNode<pvs::IDisplayable*> >::List(util::IAllocator*);

// TEMPLATE: SWBF2 0x006e02d0
// util::List<pvs::IDisplayable *,util::SLNode<pvs::IDisplayable *> >::FindRemoveSlow
template bool util::List<pvs::IDisplayable*, util::SLNode<pvs::IDisplayable*> >::FindRemoveSlow(pvs::IDisplayable* const&);

// FUNCTION: SWBF2 0x006de7b0
pvs::DynDisplayable::DynDisplayable()
{
    lastCheckPos_ = geo::Vector3<float>(0.0f, 0.0f, 0.0f);
    nextCheckDist_ = 0.0f;
}

// FUNCTION: SWBF2 0x006deae0
pvs::Manager::Manager()
{
    IManager::SetInstance(*this);
    _cframe = 0;
    curCamSector_ = 0;
    lastSaveCam_ = false;
    rootSect_.GetBound().min_ = -1.0e8f;
    rootSect_.GetBound().max_ = 1.0e8f;
    camsectors_.Reserve(32);
    scache_.entries_.Reserve(128);
    dynamics_.Reserve(128);
}

// FUNCTION: SWBF2 0x006dec50
pvs::Sector::Sector()
    : windingMult_(0), tmark_(false), atmark_(false), initdone_(false),
      dynamics_(&pvs::displayableSuballoc_)
{
}

// FUNCTION: SWBF2 0x006dedf0
pvs::BaseCamera::~BaseCamera()
{
    if (sector_)
    {
        sector_->UnregisterDynamic(*this);
        sector_ = 0;
    }
}

// FUNCTION: SWBF2 0x006dee60
pvs::DynDisplayable::~DynDisplayable()
{
    if (sector_)
    {
        sector_->UnregisterDynamic(*this);
        sector_ = 0;
    }
}

// FUNCTION: SWBF2 0x006deed0
pvs::Manager::~Manager()
{
    for (pvs::Sector** it = sectors_.Begin(); it != sectors_.End(); ++it)
    {
        delete *it;
    }
    for (pvs::Portal** it = portals_.Begin(); it != portals_.End(); ++it)
    {
        delete *it;
    }
}

// FUNCTION: SWBF2 0x006df080
pvs::Sector::~Sector()
{
}

// SYNTHETIC: SWBF2 0x006df2b0
// pvs::BaseDisplayable<pvs::IDisplayable>::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x006ded20
// pvs::BaseDisplayable<pvs::IDisplayable>::~BaseDisplayable<pvs::IDisplayable>

// SYNTHETIC: SWBF2 0x006df2e0
// util::StaticSubAlloc<util::SLNode<pvs::IDisplayable *> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006df310
// pvs::DynDisplayable::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006df340
// pvs::IManager::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006df370
// pvs::Manager::`scalar deleting destructor'



// SYNTHETIC: SWBF2 0x006df3f0
// pvs::Sector::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x006df420
// util::SubAllocator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006df450
void pvs::Sector::AddPortal(pvs::IPortal* pPortal)
{
    portals_.Add(static_cast<pvs::Portal*>(pPortal));
}

// FUNCTION: SWBF2 0x006df4b0
void pvs::Manager::Allocate(int iSectors, int iPortals)
{
    sectors_.Reserve(iSectors);
    portals_.Reserve(iPortals);

    for (int i = 0; i < sectors_.GetSize(); i++)
    {
        sectors_[i] = new Sector;
    }

    for (int i = 0; i < portals_.GetSize(); i++)
    {
        portals_[i] = new Portal;
    }
}

// FUNCTION: SWBF2 0x006df630
void util::IAllocator::AllocateEx(Buffer& rBuf, int&)
{
    Allocate(rBuf);
}

// TEMPLATE: SWBF2 0x006df730
// util::TypedArrayAllocator<pvs::IDisplayable *,util::NewAllocator>::Allocate_
template pvs::IDisplayable** util::TypedArrayAllocator<pvs::IDisplayable*>::Allocate_(int&);
// TEMPLATE: SWBF2 0x006df780
// util::TypedArrayAllocator<pvs::Portal *,util::NewAllocator>::Allocate_
template pvs::Portal** util::TypedArrayAllocator<pvs::Portal*>::Allocate_(int&);
// TEMPLATE: SWBF2 0x006df7d0
// util::TypedArrayAllocator<pvs::Sector *,util::NewAllocator>::Allocate_
template pvs::Sector** util::TypedArrayAllocator<pvs::Sector*>::Allocate_(int&);
// TEMPLATE: SWBF2 0x006df820
// util::TypedArrayAllocator<void *,util::NewAllocator>::Allocate_
template void** util::TypedArrayAllocator<void*>::Allocate_(int&);
// TEMPLATE: SWBF2 0x006df870
// util::TypedArrayAllocator<pvs::SectorCache::Entry,util::NewAllocator>::Allocate_
template pvs::SectorCache::Entry* util::TypedArrayAllocator<pvs::SectorCache::Entry>::Allocate_(int&);

// FUNCTION: SWBF2 0x006dfa00
void pvs::IManager::ClearInstance()
{
    pvs::displayableSuballoc_.Clear();
    delete _man;
    _man = 0;
}

// FUNCTION: SWBF2 0x006dfa30
void util::IAllocator::ClearMemHash()
{
}

// FUNCTION: SWBF2 0x006dfa40
bool pvs::Portal::Clip(const pvs::ICamera& rCam, geo::Box& rBox, const geo::Frustum& rFrustum, const pvs::Sector& rSect)
{
    if (clipMark_ == true)
    {
        return false;
    }

    geo::Vector3<float> camPos;
    PblMatrix camTrans;
    pvs::ISector* pRoot = &IManager::GetInstance().GetRootSector();
    if (gSaveCam)
    {
        camPos = static_cast<Manager&>(IManager::GetInstance()).saveCamPos_;
        camTrans = static_cast<Manager&>(IManager::GetInstance()).saveCamTrans_;
    }
    else
    {
        rCam.GetPos(camPos);
        rCam.GetViewTrans(camTrans);
    }

    geo::Vector3<float> dirToPortal = geo::normalize(sbound_.center_ - camPos);

    bool bRoot = GetLeft() == pRoot || GetRight() == pRoot;

    geo::Vector3<float> norm = norm_;
    if (&rSect != GetLeft())
    {
        norm = norm_ * -1.0f;
    }

    if (gProximityTest)
    {
        if (geo::SepDistance::Op(geo::Sphere(camPos, 3.0f), sbound_) < 0.0f)
        {
            travmark_ = 2;
            return true;
        }
    }

    if (!bRoot && gBackFaceCull)
    {
        if (geo::dot(norm, norm) > 0.1f && geo::dot(norm, dirToPortal) < 0.0f)
        {
            return false;
        }
    }

    if (!geo::Overlap3D::Op(rFrustum, geo::SphereBoundedPoly3D(geo::BasePoly3D(poly_), sbound_)))
    {
        clipMark_ = true;
        travmark_ = 0;
        return false;
    }

    geo::Vector3<float> aPoints[128];
    geo::BasePoly3D camPoly(aPoints, poly_.points_._numElements);
    geo::Transform::Op(geo::BasePoly3D(poly_), camPoly, camTrans);
    geo::ProjClip::Op(camPoly, camPoly);
    geo::Box bound;
    geo::BoundedPoly3D::Bound(camPoly, bound);
    bound.min_.z = 0.0f;
    rBox.min_.z = 0.0f;
    bool bVisible = geo::Intersect3D::Op(rBox, bound, rBox);
    travmark_ = bVisible ? 1 : 0;
    return bVisible;
}

// FUNCTION: SWBF2 0x006dfe40
bool pvs::Sector::Contains(const geo::Vector3<float>& rPoint)
{
    return geo::Overlap3D::Op(bound_, rPoint);
}

// FUNCTION: SWBF2 0x006dfe60
void pvs::Sector::Create()
{
    if (TestWindingOrder() == true)
    {
        windingMult_ = 1.0f;
    }
    else
    {
        windingMult_ = -1.0f;
    }

    sbound_ = geo::Sphere(bound_);
    initdone_ = true;
}

// FUNCTION: SWBF2 0x006dfee0
void util::IAllocator::DeallocateEx(Buffer& rBuf, int&)
{
    Deallocate(rBuf);
}

// FUNCTION: SWBF2 0x006dff50
pvs::IPortal* pvs::Manager::FindNearestPortal(const geo::Sphere& rSphere, float fMaxDist)
{
    pvs::IPortal* pNearest = 0;

    for (Sector** ppSect = sectors_.Begin(); ppSect != sectors_.End(); ++ppSect)
    {
        Sector* pSect = *ppSect;
        if (!pSect || !pSect->Overlap(rSphere))
        {
            continue;
        }

        for (Portal** ppPortal = pSect->portals_.Begin(), **ppEnd = pSect->portals_.End(); ppPortal != ppEnd; ++ppPortal)
        {
            Portal* pPortal = *ppPortal;
            if (!pPortal)
            {
                continue;
            }

            float fX = rSphere.center_.x - pPortal->sbound_.center_.x;
            float fY = rSphere.center_.y - pPortal->sbound_.center_.y;
            float fZ = rSphere.center_.z - pPortal->sbound_.center_.z;
            float fDist = (float)sqrt((double)(fX * fX + fY * fY + fZ * fZ));
            if (fDist < fMaxDist)
            {
                fMaxDist = fDist;
                pNearest = pPortal;
            }
        }
    }

    return pNearest;
}

// FUNCTION: SWBF2 0x006e0060
void pvs::Portal::FindNormal()
{
    geo::BasePoly3D poly(poly_);
    const geo::Vector3<float>& p0 = poly.points_[0];
    const geo::Vector3<float>& p1 = poly.points_[1];
    const geo::Vector3<float>& pn = poly.points_[poly.numPoints_ - 1];

    geo::Vector3<float> edgeA = p1 - p0;
    geo::Vector3<float> edgeB = pn - p0;
    norm_ = geo::normalize(geo::cross(edgeA, edgeB));

    geo::Vector3<float> left = sbound_.center_;
    geo::Vector3<float> right = sbound_.center_;
    pvs::IManager& rMan = pvs::IManager::GetInstance();
    if (left_ && left_ != &rMan.GetRootSector())
    {
        left = left_->GetSphereBound().center_;
    }
    if (right_ && right_ != &rMan.GetRootSector())
    {
        right = right_->GetSphereBound().center_;
    }

    geo::Vector3<float> dir = geo::normalize(right - left);
    if (!(geo::dot(norm_, dir) > 0.0f))
    {
        norm_.x *= -1.0f;
        norm_.y *= -1.0f;
        norm_.z *= -1.0f;
    }
}

// FUNCTION: SWBF2 0x006e0650
bool pvs::Sector::GetActiveTravMark()
{
    return atmark_;
}

// FUNCTION: SWBF2 0x006e0660
geo::ExtrudedPoly3D& pvs::Sector::GetBound()
{
    return bound_;
}

// FUNCTION: SWBF2 0x006e0670
void pvs::Manager::GetCamFrustum(const pvs::ICamera& rCam, geo::Frustum& rFrustum)
{
    geo::Vector3<float> camPos;
    PblMatrix camTrans;
    if (gSaveCam)
    {
        camPos = saveCamPos_;
        camTrans = saveCamTrans_;
    }
    else
    {
        rCam.GetPos(camPos);
        rCam.GetInvViewTrans(camTrans);
    }

    PblVector3 vFwd;
    PblVector3 vBack(0.0f, 0.0f, -1.0f);
    D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(&vFwd), reinterpret_cast<const D3DXVECTOR3*>(&vBack), reinterpret_cast<const D3DXMATRIX*>(&camTrans));
    geo::Vector3<float> fwd(vFwd.x, vFwd.y, vFwd.z);

    const geo::Vector3<float> yAxis(0.0f, 1.0f, 0.0f);
    geo::Vector3<float> side = geo::normalize(geo::cross(yAxis, fwd));
    geo::Vector3<float> up = geo::normalize(geo::cross(fwd, side));

    rFrustum = geo::Frustum(camPos, up, fwd, 100000.0f, 100000.0f, 0.1f, 100000.0f);
}

// FUNCTION: SWBF2 0x006e0910
pvs::IManager& pvs::IManager::GetInstance()
{
    if (!_man)
    {
        _man = new Manager;
    }
    return *_man;
}

// FUNCTION: SWBF2 0x006e0990
pvs::ISector* pvs::Portal::GetLeft()
{
    return left_;
}

// FUNCTION: SWBF2 0x006e09a0
unsigned int util::IAllocator::GetMemHash()
{
    return 0;
}

// FUNCTION: SWBF2 0x006e09b0
geo::Poly3D& pvs::Portal::GetPoly()
{
    return poly_;
}

// FUNCTION: SWBF2 0x006e09c0
const geo::Poly3D& pvs::Portal::GetPoly() const
{
    return poly_;
}

// FUNCTION: SWBF2 0x006e09d0
pvs::IPortal* pvs::Manager::GetPortal(int iIndex)
{
    return portals_[iIndex];
}

// FUNCTION: SWBF2 0x006e09e0
pvs::ISector* pvs::Portal::GetRight()
{
    return right_;
}

// FUNCTION: SWBF2 0x006e09f0
pvs::ISector& pvs::Manager::GetRootSector()
{
    return rootSect_;
}

// FUNCTION: SWBF2 0x006e0a00
pvs::ISector* pvs::Manager::GetSector(int iIndex)
{
    return sectors_[iIndex];
}

// FUNCTION: SWBF2 0x006e0a10
void pvs::BaseCamera::GetSphereBound(geo::Sphere& rSphere) const
{
    geo::Vector3<float> pos;
    GetPos(pos);
    rSphere = geo::Sphere(pos, 4.0f);
}

// FUNCTION: SWBF2 0x006e0a60
geo::Sphere& pvs::Sector::GetSphereBound()
{
    if (!initdone_)
    {
        Create();
    }
    return sbound_;
}

// FUNCTION: SWBF2 0x006e0a80
util::Vector<unsigned int>& pvs::Sector::GetStaticNames()
{
    return staticNames_;
}

// FUNCTION: SWBF2 0x006e0a90
bool pvs::Sector::GetTravMark()
{
    return tmark_;
}

// FUNCTION: SWBF2 0x006e0aa0
void pvs::Portal::Init(pvs::Sector* pLeft, pvs::Sector* pRight)
{
    left_ = pLeft;
    active_ = true;
    right_ = pRight;
}

// FUNCTION: SWBF2 0x006e0ac0
void pvs::Portal::InitBound()
{
    sbound_ = geo::Sphere(geo::BasePoly3D(poly_));
    FindNormal();
}

// TEMPLATE: SWBF2 0x006e0b00
// util::List<pvs::IDisplayable *,util::SLNode<pvs::IDisplayable *> >::InsertLast
template bool util::List<pvs::IDisplayable*, util::SLNode<pvs::IDisplayable*> >::InsertLast(pvs::IDisplayable* const&);

// TEMPLATE: SWBF2 0x006e0b70
// util::Vector<void *,util::BaseArray<void *,util::TNewArrayAlloc<void *> > >::Add
template bool util::Vector<void*>::Add(void* const&);

// FUNCTION: SWBF2 0x006e0bd0
bool pvs::Manager::IsActive()
{
    return gEnable;
}

// FUNCTION: SWBF2 0x006e0be0
bool pvs::Portal::IsActive()
{
    return active_;
}

// FUNCTION: SWBF2 0x006e0bf0
pvs::IPortal* pvs::Manager::NewPortal()
{
    Portal* pPortal = new Portal;
    portals_.Add(pPortal);
    return pPortal;
}

// FUNCTION: SWBF2 0x006e0cc0
pvs::ISector* pvs::Manager::NewSector()
{
    Sector* pSector = new Sector;
    sectors_.Add(pSector);
    return pSector;
}

// FUNCTION: SWBF2 0x006e0370
pvs::Sector* pvs::Manager::FindSector(const geo::Sphere& rSphere)
{
    for (Sector** ppSect = sectors_.Begin(); ppSect != sectors_.End(); ++ppSect)
    {
        Sector* pSect = *ppSect;
        if (pSect && pSect->Overlap(rSphere))
        {
            return pSect;
        }
    }
    return &rootSect_;
}

// FUNCTION: SWBF2 0x006e03c0
pvs::Sector* pvs::Manager::FindSector(const pvs::IDisplayable& rDisp)
{
    geo::Sphere sphere;
    rDisp.GetSphereBound(sphere);
    return FindSector(sphere);
}

// FUNCTION: SWBF2 0x006e03f0
void pvs::Manager::FindSectors(const pvs::BaseCamera& rCam, util::Vector<pvs::Sector*>& rSects, pvs::SectorCache& rCache)
{
    geo::Sphere sbound;
    rCam.GetSphereBound(sbound);
    geo::Sphere center(sbound.center_, 0.0f);

    rSects.SetNumElements(0);
    rCache.entries_.SetSize(sectors_._numElements);

    bool bAddRoot = false;
    for (int i = 0; i < rCache.entries_._numElements; ++i)
    {
        SectorCache::Entry& rEntry = rCache.entries_[i];
        float fDx = sbound.center_.x - rEntry.lastCheckPos_.x;
        float fDy = sbound.center_.y - rEntry.lastCheckPos_.y;
        float fDz = sbound.center_.z - rEntry.lastCheckPos_.z;
        if (fDx * fDx + fDy * fDy + fDz * fDz + sbound.radius_ * sbound.radius_ >=
            rEntry.nextCheckDist_ * rEntry.nextCheckDist_)
        {
            if (!rEntry.sect_)
            {
                rEntry.sect_ = sectors_[i];
            }

            float fDist = geo::SepDistance::Op(sbound, rEntry.sect_->GetSphereBound());
            if (fDist <= 0.0f)
            {
                fDist = 0.0f;
                if (rEntry.sect_->Overlap(sbound))
                {
                    rSects.Add(rEntry.sect_);
                    if (!rEntry.sect_->Overlap(center))
                    {
                        bAddRoot = true;
                    }
                }
            }
            rEntry.nextCheckDist_ = fDist;
            rEntry.lastCheckPos_ = sbound.center_;
        }
    }

    if (bAddRoot || rSects._numElements == 0)
    {
        rSects.Add(&rootSect_);
    }
}

// FUNCTION: SWBF2 0x006e0d70
bool pvs::Sector::Overlap(const geo::Sphere& rSphere)
{
    return geo::Overlap3D::Op(bound_, rSphere);
}

// FUNCTION: SWBF2 0x006e0d90
pvs::Sector* pvs::Sector::PortalCrossingCheck(const geo::Sphere& rSphere, const geo::Vector3<float>& rLastPos, float& rNextCheckDist)
{
    rNextCheckDist = 1e21f;
    for (Portal** ppPortal = portals_.Begin(); ppPortal != portals_.End(); ++ppPortal)
    {
        Portal* pPortal = *ppPortal;
        Sector* pOther = pPortal->GetOther(this);
        float fDist = geo::SepDistance::Op(rSphere, pPortal->sbound_);
        rNextCheckDist = geo::Min_(geo::Max_(fDist, 0.0f), rNextCheckDist);

        float fTol = geo::Max_(geo::Max_((float)fabs(fDist), (float)fabs(0.0f)), 1.0f) * 9.5367431640625e-07f + 0.0f;
        if (fDist <= fTol)
        {
            geo::BasePoly3D portalPoly(pPortal->GetPoly());
            geo::Vector3<float> vDir(rSphere.center_.x - rLastPos.x, rSphere.center_.y - rLastPos.y,
                                     rSphere.center_.z - rLastPos.z);
            if (geo::Overlap3D::Op(portalPoly, geo::Ray<geo::Vector3<float> >(rLastPos, vDir)))
            {
                return pOther;
            }
        }
    }
    return this;
}

// FUNCTION: SWBF2 0x006e0f10
void pvs::Sector::Process(pvs::IDisplayable& rDisp, const pvs::ICamera&, const geo::Box&, pvs::ITraverser*)
{
    int iFrame = GetCurrentFrame_();
    if (!rDisp.IsActive(iFrame))
    {
        rDisp.SetActive(iFrame);
    }
}

// FUNCTION: SWBF2 0x006e0f40
void pvs::Manager::RegisterDynamic(pvs::IDisplayable& rDisp)
{
    if (rDisp.PortalActive())
    {
        Sector* pSector = FindSector(rDisp);
        if (!pSector)
        {
            pSector = &rootSect_;
        }
        pSector->RegisterDynamic(rDisp);
    }
}

// FUNCTION: SWBF2 0x006e0f80
void pvs::Sector::RegisterDynamic(pvs::IDisplayable& rDisp)
{
    if (rDisp.PortalActive())
    {
        dynamics_.InsertLast(&rDisp);
        rDisp.SetSector(this);
    }
}

// FUNCTION: SWBF2 0x006e0fc0
void pvs::Manager::RegisterStatic(pvs::IDisplayable&)
{
}

// FUNCTION: SWBF2 0x006e0fd0
void pvs::Sector::RegisterStatic(pvs::IDisplayable& rDisp)
{
    if (rDisp.PortalActive())
    {
        Sector* pOld = rDisp.GetSector();
        if (pOld)
        {
            pOld->UnregisterDynamic(rDisp);
        }
        statics_.Add(&rDisp);
        rDisp.SetSector(this);
    }
}

// TEMPLATE: SWBF2 0x006e1060
// util::BaseArray<pvs::IDisplayable *,util::TNewArrayAlloc<pvs::IDisplayable *> >::Reserve_
template bool util::BaseArray<pvs::IDisplayable*>::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x006e1130
// util::BaseArray<pvs::Portal *,util::TNewArrayAlloc<pvs::Portal *> >::Reserve_
template bool util::BaseArray<pvs::Portal*>::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x006e1200
// util::BaseArray<pvs::Sector *,util::TNewArrayAlloc<pvs::Sector *> >::Reserve_
template bool util::BaseArray<pvs::Sector*>::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x006e12d0
// util::BaseArray<void *,util::TNewArrayAlloc<void *> >::Reserve_
template bool util::BaseArray<void*>::Reserve_(int, bool);
// TEMPLATE: SWBF2 0x006e13a0
// util::BaseArray<pvs::SectorCache::Entry,util::TNewArrayAlloc<pvs::SectorCache::Entry> >::Reserve_
template bool util::BaseArray<pvs::SectorCache::Entry>::Reserve_(int, bool);

// FUNCTION: SWBF2 0x006e14b0
void pvs::Manager::SetActive(bool bActive)
{
    gEnable = bActive;
}

// FUNCTION: SWBF2 0x006e14c0
void pvs::Portal::SetActive(bool bActive)
{
    active_ = bActive;
}

// FUNCTION: SWBF2 0x006e14d0
void pvs::Sector::SetActiveTravMark(bool bMark)
{
    atmark_ = bMark;
}

// FUNCTION: SWBF2 0x006e14e0
void pvs::Sector::SetBound(const geo::ExtrudedPoly3D& rBound)
{
    bound_ = rBound;
}

// FUNCTION: SWBF2 0x006e15a0
void pvs::IManager::SetInstance(IManager& rManager)
{
    _man = &rManager;
}

// FUNCTION: SWBF2 0x006e15b0
void pvs::Portal::SetLeft(pvs::ISector* pLeft)
{
    left_ = static_cast<pvs::Sector*>(pLeft);
}

// FUNCTION: SWBF2 0x006e15c0
void util::IAllocator::SetMaxMem(int)
{
}

// FUNCTION: SWBF2 0x006e15d0
void pvs::Portal::SetRight(pvs::ISector* pRight)
{
    right_ = static_cast<pvs::Sector*>(pRight);
}

template <class T>
void pvs::BaseDisplayable<T>::SetSector(pvs::Sector* pSector)
{
    if (!this->PortalActive())
    {
        sector_ = pSector;
        return;
    }
    if (pSector)
    {
        this->SetPortalActive(true);
    }
    sector_ = pSector;
}

// TEMPLATE: SWBF2 0x006e15e0
// pvs::BaseDisplayable<pvs::IDisplayable>::SetSector
template void pvs::BaseDisplayable<pvs::IDisplayable>::SetSector(pvs::Sector*);

// FUNCTION: SWBF2 0x006e1620
void pvs::DynDisplayable::SetSector(pvs::Sector* pSector)
{
    if (!PortalActive())
    {
        sector_ = pSector;
        return;
    }
    if (pSector)
    {
        SetPortalActive(true);
    }
    sector_ = pSector;
}

// TEMPLATE: SWBF2 0x006e1660
// util::Vector<pvs::SectorCache::Entry,util::BaseArray<pvs::SectorCache::Entry,util::TNewArrayAlloc<pvs::SectorCache::Entry> > >::SetSize_
template bool util::Vector<pvs::SectorCache::Entry>::SetSize_(int, bool);

// FUNCTION: SWBF2 0x006e1710
void pvs::Sector::SetTravMark(bool bMark)
{
    tmark_ = bMark;
}

// FUNCTION: SWBF2 0x006e1720
bool pvs::Sector::TestWindingOrder()
{
    if (bound_.points_._numElements < 2)
    {
        return false;
    }

    const geo::Vector2<float>& rP0 = bound_.points_._elements[0];
    const geo::Vector2<float>& rP1 = bound_.points_._elements[1];
    geo::Vector3<float> p0(rP0.x, rP0.y, 0.0f);
    geo::Vector3<float> p1(rP1.x, rP1.y, 0.0f);
    geo::Vector3<float> edgeDir = geo::normalize(p1 - p0);
    geo::Vector3<float> edgeNormal = geo::cross(edgeDir, geo::Vector3<float>(0.0f, 0.0f, 1.0f));
    geo::Vector3<float> edgeMid = (p0 + p1) * 0.5f;
    geo::Vector3<float> vTest = edgeMid + edgeNormal * -0.00001f;

    bool bResult = Contains(vTest);
    windingMult_ = bResult ? 1.0f : -1.0f;
    return bResult;
}

// FUNCTION: SWBF2 0x006e1870
void pvs::Sector::Traverse(const pvs::ICamera& rCam, const geo::Box& rBox, const geo::Frustum& rFrustum, pvs::ITraverser* pTrav, pvs::Sector*)
{
    if (!GetTravMark())
    {
        for (util::SLNode<pvs::IDisplayable*>* pNode = dynamics_.first_; pNode; pNode = pNode->next_)
        {
            if (pNode->elem_)
            {
                Process(*pNode->elem_, rCam, rBox, pTrav);
            }
        }
        for (pvs::IDisplayable** ppDisp = statics_.Begin(); ppDisp != statics_.End(); ++ppDisp)
        {
            if (*ppDisp)
            {
                Process(**ppDisp, rCam, rBox, pTrav);
            }
        }
    }
    SetTravMark(true);

    geo::Vector3<float> camPos;
    if (gSaveCam)
    {
        camPos = static_cast<Manager&>(IManager::GetInstance()).saveCamPos_;
    }
    else
    {
        rCam.GetPos(camPos);
    }

    SetActiveTravMark(true);
    for (Portal** ppPortal = portals_.Begin(); ppPortal != portals_.End(); ++ppPortal)
    {
        Portal* pPortal = *ppPortal;
        if (!pPortal)
        {
            continue;
        }
        Sector* pOther = pPortal->GetOther(this);
        if (pOther->GetActiveTravMark())
        {
            continue;
        }
        geo::Box box = rBox;
        if (pPortal->Clip(rCam, box, rFrustum, *this))
        {
            pOther->Traverse(rCam, box, rFrustum, pTrav, pPortal->IsActive() ? this : 0);
        }
    }
    SetActiveTravMark(false);
}

// FUNCTION: SWBF2 0x006e19f0
void pvs::Manager::TraversePortals(const pvs::ICamera& rCam, pvs::ITraverser* pTrav)
{
    const BaseCamera* pCam = static_cast<const BaseCamera*>(&rCam);
    const geo::Vector3<float> boxMin(-1.0f, -1.0f, 1.0f);
    const geo::Vector3<float> boxMax(1.0f, 1.0f, 1.0f);
    geo::Box box(boxMin, boxMax);
    geo::Frustum frustum;
    static_cast<Manager&>(IManager::GetInstance()).GetCamFrustum(*pCam, frustum);
    FindSectors(*pCam, camsectors_, scache_);
    for (int i = 0; i < camsectors_._numElements; ++i)
    {
        Sector* pSector = camsectors_[i];
        pSector->Traverse(*pCam, box, frustum, pTrav, pSector);
    }
}

// FUNCTION: SWBF2 0x006e1b00
void pvs::Manager::UnregisterDynamic(pvs::IDisplayable& rDisp)
{
    Sector* pSector = rDisp.GetSector();
    if (pSector)
    {
        pSector->UnregisterDynamic(rDisp);
    }
}

// FUNCTION: SWBF2 0x006e1b20
void pvs::Sector::UnregisterDynamic(pvs::IDisplayable& rDisp)
{
    dynamics_.FindRemoveSlow(&rDisp);
    rDisp.SetSector(0);
}

// FUNCTION: SWBF2 0x006e1b40
void pvs::Manager::UnregisterStatic(pvs::IDisplayable& rDisp)
{
    rDisp.GetSector()->UnregisterDynamic(rDisp);
}

// FUNCTION: SWBF2 0x006e1b60
void pvs::Sector::UnregisterStatic(pvs::IDisplayable& rDisp)
{
    statics_.FindRemoveLazy(&rDisp);
    rDisp.SetSector(0);
}

template <class T>
void pvs::BaseDisplayable<T>::Update(pvs::Manager&)
{
}

// TEMPLATE: SWBF2 0x006e1bc0
// pvs::BaseDisplayable<pvs::IDisplayable>::Update
template void pvs::BaseDisplayable<pvs::IDisplayable>::Update(pvs::Manager&);

// FUNCTION: SWBF2 0x006e1bd0
void pvs::DynDisplayable::Update(pvs::Manager& rMan)
{
    geo::Sphere sphere;
    GetSphereBound(sphere);

    float fDist = geo::length(lastCheckPos_ - sphere.center_);

    bool bInside = true;
    if (rand() % 10 == 3)
    {
        bInside = sector_->Overlap(sphere);
    }

    if (!bInside || !(nextCheckDist_ >= fDist))
    {
        nextCheckDist_ = 10.0f;
        Sector* pNew = sector_->PortalCrossingCheck(sphere, lastCheckPos_, nextCheckDist_);

        if (bInside)
        {
            if (sector_ != &rMan.GetRootSector())
            {
                bInside = sector_->Overlap(sphere);
            }
            if (bInside && pNew != sector_)
            {
                bInside = pNew->Overlap(sphere);
            }
        }

        if (!bInside)
        {
            pNew = rMan.FindSector(sphere);
            nextCheckDist_ = -1.0f;
        }

        if (pNew != sector_)
        {
            if (!pNew)
            {
                pNew = (Sector*)&rMan.GetRootSector();
            }
            sector_->UnregisterDynamic(*this);
            pNew->RegisterDynamic(*this);
        }

        lastCheckPos_ = sphere.center_;
    }
}

// GLOBAL: SWBF2 0x009a0834
bool gSaveCam;

// SYNTHETIC: SWBF2 0x00408810
// `dynamic initializer for 'g_SaveCam''
static RedCommandConsole g_SaveCam("vis.SaveCam", MultiVar(&gSaveCam));

// FUNCTION: SWBF2 0x006e1d40
void pvs::Manager::Update(const pvs::ICamera& rCam, pvs::ITraverser* pTrav)
{
    if (!gEnable)
    {
        return;
    }

    BaseCamera* pCam = (BaseCamera*)&rCam;

    if (gSaveCam != lastSaveCam_)
    {
        if (gSaveCam == true)
        {
            pCam->GetPos(saveCamPos_);
            pCam->GetInvViewTrans(saveCamTrans_);
            saveCamSect_ = pCam->GetSector();
        }
        lastSaveCam_ = gSaveCam;
    }

    if (gSaveCam)
    {
        return;
    }

    for (Sector** ppSect = sectors_.Begin(); ppSect != sectors_.End(); ++ppSect)
    {
        if (*ppSect)
        {
            (*ppSect)->SetTravMark(false);
        }
    }
    rootSect_.SetTravMark(false);

    for (Portal** ppPortal = portals_.Begin(); ppPortal != portals_.End(); ++ppPortal)
    {
        if (*ppPortal)
        {
            (*ppPortal)->clipMark_ = false;
            (*ppPortal)->travmark_ = 0;
        }
    }

    ++_cframe;
    UpdateDynamics();
    TraversePortals(rCam, pTrav);
}

// FUNCTION: SWBF2 0x006e1e40
void pvs::Manager::UpdateDynamics(pvs::Sector& rSect)
{
    for (util::SLNode<pvs::IDisplayable*>* pNode = rSect.dynamics_.first_; pNode; pNode = pNode->next_)
    {
        if (pNode->elem_)
        {
            pNode->elem_->Update(*this);
        }
    }
}

// FUNCTION: SWBF2 0x006e1e70
void pvs::Manager::UpdateDynamics()
{
    for (Sector** ppSect = sectors_.Begin(); ppSect != sectors_.End(); ++ppSect)
    {
        UpdateDynamics(**ppSect);
    }
    UpdateDynamics(rootSect_);
}

// GLOBAL: SWBF2 0x007df865
bool gBackFaceCull = true;

// GLOBAL: SWBF2 0x007df866
bool gProximityTest = true;

// GLOBAL: SWBF2 0x009a0835
bool gDebugDraw;

// SYNTHETIC: SWBF2 0x004087b0
// `dynamic initializer for 'g_BackFaceCull''
static RedCommandConsole g_BackFaceCull("vis.BackFaceCull", MultiVar(&gBackFaceCull));

// SYNTHETIC: SWBF2 0x004087e0
// `dynamic initializer for 'g_ProximityTest''
static RedCommandConsole g_ProximityTest("vis.ProximityTest", MultiVar(&gProximityTest));

// SYNTHETIC: SWBF2 0x00408910
// `dynamic initializer for 'g_DebugDraw''
static RedCommandConsole g_DebugDraw("vis.DebugDraw", MultiVar(&gDebugDraw));
