#include "resourceManager.hpp"
#include "fileSystem.hpp"
#include "loggers.hpp"

std::unique_ptr<ResourceManager> ResourceManager::_instance = std::make_unique<ResourceManager>();

ResourceManager* ResourceManager::instance()
{
	return _instance.get();
}

bool ResourceManager::loadTexture( const std::string& path )
{
	bool valid = true;
	Texture tex;

	if( !CheckFileExists( path ) )
	{		
		WriteToErrorLog( "Failed to open file: " + path );
		return false;
	}



	return valid;
}

void ResourceManager::shutdown()
{
	textures.clear();
}