#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "RedWarning.h"
#include "RedConsole.h"

#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x007df210
bool gBreakOnWarning = true;

// FUNCTION: SWBF2 0x006cccb0
bool GetBreakOnWarning()
{
    return gBreakOnWarning;
}

// GLOBAL: SWBF2 0x007e15cc
static char* kapcSEVERITYSHORT[6] = { "*", "?", "#", "$", "%", "!" };
// GLOBAL: SWBF2 0x007e15e4
static char* kapcSEVERITYLONG[6] = { "VERBOSE", "INFORM", "WARNING", "ERROR", "BUG", "FATAL" };

namespace RedWarning
{
// GLOBAL: SWBF2 0x007e15fc
// g_bOverwriteLogFile
bool g_bOverwriteLogFile = true;
}

// GLOBAL: SWBF2 0x0078403c
static char kpcNullFileName[7] = "[NULL]";
// GLOBAL: SWBF2 0x00784044
static char kpcNullDate[7] = "[NULL]";
// GLOBAL: SWBF2 0x0078404c
static char kpcNullTime[7] = "[NULL]";

namespace RedWarning
{

// GLOBAL: SWBF2 0x009c8448
RED_ERROR_SEVERITY g_eSeverityMin;
// GLOBAL: SWBF2 0x009c844c
// g_aeSeverityToDestination
RED_ERROR_SEVERITY g_aeSeverityToDestination[6];
// GLOBAL: SWBF2 0x009c8464
unsigned int g_auiFormatToDestination[5];
// GLOBAL: SWBF2 0x009c8478
// g_fnLogCallback
void(__cdecl* g_fnLogCallback)(char const*, ...);
// GLOBAL: SWBF2 0x009c847c
// g_eSeverity
RED_ERROR_SEVERITY g_eSeverity;
// GLOBAL: SWBF2 0x009c8480
char* g_pcFileName;
// GLOBAL: SWBF2 0x009c8484
char* g_pcCompileDate;
// GLOBAL: SWBF2 0x009c8488
char* g_pcCompileTime;
// GLOBAL: SWBF2 0x009c848c
int g_iLineNumber;
// GLOBAL: SWBF2 0x009c8490
// g_bFormatted
bool g_bFormatted;
// GLOBAL: SWBF2 0x009c8491
bool g_bValidLogFile;
// GLOBAL: SWBF2 0x009c8492
bool g_bInitialized;

// GLOBAL: SWBF2 0x009c8498
char g_pcLogFileName[64];

}

// FUNCTION: SWBF2 0x006f6af0
void RedWarning::DialogBoxMessage(char const* pcFormat, ...)
{

    char* apcDialogText[2];
    char acMessage[1024] = "";
    va_list args;

    va_start(args, pcFormat);
    vsnprintf(acMessage, sizeof(acMessage), pcFormat, args);
    va_end(args);

    apcDialogText[0] = kapcSEVERITYLONG[g_eSeverity];
    apcDialogText[1] = acMessage;

    DialogBoxParamA(0, MAKEINTRESOURCEA(101), 0, (DLGPROC)DlgProc, (LPARAM)apcDialogText);
}

// FUNCTION: SWBF2 0x006f6b80
int __stdcall RedWarning::DlgProc(HWND__* hDlg, unsigned int uiMessage, unsigned int uiWParam,
                                  long lParam)
{
    switch (uiMessage)
    {
    case WM_INITDIALOG:
        {
            char** apcDialogText = (char**)lParam;
            SetWindowTextA(hDlg, apcDialogText[0]);
            SetWindowTextA(GetDlgItem(hDlg, 1001), apcDialogText[1]);
        }
        return 1;

    case WM_COMMAND:
        switch (LOWORD(uiWParam))
        {
        case IDOK:
            EndDialog(hDlg, uiWParam);
            return 1;
        }
        return 0;
    }

    return 0;
}

// FUNCTION: SWBF2 0x006f6bf0
unsigned int RedWarning::Format(char* pcBuffer, unsigned int uiSize, char* pcMessage,
                                unsigned int uiFormat, char cSeparator)
{
    char* pcStart = pcBuffer;
    char* pcEnd = pcBuffer + uiSize;

    if (uiFormat & 0x00010000)
    {
        pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "\n");

        if (uiFormat & 0x02)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "Message Severity: %d\n", g_eSeverity);
        }
        else if (uiFormat & 0x01)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "Message Severity: %s\n",
                              kapcSEVERITYLONG[g_eSeverity]);
        }

        if (uiFormat & 0x0C)
        {
            if (uiFormat & 0x04)
            {
                pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "%s", g_pcFileName);
            }
            if (uiFormat & 0x08)
            {
                pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "(%d)", g_iLineNumber);
            }
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "\n");
        }

        if (uiFormat & 0x30)
        {
            if (uiFormat & 0x10)
            {
                pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, g_pcCompileTime);
            }
            if (uiFormat & 0x20)
            {
                pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, g_pcCompileDate);
            }
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "\n");
        }

        if (uiFormat & 0x40)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "%s\n", pcMessage);
        }

        if (pcBuffer >= pcEnd)
        {
            pcBuffer = pcEnd - 2;
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "\n");
        }
    }
    else
    {

        if (uiFormat & 0x04)
        {
            unsigned int uiLength = (unsigned int)_snprintf(pcBuffer, pcEnd - pcBuffer, "%s", g_pcFileName);
            while (uiLength < 16 && pcBuffer + 1 + uiLength < pcEnd)
            {
                pcBuffer[uiLength++] = ' ';
            }
            pcBuffer += uiLength;
            *pcBuffer = '\0';
        }

        if (uiFormat & 0x08)
        {
            unsigned int uiLength =
                (unsigned int)_snprintf(pcBuffer, pcEnd - pcBuffer, "(%d) :", g_iLineNumber);
            while (uiLength < 9 && pcBuffer + 1 + uiLength < pcEnd)
            {
                pcBuffer[uiLength++] = ' ';
            }
            pcBuffer += uiLength;
            *pcBuffer = '\0';
        }

        if (uiFormat & 0x02)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "[%s]", kapcSEVERITYLONG[g_eSeverity]);
        }
        else if (uiFormat & 0x01)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "[%s]", kapcSEVERITYSHORT[g_eSeverity]);
        }

        if (uiFormat & 0x10)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "%c%s", cSeparator, g_pcCompileTime);
        }
        if (uiFormat & 0x20)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "%c%s", cSeparator, g_pcCompileDate);
        }
        if (uiFormat & 0x40)
        {
            pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "%c%s", cSeparator, pcMessage);
        }

        if (pcBuffer > pcEnd - 2)
        {
            pcBuffer = pcEnd - 2;
        }

        pcBuffer += _snprintf(pcBuffer, pcEnd - pcBuffer, "\n");
    }

    return (unsigned int)(pcBuffer - pcStart);
}

// FUNCTION: SWBF2 0x006f6e90
RedWarning::RED_ERROR_SEVERITY RedWarning::GetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex)
{
    return g_aeSeverityToDestination[eIndex];
}

// FUNCTION: SWBF2 0x006f6ea0
void RedWarning::Init(void)
{
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_FILE] = RED_SEVERITY_NONE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_CALLBACK] = RED_SEVERITY_NONE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_CONSOLE] = RED_SEVERITY_INFORM;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_TRACE] = RED_SEVERITY_VERBOSE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_BREAK] = RED_SEVERITY_ERROR;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_OPEN_CONSOLE] = RED_SEVERITY_WARNING;
    g_eSeverityMin = RED_SEVERITY_VERBOSE;

    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_CONSOLE] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_TRACE] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_CALLBACK] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_BREAK] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_FILE] = 0x0001004e;

    g_fnLogCallback = 0;
    g_eSeverity = RED_SEVERITY_BUG;
    g_pcFileName = kpcNullFileName;
    g_pcCompileDate = kpcNullDate;
    g_pcCompileTime = kpcNullTime;
    g_iLineNumber = 0;
    g_bFormatted = true;

    if (!g_bOverwriteLogFile)
    {
        int i = 0;
        do
        {
            _snprintf(g_pcLogFileName, sizeof(g_pcLogFileName), "%s_%d.%s", "BFront2", i, "log");
            if (!PblFile::Exists(g_pcLogFileName))
            {
                break;
            }
            ++i;
        } while (i < 10);

        if (i >= 10)
        {
            memset(g_pcLogFileName, 0, sizeof(g_pcLogFileName));
        }
    }
    else
    {
        _snprintf(g_pcLogFileName, sizeof(g_pcLogFileName), "%s.%s", "BFront2", "log");
    }

    g_bInitialized = true;
}

// FUNCTION: SWBF2 0x006f6ff0
void RedWarning::LogMessage(char const* pcFormat, ...)
{
    if (g_eSeverity < g_eSeverityMin)
    {
        return;
    }

    char acFormatted[512];
    char acMessage[1024] = "";
    va_list args;

    va_start(args, pcFormat);
    char* pcText = acFormatted;
    vsnprintf(acMessage, sizeof(acMessage), pcFormat, args);
    va_end(args);

    bool bBreak = false;

    for (int i = 0; i < RED_LOG_NUM_WRITE_DESTINATIONS; ++i)
    {
        if (g_eSeverity < g_aeSeverityToDestination[i])
        {
            continue;
        }

        if (g_bFormatted)
        {
            Format(acFormatted, sizeof(acFormatted), acMessage, g_auiFormatToDestination[i], '|');
        }
        else
        {
            pcText = acMessage;
        }

        RED_ERROR_SEVERITY eBreakAt = RED_SEVERITY_ERROR;
        if (GetBreakOnWarning())
        {
            eBreakAt = RED_SEVERITY_WARNING;
        }
        if (g_bFormatted && !bBreak && g_eSeverity >= eBreakAt)
        {
            bBreak = true;
        }

        switch (i)
        {
        case RED_LOG_DESTINATION_INDEX_CONSOLE:
            if (RedConsole::sInstance)
            {
                if (g_eSeverity >= g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_OPEN_CONSOLE])
                {
                    RedConsole::sInstance->SetMode(1);
                }
                RedConsole::sInstance->PrintMsg(pcText);
            }
            break;

        case RED_LOG_DESTINATION_INDEX_FILE:
            if (g_bValidLogFile)
            {
                FILE* pFile = fopen(g_pcLogFileName, "a");
                if (pFile)
                {
                    fprintf(pFile, pcText);
                    fclose(pFile);
                }
            }
            break;

        case RED_LOG_DESTINATION_INDEX_CALLBACK:
            if (g_fnLogCallback)
            {
                g_fnLogCallback(pcText);
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006f7180
void RedWarning::SetLogData(RED_ERROR_SEVERITY eSeverity)
{
    g_eSeverity = eSeverity;
    g_bFormatted = false;
}

// FUNCTION: SWBF2 0x006f71a0
void RedWarning::SetLogData(RED_ERROR_SEVERITY eSeverity, char const* pcFileName, int iLineNumber,
                            char const* pcCompileDate, char const* pcCompileTime)
{
    g_eSeverity = eSeverity;
    g_bFormatted = true;

    if (pcFileName)
    {
        g_pcFileName = (char*)pcFileName;
        g_pcCompileDate = (char*)pcCompileDate;
        g_pcCompileTime = (char*)pcCompileTime;
        g_iLineNumber = iLineNumber;
    }
    else
    {
        g_pcFileName = kpcNullFileName;
        g_pcCompileDate = kpcNullDate;
        g_pcCompileTime = kpcNullTime;
        g_iLineNumber = 0;
    }
}

// FUNCTION: SWBF2 0x006f7210
void RedWarning::SetLogFileOverwrite(bool bOverwrite)
{
    g_bOverwriteLogFile = bOverwrite;
}

// FUNCTION: SWBF2 0x006f7220
void RedWarning::SetSeverityToDestination(unsigned int uiDestinationMask, RED_ERROR_SEVERITY eSeverity)
{
    g_eSeverityMin = RED_SEVERITY_NONE;

    for (int i = 0; i < RED_LOG_NUM_DESTINATIONS; ++i)
    {
        if (uiDestinationMask & (1u << i))
        {
            SetSeverityToDestination((RED_LOG_DESTINATION_INDEX)i, eSeverity);
        }

        RED_ERROR_SEVERITY eDestination = g_aeSeverityToDestination[i];
        if (g_eSeverityMin > eDestination)
        {
            g_eSeverityMin = eDestination;
        }
    }
}

// FUNCTION: SWBF2 0x006f7280
void RedWarning::SetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex,
                                          RED_ERROR_SEVERITY eSeverity)
{
    if (g_eSeverityMin > eSeverity)
    {
        g_eSeverityMin = eSeverity;
    }
    else if (g_eSeverityMin == g_aeSeverityToDestination[eIndex])
    {

        g_eSeverityMin = eSeverity;
        for (int i = 0; i < RED_LOG_NUM_DESTINATIONS; ++i)
        {
            if (i == eIndex)
            {
                continue;
            }
            if (g_eSeverityMin > g_aeSeverityToDestination[i])
            {
                g_eSeverityMin = g_aeSeverityToDestination[i];
            }
        }
    }

    g_aeSeverityToDestination[eIndex] = eSeverity;

    if (!g_bValidLogFile && g_pcLogFileName[0] != '\0'
        && eIndex == RED_LOG_DESTINATION_INDEX_FILE && eSeverity < RED_SEVERITY_NONE)
    {
        FILE* pFile = fopen(g_pcLogFileName, "w");
        g_bValidLogFile = (pFile != 0);

        if (g_bValidLogFile)
        {
            char acTime[256];
            memset(acTime, 0, sizeof(acTime));

            __time64_t tNow = _time64(0);
            strftime(acTime, sizeof(acTime) - 2, " %Y-%m-%d %H%M", _localtime64(&tNow));

            fprintf(pFile, "Opened logfile %s %s\n", g_pcLogFileName, acTime);
            fclose(pFile);
        }
    }
}
