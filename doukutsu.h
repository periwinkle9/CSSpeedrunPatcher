#pragma once

#include <vector>
#include <cstdint>
#include <initializer_list>
#include <filesystem>

class DoukutsuExe
{
	using byte = unsigned char;
	std::vector<byte> data;
	bool verify() const;
public:
	DoukutsuExe() = default;
	DoukutsuExe(const std::filesystem::path& path);

	// Writes the file to disk
	void write(const std::filesystem::path& path);

	// Generic patching functions
	void patch(unsigned offset, std::initializer_list<byte> bytes);
	void patch(unsigned offset, std::uint32_t value);

	// Retrieves current state of these patches
	bool runWhenUnfocused() const;
	unsigned windowMagnification() const;
	bool autoTextAdvance() const;

	// Set/undo these patches
	void setRunWhenUnfocused(bool setting);
	void setWindowMagnification(unsigned mag); // 0 = unpatch
	void setAutoTextAdvance(bool setting);
};
