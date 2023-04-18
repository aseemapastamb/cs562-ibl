#pragma once

#include "Mesh.h"
#include "Animation.h"
#include "Animator.h"

class Texture;

class Model {
private:
	std::unordered_map<std::string, BoneInfo> bone_info_map;
	int bone_count;

	void ResetVertexBoneData(Vertex& vertex);
	void SetVertexBoneData(Vertex& vertex, int bone_id, float weight);
	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, aiScene const* scene);

	void LoadModel(std::string const& path);
	void ProcessNode(aiNode* node, aiScene const* scene);
	Mesh ProcessMesh(aiMesh* mesh, aiScene const* scene);
	std::vector<Texture*> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string type_name);

public:
	std::vector<std::shared_ptr<Animation>> animations;
	Animator animator;
	std::vector<Texture*> loaded_textures;
	std::vector<Mesh> meshes;
	std::string directory;
	bool gamma_correction;

	Model(std::string const& path, bool gamma = false);

	void LoadAnimation(std::string const& anim_path);
	void Update(float dt);

	void Draw(Shader& shader);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap();
	int const GetBoneCount() const;
	void SetBoneCount(int new_bone_count);
};