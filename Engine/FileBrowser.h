#pragma once

class FileBrowser
{
private:

	uint8_t _historySize = 8u;
	std::string _searchedString;
	std::filesystem::path _curDirPath;
	std::filesystem::path _projRootPath;	// QoL
	std::deque<std::filesystem::path> _pathHistory;
	std::vector<std::filesystem::directory_entry> _contents;
	bool _badPath{ false };

	void seek();
	void stepUp();
	bool stepBack();
	void openDir(const std::filesystem::path& newPath);
	std::optional<std::filesystem::directory_entry> printContentList();
	void refreshContentList();

public:

	FileBrowser() = default;
	FileBrowser(const std::string& rootFolder);
	std::optional<std::filesystem::directory_entry> display();
};