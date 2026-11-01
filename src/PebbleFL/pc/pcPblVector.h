#pragma once

// FUNCTION: SWBF2 0x0040ca30
inline const PblVector3 operator+(const PblVector3& lhs, const PblVector3& rhs)
{
    PblVector3 Temp;
    Temp.x = lhs.x + rhs.x;
    Temp.y = lhs.y + rhs.y;
    Temp.z = lhs.z + rhs.z;
    return Temp;
}

// FUNCTION: SWBF2 0x0040ca70
inline const PblVector3 operator-(const PblVector3& lhs, const PblVector3& rhs)
{
    PblVector3 Temp;
    D3DXVec3Subtract(&Temp, &lhs, &rhs);
    return Temp;
}

// FUNCTION: SWBF2 0x0040cab0
inline const PblVector3 operator*(const PblVector3& lhs, float rhs)
{
    PblVector3 Temp;
    D3DXVec3Scale(&Temp, &lhs, rhs);
    return Temp;
}

// FUNCTION: SWBF2 0x0040caf0
inline const PblVector3 operator/(const PblVector3& lhs, float rhs)
{
    return lhs * (1.0f / rhs);
}

// FUNCTION: SWBF2 0x00410b60
inline float DotProduct(const PblVector3& vVector1, const PblVector3& vVector2)
{
    return D3DXVec3Dot(&vVector1, &vVector2);
}

inline PblVector4::operator const PblVector3&() const
{
    return *reinterpret_cast<const PblVector3*>(this);
}

// FUNCTION: SWBF2 0x004213a0
inline const float Length(const PblVector3& vVector)
{
    return D3DXVec3Length(&vVector);
}

// FUNCTION: SWBF2 0x0041bba0
inline const float SquaredLength(const PblVector3& vVector)
{
    return D3DXVec3LengthSq(&vVector);
}

inline const PblVector3& PblVector3::operator+=(const PblVector3& rhs)
{
    D3DXVec3Add(this, this, &rhs);
    return *this;
}

inline const PblVector3 PblVector3::operator-() const
{
    return PblVector3(-x, -y, -z);
}

inline const PblVector3& PblVector3::operator-=(const PblVector3& rhs)
{
    D3DXVec3Subtract(this, this, &rhs);
    return *this;
}

// FUNCTION: SWBF2 0x004171f0
inline const PblVector3 CrossProduct(const PblVector3& lhs, const PblVector3& rhs)
{
    PblVector3 Temp;
    D3DXVec3Cross(&Temp, &lhs, &rhs);
    return Temp;
}

inline const PblVector3& PblVector3::operator*=(float rhs)
{
    D3DXVec3Scale(this, this, rhs);
    return *this;
}

inline const PblVector3& PblVector3::operator/=(float rhs)
{
    D3DXVec3Scale(this, this, 1.0f / rhs);
    return *this;
}

// FUNCTION: SWBF2 0x00417260
inline const PblVector3 Normalize(const PblVector3& vVector)
{
    PblVector3 Temp;
    D3DXVec3Normalize(&Temp, &vVector);
    return Temp;
}

inline void PblVector3::AddY(float value)
{
    y += value;
}

inline void PblVector3::MulY(float value)
{
    y *= value;
}

inline const float LengthXZ(const PblVector3& vector)
{
    return PblMath::Sqrt(vector.x * vector.x + vector.z * vector.z);
}

inline const float SquaredLengthXZ(const PblVector3& vector)
{
    return vector.x * vector.x + vector.z * vector.z;
}

inline float& PblVector3::operator[](int i)
{
    return (&x)[i];
}

inline float PblVector3::operator[](int i) const
{
    return (&x)[i];
}

inline float& PblVector3::operator()(int i)
{
    return (&x)[i];
}

inline float PblVector3::operator()(int i) const
{
    return (&x)[i];
}

inline const PblVector3 operator*(float lhs, const PblVector3& rhs)
{
    PblVector3 Temp;
    Temp.x = lhs * rhs.x;
    Temp.y = lhs * rhs.y;
    Temp.z = lhs * rhs.z;
    return Temp;
}

inline const PblVector3 operator*(const PblVector3& lhs, const PblVector3& rhs)
{
    PblVector3 Temp;
    Temp.x = lhs.x * rhs.x;
    Temp.y = lhs.y * rhs.y;
    Temp.z = lhs.z * rhs.z;
    return Temp;
}

inline PblVector4::PblVector4(float fValue)
    : D3DXVECTOR4(fValue, fValue, fValue, fValue)
{
}
