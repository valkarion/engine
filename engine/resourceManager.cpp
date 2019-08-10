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
	   
	ImageInfo ii = LoadImage( path );

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

bool ResourceManager::loadMesh( const std::string& path, const std::string& objName,
	const std::string& materialPath )
{
	tinyobj::attrib_t					attribute;
	std::vector<tinyobj::shape_t>		shapes;
	std::vector<tinyobj::material_t>	materials;
	std::string warning, error;

	tinyobj::LoadObj( &attribute, &shapes, &materials, &warning, &error, path.c_str(),
		materialPath.c_str() );

	if ( !error.empty() )
	{
		PrintToOutputWindow( "Load Mesh Error: %s", error.c_str() );
		return false;
	}

	if ( !warning.empty() )
	{
		PrintToOutputWindow( "Load Mesh Warning: %s", warning.c_str() );
	}
	
	Mesh& mesh = meshes[objName];

	size_t indexOffset = 0;
	for ( const auto& shape : shapes )
	{
		for ( size_t fv = 0; fv < shape.mesh.num_face_vertices.size(); fv++ )
		{
			int vert = shape.mesh.num_face_vertices[fv];

			glm::vec4 faceVertexIndecies = { -1, -1, -1, -1 };
			for ( size_t v = 0; v < vert; v++ )
			{
				tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
				tinyobj::real_t vx = attribute.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attribute.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attribute.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t tx = attribute.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attribute.texcoords[2 * idx.texcoord_index + 1];
				
				Vertex vertex;
				vertex.position = { vx, vy, vz };
				vertex.textureCoordinates = { tx, 1.f - ty };
				vertex.color = { 1.f, 1.f, 1.f };

				mesh.vertecies.push_back( vertex );
				mesh.indicies.push_back( mesh.indicies.size() );
				mesh.trueIndecies.push_back( idx.vertex_index );
				faceVertexIndecies[v] = idx.vertex_index;
			}

			mesh.faces.push_back( faceVertexIndecies );
			
			indexOffset += vert;
		}

		MaterialRange matRange;
		int startIndex = -1;
		int lastId = -1;
		for ( size_t i = 0; i < shape.mesh.material_ids.size(); i++ )
		{
			int id = shape.mesh.material_ids[i];
			if ( id == -1 )
			{
				continue;
			}

			if ( id == lastId )
			{
				if ( startIndex == -1 )
				{
					matRange.matName = materials[id].name;
					startIndex = i;
				}

				matRange.range++;
			}
			else if ( lastId != -1 )
			{
				matRange.start = (size_t)startIndex;
				mesh.materialFaceIndexRanges.push_back( matRange );
				matRange = {};
				startIndex = -1;
			}
			
			lastId = id;
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