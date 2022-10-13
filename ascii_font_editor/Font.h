#pragma once
#include <vector>
#include <string>
#include "Utils.h"

class Font
{
private:
	std::vector<unsigned char>		_dict;
	int								_height;
	int								_width;
	std::vector<utf8char_t>			_seq;	//Empty associated char sequence means font is 0-255 ASCII representation
	bool							_utf8;

	void _parseSequence(std::string seq, size_t count);

public:
	Font(const std::string& pathToTxtFont);
	Font(const std::vector<unsigned char>& dict, int h, int w, const std::string& seq = "", bool utf8 = false);
	Font(const std::vector<unsigned char>& dict, int h, int w, const std::vector<utf8char_t>& seq, bool utf8 = false);
	Font(const Font& copy);
	Font& operator=(const Font& copy);
	~Font();

	static Font makeEmptyFont(int h, int w, int count);

	bitmap_t operator[](utf8char_t сh) const;

	int GetHeight() const;
	int GetWidth() const;
	bitmap_t GetLetterImage_8bit(utf8char_t ch) const;
	bitmap_t getFontTable(int maxColumn) const;
	size_t CharCount() const;
	std::vector<utf8char_t> GetAllSupportedChars() const;
};