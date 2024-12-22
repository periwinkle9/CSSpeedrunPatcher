#include "doukutsu.h"
#include <fstream>
#include <stdexcept>
namespace fs = std::filesystem;

DoukutsuExe::DoukutsuExe(const fs::path& path): data{}
{
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs)
		throw std::runtime_error("File doesn't exist or is unreadable");
	auto fileSize = fs::file_size(path);
	data.resize(fileSize);
	if (!ifs.read(reinterpret_cast<char*>(data.data()), fileSize))
		throw std::runtime_error("Failed to read file");
	if (!verify())
		throw std::runtime_error("This file doesn't look like a valid Doukutsu.exe.\n"
			"Double-check that you started with an unmodified Japanese version of the game.");
}

void DoukutsuExe::write(const fs::path& path)
{
	fs::path writePath = path;
	if (fs::is_directory(path))
		writePath /= "Doukutsu.exe";

	std::ofstream ofs(writePath, std::ios::binary | std::ios::trunc);
	if (!ofs.write(reinterpret_cast<char*>(data.data()), data.size()))
		throw std::runtime_error("Couldn't write to file. Check that you have permission to write to the\n"
			"directory, and that the file isn't marked as read-only.");
}

void DoukutsuExe::patch(unsigned offset, std::initializer_list<byte> bytes)
{
	for (byte val : bytes)
		data.at(offset++) = val;
}
void DoukutsuExe::patch(unsigned offset, std::uint32_t value)
{
	for (int i = 0; i < 4; ++i)
	{
		data.at(offset++) = value & 0xFF;
		value >>= 8;
	}
}

bool DoukutsuExe::runWhenUnfocused() const
{
	return data[0x13593] == 0x71 && data[0x12BC0] == 0xC3 && data[0x12BF0] == 0xC3;
}
unsigned DoukutsuExe::windowMagnification() const
{
	unsigned mag = data[0xB4A6];
	if (mag == 1)
		mag = 0;
	return mag;
}
bool DoukutsuExe::autoTextAdvance() const
{
	// We don't actually need to check the whole patch, do we now?
	return data[0x16CA0] == 0xB8 && data[0x16CA1] == 0x09;
}

void DoukutsuExe::setRunWhenUnfocused(bool setting)
{
	if (setting)
	{
		data[0x13593] = 0x71;
		data[0x12BC0] = 0xC3;
		data[0x12BF0] = 0xC3;
	}
	else
	{
		// These are the original bytes
		data[0x13593] = 0x75;
		data[0x12BC0] = 0x55;
		data[0x12BF0] = 0x55;
	}
}
void DoukutsuExe::setWindowMagnification(unsigned mag)
{
	// Just in case
	if (mag > 127)
		mag = 127;

	if (mag > 1)
	{
		std::uint32_t windowWidth = 320 * mag;
		std::uint32_t windowHeight = 240 * mag;
		data[0xB4A6] = static_cast<byte>(mag);
		data[0xCD6B] = 0x10;
		patch(0xCD7C, {0x6B, 0xC8, 0x0A, 0x89, 0x4D, 0xF8, 0x6B, 0xD0, 0x05, 0x89, 0x55, 0xFC, 0x90, 0x90});
		patch(0x127C2, windowWidth);
		patch(0x127CC, windowHeight);
	}
	else
	{
		// Restore original bytes
		data[0xB4A6] = 1;
		data[0xCD6B] = 0x1E;
		patch(0xCD7C, {0xC7, 0x45, 0xF8, 0x14, 0x00, 0x00, 0x00, 0xC7, 0x45, 0xFC, 0x0A, 0x00, 0x00, 0x00});
		patch(0x127C2, 320);
		patch(0x127CC, 240);
	}
}
void DoukutsuExe::setAutoTextAdvance(bool setting)
{
	if (setting)
	{
		patch(0x16CA0, {0xB8, 0x09, 0x5B, 0x4A, 0x00, 0xFE, 0x00, 0x82, 0x38, 0x02, 0x7E, 0x0A,
						0xC6, 0x05, 0xDC, 0x5A, 0x4A, 0x00, 0x01, 0xC6, 0x00, 0x00, 0xC3});
		data[0x25584] = 0x10;
		patch(0x2558A, {0xE8, 0x11, 0x17, 0xFF, 0xFF, 0x90, 0x90});
	}
	else
	{
		// Restore original bytes
		patch(0x16CA0, {0x55, 0x8B, 0xEC, 0x0F, 0xB6, 0x05, 0x38, 0xE6, 0x49, 0x00, 0x83, 0xE0,
						0xFE, 0xA2, 0x38, 0xE6, 0x49, 0x00, 0x5D, 0xC3, 0xCC, 0xCC, 0xCC});
		data[0x25584] = 0x14;
		patch(0x2558A, {0xC6, 0x05, 0xDC, 0x5A, 0x4A, 0x00, 0x01});
	}
}

bool DoukutsuExe::verify() const
{
	// Check file size
	if (data.size() != 0x169000)
		return false;
	// Very inefficient way of producing a presumably-unmodified Doukutsu.exe, but I don't care :P
	DoukutsuExe vanilla{};
	vanilla.data = data;
	vanilla.setRunWhenUnfocused(false);
	vanilla.setWindowMagnification(0);
	vanilla.setAutoTextAdvance(false);

	// A basic checksum algorithm
	std::uint64_t sum1 = 0, sum2 = 0;
	for (unsigned i = 0; i + 3 < data.size(); i += 4)
	{
		std::uint32_t value = vanilla.data[i] |
			(vanilla.data[i + 1] << 8) |
			(vanilla.data[i + 2] << 16) |
			(vanilla.data[i + 3] << 24);
		sum1 = (sum1 + value) % 0xFFFFFFFFu;
		sum2 = (sum2 + sum1) % 0xFFFFFFFFu;
	}
	return ((sum2 << 32) | sum1) == 0x219339c977a8c081ull; // Checksum of unmodified vanilla Doukutsu.exe
}
