//
//  Model.cpp
//  AmApAR
//
//  Created by Johannes Unterguggenberger on 10/17/11.
//  Copyright (c) 2011 none. All rights reserved.
//

#include "Model.h"
#include <sstream>

#include "TexLoadingHelper.h"
#include "MaterialData.h"
#include "VertexAttribLocation.h"

namespace cgb
{
	const char* Model::kIndent = "    ";
	const glm::vec4 Model::kDefaultDiffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	const glm::vec4 Model::kDefaultSpecularColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec3 Model::kDefaultAmbientColor = glm::vec3(1.0f);
	const glm::vec3 Model::kDefaultEmissiveColor = glm::vec3(0.0f, 0.0f, 0.0f);
	const glm::vec4 Model::kDefaultTransparentColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	const float Model::kDefaultSpecularPower = 0.0f;
	const float Model::kDefaultOpacity = 1.0f;
	const float Model::kDefaultBumpScaling = 1.0f;
	const float Model::kDefaultRefraction = 0.0f;
	const float Model::kDefaultReflectivity = 1.0f;

	Model::Model(std::shared_ptr<cgb::vulkan_resource_bundle_group> resourceBundleGroup, const glm::mat4& loadTransMatrix)
		: m_meshes(0),
		m_load_transformation_matrix(loadTransMatrix),
		mResourceBundleGroup(resourceBundleGroup)
	{
	}

	Model::~Model()
	{
	}

	Model::Model(Model&& other) noexcept :
		m_meshes(std::move(other.m_meshes)),
		m_load_transformation_matrix(std::move(other.m_load_transformation_matrix))
	{
	}

	Model& Model::operator=(Model&& other) noexcept
	{
		m_meshes = std::move(other.m_meshes);
		m_load_transformation_matrix = std::move(other.m_load_transformation_matrix);

		return *this;
	}

	std::unique_ptr<Model> Model::LoadFromFile(const std::string& path, const glm::mat4& transform_matrix, std::shared_ptr<cgb::vulkan_resource_bundle_group> resourceBundleGroup, const unsigned int model_loader_flags)
	{
		std::unique_ptr<Model> model = std::make_unique<Model>(resourceBundleGroup, transform_matrix);
		if (!model->LoadFromFile(path, model_loader_flags))
		{
			// smartpointer will be deleted automatically when it goes out of scope
			return std::unique_ptr<Model>(nullptr);
		}
		return model;
	}

	std::unique_ptr<Model> Model::LoadFromMemory(const std::string& memory, const glm::mat4& transform_matrix, const unsigned int model_loader_flags)
	{
		std::unique_ptr<Model> model = std::make_unique<Model>(nullptr, transform_matrix);
		if (!model->LoadFromMemory(memory, model_loader_flags))
		{
			// smartpointer will be deleted automatically when it goes out of scope
			return std::unique_ptr<Model>(nullptr);
		}
		return model;
	}

	unsigned int Model::CompileAssimpImportFlags(const unsigned int modelLoaderFlags)
	{
		unsigned int flags_for_assimp_importer = 0;
		// process importer-flags
		if (modelLoaderFlags & MOLF_flipUVs)
			flags_for_assimp_importer |= aiProcess_FlipUVs;
		if ((modelLoaderFlags & MOLF_smoothNormals) && (modelLoaderFlags & MOLF_faceNormals))
			LOG_WARNING("MOLF_smoothNormals and MOLF_faceNormals are mutually exclusive, ignoring MOLF_faceNormals");
		if (modelLoaderFlags & MOLF_smoothNormals)
			flags_for_assimp_importer |= aiProcess_GenSmoothNormals;
		else if (modelLoaderFlags & MOLF_faceNormals)
			flags_for_assimp_importer |= aiProcess_GenNormals;
		if (modelLoaderFlags & MOLF_triangulate)
			flags_for_assimp_importer |= aiProcess_Triangulate;
		// aiProcess_LimitBoneWeights ... limits the number of bone weights to assimp's default value of 4
		if (modelLoaderFlags & MOLF_limitBoneWeights)
			flags_for_assimp_importer |= aiProcess_LimitBoneWeights;
		if (modelLoaderFlags & MOLF_calcTangentSpace)
			flags_for_assimp_importer |= aiProcess_CalcTangentSpace;

		flags_for_assimp_importer |= aiProcess_MakeLeftHanded | aiProcess_FlipUVs | aiProcess_FlipWindingOrder;
		return flags_for_assimp_importer;
	}


	bool Model::LoadFromFile(const std::string& path, const unsigned int modelLoaderFlags)
	{
		// Create an importer and load from file (only this overload can load additional textures from the file system)
		Assimp::Importer importer;
		const auto assimp_importer_flags = CompileAssimpImportFlags(modelLoaderFlags);
		const aiScene* scene = importer.ReadFile(path.c_str(), assimp_importer_flags);
		return PostLoadProcessing(importer, scene, &path);
	}

	bool Model::LoadFromMemory(const std::string& data, const unsigned int modelLoaderFlags)
	{
		// Release the previously loaded mesh (if it exists)
		// Create an importer and load from file (can't load additional textures in this case)
		Assimp::Importer importer;
		const auto assimp_importer_flags = CompileAssimpImportFlags(modelLoaderFlags);
		const aiScene* scene = importer.ReadFileFromMemory(data.c_str(), data.size(), assimp_importer_flags);
		return PostLoadProcessing(importer, scene, nullptr);
	}

	bool Model::PostLoadProcessing(Assimp::Importer& importer, const aiScene* scene, const std::string* file_path_or_null)
	{
		if (scene)
		{
			auto retval = InitScene(scene);
			if (file_path_or_null != nullptr) {
				GatherMaterialData(scene, *file_path_or_null);
			}
			return retval;
		}
		else
		{
			throw std::runtime_error("Failed parsing mesh");
		}
	}

	void Model::GatherMaterialData(const aiScene* scene, const std::string& model_path)
	{
		TexLoadingHelper tlh(model_path);
		const auto n = num_meshes();
		for (unsigned int i = 0; i < n; i++)
		{
			auto* ai_mat = GetAssimpMaterialPtr(scene, i);
			m_meshes[i].m_material_data = std::make_shared<MaterialData>(ai_mat, tlh);
			m_meshes[i].mMaterialResourceBundle = m_meshes[i].m_material_data->create_resource_bundle(mResourceBundleGroup);
		}
	}

	void Model::AllocateMaterialData()
	{
		const auto n = num_meshes();
		for (unsigned int i = 0; i < n; i++)
		{
			mResourceBundleGroup->allocate_resource_bundle(m_meshes[i].mMaterialResourceBundle.get());
		}
	}

	bool Model::InitScene(const aiScene* scene)
	{
		m_meshes.resize(scene->mNumMeshes);

		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			// initialize mesh
			const aiMesh* paiMesh = scene->mMeshes[i];
			if (!InitMesh(i, paiMesh))
			{
				LOG_ERROR(fmt::format("Initializing mesh[{}] failed in Model::InitScene", i));
				return false;
			}
		}

		InitTransformationMatrices(scene->mRootNode, aiMatrix4x4());

		return true;
	}


	bool Model::InitMesh(const int index, const aiMesh* paiMesh)
	{
		if (!(paiMesh->HasPositions() && paiMesh->HasNormals()))
		{
			LOG_ERROR(fmt::format("A submesh is missing required vertex data! HasPositions[{}] && HasNormals[{}]",
				paiMesh->HasPositions() ? "true" : "false",
				paiMesh->HasNormals() ? "true" : "false"));
			return false;
		}

		m_meshes[index].m_index = index;
		m_meshes[index].m_name = paiMesh->mName.data;

		// calculate the size of the per-vertex memory and the strides
		// positions and normals are always available
		size_t sizeOneVtx = 0;

		size_t positionOffset = 0,
			normalOffset = 0,
			texCoordsOffset = 0,
			colorOffset = 0,
			boneIndicesOffset = 0,
			boneWeightsOffset = 0,
			tangentOffset = 0,
			bitangentOffset = 0;

		const size_t positionSize = sizeof(aiVector3D),
			normalSize = sizeof(aiVector3D),
			texCoordsSize = sizeof(aiVector2D),
			colorSize = sizeof(aiColor4D),
			boneIndicesSize = sizeof(glm::uvec4),
			boneWeightsSize = sizeof(glm::vec4),
			tangentSize = sizeof(glm::vec3),
			bitangentSize = sizeof(glm::vec3);

		sizeOneVtx = positionSize + normalSize; // positions and normals
		positionOffset = 0;
		m_meshes[index].m_vertex_data_layout = VertexAttribData::Position | VertexAttribData::Normal;
		normalOffset = positionSize;
		bitangentOffset = tangentOffset = boneWeightsOffset = boneIndicesOffset = colorOffset = texCoordsOffset = (positionSize + normalSize);
		if (paiMesh->HasTextureCoords(0))
		{
			sizeOneVtx += texCoordsSize;
			// TODO: add support for 3D-Texture coordinates
			boneIndicesOffset += texCoordsSize;
			boneWeightsOffset += texCoordsSize;
			tangentOffset += texCoordsSize;
			bitangentOffset += texCoordsSize;
			m_meshes[index].m_vertex_data_layout |= VertexAttribData::Tex2D;
		}
		if (paiMesh->HasVertexColors(0))
		{
			sizeOneVtx += colorSize;
			boneIndicesOffset += colorSize;
			boneWeightsOffset += colorSize;
			tangentOffset += colorSize;
			bitangentOffset += colorSize;
			m_meshes[index].m_vertex_data_layout |= VertexAttribData::Color;
		}
		if (paiMesh->HasBones())
		{
			sizeOneVtx += boneIndicesSize + boneWeightsSize;
			boneWeightsOffset += boneIndicesSize;
			tangentOffset += (boneIndicesSize + boneWeightsSize);
			bitangentOffset += (boneIndicesSize + boneWeightsSize);
			m_meshes[index].m_vertex_data_layout |= VertexAttribData::Bones;
			assert(sizeOneVtx == boneWeightsOffset + boneWeightsSize);
		}
		if (paiMesh->HasTangentsAndBitangents())
		{
			sizeOneVtx += tangentSize + bitangentSize;
			bitangentOffset += tangentSize;
			m_meshes[index].m_vertex_data_layout |= VertexAttribData::Tangents | VertexAttribData::Bitangents;
			assert(sizeOneVtx == bitangentOffset + bitangentSize);
		}


		// alloc the temporary storage and FILL THE MEMORY
		size_t bufferSize = paiMesh->mNumVertices * sizeOneVtx;
		m_meshes[index].m_vertex_data.resize(bufferSize);
		auto* vertexData = &m_meshes[index].m_vertex_data[0];
		for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
		{
			memcpy(&vertexData[i * sizeOneVtx + positionOffset], &paiMesh->mVertices[i].x, positionSize);
			memcpy(&vertexData[i * sizeOneVtx + normalOffset], &paiMesh->mNormals[i].x, normalSize);
			if (paiMesh->HasTextureCoords(0))
			{
				memcpy(&vertexData[i * sizeOneVtx + texCoordsOffset], &paiMesh->mTextureCoords[0][i].x, texCoordsSize);
			}
			if (paiMesh->HasVertexColors(0))
			{
				memcpy(&vertexData[i * sizeOneVtx + colorOffset], &paiMesh->mColors[0][i].r, colorSize);
			}
			if (paiMesh->HasTangentsAndBitangents())
			{
				memcpy(&vertexData[i * sizeOneVtx + tangentOffset], &paiMesh->mTangents[i].x, tangentSize);
				memcpy(&vertexData[i * sizeOneVtx + bitangentOffset], &paiMesh->mBitangents[i].x, bitangentSize);
			}
		}

		if (paiMesh->HasBones())
		{
			//read bone indices and weights for bone animation
			std::vector<aiVertexWeight>* vTempWeightsPerVertex = new std::vector<aiVertexWeight>[paiMesh->mNumVertices];
			for (unsigned int j = 0; j < paiMesh->mNumBones; j++)
			{
				const aiBone * pBone = paiMesh->mBones[j];
				for (unsigned int b = 0; b < pBone->mNumWeights; b++)
				{
					vTempWeightsPerVertex[pBone->mWeights[b].mVertexId].push_back(aiVertexWeight(j, pBone->mWeights[b].mWeight));
				}
			}

			// set the bones for each vertex, if there are any
			for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
			{
				glm::uvec4 boneIndices(0, 0, 0, 0);
				glm::vec4 boneWeights(0.0f, 0.0f, 0.0f, 0.0f);

				if (vTempWeightsPerVertex[j].size() > 4)
				{
					LOG_ERROR("The model has invalid bone weights and is not loaded.");
					return false;
				}

				// vTempWeightsPerVertex[j].size() is <= 3
				for (unsigned int k = 0; k < vTempWeightsPerVertex[j].size(); k++)
				{
					// write into glm::uvec4 and glm::vec4 via []-operator
					// NOTE: the indices are cast to GLuint here, the weights to GLfloat!
					boneIndices[k] = vTempWeightsPerVertex[j][k].mVertexId;
					boneWeights[k] = vTempWeightsPerVertex[j][k].mWeight;
				}

				memcpy(&vertexData[j * sizeOneVtx + boneIndicesOffset], &boneIndices[0], boneIndicesSize);
				memcpy(&vertexData[j * sizeOneVtx + boneWeightsOffset], &boneWeights[0], boneWeightsSize);
			}

			delete[] vTempWeightsPerVertex;
			vTempWeightsPerVertex = 0;
		}

		m_meshes[index].m_size_one_vertex = sizeOneVtx;

		m_meshes[index].m_position_offset = positionOffset;
		m_meshes[index].m_normal_offset = normalOffset;
		m_meshes[index].m_tex_coords_offset = texCoordsOffset;
		m_meshes[index].m_color_offset = colorOffset;
		m_meshes[index].m_bone_incides_offset = boneIndicesOffset;
		m_meshes[index].m_bone_weights_offset = boneWeightsOffset;
		m_meshes[index].m_tangent_offset = tangentOffset;
		m_meshes[index].m_bitangent_offset = bitangentOffset;

		m_meshes[index].m_position_size = positionSize;
		m_meshes[index].m_normal_size = normalSize;
		m_meshes[index].m_tex_coords_size = texCoordsSize;
		m_meshes[index].m_color_size = colorSize;
		m_meshes[index].m_bone_indices_size = boneIndicesSize;
		m_meshes[index].m_bone_weights_size = boneWeightsSize;
		m_meshes[index].m_tangent_size = tangentSize;
		m_meshes[index].m_bitangent_size = bitangentSize;

		
		// store the indices in a vector
		size_t indicesCount = paiMesh->mNumFaces * kNumFaceVertices;
		m_meshes[index].m_indices.reserve(indicesCount);
		for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
		{
			// we're working with triangulated meshes only
			const aiFace& Face = paiMesh->mFaces[i];
			m_meshes[index].m_indices.push_back(Face.mIndices[0]);
			m_meshes[index].m_indices.push_back(Face.mIndices[1]);
			m_meshes[index].m_indices.push_back(Face.mIndices[2]);
		}

		m_meshes[index].m_patch_size = 3;

		return true;
	}

	glm::vec3 Mesh::vertex_position_at(size_t index) const
	{
		auto* ptr = &m_vertex_data.at(index * m_size_one_vertex + m_position_offset);
		return *reinterpret_cast<const glm::vec3*>(ptr);
	}

	glm::vec3 Mesh::vertex_normal_at(size_t index) const
	{
		auto* ptr = &m_vertex_data.at(index * m_size_one_vertex + m_normal_offset);
		return *reinterpret_cast<const glm::vec3*>(ptr);
	}

	GLuint Mesh::index_at(size_t index) const
	{
		return m_indices.at(index);
	}

	void Mesh::SetVertexData(
		std::vector<uint8_t>&& vertex_data,
		VertexAttribData vertex_data_layout,
		size_t position_offset,
		size_t normal_offset,
		size_t tex_coords_offset,
		size_t color_offset,
		size_t bone_incides_offset,
		size_t bone_weights_offset,
		size_t tangent_offset,
		size_t bitangent_offset,
		size_t position_size,
		size_t normal_size,
		size_t tex_coords_size,
		size_t color_size,
		size_t bone_indices_size,
		size_t bone_weights_size,
		size_t tangent_size,
		size_t bitangent_size)
	{
		m_vertex_data		  = std::move(vertex_data);
		m_vertex_data_layout  = vertex_data_layout;
		m_position_offset	  = position_offset;
		m_normal_offset		  = normal_offset;
		m_tex_coords_offset	  = tex_coords_offset;
		m_color_offset		  = color_offset;
		m_bone_incides_offset = bone_incides_offset;
		m_bone_weights_offset = bone_weights_offset;
		m_tangent_offset	  = tangent_offset;
		m_bitangent_offset	  = bitangent_offset;
		m_position_size		  = position_size;
		m_normal_size		  = normal_size;
		m_tex_coords_size	  = tex_coords_size;
		m_color_size		  = color_size;
		m_bone_indices_size	  = bone_indices_size;
		m_bone_weights_size	  = bone_weights_size;
		m_tangent_size		  = tangent_size;
		m_bitangent_size	  = bitangent_size;
	}

	void Mesh::SetVertexData(
		const std::vector<uint8_t>& vertex_data,
		VertexAttribData vertex_data_layout,
		size_t position_offset,
		size_t normal_offset,
		size_t tex_coords_offset,
		size_t color_offset,
		size_t bone_incides_offset,
		size_t bone_weights_offset,
		size_t tangent_offset,
		size_t bitangent_offset,
		size_t position_size,
		size_t normal_size,
		size_t tex_coords_size,
		size_t color_size,
		size_t bone_indices_size,
		size_t bone_weights_size,
		size_t tangent_size,
		size_t bitangent_size)
	{
		m_vertex_data = vertex_data;
		m_vertex_data_layout = vertex_data_layout;
		m_position_offset = position_offset;
		m_normal_offset = normal_offset;
		m_tex_coords_offset = tex_coords_offset;
		m_color_offset = color_offset;
		m_bone_incides_offset = bone_incides_offset;
		m_bone_weights_offset = bone_weights_offset;
		m_tangent_offset = tangent_offset;
		m_bitangent_offset = bitangent_offset;
		m_position_size = position_size;
		m_normal_size = normal_size;
		m_tex_coords_size = tex_coords_size;
		m_color_size = color_size;
		m_bone_indices_size = bone_indices_size;
		m_bone_weights_size = bone_weights_size;
		m_tangent_size = tangent_size;
		m_bitangent_size = bitangent_size;
	}

	void Mesh::SetIndices(std::vector<GLuint>&& indices, int patch_size)
	{
		m_indices = std::move(indices);
		m_patch_size = patch_size;
	}

	void Mesh::SetIndices(const std::vector<GLuint>& indices, int patch_size)
	{
		m_indices = indices;
		m_patch_size = patch_size;
	}

	vulkan_attribute_description_binding Mesh::GetOrCreateForVertexAttribConfig(VertexAttribData vertexDataConfig)
	{
		// ensure that this config is not already stored
		auto it = m_vertex_array_objects.find(vertexDataConfig);
		if (it != m_vertex_array_objects.end())
		{
			return it->second;
		}

		auto binding = vulkan_attribute_description_binding(1, m_size_one_vertex, vk::VertexInputRate::eVertex);


		// Position
		if (!((vertexDataConfig & VertexAttribData::Position) == VertexAttribData::Position))
		{
			LOG_WARNING("VertexData_Position not specified - enabling it anyways!");
		}
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::Position), vk::Format::eR32G32B32Sfloat, m_position_offset);
		}

		// Normal
		if ((vertexDataConfig & VertexAttribData::Normal) == VertexAttribData::Normal)
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::Normal), vk::Format::eR32G32B32Sfloat, m_normal_offset);
		}

		// TexCoord
		if ((vertexDataConfig & VertexAttribData::Tex2D) == VertexAttribData::Tex2D)
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::TexCoord), vk::Format::eR32G32Sfloat, m_tex_coords_offset);
		}

		// Color
		if ((vertexDataConfig & VertexAttribData::Color) == VertexAttribData::Color)
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::Color), vk::Format::eR32G32B32A32Sfloat, m_color_offset);
		}

		// Bones
		if ((vertexDataConfig & VertexAttribData::Bones) == VertexAttribData::Bones)
		{
			// todo validate format
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::BoneIndices), vk::Format::eR32G32B32A32Sfloat, m_bone_incides_offset);
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::BoneWeights), vk::Format::eR32G32B32A32Sfloat, m_bone_weights_offset);
		}

		// Tangent Space: Tangents
		if ((vertexDataConfig & VertexAttribData::Tangents) == VertexAttribData::Tangents)
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::Tangents), vk::Format::eR32G32B32Sfloat, m_tangent_offset);
		}

		// Tangent Space: Tangents
		if ((vertexDataConfig & VertexAttribData::Bitangents) == VertexAttribData::Bitangents)
		{
			binding.add_attribute_description(static_cast<uint_fast32_t>(VertexAttribLocation::Bitangents), vk::Format::eR32G32B32Sfloat, m_bitangent_offset);
		}

		m_vertex_array_objects.insert(std::pair<VertexAttribData, vulkan_attribute_description_binding>(vertexDataConfig, binding));

		return binding;
	}


	void Model::InitTransformationMatrices(const aiNode* pNode, const aiMatrix4x4& accTrans)
	{
		for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
		{
			aiMatrix4x4 finalAccTranse = accTrans * pNode->mTransformation;
			m_meshes[pNode->mMeshes[i]].m_scene_transformation_matrix = glm::transpose(glm::make_mat4(&finalAccTranse.a1));
		}

		for (unsigned int i = 0; i < pNode->mNumChildren; i++)
		{
			InitTransformationMatrices(pNode->mChildren[i], accTrans * pNode->mTransformation);
		}
	}



	void Model::PrintIndent(std::ostream& stream, int indent)
	{
		for (int i = 0; i < indent; i++)
			stream << kIndent;
	}

	void Model::PrintMatrix(std::ostream& stream, const aiMatrix4x4& mat, int indent)
	{
		PrintIndent(stream, indent);
		stream << std::setiosflags(std::ios::fixed) << std::setiosflags(std::ios::showpoint) << std::setprecision(2) << mat.a1 << "\t" << mat.a2 << "\t" << mat.a3 << "\t" << mat.a4 << std::endl;
		PrintIndent(stream, indent);
		stream << mat.b1 << "\t" << mat.b2 << "\t" << mat.b3 << "\t" << mat.b4 << std::endl;
		PrintIndent(stream, indent);
		stream << mat.c1 << "\t" << mat.c2 << "\t" << mat.c3 << "\t" << mat.c4 << std::endl;
		PrintIndent(stream, indent);
		stream << mat.d1 << "\t" << mat.d2 << "\t" << mat.d3 << "\t" << mat.d4 << std::endl << std::resetiosflags(std::ios::fixed);
	}

	void Model::PrintMesh(const aiScene* scene, std::ostream& stream, unsigned int meshIndex, int indent)
	{
		PrintIndent(stream, indent);
		stream << "mesh at index[" << meshIndex << "] named[" << GetMeshName(scene, meshIndex) << "]" << std::endl;
	}

	void Model::PrintNodeRecursively(const aiScene* scene, std::ostream& stream, const aiNode* pNode, const aiMatrix4x4 accTranse, int indent)
	{
		PrintIndent(stream, indent);
		stream << std::endl << std::endl << "NODE at level[" << indent << "], name[" << pNode->mName.data << "]" << std::endl;

		indent += 1;

		PrintIndent(stream, indent);
		stream << "transformation matrix {" << std::endl;
		PrintMatrix(stream, pNode->mTransformation, indent + 1);
		PrintIndent(stream, indent);
		stream << "}" << std::endl;

		PrintIndent(stream, indent);
		stream << "accumulated transformation matrix {" << std::endl;
		PrintMatrix(stream, accTranse, indent + 1);
		PrintIndent(stream, indent);
		stream << "}" << std::endl;

		PrintIndent(stream, indent);
		stream << "meshes count[" << pNode->mNumMeshes << "] {" << std::endl;
		for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
		{
			PrintMesh(scene, stream, pNode->mMeshes[i], indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;

		PrintIndent(stream, indent);
		stream << "children count[" << pNode->mNumChildren << "] {" << std::endl;
		for (unsigned int i = 0; i < pNode->mNumChildren; i++)
		{
			PrintNodeRecursively(scene, stream, pNode->mChildren[i], accTranse * pNode->mTransformation, indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;
	}

	void Model::PrintNodeTree(const aiScene* scene, std::ostream& stream)
	{
		PrintNodeRecursively(scene, stream, scene->mRootNode, aiMatrix4x4(), 0);
	}


	void Model::PrintVector3D(std::ostream& stream, const aiVector3D& vec)
	{
		stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}

	void Model::PrintQuaternion(std::ostream& stream, const aiQuaternion& quat)
	{
		stream << "(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")";
	}

	void Model::PrintVectorKey(std::ostream& stream, const aiVectorKey& key, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiVectorKey time[" << key.mTime << "] value vec3";
		PrintVector3D(stream, key.mValue);
		stream << std::endl;
	}

	void Model::PrintRotationKey(std::ostream& stream, const aiQuatKey& key, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiQuatKey time[" << key.mTime << "] value quat";
		PrintQuaternion(stream, key.mValue);
		stream << std::endl;
	}

	void Model::PrintAnimationChannel(std::ostream& stream, const aiNodeAnim* channel, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiNodeAnim nodeName[" << channel->mNodeName.data << "], preState[" << channel->mPreState << "], postState[" << channel->mPostState << "]" << std::endl;

		indent += 1;

		PrintIndent(stream, indent);
		stream << "position keys count[" << channel->mNumPositionKeys << "] {" << std::endl;
		for (unsigned int i = 0; i < channel->mNumPositionKeys; i++)
		{
			PrintVectorKey(stream, channel->mPositionKeys[i], indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;

		PrintIndent(stream, indent);
		stream << "rotation keys count[" << channel->mNumRotationKeys << "] {" << std::endl;
		for (unsigned int i = 0; i < channel->mNumRotationKeys; i++)
		{
			PrintRotationKey(stream, channel->mRotationKeys[i], indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;

		PrintIndent(stream, indent);
		stream << "scaling keys count[" << channel->mNumScalingKeys << "] {" << std::endl;
		for (unsigned int i = 0; i < channel->mNumScalingKeys; i++)
		{
			PrintVectorKey(stream, channel->mScalingKeys[i], indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;
	}

	void Model::PrintMeshKey(std::ostream& stream, const aiMeshKey& key, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiMeshKey time[" << key.mTime << "], aiMesh::mAnimMeshes index[" << key.mValue << "]" << std::endl;
	}

	void Model::PrintMeshChannel(std::ostream& stream, const aiMeshAnim* channel, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiMeshAnim name[" << channel->mName.data << "]" << std::endl;

		indent += 1;

		PrintIndent(stream, indent);
		stream << "keys count[" << channel->mNumKeys << "] {" << std::endl;
		for (unsigned int i = 0; i < channel->mNumKeys; i++)
		{
			PrintMeshKey(stream, channel->mKeys[i], indent + 1);
		}
		PrintIndent(stream, indent);
		stream << "}" << std::endl;
	}

	void Model::PrintAnimationTree(const aiScene* scene, std::ostream& stream)
	{
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			aiAnimation* a = scene->mAnimations[i];
			stream << std::endl << std::endl << "ANIMATION name[" << a->mName.data << "], duration[" << a->mDuration << "], ticksPerSec[" << a->mTicksPerSecond << "]" << std::endl;
			stream << "animation-channels count[" << a->mNumChannels << "] {" << std::endl;
			for (unsigned int j = 0; j < a->mNumChannels; j++)
			{
				PrintAnimationChannel(stream, a->mChannels[j], 1);
			}
			stream << "}" << std::endl;
			stream << "mesh-animation-channels count[" << a->mNumMeshChannels << "] {" << std::endl;
			for (unsigned int j = 0; j < a->mNumMeshChannels; j++)
			{
				PrintMeshChannel(stream, a->mMeshChannels[j], 1);
			}
			stream << "}" << std::endl;
		}
	}


	void Model::PrintBone(std::ostream& stream, const aiBone* bone, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiBone name[" << bone->mName.data << "] #vertex-weights[" << bone->mNumWeights << "]" << std::endl;

		indent += 1;

		PrintIndent(stream, indent);
		stream << "offsetMatrix {" << std::endl;
		PrintMatrix(stream, bone->mOffsetMatrix, indent + 1);
		PrintIndent(stream, indent);
		stream << "}" << std::endl;
	}


	void Model::PrintAnimMesh(std::ostream& stream, const aiAnimMesh* am, int indent)
	{
		PrintIndent(stream, indent);
		stream << "aiAnimMesh numVertices[" << am->mNumVertices << "]" << std::endl;
	}


	void Model::PrintMeshes(const aiScene* scene, std::ostream& stream)
	{
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];
			stream << std::endl << std::endl << "MESH name[" << mesh->mName.data << "], primitiveTypes[" << mesh->mPrimitiveTypes << "], #faces[" << mesh->mNumFaces << "], #vertices[" << mesh->mNumVertices << "], materialIndex[" << mesh->mMaterialIndex << "]" << std::endl;
			stream << "bones count[" << mesh->mNumBones << "] {" << std::endl;
			for (unsigned int j = 0; j < mesh->mNumBones; j++)
			{
				PrintBone(stream, mesh->mBones[j], 1);
			}
			stream << "}" << std::endl;
			stream << "anim-meshes count[" << mesh->mNumAnimMeshes << "] {" << std::endl;
			for (unsigned int j = 0; j < mesh->mNumAnimMeshes; j++)
			{
				PrintAnimMesh(stream, mesh->mAnimMeshes[j], 1);
			}
			stream << "}" << std::endl;
		}
	}


	const glm::mat4& Model::transformation_matrix() const
	{
		return m_load_transformation_matrix;
	}

	const glm::mat4 Model::transformation_matrix(unsigned int meshIndex) const
	{
		return m_load_transformation_matrix * m_meshes[meshIndex].m_scene_transformation_matrix;
	}


	size_t Model::num_meshes() const
	{
		return m_meshes.size();
	}

	size_t Model::num_vertices(unsigned int meshIndex) const
	{
		const auto& mesh = m_meshes.at(meshIndex);
		return mesh.m_vertex_data.size() / mesh.m_size_one_vertex;
	}

	size_t Model::indices_length(unsigned int meshIndex) const
	{
		const auto& mesh = m_meshes.at(meshIndex);
		return mesh.m_indices.size();
	}



	std::string Model::GetMeshName(const aiScene* scene, unsigned int meshIndex) 
	{
		aiMaterial* pMaterial = scene->mMaterials[scene->mMeshes[meshIndex]->mMaterialIndex];
		if (!pMaterial) return "";
		aiString name;
		if (pMaterial->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
			return name.data;
		else
			return "";
	}


	std::string Model::GetTextureNameFromMaterials(const aiScene* scene, unsigned int meshIndex, aiTextureType type)
	{
		aiMaterial* pMaterial = scene->mMaterials[scene->mMeshes[meshIndex]->mMaterialIndex];
		if (!pMaterial) return "";
		aiString filename;
		if (pMaterial->GetTexture(type, 0, &filename, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS)
			return filename.data;
		else
			return "";
	}

	std::string Model::GetDiffuseTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_DIFFUSE);
	}
	std::string Model::GetSpecularTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_SPECULAR);
	}
	std::string Model::GetAmbientTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_AMBIENT);
	}
	std::string Model::GetEmissiveTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_EMISSIVE);
	}
	std::string Model::GetHeightTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_HEIGHT);
	}
	std::string Model::GetNormalsTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_NORMALS);
	}
	std::string Model::GetShininessTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_SHININESS);
	}
	std::string Model::GetOpacityTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_OPACITY);
	}
	std::string Model::GetDisplacementTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_DISPLACEMENT);
	}
	std::string Model::GetReflectionTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_REFLECTION);
	}
	std::string Model::GetLightmapTextureName(const aiScene* scene, unsigned int meshIndex) 
	{
		return GetTextureNameFromMaterials(scene, meshIndex, aiTextureType_LIGHTMAP);
	}

	aiMaterial* Model::GetAssimpMaterialPtr(const aiScene* scene, unsigned int meshIndex)
	{
		return scene->mMaterials[scene->mMeshes[meshIndex]->mMaterialIndex];
	}



	Mesh& Model::mesh_at(unsigned int meshIndex)
	{
		return m_meshes[meshIndex];
	}



}
