#pragma once

class AssimpLoader;

// This class is the next logical step towards the engine.
// Use some of assimp loader functionality and allow a loader to be spawned from the scene editor through this.
// Among other things. This should be a high level overview of files (in the project or to be imported)

class AssetExplorer
{
public:

	AssimpLoader* _loaderLevel;

private:

};