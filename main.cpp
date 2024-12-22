#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#endif
#include "doukutsu.h"
namespace fs = std::filesystem;

int getInput(const char* prompt, int min, int max);
fs::path getDoukutsuPath();

int main(int argc, char* argv[])
{
	fs::path doukutsuPath;
	if (argc > 1)
		doukutsuPath = argv[1];
	else
		doukutsuPath = getDoukutsuPath();

	if (doukutsuPath.empty())
		return 0;

	std::cout << "Selected file: " << doukutsuPath.string() << '\n';
	try
	{
		DoukutsuExe douExe(doukutsuPath);
		bool runUnfocused = douExe.runWhenUnfocused();
		unsigned windowMag = douExe.windowMagnification();
		bool autoText = douExe.autoTextAdvance();
		if (runUnfocused)
			std::cout << "Detected run-when-unfocused patch\n";
		if (windowMag > 0)
			std::cout << "Detected window magnification patch: " << windowMag << "x ("
			          << 320*windowMag << 'x' << 240*windowMag << ")\n";
		if (autoText)
			std::cout << "Detected auto text advance patch\n";

		int choice;
		bool isEdited = false;
		do
		{
			std::cout << "What do you want to do?\n";
			std::cout << "1 - " << (runUnfocused ? "Undo" : "Apply") << " run-when-unfocused patch\n";
			std::cout << "2 - " << (windowMag > 0 ? "Adjust or undo" : "Apply") << " window magnification patch\n";
			std::cout << "3 - " << (autoText ? "Undo" : "Apply") << " auto text advance patch\n";
			std::cout << "4 - Save & Quit\n";
			choice = getInput("Enter input: ", 1, 4);
			std::cout << '\n';

			switch (choice)
			{
			case 1:
				runUnfocused = !runUnfocused;
				std::cout << (runUnfocused ? "Applying" : "Undoing") << " run-when-unfocused patch\n\n";
				douExe.setRunWhenUnfocused(runUnfocused);
				isEdited = true;
				break;
			case 2:
				std::cout << "Input your desired window size magnification scale.\n"
				             "This will scale the 320x240 DoConfig option by that factor.\n"
							 "For example: Enter 3 for 960x720, 4 for 1280x960, etc.\n"
							 "Enter 0 to undo the patch entirely.\n";
				windowMag = getInput("Enter magnification scale: ", 0, 127);
				if (windowMag > 1)
					std::cout << "\nSetting window size to " << 320 * windowMag << 'x' << 240 * windowMag << "\n\n";
				else
				{
					windowMag = 0;
					std::cout << "\nUndoing window magnification patch\n\n";
				}
				if (windowMag != douExe.windowMagnification())
				{
					douExe.setWindowMagnification(windowMag);
					isEdited = true;
				}
				break;
			case 3:
				autoText = !autoText;
				std::cout << (autoText ? "Applying" : "Undoing") << " auto text advance patch\n\n";
				douExe.setAutoTextAdvance(autoText);
				isEdited = true;
				break;
			default:
				break;
			}
		} while (choice != 4);

		if (isEdited)
		{
			// Back up original file
			fs::path backupPath = doukutsuPath;
			backupPath += ".bak";
			fs::rename(doukutsuPath, backupPath);

			douExe.write(doukutsuPath);
			std::cout << "All done! A backup of your old exe has been saved to: " << backupPath.string();
		}
	}
	catch (const std::runtime_error& e)
	{
		std::clog << "Error: " << e.what();
	}
	// Just a quick & dirty hack to keep the console window open so you can read the last lines of output
	std::cout << "\n\nPress Enter to quit...";
	std::cin.get();
}

int getInput(const char* prompt, int min, int max)
{
	const char* ErrorMsg = "That's not a valid input.\n";
	int choice;
	while (true)
	{
		try
		{
			std::cout << prompt;
			std::string input;
			std::getline(std::cin >> std::ws, input);
			choice = std::stoi(input);
			if (choice >= min && choice <= max)
				break;
			std::cout << ErrorMsg;
		}
		catch (const std::invalid_argument&)
		{
			std::cout << ErrorMsg;
		}
		std::cin.clear();
	}
	return choice;
}

#ifdef _WIN32
fs::path getDoukutsuPath()
{
	char openFile[260] = "Doukutsu.exe";
	OPENFILENAME ofn{};
	ofn.lStructSize = sizeof ofn;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = openFile;
	ofn.nMaxFile = sizeof openFile;
	ofn.lpstrTitle = "Select Doukutsu.exe to patch";
	ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 9;
	ofn.lpstrDefExt = "exe";

	if (GetOpenFileNameA(&ofn) != 0)
		return fs::path{ofn.lpstrFile};
	else
		return fs::path{};
}
#else
fs::path getDoukutsuPath()
{
	std::cout << "Enter path to Doukutsu.exe: ";
	std::string path;
	std::getline(std::cin, path);
	return fs::path{path};
}
#endif
