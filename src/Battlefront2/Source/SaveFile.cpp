#include "Battlefront2/Platform/Pc/pcSaveFile.h"
#include <ctype.h>

// FUNCTION: SWBF2 0x00631460
void SaveFile::StringToTime(const unsigned short* pusString, int& iYear, int& iMonth,
                            int& iDay, int& iHour, int& iMinute, int& iSecond)
{
    unsigned int value = 0;
    for (const unsigned short* p = pusString; *p; ++p)
    {
        value *= 16;
        if (*p >= '0' && *p <= '9')
        {
            value += *p - '0';
        }
        else
        {
            value += tolower(*p) - 'a' + 10;
        }
    }

    iSecond = value % 60;
    value /= 60;
    iMinute = value % 60;
    value /= 60;
    iHour = value % 24;
    value /= 24;
    iDay = value & 31;
    value /= 32;
    iMonth = value % 13;
    value /= 13;
    iYear = value + 100 * (20 - value / 100);
}
