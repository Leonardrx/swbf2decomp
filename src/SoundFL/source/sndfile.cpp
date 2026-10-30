#include "sndfile.h"

#include "PebbleFL/Common/PblHashTable.h"

namespace Snd
{

// GLOBAL: SWBF2 0x009cf468
// File::smExitFileThread
bool File::smExitFileThread;
// GLOBAL: SWBF2 0x009cf469
// File::smFileSystemOpen
bool File::smFileSystemOpen;
// SYNTHETIC: SWBF2 0x0040b400
// `dynamic initializer for 'Snd::File::smQueuedFileOpListLock''
// SYNTHETIC: SWBF2 0x0076a130
// `dynamic atexit destructor for 'Snd::File::smQueuedFileOpListLock''

// GLOBAL: SWBF2 0x009cf46c
// File::smQueuedFileOpListLock
Semaphore File::smQueuedFileOpListLock;
// GLOBAL: SWBF2 0x009cf47c
Thread File::smFileThread;
// SYNTHETIC: SWBF2 0x0040b3e0
// `dynamic initializer for 'Snd::File::smQueuedCommands''
// SYNTHETIC: SWBF2 0x0076a100
// `dynamic atexit destructor for 'Snd::File::smQueuedCommands''

// GLOBAL: SWBF2 0x009cf48c
// File::smQueuedCommands
Semaphore File::smQueuedCommands;
// SYNTHETIC: SWBF2 0x0040b3f0
// `dynamic initializer for 'Snd::File::smQueuedFileOpList''
// SYNTHETIC: SWBF2 0x0076a110
// `dynamic atexit destructor for 'Snd::File::smQueuedFileOpList''

// GLOBAL: SWBF2 0x007e3228
// File::smQueuedFileOpList
PblListDoubleSorted File::smQueuedFileOpList(true);
// SYNTHETIC: SWBF2 0x0040b3d0
// `dynamic initializer for 'Snd::File::smFileList''
// SYNTHETIC: SWBF2 0x0076a0e0
// `dynamic atexit destructor for 'Snd::File::smFileList''

// GLOBAL: SWBF2 0x007e3234
// File::smFileList
PblListDoubleSorted File::smFileList(false);

// FUNCTION: SWBF2 0x00730ae0
bool File::SystemOpen(unsigned int uiPriority)
{
    if (!smFileSystemOpen)
    {
        smFileThread.Init(FileThread, uiPriority, 0x1000);
        smExitFileThread = false;

        if (!smQueuedCommands.Create(0, 0x7FFFFFFF))
        {
            return false;
        }

        if (!smQueuedFileOpListLock.Create(1, 1))
        {
            return false;
        }

        Semaphore startedSignal;
        if (!startedSignal.Create(0, 1))
        {
            smQueuedFileOpListLock.Destroy();
            return false;
        }

        if (smFileThread.Start(&startedSignal))
        {
            startedSignal.Wait();
            startedSignal.Destroy();
            smFileSystemOpen = true;
        }
        else
        {
            startedSignal.Destroy();
            smQueuedFileOpListLock.Destroy();
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00730ac0
void File::SystemClose()
{

    smExitFileThread = true;
    smQueuedCommands.Signal();
    smFileSystemOpen = false;
}

// FUNCTION: SWBF2 0x00730500
File::File(void(__cdecl* pfnOpComplete)(File*, void*, bool), void* pCallbackData)
    : mOpCompleteFunc(pfnOpComplete), mCallbackData(pCallbackData), mFileOp(fileOp_nothing), mHashID(0),
      mAccessFlags(0), mBufferSize(0), mSectorSize(0), mAlignment(0)
{
    mLock.Create(1, 1);
}

// FUNCTION: SWBF2 0x007305b0
File::File()
    : mOpCompleteFunc(0),
      mCallbackData(0),
      mFileOp(fileOp_nothing),
      mHashID(0),
      mAccessFlags(0),
      mBufferSize(0),
      mSectorSize(0),
      mAlignment(0)
{
    mLock.Create(1, 1);
}

// FUNCTION: SWBF2 0x00730660
File::~File()
{

    if (mFile.IsOpen())
    {
        Close();
    }
    mLock.Destroy();
}

// FUNCTION: SWBF2 0x00730720
File& File::operator=(File& rOther)
{

    mLock.Wait();
    mFile = rOther.mFile;
    mOpCompleteFunc = rOther.mOpCompleteFunc;
    mCallbackData = rOther.mCallbackData;
    mFilePos = rOther.mFilePos;
    mFileOp = rOther.mFileOp;
    mHashID = rOther.mHashID;
    mNode.mSortValue = rOther.mNode.mSortValue;
    mAccessFlags = rOther.mAccessFlags;
    mBufferSize = rOther.mBufferSize;
    mSectorSize = rOther.mSectorSize;
    mAlignment = rOther.mAlignment;
    mLock.Signal();
    return *this;
}

// FUNCTION: SWBF2 0x00730790
void File::Close()
{
    mLock.Wait();
    mFile.Close();
    mHashID = 0;
    mLock.Signal();
}

// FUNCTION: SWBF2 0x007307c0
void File::CompleteFileOp()
{

    bool bOk;
    unsigned int uiDone;

    switch (mFileOp)
    {
    case fileOp_open:
        bOk = mFile.Open(mFileOpData.filename, mAccessFlags, mBufferSize, mSectorSize, mAlignment);
        if (bOk)
        {
            mHashID = PblHash(mFileOpData.filename);
        }
        else
        {
            mHashID = 0;
        }
        break;

    case fileOp_read:
        mFile.Seek(mFilePos);
        uiDone = mFile.Read(mFileOpData.dataBlock, mBlockSize);
        mFilePos += uiDone;
        bOk = uiDone == mBlockSize;
        mBlockSize = uiDone;
        break;

    case fileOp_write:
        mFile.Seek(mFilePos);
        uiDone = mFile.Write(mFileOpData.dataBlock, mBlockSize);
        mFilePos += uiDone;
        bOk = uiDone == mBlockSize;
        mBlockSize = uiDone;
        break;

    default:
        bOk = false;
        break;
    }

    mFileOp = fileOp_nothing;
    mOpCompleteFunc(this, mCallbackData, bOk);
    mBlockSize = 0;
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00730890
void File::FileThread(void* pStartedSignal)
{
    ((Semaphore*)pStartedSignal)->Signal();

    while (!smExitFileThread)
    {
        smQueuedCommands.Wait();
        if (smExitFileThread)
        {
            break;
        }

        smQueuedFileOpListLock.Wait();

        PblListDoubleSorted::Node* pNode = smQueuedFileOpList.GetFirst();
        File* const pFile = (File*)pNode->GetObjectA(&smQueuedFileOpList, offsetof(File, mNode));
        if (pFile)
        {
            pFile->mNode.mNode.Remove();
            pFile->mLock.Wait();
        }

        smQueuedFileOpListLock.Signal();

        if (pFile)
        {
            pFile->CompleteFileOp();
        }
    }
}

// FUNCTION: SWBF2 0x00730940
unsigned int File::GetPos() const
{

    Semaphore& rLock = const_cast<Semaphore&>(mLock);
    rLock.Wait();
    unsigned int uiPos = mFilePos;
    rLock.Signal();
    return uiPos;
}

// FUNCTION: SWBF2 0x00730960
void File::Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
                unsigned int uiSectorSize, unsigned int uiAlignment)
{

    mLock.Wait();
    mFileOpData.filename = (char*)pcFileName;
    mAccessFlags = uiAccessFlags;
    mBufferSize = uiBufferSize;
    mSectorSize = uiSectorSize;
    mFilePos = 0;
    mBlockSize = 0;
    mFileOp = fileOp_open;
    mAlignment = uiAlignment;
    CompleteFileOp();
}

bool File::InsertFile(File* pFile)
{
    smQueuedFileOpListLock.Wait();
    smQueuedFileOpList.Add(&pFile->mNode);
    smQueuedCommands.Signal();
    smQueuedFileOpListLock.Signal();
    return true;
}

// FUNCTION: SWBF2 0x007309b0
void File::Read(void* pBuffer, unsigned int uiSize)
{
    mLock.Wait();
    mFileOpData.dataBlock = pBuffer;
    mBlockSize = uiSize;
    mFileOp = fileOp_read;

    if (mFile.IsDataBuffered(mFilePos, uiSize))
    {
        CompleteFileOp();
        return;
    }
    if (mFileOp == fileOp_open)
    {
        CompleteFileOp();
        return;
    }

    InsertFile(this);
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00730a30
void File::Seek(unsigned int uiPos)
{
    mLock.Wait();
    mFilePos = uiPos;
    mLock.Signal();
}

// FUNCTION: SWBF2 0x00730a60
void File::SetPriority(unsigned int uiPriority)
{

    mLock.Wait();
    mNode.mSortValue = 1.0f - (float)uiPriority * 2.3283064e-10f;
    mLock.Signal();
}

}
