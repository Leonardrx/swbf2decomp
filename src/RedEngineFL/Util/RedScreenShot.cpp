#include "RedScreenShot.h"

#include <direct.h>
#include <new>
#include <stdio.h>
#include <math.h>

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

// FUNCTION: SWBF2 0x006f33a0
template <class T>
Accumulator<T>::Accumulator()
{
    m_bufferSize = 176128;
    int iHeap = RedSetCurrentHeap(__RedDebugHeap);
    m_buffer = new T[m_bufferSize];
    RedSetCurrentHeap(iHeap);
}

namespace RedScreenShot
{

// FUNCTION: SWBF2 0x006f3ad0
bool StitchImage(Accumulator<RedScreenShot::Pixel>* pAccumulators, int iTilesX, int iTilesY,
                int iSamplesPerPixel, const RedScreenShot::GammaRamp& gamma, const char* pcFileName,
                unsigned short uiWidth, unsigned short uiHeight)
{
    PblFile file;
    if (!file.OpenCreate(pcFileName, false, true, false))
    {
        return false;
    }

    struct TgaHeader
    {
        unsigned char idLength;
        unsigned char colorMapType;
        unsigned char imageType;
        unsigned char colorMapSpec[5];
        unsigned short xOrigin;
        unsigned short yOrigin;
        unsigned short width;
        unsigned short height;
        unsigned char bitsPerPixel;
        unsigned char descriptor;
    };
    const unsigned short outWidth = static_cast<unsigned short>(uiWidth * iTilesX);
    const unsigned short outHeight = static_cast<unsigned short>(uiHeight * iTilesY);
    TgaHeader header;
    header.idLength = 0;
    header.colorMapType = 0;
    header.imageType = 2;
    header.bitsPerPixel = 24;
    header.descriptor = 32;
    header.width = outWidth;
    memset(header.colorMapSpec, 0, sizeof(header.colorMapSpec));
    header.xOrigin = 0;
    header.yOrigin = 0;
    header.height = outHeight;
    file.WriteData(&header, sizeof(header));

    const int iRowBytes = 3 * uiWidth;
    const int iBufferSize = 8 * iRowBytes;
    unsigned char* pBuffer = new unsigned char[iBufferSize];
    unsigned char* const pBufferEnd = pBuffer + iBufferSize;
    unsigned char* pNext = pBuffer;
    int iPixelBase = 0;
    for (unsigned int y = 0; y < outHeight; ++y)
    {
        for (int iTileY = 0; iTileY < iTilesY; ++iTileY)
        {
            for (int x = 0; x < uiWidth; ++x)
            {
                const int iPixelIndex = x + iPixelBase;
                for (int iTileX = 0; iTileX < iTilesX; ++iTileX)
                {
                    Accumulator<RedScreenShot::Pixel>& accum = pAccumulators[iTileY * iTilesX + iTileX];
                    const RedScreenShot::Pixel pixel = accum.Get(iPixelIndex);
                    *pNext++ = gamma.blue[static_cast<unsigned char>(static_cast<int>(pixel.b / iSamplesPerPixel * 255.0f))];
                    *pNext++ = gamma.green[static_cast<unsigned char>(static_cast<int>(pixel.g / iSamplesPerPixel * 255.0f))];
                    *pNext++ = gamma.red[static_cast<unsigned char>(static_cast<int>(pixel.r / iSamplesPerPixel * 255.0f))];
                }
                if (pNext + iRowBytes > pBufferEnd)
                {
                    file.WriteData(pBuffer, static_cast<int>(pNext - pBuffer));
                    pNext = pBuffer;
                }
            }
        }
        iPixelBase += uiWidth;
    }
    if (pNext != pBuffer)
    {
        file.WriteData(pBuffer, static_cast<int>(pNext - pBuffer));
    }
    delete[] pBuffer;
    file.Close();
    return true;
}
}

// FUNCTION: SWBF2 0x006f3430
template <class T>
Accumulator<T>::~Accumulator()
{
    Close();
    int iOldHeap = RedSetCurrentHeap(__RedDebugHeap);
    delete[] m_buffer;
    RedSetCurrentHeap(iOldHeap);
}

// FUNCTION: SWBF2 0x006f39d0
template <class T>
bool Accumulator<T>::Open()
{
    int i = 0;
    do
    {
        sprintf(m_fileName, "ScreenShots\\temp%03d", i++);
    } while (PblFile::Exists(m_fileName));

    if (!m_file.OpenCreate(m_fileName, false, false, false))
    {
        return false;
    }

    m_curPos = 0;
    m_bufferOffset = 0;
    m_bufferDirty = false;
    return true;
}

// FUNCTION: SWBF2 0x006f36a0
template <class T>
void Accumulator<T>::Clear(int iValue, int iNumBytes)
{
    int iRemaining = iNumBytes;
    int iBufferBytes = m_bufferSize * sizeof(T);
    m_file.Seek(0, PblFile::SeekFromStart);
    memset(m_buffer, iValue, iBufferBytes);
    while (iRemaining)
    {
        int iWrite = iRemaining > iBufferBytes ? iBufferBytes : iRemaining;
        m_file.WriteData(m_buffer, iWrite);
        iRemaining -= iWrite;
    }
    m_file.Seek(0, PblFile::SeekFromStart);
    m_file.ReadData(m_buffer, m_bufferSize * sizeof(T));
    m_bufferOffset = 0;
    m_curPos = 0;
    m_bufferDirty = false;
}

// FUNCTION: SWBF2 0x006f3a30
template <class T>
void Accumulator<T>::Seek(int iIndex)
{
    int iPos = iIndex * sizeof(T);
    int iOffset = (iPos - m_curPos) / sizeof(T);
    if (iOffset >= 0 && iOffset < m_bufferSize)
    {
        m_bufferOffset = iOffset;
        return;
    }

    if (m_bufferDirty)
    {
        m_file.Seek(m_curPos, PblFile::SeekFromStart);
        m_file.WriteData(m_buffer, m_bufferSize * sizeof(T));
        m_bufferDirty = false;
    }
    m_file.Seek(iPos, PblFile::SeekFromStart);
    if (iPos < m_file.GetSize())
    {
        m_file.ReadData(m_buffer, m_bufferSize * sizeof(T));
    }
    m_curPos = iPos;
    m_bufferOffset = 0;
}

template class Accumulator<RedScreenShot::Pixel>;

namespace RedScreenShot
{

// GLOBAL: SWBF2 0x007e1490
int size_increase = 1;
// GLOBAL: SWBF2 0x007e1494
int samples_per_pixel = 1;
// GLOBAL: SWBF2 0x007e1498
float gamma_correction = 1.2f;

// FUNCTION: SWBF2 0x006f3730
void GPUSync()
{
    IDirect3DQuery9* pQuery;
    RedRenderer::pcGetDevice()->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery);
    if (pQuery)
    {
        pQuery->Issue(D3DISSUE_END);
        BOOL bData;
        while (pQuery->GetData(&bData, sizeof(bData), D3DGETDATA_FLUSH) == S_FALSE)
        {
            Sleep(0);
        }
        pQuery->Release();
    }
}

// FUNCTION: SWBF2 0x006f37a0
void GetFrameBufferDimensions(unsigned int& uiWidth, unsigned int& uiHeight)
{
    D3DDISPLAYMODE mode;
    RedRenderer::pcGetDevice()->GetDisplayMode(0, &mode);
    uiWidth = mode.Width;
    uiHeight = mode.Height;
}

// FUNCTION: SWBF2 0x006f37f0
bool GetPoissonDiscSamples(SamplePoint* pSamples, int iNumSamples, const SamplePoint& start,
                           float fMinDist, const SamplePoint& min, const SamplePoint& max)
{
    const float fMinDistSq = fMinDist * fMinDist;
    pSamples[0] = start;
    --iNumSamples;
    int iNew = 1;
    for (int i = 0; i < iNumSamples; ++i, ++iNew)
    {
        int iAttempts = 0;
        bool bPlaced = false;
        while (!bPlaced)
        {
            pSamples[iNew].x = (max.x - min.x) * PblRandom::_Global.GetFloat() + min.x;
            pSamples[iNew].y = (max.y - min.y) * PblRandom::_Global.GetFloat() + min.y;

            bPlaced = true;
            for (int j = 0; j < iNew; ++j)
            {
                float dy = pSamples[iNew].y - pSamples[j].y;
                float dx = pSamples[iNew].x - pSamples[j].x;
                if (fMinDistSq > dx * dx + dy * dy)
                {
                    bPlaced = false;
                    break;
                }
            }
            if (!bPlaced && ++iAttempts >= 100)
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006f3960
bool GetScreenShotFileName(char* const pcFileName)
{
    if (!PblFile::Exists("ScreenShots"))
    {
        _mkdir("ScreenShots");
    }

    bool bFound = false;
    for (int i = 0; i < 10000; ++i)
    {
        sprintf(pcFileName, "ScreenShots\\screenshot_%04d.tga", i);
        if (!PblFile::Exists(pcFileName))
        {
            bFound = true;
            break;
        }
    }
    return bFound;
}

// FUNCTION: SWBF2 0x006f34c0
void AccumulateFrameBuffer(Accumulator<Pixel>& accum)
{
    GPUSync();

    D3DDISPLAYMODE mode;
    RedRenderer::pcGetDevice()->GetDisplayMode(0, &mode);
    IDirect3DSurface9* pSurface;
    RedRenderer::pcGetDevice()->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8,
                                                            D3DPOOL_SYSTEMMEM, &pSurface, NULL);
    RedRenderer::pcGetDevice()->GetFrontBufferData(0, pSurface);

    D3DSURFACE_DESC desc;
    pSurface->GetDesc(&desc);
    D3DLOCKED_RECT rect;
    pSurface->LockRect(&rect, NULL, D3DLOCK_READONLY);

    for (unsigned short y = 0; y < desc.Height; ++y)
    {
        const unsigned int* pRow = (const unsigned int*)((unsigned char*)rect.pBits + rect.Pitch * y);
        for (unsigned short x = 0; x < desc.Width; ++x)
        {
            const unsigned char ucBlue = (unsigned char)pRow[x];
            const unsigned int uColor = pRow[x];
            const unsigned char ucGreen = (unsigned char)(uColor >> 8);
            const unsigned char ucRed = (unsigned char)(uColor >> 16);
            int iIndex = x + desc.Width * y;
            Pixel pixel = accum.Get(iIndex);
            pixel.r += ucRed / 255.0f;
            pixel.g += ucGreen / 255.0f;
            pixel.b += ucBlue / 255.0f;
            accum.Set(iIndex, pixel);
        }
    }

    pSurface->UnlockRect();
    pSurface->Release();
}

}

namespace RedScreenShot
{

// FUNCTION: SWBF2 0x006f3d40
void TakeScreenShot()
{
    using namespace RedScreenShot;
    char acFileName[52];
    if (!GetScreenShotFileName(acFileName))
    {
        return;
    }

    unsigned int uiWidth, uiHeight;
    GetFrameBufferDimensions(uiWidth, uiHeight);
    const int iIncrease = size_increase;
    const int iNumTiles = iIncrease * iIncrease;
    SamplePoint* pSamples = new SamplePoint[samples_per_pixel];
    const SamplePoint min = { 0.0f, 0.0f };
    const SamplePoint start = { 0.5f, 0.5f };
    const SamplePoint max = { 1.0f / iIncrease, 1.0f / iIncrease };
    if (!GetPoissonDiscSamples(pSamples, samples_per_pixel, start, 0.08f, min, max))
    {
        delete[] pSamples;
        return;
    }

    Accumulator<Pixel>* pAccums = new Accumulator<Pixel>[iNumTiles];
    for (int i = 0; i < iNumTiles; ++i)
    {
        if (!pAccums[i].Open())
        {
            delete[] pAccums;
            delete[] pSamples;
            return;
        }
        pAccums[i].Clear(0, 12 * uiWidth * uiHeight);
    }

    GammaRamp gamma;
    const float fInverseGamma = 1.0f / gamma_correction;
    for (int i = 0; i < 256; ++i)
    {
        float fValue = static_cast<float>(pow(static_cast<double>(static_cast<float>(i) / 255.0f),
                                              static_cast<double>(fInverseGamma)));
        if (fValue < 0.0f)
        {
            fValue = 0.0f;
        }
        else if (fValue > 1.0f)
        {
            fValue = 1.0f;
        }
        const unsigned char ucValue = static_cast<unsigned char>(static_cast<int>(fValue * 255.0f));
        gamma.red[i] = ucValue;
        gamma.green[i] = ucValue;
        gamma.blue[i] = ucValue;
    }

    GPUSync();
    const float fTileStep = 1.0f / iIncrease;
    for (int y = 0; y < iIncrease; ++y)
    {
        const float fOffsetY = -y * fTileStep;
        for (int x = 0; x < iIncrease; ++x)
        {
            const float fOffsetX = -x * fTileStep;
            for (int iSample = 0; iSample < samples_per_pixel; ++iSample)
            {
                RedRenderer::SetScreenOffset(pSamples[iSample].x + fOffsetX - 0.5f,
                                             pSamples[iSample].y + fOffsetY - 0.5f);
                FLRenderer::Render();
                AccumulateFrameBuffer(pAccums[y * iIncrease + x]);
            }
        }
    }
    RedRenderer::SetScreenOffset(0.0f, 0.0f);
    RedScreenShot::StitchImage(pAccums, iIncrease, iIncrease, samples_per_pixel, gamma, acFileName,
                static_cast<unsigned short>(uiWidth), static_cast<unsigned short>(uiHeight));

    for (int i = 0; i < iNumTiles; ++i)
    {
        if (pAccums[i].m_file.IsOpen())
        {
            pAccums[i].m_file.Close();
        }
        PblFile::Delete(pAccums[i].m_fileName);
    }
    delete[] pAccums;
    delete[] pSamples;
}
}
