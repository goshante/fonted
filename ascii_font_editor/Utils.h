#pragma once
#include <string>
#include <functional>
#include <vector>
#include <Windows.h>

using pixel_t = unsigned char;
using bitmap_t = std::vector<std::vector<pixel_t>>;
using utf8char_t = unsigned long;

#define Min(a,b) (a < b ? a : b)
#define Max(a,b) (a > b ? a : b)

#define ET_STATIC						0
#define ET_EDIT							1
#define ET_COMBOBOX						2
#define ET_LISTBOX						3
#define ET_MDICLIENT					4
#define ET_SCROLLBAR					5
#define ET_BUTTON						6
#define ET_CHECKBOX						7
#define ET_RADIOBUTTON					8
#define ET_GROUPBOX						9
#define ET_RICHEDIT						10
#define ET_PROGRESS						11
#define ET_SLIDER_V						12
#define ET_SLIDER_H						13

enum class FileDialogType
{
	Open,
	Save,
	SelectFolder
};

void InitBitmap(bitmap_t& bmp, int h, int w);
void RemoveBOMFromString(std::string& str);
size_t strlen_utf8(const std::string& u8str);
void enumerateUTF8String(const std::string& u8str, std::function<void(utf8char_t ch, size_t n, size_t cpsz)> callback);
std::string utf8char_to_stdString(utf8char_t ch);
std::vector<unsigned char> bmp2raw(const bitmap_t& bmp);
void bmpUpscaleLinear(bitmap_t& bmp, int scale);
HWND CreateWindowElement(HWND Parent, UINT Type, const char* Title, HINSTANCE hInst, DWORD Style, DWORD StyleEx, HMENU ElementID, INT pos_x, INT pos_y, INT Width, INT Height, BOOL NewRadioGroup);
std::string browse(HWND hwnd, HWND outputWindow, FileDialogType fdType);