#include <new>
#include "SoundFL/source/sndsampleinfo.h"
#include "SoundFL/source/sndstreamformat.h"
#include "SoundFL/source/sndstreampacket.h"
#include "SoundFL/source/sndstreamagc.h"
#include "SoundFL/source/sndstreamgsmencoder.h"
#include "SoundFL/source/sndstreamvad.h"
#include "SoundFL/source/win/sndstreamcapturesource.h"
#include "GameVoiceChatEncoder.h"
#include "GameVoiceChatNode.h"
#include "VoiceChatNode.h"

#include "SoundFL/source/sndsourcetransport.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatTransport.h"

#include "SoundFL/source/sndstreamcodec.h"

namespace GameVoiceChat
{
unsigned int CalculatePlayCaptureBufferSizeSamples(float fMilliseconds);
}
namespace GameVoiceChat
{
unsigned int CalculateEncodedPacketSize(unsigned int uiSamples);
}

namespace VoiceChat
{
void* Alloc(unsigned int uiSize);
}
namespace VoiceChat
{
void Free(void* pBlock);
}

namespace Snd
{

}


// FUNCTION: SWBF2 0x0053d4c0
bool GameVoiceChat::Encoder::Open(unsigned int captureDeviceIndex, float fMaxPacketMs, unsigned int minBuffers)
{

    float fMs = fMaxPacketMs;
    if (200.0f > fMs)
    {
        fMs = 200.0f;
    }

    const unsigned int uiSamples = GameVoiceChat::CalculatePlayCaptureBufferSizeSamples(fMs);
    const unsigned int uiFrameBytes = uiSamples * 2u;
    const unsigned int uiAlignedFrame = (uiFrameBytes + 3u) & ~3u;
    const unsigned int uiEncodedBytes = GameVoiceChat::CalculateEncodedPacketSize(uiSamples);
    const unsigned int uiAlignedEncoded = (uiEncodedBytes + 3u) & ~3u;

    mCaptureBuffer = (unsigned char*)VoiceChat::Alloc(uiAlignedEncoded + (uiAlignedFrame + 0x2Eu) * 2u);

    if (!mCaptureBuffer)
    {
        return false;
    }

    unsigned char* pCursor = (unsigned char*)(((unsigned long)mCaptureBuffer + 3u) & ~3u);
    mEncoderBuffer = pCursor + uiAlignedFrame * 2u;
    mCapturePackets = (Snd::StreamPacket*)(mEncoderBuffer + uiAlignedEncoded);
    mEncoderPackets = mCapturePackets + 2;
    mAudioPacket = (VoiceChat::AudioPacket*)(mEncoderPackets + 1);

    unsigned int i = 0;
    do
    {
        ::new (&mCapturePackets[i]) Snd::StreamPacket();
        mCapturePackets[i].mBuffer = pCursor;
        pCursor += uiAlignedFrame;
        ++i;
    } while (i < 2);

    ::new (mEncoderPackets) Snd::StreamPacket();
    mEncoderPackets->mBuffer = mEncoderBuffer;

    VoiceChat::AudioPacket* pAudioPacket = mAudioPacket;
    pAudioPacket->mDiscontinuity = false;
    pAudioPacket->mData = 0;
    pAudioPacket->mSize = 0;
    pAudioPacket->mAppData = 0;

    Snd::StreamFormat cFormat(Snd::SampleInfo::format_PCM16, 8000, 1);

    if (!mSource.Open(0xFF, cFormat, uiFrameBytes, 2, mCapturePackets))
    {

        FreeBuffers();
        return false;
    }

    mEncoder.SetPackets(uiEncodedBytes, mEncoderPackets, 1);

    mEncoder.Connect(&mVAD, 0);
    mVAD.Connect(&mAGC, 0);
    mAGC.Connect(&mSource, 0);
    mAudioPacketIndex = 0;
    mSourceDeviceIndex = 0xFF;
    mPacketSizeMs = fMs;
    mPacketSizeBytes = uiEncodedBytes;
    mAppData = this;
    mNumBuffers = 2;

    SetGain(1.0f);
    return true;
}

// FUNCTION: SWBF2 0x0053d6c0
void GameVoiceChat::Encoder::FreeBuffers()
{
    VoiceChat::Free(mCaptureBuffer);

    mCaptureBuffer = 0;
    mCapturePackets = 0;
    mEncoderBuffer = 0;
    mEncoderPackets = 0;
    mAudioPacket = 0;
}

// FUNCTION: SWBF2 0x0053d710
void GameVoiceChat::Encoder::Close()
{
    mEncoder.Connect(0, 0);
    mVAD.Connect(0, 0);
    mAGC.Connect(0, 0);

    if (IsPlaying())
    {
        Stop();
    }

    if (mAudioPacket->mAppData != 0)
    {
        FreeAudioPacket(mAudioPacket);
    }

    FreeBuffers();

    mPacketSizeMs = 0.0f;
    mPacketSizeBytes = 0;
    mAudioPacketIndex = 0;
    mSourceDeviceIndex = 0;
    mSourceGain = 0.0f;
    mAppData = 0;
    mNumBuffers = 0;

    mSource.Close();
}

// FUNCTION: SWBF2 0x0053d850
void GameVoiceChat::Encoder::FreeAudioPacket(VoiceChat::AudioPacket* packet)
{

    ((Snd::StreamPacket*)packet->mAppData)->Unlock();

    packet->mDiscontinuity = false;
    packet->mData = 0;
    packet->mSize = 0;
    packet->mAppData = 0;
}

// FUNCTION: SWBF2 0x0053d8b0
void GameVoiceChat::Encoder::Flush()
{

    mSource.Flush(false);
    mAGC.Flush();
    mVAD.Flush();
    mEncoder.Flush();
}

// FUNCTION: SWBF2 0x0053d9a0
void GameVoiceChat::Encoder::Stop()
{
    Flush();

    mSource.Stop();
    mAGC.Stop();
    mVAD.Stop();
    mEncoder.Stop();

    mPlaying = false;
}

// FUNCTION: SWBF2 0x0053da00
bool GameVoiceChat::Encoder::IsPlaying()
{
    return mPlaying;
}

// FUNCTION: SWBF2 0x0053da30
void GameVoiceChat::Encoder::SetGain(float fGain)
{
    mSourceGain = fGain;
}

// FUNCTION: SWBF2 0x0053da40
unsigned int GameVoiceChat::Encoder::GetMaxOutputPacketSizeBytes() const
{
    return mPacketSizeBytes;
}

// FUNCTION: SWBF2 0x0053da50
float GameVoiceChat::Encoder::GetMaxOutputPacketSizeMs() const
{
    return mPacketSizeMs;
}

// FUNCTION: SWBF2 0x0053da70
bool VoiceChat::Encoder::Open(unsigned int captureDeviceIndex, float minBufferSizeMs, unsigned int minBuffers)
{
    return static_cast<GameVoiceChat::Encoder*>(this)->Open(captureDeviceIndex, minBufferSizeMs, minBuffers);
}

// FUNCTION: SWBF2 0x0053daa0
void VoiceChat::Encoder::Close()
{
    static_cast<GameVoiceChat::Encoder*>(this)->Close();
}

// FUNCTION: SWBF2 0x0053dae0
void VoiceChat::Encoder::Stop()
{
    static_cast<GameVoiceChat::Encoder*>(this)->Stop();
}

// FUNCTION: SWBF2 0x0053db10
unsigned int VoiceChat::Encoder::GetMaxOutputPacketSizeBytes() const
{
    return static_cast<const GameVoiceChat::Encoder*>(this)->GetMaxOutputPacketSizeBytes();
}

// FUNCTION: SWBF2 0x0053db20
float VoiceChat::Encoder::GetMaxOutputPacketSizeMs() const
{
    return static_cast<const GameVoiceChat::Encoder*>(this)->GetMaxOutputPacketSizeMs();
}

namespace Snd
{

}

// FUNCTION: SWBF2 0x0053da60
VoiceChat::Encoder::Encoder()
    : mAppData(0)
{
}

// FUNCTION: SWBF2 0x0053d370
GameVoiceChat::Encoder::Encoder()
{
    mPlaying = false;
    mPacketSizeMs = 0.0f;
    mPacketSizeBytes = 0;
    mSourceGain = 0.0f;
    mNumBuffers = 0;
    mCaptureBuffer = 0;
    mCapturePackets = 0;
    mEncoderBuffer = 0;
    mEncoderPackets = 0;
    mAudioPacket = 0;
    mAudioPacketIndex = 0;
    mSourceDeviceIndex = 0;
}

// FUNCTION: SWBF2 0x0053d450
GameVoiceChat::Encoder::~Encoder()
{
}

namespace GameVoiceChat
{
bool IsSuspended();
}
namespace GameVoiceChat
{
bool IsBanned();
}

// FUNCTION: SWBF2 0x0053da10
bool GameVoiceChat::Encoder::IsEnabled()
{
    return mPlaying
        && mSource.GetPlayState()
               != Snd::SourceTransport::playState_stopped;
}

// FUNCTION: SWBF2 0x0053daf0
bool VoiceChat::Encoder::IsEnabled()
{
    return static_cast<GameVoiceChat::Encoder*>(this)->IsEnabled();
}

// FUNCTION: SWBF2 0x0053d7c0
VoiceChat::AudioPacket* GameVoiceChat::Encoder::GetData()
{
    VoiceChat::AudioPacket* const pAudio = mAudioPacket + mAudioPacketIndex;

    if (IsPlaying() && pAudio->mAppData == 0)
    {
        mAGC.Update();
        mVAD.Update();
        mEncoder.Update();

        Snd::StreamPacket* const pPacket = mEncoder.GetPacket(0);
        if (pPacket != 0)
        {
            pAudio->mAppData = pPacket;
            pAudio->mData = pPacket->mBuffer;
            pAudio->mDiscontinuity = pPacket->GetDiscontinuity();
            pAudio->mSize = pPacket->mBufferUsed;

            if (++mAudioPacketIndex == 1)
            {
                mAudioPacketIndex = 0;
            }

            return pAudio;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0053d8e0
void GameVoiceChat::Encoder::Play()
{
    mSource.Play(true);
    mAGC.Play(true);
    mVAD.Play(true);
    mEncoder.Play(true);

    mPlaying = true;
}

// FUNCTION: SWBF2 0x0053dab0
VoiceChat::AudioPacket* VoiceChat::Encoder::GetData()
{
    return static_cast<GameVoiceChat::Encoder*>(this)->GetData();
}

// FUNCTION: SWBF2 0x0053dad0
void VoiceChat::Encoder::Play()
{
    static_cast<GameVoiceChat::Encoder*>(this)->Play();
}

// FUNCTION: SWBF2 0x0053dac0
void VoiceChat::Encoder::FreeAudioPacket(VoiceChat::AudioPacket* packet) { static_cast<GameVoiceChat::Encoder*>(this)->FreeAudioPacket(packet); }

// FUNCTION: SWBF2 0x0053db00
void VoiceChat::Encoder::SetGain(float fGain)
{
    static_cast<GameVoiceChat::Encoder*>(this)->SetGain(fGain);
}
