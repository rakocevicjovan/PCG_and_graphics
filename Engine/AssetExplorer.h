#pragma once

#include "AssimpLevel.h"
#include "AssetLedger.h"

// This class is the next logical step towards the engine.
// Use some of assimp loader functionality and allow a loader to be spawned from the scene editor through this.
// Among other things. This should be a high level overview of files (in the project or to be imported)

class AssetExplorer
{
private:

	AssimpLevel* _loaderLevel{};
	AssetLedger* _pLedger{};

public:

	void init(AssetLedger* ledger)
	{
		_pLedger = ledger;
	}



};