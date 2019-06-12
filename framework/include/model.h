#pragma once


namespace cgb
{
	class model_data;
	using model = std::variant<
		model_data,
		std::shared_ptr<model_data>,
		std::unique_ptr<model_data>
	>;



	enum ModelLoaderFlags
	{
		MOLF_none = 0x00000000,
		// Assimp flags
		MOLF_triangulate = 0x000001,
		MOLF_smoothNormals = 0x000002,
		MOLF_faceNormals = 0x000004,
		MOLF_flipUVs = 0x000008,
		MOLF_limitBoneWeights = 0x000010,
		MOLF_calcTangentSpace = 0x000020,
		// vertex-data flags
		MOLF_loadNormals = 0x010000,
		MOLF_loadColors = 0x020000,
		MOLF_loadTexCoords = 0x040000,
		MOLF_loadTexCoords2 = 0x080000,
		MOLF_loadTexCoords3 = 0x100000,
		MOLF_loadBones = 0x200000,
		// the default flags
		MOLF_default = MOLF_triangulate | MOLF_smoothNormals | MOLF_limitBoneWeights,
	};

	enum struct VertexAttribData : uint32_t
	{
		Nothing = 0x0000,
		Position = 0x0001,
		Normal = 0x0002,
		Color = 0x0004,
		Bones = 0x0008,
		Tex2D = 0x0010,
		Tex2D_2 = 0x0020,
		Tex2D_3 = 0x0040,
		Tangents = 0x0080,
		Bitangents = 0x0100,
		Param0 = 0x0200,
		Param1 = 0x0400,
		Param2 = 0x0800,
		Param3 = 0x1000,
		Param4 = 0x2000,
		Param5 = 0x4000,
		//TangentSpace	= 0x0182
	};

	inline VertexAttribData operator| (VertexAttribData a, VertexAttribData b)
	{
		typedef std::underlying_type<VertexAttribData>::type EnumType;
		return static_cast<VertexAttribData>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
	}

	inline VertexAttribData operator& (VertexAttribData a, VertexAttribData b)
	{
		typedef std::underlying_type<VertexAttribData>::type EnumType;
		return static_cast<VertexAttribData>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
	}

	inline VertexAttribData& operator |= (VertexAttribData& a, VertexAttribData b)
	{
		return a = a | b;
	}

	inline VertexAttribData& operator &= (VertexAttribData& a, VertexAttribData b)
	{
		return a = a & b;
	}



	using MeshIdx = int;
	using VAOMap = std::unordered_map<VertexAttribData, uint32_t>;


	class mesh_data
	{
	public:

		MeshIdx m_index;
		std::string m_name;

		VertexAttribData m_vertex_data_layout;

		/// stores different VAOs for different VertexData-combinations
		VAOMap m_vertex_array_objects;

		std::vector<uint8_t> m_vertex_data;
		
		std::vector<uint32_t> m_indices;
		
		/*! If indices are intended to be used with GL_PATCHES, this holds the patch size */
		int m_patch_size;

		size_t m_size_one_vertex;

		size_t m_position_offset;
		size_t m_normal_offset;
		size_t m_tex_coords_offset;
		size_t m_color_offset;
		size_t m_bone_incides_offset;
		size_t m_bone_weights_offset;
		size_t m_tangent_offset;
		size_t m_bitangent_offset;

		size_t m_position_size;
		size_t m_normal_size;
		size_t m_tex_coords_size;
		size_t m_color_size;
		size_t m_bone_indices_size;
		size_t m_bone_weights_size;
		size_t m_tangent_size;
		size_t m_bitangent_size;

		glm::mat4 m_scene_transformation_matrix;

	public:
		// Constructor - initialize everything
		mesh_data() :
			m_index(-1),
			m_vertex_data_layout(VertexAttribData::Nothing),
			m_size_one_vertex(0),
			m_position_offset(0),
			m_normal_offset(0),
			m_tex_coords_offset(0),
			m_color_offset(0),
			m_bone_incides_offset(0),
			m_bone_weights_offset(0),
			m_tangent_offset(0),
			m_bitangent_offset(0),
			m_position_size(0),
			m_normal_size(0),
			m_tex_coords_size(0),
			m_color_size(0),
			m_bone_indices_size(0),
			m_bone_weights_size(0),
			m_tangent_size(0),
			m_scene_transformation_matrix(glm::mat4(1.0f)),
			m_bitangent_size(0)
		{}

		MeshIdx index() const { return m_index; }
		const std::string& name() const { return m_name; }
		VertexAttribData vertex_data_layout() const { return m_vertex_data_layout; }
		const std::vector<uint8_t>& vertex_data() const { return m_vertex_data; }
		const std::vector<uint32_t>& indices() const { return m_indices; }
		uint32_t indices_length() const { return static_cast<uint32_t>(m_indices.size()); }

		int patch_size() const { return m_patch_size; }
		const glm::mat4& transformation_matrix() const { return m_scene_transformation_matrix; }

		glm::vec3 vertex_position_at(size_t index) const;
		glm::vec3 vertex_normal_at(size_t index) const;
		uint32_t index_at(size_t index) const;

		void SetVertexData(
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
			size_t bitangent_size);
		void SetVertexData(
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
			size_t bitangent_size);

		void SetIndices(std::vector<uint32_t>&& indices, int patch_size);
		void SetIndices(const std::vector<uint32_t>& indices, int patch_size);

	};

	using MeshRef = std::reference_wrapper<mesh_data>;
	
	class model_data
	{
	private:
		static const int kNumFaceVertices = 3;
		static const int kVertexSize = 3;
		static const int kColorSize = 4;
		static const int kNormalSize = 3;
		static const char* kIndent;
		static const glm::vec4 kDefaultDiffuseColor;
		static const glm::vec4 kDefaultSpecularColor;
		static const glm::vec3 kDefaultAmbientColor;
		static const glm::vec3 kDefaultEmissiveColor;
		static const glm::vec4 kDefaultTransparentColor;
		static const float kDefaultSpecularPower;
		static const float kDefaultOpacity;
		static const float kDefaultBumpScaling;
		static const float kDefaultRefraction;
		static const float kDefaultReflectivity;

		std::vector<mesh_data> m_meshes;

#ifdef false
		std::vector<Animator*> m_animators;
		std::map<unsigned int, AnimationClip> m_animation_clips;
#endif

		/// The transformation matrix specified while
		glm::mat4 m_load_transformation_matrix;

	public:
		model_data(const glm::mat4& loadTransMatrix = glm::mat4(1.0f));
		model_data(const model_data& other) = delete;
		model_data(model_data&& other) noexcept;
		model_data& operator=(const model_data& other) = delete;
		model_data& operator=(model_data&& other) noexcept;
		~model_data();

	public:
		static std::unique_ptr<model_data> LoadFromFile(const std::string& path, const glm::mat4& transform_matrix, const unsigned int model_loader_flags = MOLF_default);
		static std::unique_ptr<model_data> LoadFromMemory(const std::string& memory, const glm::mat4& transform_matrix, const unsigned int model_loader_flags = MOLF_default);

	private:
		unsigned static int CompileAssimpImportFlags(const unsigned int modelLoaderFlags);
		bool LoadFromFile(const std::string& path, const unsigned int modelLoaderFlags = MOLF_default);
		bool LoadFromMemory(const std::string& data, const unsigned int modelLoaderFlags = MOLF_default);
		bool PostLoadProcessing(Assimp::Importer& importer, const aiScene* scene, const std::string* file_path_or_null);

		bool InitScene(const aiScene* scene);
		bool InitMesh(const int index, const aiMesh* paiMesh);
		void InitTransformationMatrices(const aiNode* pNode, const aiMatrix4x4& accTrans);

		static void PrintIndent(std::ostream& stream, int indent);
		static void PrintMatrix(std::ostream& stream, const aiMatrix4x4& mat, int indent);
		static void PrintMesh(const aiScene* scene, std::ostream& stream, unsigned int meshIndex, int indent);
		static void PrintNodeRecursively(const aiScene* scene, std::ostream& stream, const aiNode* pNode, const aiMatrix4x4 accTranse, int indent);
		static void PrintVector3D(std::ostream& stream, const aiVector3D& vec);
		static void PrintQuaternion(std::ostream& stream, const aiQuaternion& quat);
		static void PrintVectorKey(std::ostream& stream, const aiVectorKey& key, int indent);
		static void PrintRotationKey(std::ostream& stream, const aiQuatKey& key, int indent);
		static void PrintAnimationChannel(std::ostream& stream, const aiNodeAnim* channel, int indent);
		static void PrintMeshKey(std::ostream& stream, const aiMeshKey& key, int indent);
		static void PrintMeshChannel(std::ostream& stream, const aiMeshAnim* channel, int indent);
		static void PrintBone(std::ostream& stream, const aiBone* bone, int indent);
		static void PrintAnimMesh(std::ostream& stream, const aiAnimMesh* am, int indent);

		static std::string GetTextureNameFromMaterials(const aiScene* scene, unsigned int meshIndex, aiTextureType type);

	public:
		const glm::mat4& transformation_matrix() const;
		const glm::mat4 transformation_matrix(unsigned int meshIndex) const;

		static void PrintNodeTree(const aiScene* scene, std::ostream& stream);
		static void PrintAnimationTree(const aiScene* scene, std::ostream& stream);
		static void PrintMeshes(const aiScene* scene, std::ostream& stream);

		size_t num_meshes() const;
		size_t num_vertices(unsigned int meshIndex) const;
		size_t indices_length(unsigned int meshIndex) const;

		static std::string GetMeshName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetDiffuseTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetSpecularTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetAmbientTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetEmissiveTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetHeightTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetNormalsTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetShininessTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetOpacityTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetDisplacementTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetReflectionTextureName(const aiScene* scene, unsigned int meshIndex);
		static std::string GetLightmapTextureName(const aiScene* scene, unsigned int meshIndex);

		static aiMaterial* GetAssimpMaterialPtr(const aiScene* scene, unsigned int meshIndex);

		template<typename Func>
		std::vector<MeshRef> SelectMeshes(Func predicate)
		{
			std::vector<MeshRef> selection;
			size_t n = m_meshes.size();
			for (size_t i = 0; i < n; ++i)
			{
				if (predicate(m_meshes[i]))
				{
					selection.push_back(m_meshes[i]);
				}
			}
			return selection;
		}

		std::vector<MeshRef> SelectAllMeshes()
		{
			return SelectMeshes([](const mesh_data& mesh) { return true; });
		}

		mesh_data& mesh_at(unsigned int meshIndex);

	};

}
