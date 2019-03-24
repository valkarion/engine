#pragma once

#include <map>
#include <vector>
#include <memory>

struct Texture
{
	std::string				filename;
	uint32_t				width;
	uint32_t				height;	
	uint8_t					pixelDepth;
	std::vector<uint8_t>	colorData;
};

class ResourceManager
{
	static std::unique_ptr<ResourceManager> _instance;
public:
	std::map<std::string, Texture> textures;

	bool loadTexture( const std::string& path );
	void shutdown();

	static ResourceManager* instance();
};