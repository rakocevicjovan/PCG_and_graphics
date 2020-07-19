#pragma once
#include "AssetLedger.h"
#include "FileUtilities.h"

class AssetScanner
{
public:

	bool scan(AssetLedger* ledger, std::string& dirPath)
	{
		std::filesystem::directory_entry dir(dirPath);
		
		if (!dir.is_directory())
			return false;

		std::string ext(".aeon");

		std::filesystem::recursive_directory_iterator rdi(dir);

		for (auto& entry : rdi)
		{
			if (entry.path().extension() == ext)
			{
				// Do something... not sure what.
			}
		}

		return true;
	}

};