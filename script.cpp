/*
	ViewportEditor

	RAGE GTA 5 b1290 CViewportGame Editor

	Transmet 2018
*/

#include "script.h"
#include "keyboard.h"
#include <string>
#include <Psapi.h>

#pragma warning(disable : 4244 4305) // double <-> float conversions


void DrawTextShadow(char* caption, float x, float y, int r, int g, int b, int a, int font = 0, float text_scale = 0.35)
{
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(r, g, b, a);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(5, 0, 0, 0, 255);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(caption);
	UI::END_TEXT_COMMAND_DISPLAY_TEXT(x, y);

	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(r, g, b, a);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(5, 0, 0, 0, 255);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_BEGIN_TEXT_COMMAND_LINE_COUNT("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(caption);

	UI::_GET_TEXT_SCREEN_LINE_COUNT(x, y);
}


typedef unsigned int long long QWORD;

static QWORD PatternSearch(const char* pattern, const char* mask, const int length)
{
	QWORD base = (QWORD)GetModuleHandle(nullptr);
	MODULEINFO modinfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)base, &modinfo, sizeof(MODULEINFO));
	QWORD sizeImage = modinfo.SizeOfImage;

	for (QWORD i = base; i < base + sizeImage; i++)
	{
		bool bypass = true;
		for (unsigned short it = 0; it < length; it++)
		{
			if (mask[it] == '1')
				if (pattern[it] != *((char*)(i + it)))
				{
					bypass = false;
					break;
				}
		}

		if (bypass)
			return i;
	}

	return 0;
}


struct {
	unsigned short offset;
	const char* name;
} properties[] = {
	{ 0x0420, "SceneX" },
	{ 0x0424, "SceneY" },
	{ 0x0428, "SceneWidth" },
	{ 0x042C, "SceneHeight" },

	{ 0x0460, "ScaleX" },
	{ 0x0464, "ScaleY" },

	{ 0x0468, "Val1_0" },
	{ 0x046C, "Val1_1" },

	{ 0x0430, "Val2_0" },
	{ 0x0434, "Val2_1" },
};
unsigned short countProps = sizeof(properties) / sizeof(properties[0]);

void ScriptMain()
{
	// E8????????488B??????????488D??????????488BCDE8????????80??????????01751A
	QWORD CViewportGamePat = PatternSearch(
		"\xE8\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8D\x00\x00\x00\x00\x00\x48\x8B\xCD\xE8\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x01\x75\x1A",
		"100001100000110000011110000100000111",
		36
	);

	if (!CViewportGamePat)
	{
		MessageBoxA(0, "Pattern not found !", "ViewportEditor", 0);
		return;
	}

	CViewportGamePat += 8;
	DWORD offsetPtr = *(DWORD*)(CViewportGamePat);
	CViewportGamePat += offsetPtr + 4;
	CViewportGamePat = *(QWORD*)(CViewportGamePat);


	char bufOut[128];
	bool renderSelector = false;
	unsigned short index = 0;
	DWORD ticktick = 0;
	for (;;)
	{
		if (renderSelector)
		{
			GRAPHICS::DRAW_RECT(0.075f, 0.042f, 0.135f, 0.05f, 0, 155, 255, 200);
			DrawTextShadow("ViewportEditor 0.1", 0.02f, 0.024f, 255, 255, 255, 255, 6, 0.5f);

			for (unsigned short i = 0; i < countProps; i++)
			{
				sprintf_s(
					bufOut,
					"[%.3f]     %s",
					*(float*)(CViewportGamePat + properties[i].offset),
					properties[i].name
				);

				if (i == index)
				{
					GRAPHICS::DRAW_RECT(0.075f, 0.1f + (i * 0.043f), 0.135f, 0.04f, 0, 155, 255, 200);
					DrawTextShadow(bufOut, 0.02f, 0.083f + (i * 0.043f), 255, 255, 255, 255, 6, 0.5f);
				}
				else
				{
					GRAPHICS::DRAW_RECT(0.075f, 0.1f + (i * 0.043f), 0.135f, 0.04f, 255, 255, 255, 170);
					DrawTextShadow(bufOut, 0.02f, 0.083f + (i * 0.043f), 0, 155, 255, 255, 6, 0.5f);
				}
			}

			float eom = 0.153f + ((countProps - 1) * 0.043f);
			GRAPHICS::DRAW_RECT(0.075f, eom, 0.135f, 0.05f, 0, 155, 255, 200);
			DrawTextShadow("Numpad 4/6 to increment/decrement\nNumpad 8/2 to up/down in menu", 0.02f, eom - 0.022f, 255, 255, 255, 255, 6, 0.35f);

			if (IsKeyJustUp(VK_NUMPAD8))
			{
				if (index - 1 < 0)
					index = countProps - 1;
				else
					index--;
			}
			else if (IsKeyJustUp(VK_NUMPAD2))
			{
				if (index + 1 >= countProps)
					index = 0;
				else
					index++;
			}
		}

		DWORD curTick = GetTickCount();
		if (ticktick < curTick)
		{
			if (renderSelector)
			{
				if (IsKeyDown(VK_NUMPAD4))
					*(float*)(CViewportGamePat + properties[index].offset) -= 0.003f;
				else if (IsKeyDown(VK_NUMPAD6))
					*(float*)(CViewportGamePat + properties[index].offset) += 0.003f;

				ticktick = curTick + 5;
			}

			if (IsKeyDown(VK_CONTROL) && IsKeyDown(0x43))
			{
				renderSelector = !renderSelector;
				ticktick = curTick + 300;
			}
		}

		WAIT(0);
	}
}
