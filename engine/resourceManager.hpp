#pragma once

#include <map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "vulkanVertex.hpp"

// Holds pixel RGBA data that can directly be loaded into the renderer
struct Image
{
	std::string				filename;
	uint32_t				width;
	uint32_t				height;	
	uint8_t					pixelDepth;
	std::vector<uint8_t>	colorData;
};

// For multitexture objects this object will hold 
// what indecies/faces belong to one texture
struct MaterialRange
{
	std::string matName = UNSET_S;
	// staring Mesh::faces index 
	uint32_t	start = 0;
	// number of Mesh::faces elements
	uint32_t	nFaces = 0;
	// starting vertex index
	uint32_t	startIndex = 0;
	// sum number of vertex indecies
	uint32_t	range = 0;
};

struct MeshFace
{
	uint32_t x, y, z;

	uint32_t& operator []( const size_t index )
	{
		assert( index < 3 );

		switch ( index )
		{
			case 0: return x; break;
			case 1: return y; break;
			case 2: return z; break;
			default: 
				return x;
				break;
		}
	}
};

struct Mesh
{
// Use these for rending 
	std::vector<Vertex>			vertecies;
	std::vector<uint32_t>		indicies;
	std::vector<MaterialRange>	materialFaceIndexRanges;
	
// Use these for physics
	std::vector<MeshFace>		faces;
	std::vector<glm::vec3>		points;

	// bounding box range 
	glm::vec3 topLeftNear;
	glm::vec3 botRightFar;
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