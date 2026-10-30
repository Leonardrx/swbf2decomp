#pragma once

#include <stddef.h>
#include <string.h>
#include "PblVector.h"

class PblVector3;
class PblVector4;
class PblQuaternion;
struct _D3DXMATRIXA16;

class __declspec(align(16)) PblMatrix
{
public:

    // FUNCTION: SWBF2 0x0040fe00
    PblMatrix() {}

    // FUNCTION: SWBF2 0x0040cb30
    PblMatrix(const PblMatrix& other)
    {
        memcpy(m, other.m, sizeof(m));
    }

    PblMatrix(const _D3DXMATRIXA16& other)
    {
        memcpy(m, &other, sizeof(m));
    }

    // FUNCTION: SWBF2 0x0043a880
    PblMatrix(const _D3DXMATRIXA16* other)
    {
        memcpy(m, other, sizeof(m));
    }

    PblMatrix(const PblQuaternion& rotation, const PblVector3& translation);

    PblMatrix(const PblVector4& x, const PblVector4& y,
              const PblVector4& z, const PblVector4& t);

    PblMatrix(const PblVector3& x, const PblVector3& y,
              const PblVector3& z, const PblVector3& t);

    const PblMatrix& operator*=(const PblMatrix& rhs);

    const PblMatrix GetInverse() const;
    const PblMatrix GetInverseFast() const;

    const PblVector3 ApplyRotation(const PblVector3& vector) const;
    const PblVector3 ApplyTransform(const PblVector3& vector) const;
    const PblVector3 ApplyTranslation(const PblVector3& vector) const;

    void Reset();

    void SetRotateX(float fAngle);
    void SetRotateY(float fAngle);
    void SetRotateZ(float fAngle);
    void RotateX(float fAngle);
    void RotateY(float fAngle);
    void RotateZ(float fAngle);
    void Scale(float fScaleX, float fScaleY, float fScaleZ);
    void Scale(float fScale) { Scale(fScale, fScale, fScale); }

    void Transpose3x3();

    void Transpose4x4()
    {
        float t;
        t = m[0][1]; m[0][1] = m[1][0]; m[1][0] = t;
        t = m[0][2]; m[0][2] = m[2][0]; m[2][0] = t;
        t = m[0][3]; m[0][3] = m[3][0]; m[3][0] = t;
        t = m[1][2]; m[1][2] = m[2][1]; m[2][1] = t;
        t = m[1][3]; m[1][3] = m[3][1]; m[3][1] = t;
        t = m[2][3]; m[2][3] = m[3][2]; m[3][2] = t;
    }

    // FUNCTION: SWBF2 0x00410890
    const PblVector3& GetTranslation() const { return *(const PblVector3*)&m[3][0]; }

    const PblVector3& GetXAxis() const { return *(const PblVector3*)&m[0][0]; }
    const PblVector3& GetYAxis() const { return *(const PblVector3*)&m[1][0]; }

    // FUNCTION: SWBF2 0x0040cb70
    const PblVector3& GetZAxis() const { return *(const PblVector3*)&m[2][0]; }

    void SetXAxis(float fX, float fY, float fZ, float fW);
    void SetYAxis(float fX, float fY, float fZ, float fW);
    void SetZAxis(float fX, float fY, float fZ, float fW);
    void SetTranslation(float fX, float fY, float fZ, float fW);
    void SetTranslation(const PblVector3& vTranslation);

    float& operator()(int iRow, int iColumn);
    float operator()(int iRow, int iColumn) const;

    PblVector3& operator[](int iRow);
    const PblVector3& operator[](int iRow) const;

    float m[4][4];
};

#include "PebbleFL/pc/pcPblMatrix.h"

extern PblMatrix gMatrixIdentity;
