#if !defined(DIRECTSOUND_VERSION) || DIRECTSOUND_VERSION < 0x0800
#undef DIRECTSOUND_VERSION
#define DIRECTSOUND_VERSION 0x0800
#endif

#include <windows.h>
#include <dsound.h>

#include <stdlib.h>
#include <string.h>

#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndsourcetransport.h"

// GLOBAL: SWBF2 0x007869d4
// kFile
// GLOBAL: SWBF2 0x007874b4
// kDSDevIdDefaultCapture
// GLOBAL: SWBF2 0x007874d4
// kDSDevIdDefaultVoiceCapture
#include "SoundFL/source/sndstreamformat.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreampacketio.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "snddsbuffer.h"
#include "snddsdeviceenumerator.h"
#include "sndengine.h"
#include "sndsemaphore.h"
#include "sndstreamcapturesource.h"

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

static const char kFile[] = "source\\win\\sndstreamcapturesource.cpp";

static const GUID kDSDevIdDefaultCapture =
    { 0xDEF00001, 0x9C6D, 0x47ED, { 0xAA, 0xF1, 0x4D, 0xDA, 0x8F, 0x2B, 0x5C, 0x03 } };
static const GUID kDSDevIdDefaultVoiceCapture =
    { 0xDEF00003, 0x9C6D, 0x47ED, { 0xAA, 0xF1, 0x4D, 0xDA, 0x8F, 0x2B, 0x5C, 0x03 } };

// FUNCTION: SWBF2 0x00740e70
StreamCaptureSource::StreamCaptureSource()
    : mCaptureDevice(0),
      mCaptureBuffer(0),
      mCaptureBufferSize(0),
      mNextLockPos(0),
      mFrameSize(0)
{

    if (!mLock.Create(1, 1))
    {
        PrintDebugMessage("Unable to create semaphore : out of system "
                          "resources!");
        exit(0);
    }

    Engine::GetThreadUpdate()->AddObject(this);
}

// FUNCTION: SWBF2 0x00740f60
StreamCaptureSource::~StreamCaptureSource()
{

    if (mCaptureBuffer != 0)
    {
        Close();
    }

    mDestructing = 1;
}

// SYNTHETIC: SWBF2 0x00741020
// Snd::StreamCaptureSource::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0074100e SYMBOL
// ??_EStreamCaptureSource@Snd@@WCA@AEPAXI@Z

// FUNCTION: SWBF2 0x00741050
void StreamCaptureSource::Close()
{
    mLock.Wait();
    StreamCaptureSourceBase::Flush();

    if (mCaptureBuffer != 0)
    {
        mCaptureBuffer->Stop();

        DWORD dwAudio1;
        void* pvAudio1;
        DWORD dwAudio2;
        void* pvAudio2;
        mCaptureBuffer->Lock(0, mPacketSize,
                             &pvAudio1, &dwAudio1, &pvAudio2, &dwAudio2, 0);

        if (dwAudio1 != 0)
        {
            memset(pvAudio1, 0, dwAudio1);
        }

        if (dwAudio2 != 0)
        {
            memset(pvAudio1, 0, dwAudio2);
        }

        mCaptureBuffer->Unlock(pvAudio1, dwAudio1,
                               pvAudio2, dwAudio2);

        mNextLockPos = 0;
        mOutputPacket.Reset();
    }

    mCaptureBuffer->Release();

    mCaptureBuffer = 0;
    mCaptureBufferSize = 0;

    mCaptureDevice->Release();

    mCaptureDevice = 0;
    mFrameSize = 0;

    StreamCaptureSourceBase::Close(true);
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00741130
void StreamCaptureSource::Flush(bool bAlreadyLocked)
{
    if (!bAlreadyLocked)
    {
        mLock.Wait();
    }

    StreamCaptureSourceBase::Flush();

    if (mCaptureBuffer != 0)
    {
        mCaptureBuffer->Stop();

        DWORD dwAudio1;
        void* pvAudio1;
        DWORD dwAudio2;
        void* pvAudio2;
        mCaptureBuffer->Lock(0, mPacketSize,
                             &pvAudio1, &dwAudio1, &pvAudio2, &dwAudio2, 0);

        if (dwAudio1 != 0)
        {
            memset(pvAudio1, 0, dwAudio1);
        }

        if (dwAudio2 != 0)
        {
            memset(pvAudio1, 0, dwAudio2);
        }

        mCaptureBuffer->Unlock(pvAudio1, dwAudio1, pvAudio2, dwAudio2);

        mNextLockPos = 0;
        mOutputPacket.Reset();
    }

    if (!bAlreadyLocked)
    {
        mLock.Signal();
    }
}

// FUNCTION: SWBF2 0x007411f0
bool StreamCaptureSource::Open(unsigned char ucDeviceIndex,
                               StreamFormat const& cFormat,
                               unsigned int uiPacketSize,
                               unsigned char ucNumPackets,
                               StreamPacket* pPackets)
{
    mLock.Wait();

    const unsigned int uiIndex = ucDeviceIndex;

    GUID cGuidDevice;
    switch (uiIndex)
    {
    case 0xFE:
        cGuidDevice = kDSDevIdDefaultCapture;
        break;

    case 0xFF:
        cGuidDevice = kDSDevIdDefaultVoiceCapture;
        break;

    default:
    {
        DSDeviceEnumerator cEnumerator;

        if (!cEnumerator.Enumerate(DSDeviceEnumerator::type_Capture, false))
        {
            mLock.Signal();
            return false;
        }

        cGuidDevice = cEnumerator.GetDeviceIDByIndex(uiIndex);
        break;
    }
    }

    HRESULT hr = DirectSoundCaptureCreate8(&cGuidDevice, &mCaptureDevice, 0);
    if (Engine::Failed(hr, kFile, 112))
    {
        mLock.Signal();
        return false;
    }

    mCaptureBufferSize = uiPacketSize * 2;

    DSBuffer::WaveFormat cWaveFormat(cFormat.GetFormat(),
                                     cFormat.GetNumChannels(),
                                     cFormat.GetFrequency(), false, 0);

    DSCBUFFERDESC cDesc;
    memset(&cDesc, 0, sizeof(cDesc));
    cDesc.dwSize = sizeof(DSCBUFFERDESC);
    cDesc.dwFlags = DSCBCAPS_WAVEMAPPED;
    cDesc.dwBufferBytes = mCaptureBufferSize;
    cDesc.lpwfxFormat = &cWaveFormat.Format;
    cDesc.dwFXCount = 0;
    cDesc.lpDSCFXDesc = 0;

    IDirectSoundCapture8* const pDevice =
        mCaptureDevice;

    hr = pDevice->CreateCaptureBuffer(
        &cDesc, (IDirectSoundCaptureBuffer**)&mCaptureBuffer, 0);
    if (Engine::Failed(hr, kFile, 133))
    {

        pDevice->Release();
        mLock.Signal();
        return false;
    }

    mFrameSize =
        SampleInfo::GetFrameSize(cFormat.GetFormat())
        * cFormat.GetNumChannels();

    StreamCaptureSourceBase::Open((unsigned char)uiIndex, cFormat,
                                  uiPacketSize, ucNumPackets, pPackets);

    Flush(true);
    mLock.Signal();
    return true;
}

// FUNCTION: SWBF2 0x00741470
void StreamCaptureSource::UpdateObject()
{

    mLock.Wait();

    const SourceTransport::PlayState eAction =
        Update(mCaptureBuffer != 0);

    switch (eAction)
    {
    case SourceTransport::playState_stopped:

        Flush(true);
        break;

    case SourceTransport::playState_paused:
        if (mCaptureBuffer != 0)
        {
            mCaptureBuffer->Stop();
        }
        else
        {

            SourceTransport::Stop();
        }
        break;

    case SourceTransport::playState_playing:
        if (mCaptureBuffer != 0)
        {
            mCaptureBuffer->Start(DSCBSTART_LOOPING);
        }
        else
        {

            SourceTransport::Stop();
        }
        break;

    default:

        break;
    }

    if (mCaptureBuffer != 0)
    {
        const SourceTransport::PlayState eState =
            GetCurrentPlayState();

        if (eState == SourceTransport::playState_playing
            || eState == SourceTransport::playState_paused
            || eState == SourceTransport::playState_pausing)
        {
            bool bContinue = true;

            do
            {

                StreamPacket* const pPacket =
                    mPackets
                    + mReadPacketIndex;

                if (pPacket->mRefCount > 0)
                {
                    break;
                }
                if (pPacket->mBufferUsed != 0)
                {
                    break;
                }

                DWORD dwCapturePos;
                DWORD dwReadPos;
                mCaptureBuffer->GetCurrentPosition(&dwCapturePos,
                                            &dwReadPos);

                {
                    const unsigned int uiNext =
                        mNextLockPos;
                    bool bSafe;
                    if (dwCapturePos >= dwReadPos)
                    {
                        bSafe = (uiNext <= dwReadPos)
                                || (uiNext >= dwCapturePos);
                    }
                    else
                    {
                        bSafe = (uiNext < dwReadPos)
                                || (uiNext > dwCapturePos);
                    }

                    if (!bSafe)
                    {

                        PblTraceP("StreamCaptureSource : NLP %8d RP %8d "
                                  "CP %8d - buffer underrun!\n",
                                  uiNext, dwReadPos, dwCapturePos);
                        mNextLockPos =
                            dwCapturePos;
                    }
                }

                unsigned int uiAvailable;
                {
                    const unsigned int uiNext =
                        mNextLockPos;
                    if (dwReadPos >= uiNext)
                    {
                        uiAvailable = dwReadPos - uiNext;
                    }
                    else
                    {
                        uiAvailable = dwReadPos
                                      - (mCaptureBufferSize
                                         - uiNext);
                    }
                }

                if (uiAvailable == 0)
                {
                    bContinue = false;
                }

                unsigned int uiChunk = uiAvailable;
                {
                    const unsigned int uiDataLeft =
                        mOutputPacket.GetDataLeft(mPacketSize);
                    if (uiChunk > uiDataLeft)
                    {
                        uiChunk = uiDataLeft;
                    }
                }

                if (uiChunk != 0)
                {
                    DWORD dwAudio1;
                    void* pvAudio1;
                    DWORD dwAudio2;
                    void* pvAudio2;
                    mCaptureBuffer->Lock(mNextLockPos,
                                  uiChunk,
                                  &pvAudio1, &dwAudio1,
                                  &pvAudio2, &dwAudio2, 0);

                    mNextLockPos += uiChunk;
                    if (mNextLockPos
                        >= mCaptureBufferSize)
                    {
                        mNextLockPos -=
                            mCaptureBufferSize;
                    }

                    if (dwAudio1 != 0)
                    {
                        memcpy(mOutputPacket.GetData(pPacket), pvAudio1, dwAudio1);
                        mOutputPacket.UpdateOffset(mPacketSize, dwAudio1);
                    }

                    if (dwAudio2 != 0)
                    {
                        memcpy(mOutputPacket.GetData(pPacket), pvAudio2, dwAudio2);
                        mOutputPacket.UpdateOffset(mPacketSize, dwAudio2);
                    }

                    mCaptureBuffer->Unlock(pvAudio1, dwAudio1,
                                    pvAudio2, dwAudio2);

                    if (mOutputPacket.GetDataLeft(mPacketSize) == 0)
                    {
                        const unsigned int uiPos =
                            mPlaybackPosition;

                        pPacket->mBufferUsed = mPacketSize;
                        pPacket->mFlags = (unsigned short)
                            (pPacket->mFlags & 0xFFFCu);
                        pPacket->mPos = uiPos;
                        mOutputPacket.Reset();

                        mPlaybackPosition =
                            uiPos
                            + mPacketSize / mFrameSize;

                        ++mReadPacketIndex;
                        if (mReadPacketIndex
                            == mNumPackets)
                        {
                            mReadPacketIndex = 0;
                        }
                    }
                }
            } while (bContinue);
        }
    }

    mLock.Signal();
}

}
