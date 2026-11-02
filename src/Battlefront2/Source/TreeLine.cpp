#include "PebbleFL/Common/PblMath.h"
#include "PropGenerator.h"
#include "PebbleFL/Common/PblVector.h"
#include <float.h>
#include <math.h>
#include "CameraManager.h"
#include "PropModelMesh.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/RedPath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Entity.h"

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace TreeLine
{
void CreateTreeLine(PblFileChunk& chunk);
}

// FUNCTION: SWBF2 0x0065c890
void CreateTreeLineOnPath(unsigned int uPathName, PblFileChunk& chunk)
{
    EntityClass* aClasses[10];
    EntityClass* aOtherClasses[5];
    int iNumClasses = 0;
    int iNumOtherClasses = 0;
    float fSpacing = 18.0f;
    bool bReverse = false;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        unsigned int uProperty = child._pFile->Read32();
        child._pFile->Read8();
        switch (uProperty)
        {
        case 0x6850acc6:
            aClasses[iNumClasses] = EntityClass::Find(child._pFile->Read32());
            if (aClasses[iNumClasses])
            {
                ++iNumClasses;
            }
            break;
        case 0xef838cc7:
            aOtherClasses[iNumOtherClasses] = EntityClass::Find(child._pFile->Read32());
            if (aOtherClasses[iNumOtherClasses])
            {
                ++iNumOtherClasses;
            }
            break;
        case 0xbad71c66:
            bReverse = true;
            break;
        case 0x2eb31462:
            fSpacing = child._pFile->ReadFloat();
            break;
        }
    }

    const RedPath* pPath = RedPath::Find(uPathName);
    if (!pPath)
    {
        return;
    }

    PblVector3 vPrevious = pPath->GetPointPosition(0);
    for (int iPoint = 1; iPoint < pPath->GetNumPoints(); ++iPoint)
    {
        const PblVector3 vEnd = pPath->GetPointPosition(iPoint);
        // The remaining offset to the point, normalised in place into the step direction.
        PblVector3 vStep = vEnd - vPrevious;
        while (SquaredLength(vStep) > fSpacing * fSpacing)
        {
            const float fInvLength = 1.0f / PblMath::Sqrt(vStep.x * vStep.x + vStep.y * vStep.y + vStep.z * vStep.z);
            vStep.x *= fInvLength;
            vStep.y *= fInvLength;
            vStep.z *= fInvLength;
            if (iNumClasses > 0)
            {
                EntityDesc desc;
                const float fSign = bReverse ? -1.0f : 1.0f;
                const PblVector3 vUp(0.0f, 1.0f, 0.0f);
                const PblVector3 vCenter(vPrevious.x + vStep.x * fSpacing * 0.5f,
                                         vPrevious.y + vStep.y * fSpacing * 0.5f,
                                         vPrevious.z + vStep.z * fSpacing * 0.5f);
                const PblVector3 vForward = CrossProduct(vStep, vUp) * fSign;
                desc.mMatrix = DirectionMatrix(vForward, vUp, vCenter);
                aClasses[PblRandom::_Global.GetInt() % iNumClasses]->Build(desc);
            }
            vPrevious.x += vStep.x * fSpacing;
            vPrevious.y += vStep.y * fSpacing;
            vPrevious.z += vStep.z * fSpacing;
            vStep = vEnd - vPrevious;
        }
    }
}

// FUNCTION: SWBF2 0x0065cd80
void TreeLine::CreateTreeLine(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uProperty = child._pFile->Read32();
        child._pFile->Read8();
        if (uProperty == 0x84874d36)
        {
            const unsigned int uPathName = child._pFile->Read32();
            PblFileChunk pathChunk = chunk.ReadNextChild();
            CreateTreeLineOnPath(uPathName, pathChunk);
        }
    }
}
