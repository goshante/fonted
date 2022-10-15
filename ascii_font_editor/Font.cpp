#include "Font.h"
#include <Windows.h>
#include <stdexcept>
#include <sstream>
#include "reutils.h"
#include "Utils.h"

Font::Font(const std::string& pathToTxtFont)
{
	HANDLE hFile = CreateFileA(pathToTxtFont.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Failed to open font file");

	DWORD dwWritten = 0, dwSize = GetFileSize(hFile, NULL);
	char* buf = new char[dwSize + 1];
	if (!ReadFile(hFile, buf, dwSize, &dwWritten, NULL))
	{
		CloseHandle(hFile);
		throw std::runtime_error("Failed to read file");
	}
	CloseHandle(hFile);
	buf[dwSize] = '\0';

	std::string fileContent = buf;
	delete[] buf;
	RemoveBOMFromString(fileContent);
	auto match = reu::Search(fileContent, "^([0-9]+)x([0-9]+)\\r{0,1}\\n\\[(.+)\\]\\r{0,1}\\ni([0-9]{1,2})\\r{0,1}\\n");
	if (!match.IsMatching())
		throw std::runtime_error("Font file has invalid format");

	_width = std::atoi(match[1].c_str());
	_height = std::atoi(match[2].c_str());
	_interval = std::atoi(match[4].c_str());

	if (_width <= 0 || _height <= 0)
		throw std::runtime_error("Invalid font resolution");

	if (_interval < 0 || _interval > _width)
		throw std::runtime_error("Invalid interval value");

	for (size_t i = match.End() + 1; i < fileContent.length(); i++)
		_dict.push_back(fileContent[i] == '0' ? 0 : 1);

	auto count = _dict.size() / (_width * _height);

	if (count == 0)
		throw std::runtime_error("Font character count is zero");
	_parseSequence(match[3], count);

	if (_dict.size() % (_width * _height) != 0)
		throw std::runtime_error("Font is corrupted or has wrong resolution");

	if (count != _seq.size())
		throw std::runtime_error("Font character count not matching font character sequence.");
}

Font::Font(const std::vector<unsigned char>& dict, int h, int w, int interval, const std::string& seq, bool utf8)
	: _height(h)
	, _width(w)
	, _interval(interval)
	, _utf8(utf8)
{
	if (w <= 0 || h <= 0)
		throw std::runtime_error("Invalid font resolution");

	if (dict.size() % (_width * _height) != 0)
		throw std::runtime_error("Font is corrupted or has wrong resolution");

	if (interval < 0 || interval > w)
		throw std::runtime_error("Invalid interval value");

	auto count = dict.size() / (_width * _height);

	if (count == 0)
		throw std::runtime_error("Font character count is zero");
	_parseSequence(seq, count);

	if (count != _seq.size())
		throw std::runtime_error("Font character count not matching font character sequence.");

	for (auto& c : dict)
		_dict.push_back(c == '0' ? 0 : 1);
}

Font::Font(const std::vector<unsigned char>& dict, int h, int w, int interval, const std::vector<utf8char_t>& seq, bool utf8)
	: _height(h)
	, _width(w)
	, _interval(interval)
	, _seq(seq)
	, _utf8(utf8)
{
	if (w <= 0 || h <= 0)
		throw std::runtime_error("Invalid font resolution");

	if (dict.size() % (_width * _height) != 0)
		throw std::runtime_error("Font is corrupted or has wrong resolution");

	if (interval < 0 || interval > w)
		throw std::runtime_error("Invalid interval value");

	auto count = dict.size() / (_width * _height);

	if (utf8)
	{
		if (!seq.empty() && count != seq.size())
			throw std::runtime_error("Font character count not matching font character sequence.");
	}
	else
	{
		if (!seq.empty() && count != seq.size())
			throw std::runtime_error("Font character count not matching font character sequence.");
	}

	for (auto& c : dict)
		_dict.push_back(c == '0' ? 0 : 1);
}

Font::Font(const Font& copy)
	: _dict(copy._dict)
	, _height(copy._height)
	, _width(copy._width)
	, _seq(copy._seq)
	, _utf8(copy._utf8)
{
}

Font& Font::operator=(const Font& copy)
{
	_dict = copy._dict;
	_height = copy._height;
	_width = copy._width;
	_seq = copy._seq;
	_utf8 = copy._utf8;
	return *this;
}

Font::~Font()
{
}

void Font::_parseSequence(std::string seq, size_t count)
{
	if (seq.empty())
	{
		std::stringstream ss;
		ss << 0 << "-" << count - 1;
		seq = ss.str();
	}

	if (!reu::IsMatching(seq, "^[0-9a-fA-Fx\\ \\,\\-]+$"))
		throw std::runtime_error("Font alphabet sequence has invalid format");

	seq.erase(remove_if(seq.begin(), seq.end(), isspace), seq.end());
	std::stringstream ss;
	std::string str;

	auto str2uch = [](const std::string& str) ->utf8char_t
	{
		utf8char_t ch;
		if (reu::IsMatching(str, "^([0-9]+)$"))
			ch = std::atoi(str.c_str());
		else
		{
			std::stringstream conv;
			conv << std::hex << str;
			conv >> ch;
		}
		return ch;
	};

	bool utf8 = false;
	ss << seq;
	while (std::getline(ss, str, ','))
	{
		if (str.empty())
			continue;

		auto m = reu::Search(str, "^([0-9a-fA-Fx]+)\\-([0-9a-fA-Fx]+)$");
		if (m.IsMatching())
		{
			utf8char_t lower = str2uch(m[1]);
			utf8char_t higher = str2uch(m[2]);
			if (lower > higher)
			{
				utf8char_t t = lower;
				lower = higher;
				higher = t;
			}

			if (higher > 255)
				utf8 = true;

			for (utf8char_t i = lower; i <= higher; i++)
				_seq.push_back(i);
		}
		else if (reu::IsMatching(str, "^([0-9a-fA-Fx]+)$"))
		{
			auto ch = str2uch(str);
			if (ch > 255)
				utf8 = true;
			_seq.push_back(str2uch(str));
		}
		else
			throw std::runtime_error("Font alphabet sequence has invalid format");
	}
}

int Font::GetHeight() const
{
	return _height;
}

int Font::GetWidth() const
{
	return _width;
}

Font Font::makeEmptyFont(int h, int w, int count, const std::string& seq)
{
	std::vector<pixel_t> emptyFontDict(h * w * count);
	for (auto& c : emptyFontDict)
		c = '0';
	return Font(emptyFontDict, h, w, 0, seq);
}

//Each pixel is a byte
bitmap_t Font::GetCharImage_8bit(utf8char_t ch, bool monospace) const
{
	auto adaptiveSpace = [](bitmap_t& ch)
	{
		int emptyFirst = 0, emptyLast = 0;
		bool br = false;
		for (size_t x = 0; x < ch[0].size(); x++)
		{
			for (size_t y = 0; y < ch.size(); y++)
			{
				if (ch[y][x] != 0)
				{
					br = true;
					break;
				}
			}
			if (br)
				break;

			emptyFirst++;
		}

		if (emptyFirst == ch[0].size()) //Allow full empty characters
			return;

		br = false;
		for (size_t x = ch[0].size() - 1; x < size_t(-1); x--)
		{
			for (size_t y = 0; y < ch.size(); y++)
			{
				if (ch[y][x] != 0)
				{
					br = true;
					break;
				}
			}

			if (br)
				break;

			emptyLast++;
		}

		for (size_t x = 0; x < emptyFirst; x++)
		{
			for (size_t y = 0; y < ch.size(); y++)
				ch[y].erase(ch[y].begin());
		}

		for (size_t x = 0; x < emptyLast; x++)
		{
			for (size_t y = 0; y < ch.size(); y++)
				ch[y].erase(ch[y].end()-1);
		}
	};

	bitmap_t letter;
	InitBitmap(letter, _height, _width);
	size_t letterSize = _width * _height;

	if (_seq.empty())
	{
		bool skip_retry = false;
	noseq_search:
		for (unsigned char c = 0; c < 256; c++)
		{
			if (c == (unsigned char)ch)
			{
				size_t q = 0;
				for (size_t y = 0; y < letter.size(); y++)
				{
					for (size_t x = 0; x < letter[y].size(); x++)
					{
						letter[y][x] = _dict[(c * letterSize) + q];
						q++;
					}
				}
				if (!monospace)
					adaptiveSpace(letter);
				return letter;
			}
		}

		if (!skip_retry)
		{
			skip_retry = true;
			ch = '?';
			goto noseq_search;
		}

		return {};
	}

	if (_utf8)
	{
		bool found = false;
		bool skip_retry = false;
	utf8_enum:
		for (size_t n = 0; n < _seq.size(); n++)
		{
			utf8char_t uc = _seq[n];
			if (uc == ch)
			{
				size_t q = 0;
				for (size_t y = 0; y < letter.size(); y++)
				{
					for (size_t x = 0; x < letter[y].size(); x++)
					{
						letter[y][x] = _dict[(n * letterSize) + q];
						q++;
					}
				}
				found = true;
			}
		}
		if (found)
		{
			if (!monospace)
				adaptiveSpace(letter);
			return letter;
		}


		if (!skip_retry)
		{
			ch = u8'?';
			skip_retry = true;
			goto utf8_enum;
		}

	}
	else
	{
		bool skip_retry = false;
	custom_seq_loop:
		for (size_t i = 0; i < _seq.size(); i++)
		{
			if (_seq[i] == (unsigned char)ch)
			{
				size_t q = 0;
				for (size_t y = 0; y < letter.size(); y++)
				{
					for (size_t x = 0; x < letter[y].size(); x++)
					{
						letter[y][x] = _dict[(i * letterSize) + q];
						q++;
					}
				}
				if (!monospace)
					adaptiveSpace(letter);
				return letter;
			}
		}

		if (!skip_retry)
		{
			ch = '?';
			skip_retry = true;
			goto custom_seq_loop;
		}
	}

	return {};
}

bitmap_t Font::getFontTable(int maxColumn) const
{
	bitmap_t font;
	int h, w, count = (int)CharCount();
	int rows = count / maxColumn;
	if (rows == 0)
		rows = 1;
	if (count > maxColumn && count - (rows * maxColumn) > 0)
		rows++;

	w = maxColumn * _width;
	h = rows * _height;
	w = w + maxColumn - 1;
	h = h + rows - 1;

	InitBitmap(font, h, w);
	int i = 0;
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < maxColumn; c++)
		{
			for (int y = 0; y < _height + 1; y++)
			{
				for (int x = 0; x < _width + 1; x++)
				{
					if (y == _height) //Do not draw when exceed font height, do grid draw below
						break;

					if (x < _width) //Draw character
					{
						font[r * _height + y + r][c * _width + x + c] = i < _dict.size() ? _dict[i] : 5; //Fill empty cells after all chars done
						i++;
					}
					else if (c != maxColumn - 1) //Draw horizontal grid lines
						font[r * _height + y + r][c * _width + x + c] = 3;
				}
			}

			if (r != rows - 1)
			{
				int y = _height;
				for (int x = 0; x < _width + 1; x++) //Draw vertical grid lines
				{
					if (c != maxColumn - 1 || x < _width)
						font[r * _height + y + r][c * _width + x + c] = 3;
				}
			}
		}
	}

	return font;
}

size_t Font::CharCount() const
{
	if (_seq.empty())
		return 256;
	else
		return _seq.size();
}

bitmap_t Font::operator[](utf8char_t ch) const
{
	return GetCharImage_8bit(ch);
}

std::vector<utf8char_t> Font::GetAllSupportedChars() const
{
	return _seq;
}

int Font::GetInterval() const
{
	return _interval;
}