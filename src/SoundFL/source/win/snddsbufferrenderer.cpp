#include "snddsbufferrenderer.h"

#include "sndengine.h"

void __cdecl PblTraceP(const char* Format, ...);

namespace Snd
{

// SYNTHETIC: SWBF2 0x0040be90
// `dynamic initializer for 'Snd::DSBufferRenderer::smRendererList''
// SYNTHETIC: SWBF2 0x0076a460
// `dynamic atexit destructor for 'Snd::DSBufferRenderer::smRendererList''

// GLOBAL: SWBF2 0x007e37f8
// DSBufferRenderer::smRendererList
PblListSingle DSBufferRenderer::smRendererList;

// FUNCTION: SWBF2 0x0073d740
DSBufferRenderer::DSBufferRenderer()
    : Source3D(0),
      mFrequency(0),
      mNumChannels(0),
      mFormat(SampleInfo::format_Unknown),
      mBufferSize(0),
      mPrevWritePos(0),
      mPrevPlayPos(0),
      mNextLockPos(0),
      mBufferedData(0),
      mLeftToPlay(0),
      mPacketRead(0),
      mFrameSize(0),
      mBufferFillSize(0),
      mFlags(8)
{

    mGainTarget[0] = 0x7FFF;
    mGainCurrent[0] = 0x7FFF;
    mGainTarget[1] = 0x7FFF;
    mGainCurrent[1] = 0x7FFF;

    Engine::GetThreadUpdate()->AddObject(this);

    Source3D::mProps = static_cast<Source3D::Properties*>(this);

    ThreadUpdate::Object::GetThreadUpdate()->Pause(false);
    smRendererList.AddLast(&mNode);
    ThreadUpdate::Object::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x0073dcb0
float DSBufferRenderer::GetBufferSizeMs() const
{

    const float fBufferBytes = (float)(unsigned int)mBufferSize;
    const unsigned int uiFrameBytes = SampleInfo::GetFrameSize(mFormat) * mNumChannels;
    const float fFrames = fBufferBytes / (float)(unsigned int)uiFrameBytes;
    const float fSamples = (float)(unsigned int)SampleInfo::GetSamplesPerFrame(mFormat)
                         * fFrames;
    const float fRate = (float)(unsigned int)mFrequency
                       * static_cast<const Source*>(static_cast<const Source2D3D*>(this))->mPitch;

    return fSamples / fRate * 1000.0f;
}

// FUNCTION: SWBF2 0x0073e200
void DSBufferRenderer::SetLowTideMark(float fMilliseconds)
{
    const float fBufferMs = GetBufferSizeMs();

    mBufferFillSize = (unsigned int)((float)(unsigned int)mBufferSize
                                     * (fMilliseconds / fBufferMs));
}

// FUNCTION: SWBF2 0x0073e2a0
void DSBufferRenderer::UpdateBufferParameters(IDirectSoundBuffer* const pBuffer,
                                              bool bUnusedArg4,
                                              bool bImmediate)
{
    (void)pBuffer;
    (void)bUnusedArg4;

    Source::mFlags &= (unsigned char)~(unsigned int)Source::flags_pitchChanged;

    DSBuffer::SetFrequency(
        (int)((float)(unsigned int)mFrequency * Source::mPitch));

    if (Source2D3D::Get3DMode())
    {
        Listener* pListener = Source3D::mClosestListener;
        if (pListener == 0)
        {
            Source3D::mFlags |=
                (unsigned char)(unsigned int)Source3D::flags_updateListener;

            Source3D::DetermineClosestListener();
            pListener = Source3D::mClosestListener;
        }

        if (pListener->mSpace != 0)
        {
            const float fSpaceDistance = Source3D::mProps->mSpaceDistance;

            SourceI3DL2Interpolated::Update(
                pListener,
                Source3D::mPos3D,
                fSpaceDistance,
                SourceTransport::GetCurrentPlayState()
                    == SourceTransport::playState_playing);
        }

        if (DSBuffer::GetBuffer3D() != 0)
        {
            DS3DBUFFER params;
            Engine::Calculate3DParams(this, &params);
            if (!DSBuffer::Set3DParams(params, bImmediate))
            {
                PblTraceP("DSBufferRenderer : unable to set 3D parameters\n");

            }
        }

        float fGain;
        Source3D::CalculateGain(fGain);

        if (DSBuffer::GetPropertySet() != 0 && mNumChannels <= 1)
        {
            const unsigned short usFlags = SourceI3DL2::mFlags;
            const bool bChanged =
                (usFlags & SourceI3DL2::flags_changed) != 0;

            SourceI3DL2::mFlags = (unsigned short)(usFlags & 0xFE00u);

            if (bChanged)
            {
                Engine::SetupDirectSoundEffects(this, this);
            }
        }
        else
        {

            fGain *= SourceI3DL2::CalculateGainEstimate();
        }

        UpdateGain(true, fGain);

        DSBuffer::SetFrequency(
            (unsigned int)((float)(unsigned int)mFrequency
                           * (Source3D::CalculatePitch() * Source::mPitch)));
    }
    else
    {
        UpdateGain(false, 1.0f);
    }
}

// FUNCTION: SWBF2 0x0073dd80
void DSBufferRenderer::QueuePackets()
{

    typedef StreamPacket* (__thiscall* GetPacketFn)(StreamNodeSource* pThis,
                                                    unsigned int uiOutput);

    while (!mPacketQueue.IsFull())
    {
        StreamNodeSource* const pSource =
            StreamNodeSink::mConnection.mInputNode;
        if (pSource == 0)
        {
            break;
        }

        StreamPacket* const pPacket =
            pSource->GetPacket(StreamNodeSink::mConnection.mInputNodeOutputIndex);

        if (pPacket == 0)
        {
            break;
        }

        mPacketQueue.Add(pPacket);
    }
}

// FUNCTION: SWBF2 0x0073ddf0
void DSBufferRenderer::ResetGain()
{

    if ((mFlags & 0x10) != 0)
    {
        DSBuffer::ResetGainInterpolation();
        return;
    }

    for (unsigned int i = 0; i < mNumChannels; ++i)
    {
        mGainCurrent[i] = mGainCurrent[i - 2];
    }
}

// FUNCTION: SWBF2 0x0073e250
void DSBufferRenderer::StopBuffer()
{
    IDirectSoundBuffer8* pBuffer = DSBuffer::GetBuffer();
    if (pBuffer == 0)
    {
        return;
    }

    if (!DSBuffer::IsPlaying())
    {
        return;
    }

    DSBuffer::Stop();

    HRESULT hr = pBuffer->SetCurrentPosition(0);
    if (!Engine::Succeeded(hr, "source\\win\\snddsbufferrenderer.cpp", 578))
    {
        PblTraceP("DSBufferRenderer : unable to reset buffer play cursor\n");

    }
}

// FUNCTION: SWBF2 0x0073e490
void DSBufferRenderer::UpdateGain(bool bEnable, float fGain)
{
    (void)bEnable;

    const float fSourceGain = Source::mGain;

    if ((mFlags & 0x10) != 0)
    {
        if (Source2D3D::Get3DMode())
        {
            DSBuffer::SetGain(fSourceGain * fGain);
        }
        else
        {
            DSBuffer::SetGain(fSourceGain);
            DSBuffer::SetPan(Source2D::mPan);
        }
    }
    else if (Source2D3D::Get3DMode())
    {
        const int iGain =
            (int)(fSourceGain * fGain * 32767.0f) << 16;
        for (unsigned int i = 0; i < mNumChannels; ++i)
        {
            mGainTarget[i] = iGain;
        }
    }
    else if (mNumChannels == 2)
    {
        GainMatrix gains;
        Source2D::CalculateGain(fSourceGain, 1.0f, gains);

        mGainTarget[0] = (int)(gains.frontLeft * 32767.0f) << 16;

        mGainTarget[1] = (int)(gains.frontRight * 32767.0f) << 16;
    }
    else
    {
        const int iGain = (int)(fSourceGain * 32767.0f) << 16;
        for (unsigned int i = 0; i < mNumChannels; ++i)
        {
            mGainTarget[i] = iGain;
        }

        DSBuffer::SetPan(Source2D::mPan);
    }
}

// FUNCTION: SWBF2 0x0073eab0
void DSBufferRenderer::UpdateParameters()
{

    for (PblListSingle::Node* pNode = smRendererList.mHead.mNext;
         pNode != &smRendererList.mHead;
         pNode = pNode->mNext)
    {

        DSBufferRenderer* const pRenderer = static_cast<DSBufferRenderer*>(pNode->GetObjectA(&smRendererList, offsetof(DSBufferRenderer, mNode)));

        IDirectSoundBuffer* const pBuffer =
            pRenderer->DSBuffer::GetBuffer();
        if (pBuffer == 0)
        {
            continue;
        }

        if (pRenderer->SourceTransport::GetPlayState()
            != SourceTransport::playState_playing)
        {
            continue;
        }

        pRenderer->UpdateBufferParameters(
            pBuffer, pRenderer->StreamNode::IsStarving(), true);
    }
}

// FUNCTION: SWBF2 0x0073d950
DSBufferRenderer::~DSBufferRenderer()
{

    ThreadUpdate::Object::mDestructing = 1;

    ThreadUpdate::Object::GetThreadUpdate()->Pause(false);

    mNode.Remove();

    StreamNode::SetPlayState(SourceTransport::playState_stopped);

    Flush();

    ThreadUpdate::Object::GetThreadUpdate()->Resume();
}

// FUNCTION: SWBF2 0x0073dad0
void DSBufferRenderer::Flush()
{
    IDirectSoundBuffer8* const pBuffer = DSBuffer::GetBuffer();
    if (pBuffer != 0)
    {
        if (DSBuffer::IsPlaying())
        {
            DSBuffer::Stop();

            const HRESULT hResult = pBuffer->SetCurrentPosition(0);
            if (!Engine::Succeeded(hResult, "source\\win\\snddsbufferrenderer.cpp", 578))
            {
                PblTraceP("DSBufferRenderer : unable to reset buffer"
                          " play cursor\n");
            }
        }

        void* pAudio1;
        unsigned long ulBytes1;
        void* pAudio2;
        unsigned long ulBytes2;
        const HRESULT hResult = pBuffer->Lock(0, mBufferSize,
                                              &pAudio1, &ulBytes1,
                                              &pAudio2, &ulBytes2, 0);
        if (Engine::Succeeded(hResult, "source\\win\\snddsbufferrenderer.cpp", 85))
        {
            if (pAudio1 != 0)
            {
                memset(pAudio1, 0, ulBytes1);
            }
            if (pAudio2 != 0)
            {
                memset(pAudio2, 0, ulBytes2);
            }

            pBuffer->Unlock(pAudio1, ulBytes1, pAudio2, ulBytes2);

        }
    }

    while (!mPacketQueue.IsEmpty())
    {
        StreamPacket* pPacket;
        mPacketQueue.Get(pPacket);
        pPacket->Unlock();
    }

    mFlags = (unsigned char)(mFlags & 8u);

    mPrevWritePos = 0;
    mPrevPlayPos = 0;
    mNextLockPos = 0;
    mBufferedData = 0;
    mLeftToPlay = 0;
    mPacketRead = 0;

    switch (StreamNode::GetPlayState())
    {
    case SourceTransport::playState_playing:
        StreamNode::Play(true);
        break;

    case SourceTransport::playState_paused:
        StreamNode::Pause();
        break;

    default:
        break;
    }
}

const StreamPacket* DSBufferRenderer::PeekPacket() const
{
    if (mPacketQueue.IsEmpty())
    {
        return 0;
    }

    StreamPacket* const* const ppPacket = mPacketQueue.Peek(0);

    if (ppPacket == 0)
    {
        return 0;
    }

    return *ppPacket;
}

static int ClampSample(int iSample)
{
    if (iSample > 32767)
    {
        return 32767;
    }
    if (iSample < -32768)
    {
        return -32768;
    }
    return iSample;
}

// FUNCTION: SWBF2 0x0073eb00
unsigned int DSBufferRenderer::WriteData(void* pData, unsigned int uiSize,
                                         SourceTransport::PlayState ePlayState)
{
    unsigned int uiWritten = 0;
    const StreamPacket* pPacket;

    do
    {
        pPacket = PeekPacket();
        if (pPacket == 0)
        {

            QueuePackets();
            pPacket = PeekPacket();
            if (pPacket == 0)
            {
                break;
            }
        }
        else
        {
            const unsigned char* const pSrc =
                (const unsigned char*)pPacket->GetBuffer() + mPacketRead;

            const unsigned int uiUsed = pPacket->GetUsed();

            unsigned int uiChunk = uiUsed - mPacketRead;
            if (uiChunk > uiSize)
            {
                uiChunk = uiSize;
            }

            StreamNode::SetStarved(false);

            if ((mFlags & 0x10) != 0)
            {

                memcpy((unsigned char*)pData + uiWritten, pSrc, uiChunk);
            }
            else
            {

                short* output = (short*)((unsigned char*)pData + uiWritten);
                const short* input = (const short*)pSrc;
                unsigned int readSizeSamples = uiChunk >> 1;

                switch (mNumChannels)
                {
                case 1:
                {
                    mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                    unsigned int remaining = readSizeSamples & 7u;
                    readSizeSamples &= ~7u;
                    while (readSizeSamples != 0)
                    {
                        int sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        readSizeSamples -= 4;
                    }
                    while (remaining-- != 0)
                    {
                        int sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                    }
                    break;
                }
                case 2:
                {
                    mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                    mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                    unsigned int remaining = readSizeSamples & 15u;
                    readSizeSamples &= ~15u;
                    while (readSizeSamples != 0)
                    {
                        int sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[1] >> 16) * *input++) >> 16;
                        mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[1] >> 16) * *input++) >> 16;
                        mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[1] >> 16) * *input++) >> 16;
                        mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[1] >> 16) * *input++) >> 16;
                        mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        readSizeSamples -= 8;
                    }
                    while (remaining != 0)
                    {
                        int sample = ((mGainCurrent[0] >> 16) * *input++) >> 16;
                        mGainCurrent[0] += (mGainTarget[0] - mGainCurrent[0]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        sample = ((mGainCurrent[1] >> 16) * *input++) >> 16;
                        mGainCurrent[1] += (mGainTarget[1] - mGainCurrent[1]) >> 1;
                        *output++ = (short)ClampSample(sample);
                        remaining -= 2;
                    }
                    break;
                }
                default:
                {
                    for (unsigned int ch = 0; ch < mNumChannels; ++ch)
                    {
                        for (unsigned int i = ch; i < readSizeSamples; i += mNumChannels)
                        {
                            output[i] = (short)ClampSample(
                                (input[i] * (mGainCurrent[ch] >> 16)) >> 16);
                            mGainCurrent[ch] += (mGainTarget[ch] - mGainCurrent[ch]) >> 1;
                        }
                    }
                    break;
                }
                }
            }

            uiWritten += uiChunk;
            mPacketRead += uiChunk;
            uiSize -= uiChunk;
            mBufferedData += uiChunk;
            mLeftToPlay += uiChunk;

            if (mPacketRead == uiUsed)
            {

                mFlags = (unsigned char)(mFlags & 0xFCu);
                mFlags = (unsigned char)(mFlags | (pPacket->GetLast() << 1));
                mFlags = (unsigned char)(mFlags | pPacket->GetDiscontinuity());

                StreamPacket* pDone;
                mPacketQueue.Get(pDone);

                SourceTransport::mPlaybackPosition = pDone->GetPosition();

                pDone->Unlock();
                mPacketRead = 0;
            }
        }

        if (uiSize == 0)
        {
            return uiWritten;
        }
    } while (pPacket != 0);

    const unsigned char ucLastPacketFlags = (unsigned char)(mFlags & 3u);

    if (ePlayState != SourceTransport::playState_playing)
    {
        return uiWritten;
    }

    if (mBufferedData != 0 && ucLastPacketFlags == 0)
    {
        return uiWritten;
    }

    if (mLeftToPlay == 0 && (mFlags & 8) != 0)
    {
        if (ucLastPacketFlags != 0)
        {
            mFlags = (unsigned char)(mFlags | 4u);

            if ((mFlags & 2) != 0)
            {
                StreamNode::Stop();
            }

            mFlags = (unsigned char)(mFlags & 0xFCu);
            return uiWritten;
        }

        StreamNode::SetStarved((mFlags & 4) == 0);
        return uiWritten;
    }

    memset((unsigned char*)pData + uiWritten, 0, uiSize);
    return uiWritten + uiSize;
}

// FUNCTION: SWBF2 0x0053c660
void DSBufferRenderer::Play(bool bRestart)
{
    StreamNode::Play(bRestart);
}

// FUNCTION: SWBF2 0x0073e630
void DSBufferRenderer::UpdateObject()
{

    IDirectSoundBuffer8* const pBuffer = DSBuffer::GetBuffer();

    SourceTransport* const pTransport =
        static_cast<SourceTransport*>(this);

    const SourceTransport::PlayState eDesired =
        StreamNode::QueryDesiredPlayState();

    pTransport->mFlags =
        (unsigned short)(pTransport->mFlags
                         & ~(unsigned int)SourceTransport::flags_updateTransport);

    switch (eDesired)
    {
    case SourceTransport::playState_playing:

        if (DSBuffer::GetBuffer() != 0
            && (mBufferedData >= mBufferSize / 2 || (mFlags & 3) != 0))
        {

            const bool bWasStopped = !DSBuffer::IsPlaying();

            if (Source2D3D::Get3DMode())
            {
                SourceI3DL2Interpolated* const pI3DL2 = this;

                pI3DL2->SourceI3DL2::mFlags =
                    (unsigned short)(pI3DL2->SourceI3DL2::mFlags
                                     | (unsigned int)
                                           SourceI3DL2::flags_forceUpdate);

                pI3DL2->Update(Source3D::mClosestListener,
                               Source3D::mPos3D,
                               Source3D::mProps->mSpaceDistance,
                               false);
            }

            Engine::SetupDirectSoundEffects(this, this);

            UpdateBufferParameters(pBuffer, false, false);

            ResetGain();
            DSBuffer::ResetFrequencyInterpolation();

            if (pBuffer != 0 && bWasStopped)
            {

                if (!DSBuffer::Play(true, 0xFFFFFFFFu, true, true))
                {
                    Play(true);
                }
            }

            StreamNode::SetPlayState(SourceTransport::playState_playing);

            mFlags = (unsigned char)(mFlags & 0xFBu);
        }
        else
        {

            StreamNode::Play(true);
        }
        break;

    case SourceTransport::playState_stopped:
        SourceTransport::mPlaybackPosition = 0;
        StreamNode::SetPlayState(SourceTransport::playState_stopped);

        Flush();

    case SourceTransport::playState_paused:
        if (DSBuffer::IsPlaying())
        {
            StopBuffer();
        }

        if (eDesired == SourceTransport::playState_stopped
            || (DSBuffer::GetBuffer() != 0
                && (mBufferedData >= mBufferSize || (mFlags & 3) != 0)))
        {
            StreamNode::SetStarved(false);
            StreamNode::SetPlayState(eDesired);
        }
        else
        {
            StreamNode::Pause();
            StreamNode::SetPlayState(SourceTransport::playState_pausing);

        }
        break;

    default:
        break;
    }

    const SourceTransport::PlayState eCurrent =
        StreamNode::GetCurrentPlayState();

    if (pBuffer == 0)
    {
        return;
    }

    if (eCurrent != SourceTransport::playState_playing
        && eCurrent != SourceTransport::playState_pausing
        && eDesired != SourceTransport::playState_playing)
    {
        return;
    }

    unsigned long ulPlayPos;
    unsigned long ulWritePos;
    const HRESULT hrPosition = pBuffer->GetCurrentPosition(&ulPlayPos, &ulWritePos);
    if (!Engine::Succeeded(hrPosition,
                           "source\\win\\snddsbufferrenderer.cpp", 268))
    {
        return;
    }

    const unsigned int uiWriteAdvance =
        (ulWritePos >= mPrevWritePos)
            ? (ulWritePos - mPrevWritePos)
            : (ulWritePos + (mBufferSize - mPrevWritePos));

    if (DSBuffer::IsPlaying() && uiWriteAdvance < mBufferFillSize)
    {
        return;
    }

    QueuePackets();

    const unsigned int uiBuffered = (mBufferedData > uiWriteAdvance)
                                        ? (mBufferedData - uiWriteAdvance)
                                        : 0u;
    mBufferedData = uiBuffered;
    mPrevWritePos = ulWritePos;

    const unsigned int uiPlayAdvance =
        (ulPlayPos >= mPrevPlayPos)
            ? (ulPlayPos - mPrevPlayPos)
            : (ulPlayPos + (mBufferSize - mPrevPlayPos));
    mLeftToPlay = (mLeftToPlay > uiPlayAdvance) ? (mLeftToPlay - uiPlayAdvance)
                                                : 0u;
    mPrevPlayPos = ulPlayPos;

    if (ulWritePos >= ulPlayPos)
    {
        if (mNextLockPos > ulPlayPos && mNextLockPos < ulWritePos)
        {
            mNextLockPos = ulWritePos;
        }
    }
    else
    {
        if (mNextLockPos >= ulPlayPos && mNextLockPos <= ulWritePos)
        {
            mNextLockPos = ulWritePos;
        }
    }

    unsigned int uiLockSize = mBufferSize - uiBuffered;
    const int iLockBehindPlay =
        DSBuffer::IsPlaying() ? (ulPlayPos >= mNextLockPos)
                              : (ulPlayPos > mNextLockPos);
    const unsigned int uiToPlayCursor =
        iLockBehindPlay ? (ulPlayPos - mNextLockPos)
                        : (ulPlayPos + (mBufferSize - mNextLockPos));

    if (uiLockSize > uiToPlayCursor)
    {
        uiLockSize = uiToPlayCursor;
    }

    if (uiLockSize != 0)
    {

        void* pAudio1;
        unsigned long ulBytes1;
        void* pAudio2;
        unsigned long ulBytes2;
        unsigned int uiWritten = 0;
        const HRESULT hrLock = pBuffer->Lock(mNextLockPos, uiLockSize,
                                             &pAudio1, &ulBytes1,
                                             &pAudio2, &ulBytes2, 0);
        if (!Engine::Succeeded(hrLock,
                               "source\\win\\snddsbufferrenderer.cpp", 361))
        {
            return;
        }

        if (pAudio1 != 0)
        {
            uiWritten = WriteData(pAudio1, ulBytes1, eCurrent);
        }

        if (pAudio2 != 0)
        {
            uiWritten += WriteData(pAudio2, ulBytes2, eCurrent);
        }

        mNextLockPos += uiWritten;
        if (mNextLockPos >= mBufferSize)
        {
            mNextLockPos -= mBufferSize;
        }

        pBuffer->Unlock(pAudio1, ulBytes1, pAudio2, ulBytes2);

    }

    const bool bTerminated = DSBuffer::IsTerminated();

    if (DSBuffer::IsPlaying() && (mFlags & 8) != 0)
    {

        const bool bStarving = StreamNode::IsStarving();
        StreamNode::SetStarved(bStarving);

        if ((!bStarving && (mFlags & 4) == 0)
            || StreamNode::QueryDesiredPlayState()
                   == SourceTransport::playState_stopped)
        {
            return;
        }
    }
    else if (!bTerminated)
    {
        return;
    }

    StreamNode::Stop();
    UpdateObject();
    Play(false);
}

// FUNCTION: SWBF2 0x0073de90
bool DSBufferRenderer::SetFormat(SampleInfo::Format eFormat,
                                 unsigned int uiNumChannels,
                                 unsigned int uiFrequency, bool b3D,
                                 float fBufferMs, bool bRestart,
                                 IDirectSound* pDirectSound,
                                 unsigned long ulFlags, bool bPrimary)
{
    const bool bHardwareEffects = Engine::smHardwareEffectsAvailable;
    const bool b3DMono = b3D && uiNumChannels == 1;

    float fMs = (Engine::GetDSCaps().dwMaxHwMixingAllBuffers == 0 && 500.0f > fBufferMs)
                    ? 500.0f
                    : fBufferMs;

    if (Engine::GetMinStreamBufferSizeMs() > fMs)
    {
        fMs = Engine::GetMinStreamBufferSizeMs();
    }

    Flush();
    DSBuffer::Destroy();

    mPrevWritePos = 0;
    mNextLockPos = 0;

    if (eFormat != SampleInfo::format_Unknown
        && uiNumChannels != 0
        && uiFrequency != 0)
    {

        const float fFrames =
            (float)(uiFrequency / SampleInfo::GetSamplesPerFrame(eFormat))
            * (fMs / 1000.0f);

        unsigned int uiLength =
            (unsigned int)((float)SampleInfo::GetFrameSize(eFormat) * fFrames
                           * (float)uiNumChannels);

        uiLength =
            (uiLength + SampleInfo::GetFrameSize(eFormat) * uiNumChannels - 1)
            / (SampleInfo::GetFrameSize(eFormat) * uiNumChannels)
            * (SampleInfo::GetFrameSize(eFormat) * uiNumChannels);

        const unsigned int uiLocation =
            (Engine::GetMixConfig() == Engine::mixConfig_DirectSoundHardware
             && ((!bPrimary && b3DMono) || ulFlags != 0))
                ? DSBCAPS_LOCHARDWARE
                : DSBCAPS_LOCSOFTWARE;

        if (DSBuffer::Create(eFormat, uiNumChannels, uiFrequency, uiLength,
                             b3DMono, bHardwareEffects && b3DMono,
                             (unsigned int)(b3DMono ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN)
                                 | uiLocation
                                 | (unsigned int)(DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME
                                                  | DSBCAPS_CTRLFREQUENCY),
                             false, true,
                             pDirectSound, ulFlags, true))
        {

            if (DSBuffer::GetFormat(mFormat, mNumChannels,
                                    mFrequency))
            {

                DSBCAPS caps;
                caps.dwSize = sizeof(DSBCAPS);

                const HRESULT hrCaps = DSBuffer::GetBuffer()->GetCaps(&caps);
                if (Engine::Succeeded(hrCaps,
                                      "source\\win\\snddsbufferrenderer.cpp", 884))
                {
                    mBufferSize = caps.dwBufferBytes;
                    mFrameSize = SampleInfo::GetFrameSize(mFormat)
                                 * mNumChannels;

                    unsigned int uiFillSize =
                        (unsigned int)((float)uiFrequency * 0.005f
                                       * (float)mFrameSize);
                    if (uiFillSize > mBufferSize / 3)
                    {
                        uiFillSize = 0;
                    }
                    mBufferFillSize = uiFillSize;

                    Source2D3D::Set3DMode(b3D);

                    Flush();

                    if ((mFlags & 0x10) == 0)
                    {
                        DSBuffer::ResetGainInterpolation();
                        DSBuffer::SetGain(1.0f);
                    }

                    return true;
                }
            }
        }
    }

    mFrequency = 0;
    mNumChannels = 0;
    mFormat = SampleInfo::format_Unknown;
    mBufferSize = 0;
    return false;
}

// SYNTHETIC: SWBF2 0x0073da40
// Snd::DSBufferRenderer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0073de40
void DSBufferRenderer::Set3DMode(bool b3D, unsigned long ulFlags)
{
    SetFormat(mFormat, mNumChannels, mFrequency, b3D, GetBufferSizeMs(),
              ulFlags != 0, 0, 0, false);
}

}
