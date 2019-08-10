#pragma once

#include <map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "vulkanVertex.hpp"

struct Image
{
	std::string				filename;
	uint32_t				width;
	uint32_t				height;	
	uint8_t					pixelDepth;
	std::vector<uint8_t>	colorData;
};

struct MaterialRange
{
	std::string matName = UNSET_S;
	// staring Mesh::faces index 
	size_t		start = 0;
	// number of Mesh::faces elements
	size_t		range = 0;
};

struct Mesh
{
	std::vector<Vertex>			vertecies;
	std::vector<uint32_t>		indicies;

	std::vector<glm::vec4>		faces;
	std::vector<MaterialRange>	materialFaceIndexRanges;
	std::vector<uint32_t>		trueIndecies;
};

class ResourceManager
{
	static std::unique_ptr<ResourceManager> _instance;
	
	std::map<std::string, Image>	images;
	std::map<std::string, Mesh>		meshes;
public:

	bool loadImage( const std::string& path, const std::string& imgName );
	bool loadMesh( const std::string& path, const std::string& objName, const std::string& materialPath = "" );

	const Image* getImage( const std::string& name ) const;
	const Mesh* getMesh( const std::string& name ) const;

	void shutdown();
	static ResourceManager* instance();
};