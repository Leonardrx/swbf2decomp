#ifndef SWBF2_REDENGINEFL_GRAPHICS_REDMATERIAL_H
#define SWBF2_REDENGINEFL_GRAPHICS_REDMATERIAL_H

#include <stddef.h>
#include <d3d9.h>

#include "RedColor.h"
#include "RedTextureTransform.h"

struct RedMaterialAnimationData;

class RedColorValue
{
public:

    // FUNCTION: SWBF2 0x0048ebf0
    RedColorValue() {}
    RedColorValue(float fR, float fG, float fB, float fA) : r(fR), g(fG), b(fB), a(fA) {}
    RedColorValue(const RedColorValue& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
    RedColorValue(const RedColor& color);

    operator D3DCOLORVALUE() const
    {
        return *reinterpret_cast<const D3DCOLORVALUE*>(this);
    }

    float r;
    float g;
    float b;
    float a;

    static RedColorValue WHITE;

    static RedColorValue BLACK;
};

// FUNCTION: SWBF2 0x004bd900
inline RedColorValue::RedColorValue(const RedColor& color)
{
    const float kInv255 = 1.0f / 255.0f;
    r = color.r * kInv255;
    g = color.g * kInv255;
    b = color.b * kInv255;
    a = color.a * kInv255;
}
 
 
 
 
 

class RedLight;

struct RedMaterialAnimationData
{
    RedColorValue diffuseColor;
    unsigned int attachedLightName;
    RedLight* attachedLight;
    unsigned char scrollRateU;
    unsigned char scrollRateV;
    unsigned char minIntensity;
    unsigned char blinkRate;
    unsigned char numRows;
    unsigned char numCols;
    unsigned char frameRate;
};
 

struct RedMaterial
{
    unsigned int diffuseMapHash;
    unsigned int detailMapHash;
    unsigned int bumpMapHash;
    unsigned int envMapHash;
    unsigned int materialFlags;
    RedColorValue diffuseColor;
    RedColorValue specularColor;
    float specularPower;
    RedTextureTransform diffuseTexTransform;
    RedTextureTransform detailTexTransform;
    RedMaterialAnimationData* animationData;
};
 
 
 
 
 
 
 
 

#endif
