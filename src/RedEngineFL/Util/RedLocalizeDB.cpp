#include <windows.h>
#include <string.h>

#include "RedLocalizeDB.h"

#include "Battlefront2/Source/GameMemory.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Interface/RedFont.h"

// GLOBAL: SWBF2 0x00940bf0
FLLocalizeEntry* mPatchLocalizeFile;

void __cdecl PblTraceP(const char* Format, ...);

// GLOBAL: SWBF2 0x00940bec
// mMasterLocalizeFile
FLLocalizeEntry* mMasterLocalizeFile;

// GLOBAL: SWBF2 0x00940bf4
bool gbDeterminedLang;
// GLOBAL: SWBF2 0x00940bf8
eGAMELANGUAGE gUseLang;
// GLOBAL: SWBF2 0x00940bfc
static int mSkipCount;
// SYNTHETIC: SWBF2 0x00407d60
// `dynamic initializer for 'sMasterLocalizeHash''

// GLOBAL: SWBF2 0x00940c00
PblHashTable<FLLocalizeEntry, 16384> sMasterLocalizeHash;

// GLOBAL: SWBF2 0x00960c04
char const* gLanguageStr;

// GLOBAL: SWBF2 0x007def94
eGAMELANGUAGE gForceLang = eLANG_AUTODETERMINE;

namespace PblBuildType
{
// GLOBAL: SWBF2 0x007e2e00
eGAMELANGUAGE gForceLang1 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e04
eGAMELANGUAGE gForceLang2 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e08
eGAMELANGUAGE gForceLang3 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e10
bool gDemoEnglishOnly = true;
// GLOBAL: SWBF2 0x009cef71
bool gDemoBuild;
}

namespace RedLocalizeDB
{

// FUNCTION: SWBF2 0x006c6010
void SetSkipCount(int count)
{
    mSkipCount = count;
}

// FUNCTION: SWBF2 0x006c5c30
void PickLanguage()
{
    gUseLang = eLANG_ENGLISH;

    eGAMELANGUAGE eLang;
    switch (PRIMARYLANGID(GetUserDefaultLangID()))
    {
    case LANG_FRENCH:   eLang = eLANG_FRENCH;   break;
    case LANG_SPANISH:  eLang = eLANG_SPANISH;  break;
    case LANG_GERMAN:   eLang = eLANG_GERMAN;   break;
    case LANG_ITALIAN:  eLang = eLANG_ITALIAN;  break;
    case LANG_JAPANESE: eLang = eLANG_JAPANESE; break;
    case LANG_POLISH:   eLang = eLANG_POLISH;   break;
    case LANG_THAI:     eLang = eLANG_THAI;     break;
    case LANG_NEUTRAL:
    case LANG_ARABIC:
    case LANG_BULGARIAN:
    case LANG_CATALAN:
    case LANG_CHINESE:
    case LANG_CZECH:
    case LANG_DANISH:
    case LANG_GREEK:
    case LANG_ENGLISH:
    case LANG_FINNISH:
    case LANG_HEBREW:
    case LANG_HUNGARIAN:
    case LANG_ICELANDIC:
    case LANG_KOREAN:
    case LANG_DUTCH:
    case LANG_NORWEGIAN:
    case LANG_PORTUGUESE:
    case LANG_ROMANIAN:
    case LANG_RUSSIAN:
    case LANG_SERBIAN:
    case LANG_SLOVAK:
    case LANG_ALBANIAN:
    case LANG_SWEDISH:
    case LANG_TURKISH:
    case LANG_URDU:
    case LANG_INDONESIAN:
    case LANG_UKRAINIAN:
    case LANG_BELARUSIAN:
    case LANG_SLOVENIAN:
    case LANG_ESTONIAN:
    case LANG_LATVIAN:
    case LANG_LITHUANIAN:
    case LANG_FARSI:
    case LANG_VIETNAMESE:
    case LANG_ARMENIAN:
    case LANG_AZERI:
    case LANG_BASQUE:
    case LANG_MACEDONIAN:
    case LANG_AFRIKAANS:
    case LANG_GEORGIAN:
    case LANG_FAEROESE:
    case LANG_HINDI:
    case LANG_MALAY:
    case LANG_KAZAK:
    case LANG_KYRGYZ:
    case LANG_SWAHILI:
    case LANG_UZBEK:
    case LANG_TATAR:
    case LANG_BENGALI:
    case LANG_PUNJABI:
    case LANG_GUJARATI:
    case LANG_ORIYA:
    case LANG_TAMIL:
    case LANG_TELUGU:
    case LANG_KANNADA:
    case LANG_MALAYALAM:
    case LANG_ASSAMESE:
    case LANG_MARATHI:
    case LANG_SANSKRIT:
    case LANG_MONGOLIAN:
    case LANG_GALICIAN:
    case LANG_KONKANI:
    case LANG_MANIPURI:
    case LANG_SINDHI:
    case LANG_SYRIAC:
    case LANG_KASHMIRI:
    case LANG_NEPALI:
    case LANG_DIVEHI:
    case LANG_INVARIANT:
        eLang = eLANG_ENGLISH;
        break;
    default:            eLang = eLANG_ENGLISH;  break;
    }

    if (PblBuildType::gDemoBuild && PblBuildType::gDemoEnglishOnly)
    {
        eLang = eLANG_ENGLISH;
    }

    if (PblBuildType::gForceLang1 >= 0 && eLang != PblBuildType::gForceLang1
        && eLang != PblBuildType::gForceLang2 && eLang != PblBuildType::gForceLang3)
    {
        eLang = PblBuildType::gForceLang1;
    }

    if (gForceLang != eLANG_AUTODETERMINE)
    {
        eLang = gForceLang;
    }

    gUseLang = eLang;

    switch (gUseLang)
    {
    case eLANG_ENGLISH:  gLanguageStr = "english";    break;
    case eLANG_SPANISH:  gLanguageStr = "spanish";    break;
    case eLANG_ITALIAN:  gLanguageStr = "italian";    break;
    case eLANG_FRENCH:   gLanguageStr = "french";     break;
    case eLANG_GERMAN:   gLanguageStr = "german";     break;
    case eLANG_JAPANESE: gLanguageStr = "japanese";   break;
    case eLANG_UK:       gLanguageStr = "uk_english"; break;
    case eLANG_THAI:     gLanguageStr = "thai";       break;
    case eLANG_POLISH:   gLanguageStr = "polish";     break;
    default:             gLanguageStr = "english";    break;
    }

    gbDeterminedLang = true;
}

// FUNCTION: SWBF2 0x006c5b30
const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing)
{
    switch (uiHash)
    {
    case 0xb0a4d342: /* "ifs.mp.joinerrors.doesnotexist" */
        // STRING: SWBF2 0x00772c08
        return (const unsigned short*)L"Cannot join: does not exist";
    case 0xe6407c4d: /* "ifs.mp.joinerrors.cantconnect" */
        // STRING: SWBF2 0x00772c40
        return (const unsigned short*)L"Cannot join: unable to connect";
    }

    FLLocalizeEntry* pEntry = sMasterLocalizeHash.Find(uiHash);
    if (!pEntry)
    {
        // STRING: SWBF2 0x00772c80
        return bNullIfMissing ? 0 : (const unsigned short*)L"[NULL]";
    }
    return pEntry->mData;
}

// FUNCTION: SWBF2 0x006c5b80
void ForceLanguageID(int iLanguage)
{
    if (iLanguage == 5)
    {
        RedFont::sJapaneseOnly = true;
        gForceLang = static_cast<eGAMELANGUAGE>(iLanguage);
    }
    else if (iLanguage >= -1 && iLanguage < 9)
    {
        gForceLang = static_cast<eGAMELANGUAGE>(iLanguage);
    }
}

// FUNCTION: SWBF2 0x006c5bf0
void GetLocalizedString(unsigned short* pusBuffer, unsigned int uiSize, unsigned int uiHash)
{
    memset(pusBuffer, 0, uiSize);
    const unsigned short* pusString = Find(uiHash, false);
    if (pusString)
    {
        PblUnicode::ustrncpy(pusBuffer, pusString, uiSize - 1);
    }
}

// FUNCTION: SWBF2 0x006c5bb0
eGAMELANGUAGE GetLanguageID()
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    return gUseLang;
}

// FUNCTION: SWBF2 0x006c5bd0
char const* GetLanguageStr()
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    return gLanguageStr;
}

// FUNCTION: SWBF2 0x006c5e50
bool Read(PblFileChunk& chunk)
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    bool bLanguage = false;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x454D414Eu:
        {
            char acName[256];
            child.ReadString(acName, 256);
            bLanguage = _stricmp(acName, gLanguageStr) == 0;
            break;
        }
        case 0x59444F42u:
            if (bLanguage)
            {
                if (mSkipCount)
                {
                    --mSkipCount;
                }
                else
                {
                    ReadMasterLocalizeFile(child);
                }
            }
            break;
        default:
            PblTraceP("Uhoh? Unknown chunk %08X\n", child._iId);
            break;
        }
    }

    return bLanguage;
}

// FUNCTION: SWBF2 0x006c5f40
void ReadMasterLocalizeFile(PblFileChunk& chunk)
{
    const int iSize = chunk._iSize;
    mMasterLocalizeFile = (FLLocalizeEntry*)RedAllocFromHeap(GameMemory::RunTimeHeap, iSize, 0);
    chunk._pFile->ReadData(mMasterLocalizeFile, iSize);

    for (FLLocalizeEntry* pEntry = mMasterLocalizeFile; pEntry->mEntryLen;
         pEntry = (FLLocalizeEntry*)((char*)pEntry + pEntry->mEntryLen))
    {
        if (!sMasterLocalizeHash.Find(pEntry->mEntryHash))
        {
            sMasterLocalizeHash.Store(pEntry->mEntryHash, pEntry);
        }
    }
}

}

// FUNCTION: SWBF2 0x006c5fd0
void RedLocalizeDB::Reset()
{
    sMasterLocalizeHash.Clear();
    mMasterLocalizeFile = 0;
    mPatchLocalizeFile = 0;
}
