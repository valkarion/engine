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
	if( !FileSystem::CheckFileExists( path ) )
	{		
		Logger::WriteToErrorLog( "Failed to open file: " + path );
		return false;
	}
	   
	ImageInfo ii = FileSystem::LoadImage( path );

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
		Logger::PrintToOutputWindow( "Load Mesh Error: %s", error.c_str() );
		return false;
	}

	if ( !warning.empty() )
	{
		Logger::PrintToOutputWindow( "Load Mesh Warning: %s", warning.c_str() );
	}
	
	Mesh& mesh = meshes[objName];

	size_t indexOffset = 0;
	for ( const auto& shape : shapes )
	{
		for ( size_t fv = 0; fv < shape.mesh.num_face_vertices.size(); fv++ )
		{
			int vert = shape.mesh.num_face_vertices[fv];

			MeshFace faceVertexIndecies = { 0, 0, 0 };
			for ( size_t v = 0; v < vert; v++ )
			{
				tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
				tinyobj::real_t vx = attribute.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attribute.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attribute.vertices[3 * idx.vertex_index + 2];
				
				tinyobj::real_t tx = 0;
				tinyobj::real_t ty = 0;
				if ( idx.texcoord_index >= 0 )
				{
					tx = attribute.texcoords[2 * idx.texcoord_index + 0];
					ty = attribute.texcoords[2 * idx.texcoord_index + 1];
				}
				
			// calc bounding box vertecies 
				if ( mesh.topLeftNear.x > vx ) mesh.topLeftNear.x = vx;
				if ( mesh.botRightFar.x < vx ) mesh.botRightFar.x = vx;
				if ( mesh.topLeftNear.y > vy ) mesh.topLeftNear.y = vy;
				if ( mesh.botRightFar.y < vy ) mesh.botRightFar.y = vy;
				if ( mesh.topLeftNear.z > vz ) mesh.topLeftNear.z = vz;
				if ( mesh.botRightFar.z < vz ) mesh.botRightFar.z = vz;

				Vertex vertex;
				vertex.position = { vx, vy, vz };
			// Y texture coordinate needs to be flipped because .obj texture 0 coordinate is bottom not top
				vertex.textureCoordinates = { tx, 1.f - ty };
				vertex.color = { 1.f, 1.f, 1.f };

				mesh.vertecies.push_back( vertex );
				mesh.indicies.push_back( (uint32_t)mesh.indicies.size() );
// compiler throws a warning about conversion to glm's weird type casting, but it's okay here.
#pragma warning( push )
#pragma warning( disable: 4244 4267 )
				// generate the face 
				faceVertexIndecies[v] = idx.vertex_index;
#pragma warning( pop )
			}

			mesh.faces.push_back( faceVertexIndecies ); 

			indexOffset += vert;
		}

		for ( int i = 0; i < attribute.vertices.size(); i += 3 )
		{			
			glm::vec3 v;
			v.x = attribute.vertices[i];
			v.y = attribute.vertices[i + 1];
			v.z = attribute.vertices[i + 2];

			mesh.points.push_back( v );
		}
		
		if ( !materials.empty() )
		{
			const size_t nIds = shape.mesh.material_ids.size();
			if ( nIds > 0 )
			{
				MaterialRange matRange;
				int indexer = 0;
				int lastId = shape.mesh.material_ids[0];
				int nVerteciesCovered = 0;
				while ( indexer < nIds )
				{
					int id = shape.mesh.material_ids[indexer];
					if ( id == lastId )
					{
						matRange.matName = id == -1 ? "notexture" : materials[id].name;
						matRange.nFaces++;
						matRange.range += shape.mesh.num_face_vertices[indexer];
						nVerteciesCovered += shape.mesh.num_face_vertices[indexer];
					}
					else
					{
						mesh.materialFaceIndexRanges.push_back( matRange );
						matRange = {};
						matRange.startIndex = nVerteciesCovered;
						matRange.start = indexer;
						lastId = id;
						continue;
					}

					lastId = id;
					indexer++;
				}

				if ( matRange.range != 0 )
				{
					mesh.materialFaceIndexRanges.push_back( matRange );
				}
			}
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