#pragma once
#include <stdio.h>
#include <stdlib.h> 
#include <imgui.h>
#include <ctype.h>

// IMGUI CONSOLE

struct ConsoleLine
{
    ImVec4 color;
    const char *text;
};

struct ConsoleUI
{
    char                  InputBuf[256];
    ImVector<ConsoleLine> Items;
    bool                  ScrollToBottom;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImVector<const char*> candidates;
	bool                  consoleActive;

    ConsoleUI();
    ~ConsoleUI();

    static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
    static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
    static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buff = malloc(len); return (char*)memcpy(buff, (const void*)str, len); }

    void ClearLog();
    void AddLog(const char* fmt, ...) IM_FMTARGS(2);
    void AddLog(ImVec4 color, const char * fmt, ...) IM_FMTARGS(3);
    void Draw(int width, int height);
    void ExecCommand(const char * command_line);
    static int TextEditCallbackStub(ImGuiInputTextCallbackData * data);
    int TextEditCallback(ImGuiInputTextCallbackData* data);
};

ConsoleUI* IMConsole();