#include "resourceManager.hpp"
#include "fileSystem.hpp"
#include "loggers.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

std::unique_ptr<ResourceManager> ResourceManager::_instance = std::make_unique<ResourceManager>();

ResourceManager* ResourceManager::instance()
{
	return _instance.get();
}

bool ResourceManager::loadImage( const std::string& path, const std::string& imgName )
{
	if( !CheckFileExists( path ) )
	{		
		WriteToErrorLog( "Failed to open file: " + path );
		return false;
	}
	   
	ImageInfo ii = LoadBMP32( path );

	Image& tex = images[imgName];
	tex.filename = path;
	tex.width = ii.width;
	tex.height = ii.height;
	tex.pixelDepth = 32; // fix in the engine 
	
	size_t size = ii.bytes.size();
	tex.colorData.resize( size );
	std::memcpy( tex.colorData.data(), ii.bytes.data(), 
		size * sizeof( uint8_t ) );

	return true;
}

bool ResourceManager::loadMesh( const std::string& path, const std::string& objName )
{
	tinyobj::attrib_t attribute;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	if ( !tinyobj::LoadObj( &attribute, &shapes, &materials,
		&warning, &error, path.c_str() ) )
	{
		WriteToErrorLog( "Failed to load model: %s. w: %s, e: %s",
			path.c_str(), warning.c_str(), error.c_str() );

		return false;
	}

	Mesh& mesh = meshes[objName];

	for ( auto& shape : shapes )
	{
		for ( auto& index : shape.mesh.indices )
		{
			Vertex v;

			v.position = {
				attribute.vertices[3 * index.vertex_index],
				attribute.vertices[3 * index.vertex_index + 1],
				attribute.vertices[3 * index.vertex_index + 2]
			};

			v.textureCoordinates = {
				attribute.texcoords[2 * index.texcoord_index],
				attribute.texcoords[2 * index.texcoord_index + 1]
			};

			v.color = { 1.f, 1.f, 1.f };

			mesh.vertecies.push_back( v );
			mesh.indicies.push_back( (uint32_t)mesh.indicies.size() );
		}
	}

	return true;
}

const Image* ResourceManager::getImage( const std::string& name ) const
{
	if ( images.count( name ) == 0 )
	{
		return nullptr;
	}

	return &images.at( name );
}

const Mesh* ResourceManager::getMesh( const std::string& name ) const
{
	if ( meshes.count( name ) == 0 )
	{
		return nullptr;
	}

	return &meshes.at( name );
}

void ResourceManager::shutdown()
{
	images.clear();
}