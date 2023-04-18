#include "stdafx.h"

#include "AnimationData.h"
#include "Shader.h"
#include "VQS.h"
#include "Bone.h"

#include "Model.h"

#include "GraphicsManager.h"
#include "AssimpHelper.h"

#include "Texture.h"

// filepath to 3D model
Model::Model(std::string const& path, bool gamma) :
	bone_count{ 0 },
	gamma_correction{ gamma } {
	LoadModel(path);
}

void Model::LoadAnimation(std::string const& anim_path) {
	for (auto& i : animations) {
		if (anim_path == i->m_anim_path) {
			animator.PlayAnimation(i);
			return;
		}
	}

	// animation has not been loaded yet
	std::shared_ptr<Animation> animation = std::make_shared<Animation>();
	animation->LoadAnimation(anim_path, this);
	animations.push_back(animation);

	animator.PlayAnimation(animations.back());
}

void Model::Update(float dt) {
	animator.UpdateAnimation(dt);
}

// draw model and all its meshes
void Model::Draw(Shader& shader) {
	for (uint32_t i = 0; i < meshes.size(); ++i) {
		meshes[i].Draw(shader);
	}
}

std::unordered_map<std::string, BoneInfo>& Model::GetBoneInfoMap() {
	return bone_info_map;
}

int const Model::GetBoneCount() const {
	return bone_count;
}

void Model::SetBoneCount(int new_bone_count) {
	bone_count = new_bone_count;
}

// HELPERS

void Model::ResetVertexBoneData(Vertex& vertex) {
	for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; ++i) {
		vertex.bone_ids[i] = -1;
		vertex.weights[i] = 0.0f;
	}
}

void Model::SetVertexBoneData(Vertex& vertex, int bone_id, float weight) {
	for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; ++i) {
		if (vertex.bone_ids[i] < 0) {
			vertex.bone_ids[i] = bone_id;
			vertex.weights[i] = weight;
			break;
		}
	}
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, aiScene const* scene) {
	for (uint32_t bone_idx = 0; bone_idx < mesh->mNumBones; ++bone_idx) {
		int bone_id = -1;
		std::string bone_name = mesh->mBones[bone_idx]->mName.C_Str();
		if (bone_info_map.find(bone_name) == bone_info_map.end()) {
			BoneInfo new_bone_info;
			new_bone_info.id = bone_count;
			new_bone_info.offset = AssimpHelper::ConvertMatrixToGLMFormat(mesh->mBones[bone_idx]->mOffsetMatrix);
			bone_info_map[bone_name] = new_bone_info;
			bone_id = bone_count;
			++bone_count;
		}
		else {
			bone_id = bone_info_map[bone_name].id;
		}
		assert(bone_id != -1);

		aiVertexWeight* p_weights = mesh->mBones[bone_idx]->mWeights;
		int num_weights = mesh->mBones[bone_idx]->mNumWeights;
		for (int weight_idx = 0; weight_idx < num_weights; ++weight_idx) {
			int vertex_id = p_weights[weight_idx].mVertexId;
			float weight = p_weights[weight_idx].mWeight;
			assert(vertex_id <= vertices.size());
			SetVertexBoneData(vertices[vertex_id], bone_id, weight);
		}
	}
}

// load a model with an assimp supported extension
void Model::LoadModel(std::string const& path) {
	// read file via assimp
	Assimp::Importer importer;
	aiScene const* scene = importer.ReadFile(path, aiProcess_Triangulate
		| aiProcess_GenSmoothNormals
		| aiProcess_FlipUVs
		| aiProcess_CalcTangentSpace);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ASSIMP error: " << importer.GetErrorString() << std::endl;
		return;
	}
	// retrieve directory path
	directory = path.substr(0, path.find_last_of('\\'));
	// recursively process root node
	ProcessNode(scene->mRootNode, scene);
}

// recursively process nodes
// process individual mesh in node and repeat for children nodes
void Model::ProcessNode(aiNode* node, aiScene const* scene) {
	for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}
	// process children nodes
	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, aiScene const* scene) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture*> textures;

	// loop through each vertex of mesh
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		ResetVertexBoneData(vertex);

		// position
		vertex.position = AssimpHelper::GetGLMVec3(mesh->mVertices[i]);
		// normal
		vertex.normal = AssimpHelper::GetGLMVec3(mesh->mNormals[i]);

		// texture coords
		//if (mesh->HasTextureCoords(0)) {
		//	glm::vec2 temp_vec2;
		//	temp_vec2.x = mesh->mTextureCoords[0][i].x;
		//	temp_vec2.y = mesh->mTextureCoords[0][i].y;
		//	vertex.tex_coords = temp_vec2;
		//	// tangent
		//	glm::vec3 temp_vec3;
		//	temp_vec3.x = mesh->mTangents[i].x;
		//	temp_vec3.y = mesh->mTangents[i].y;
		//	temp_vec3.z = mesh->mTangents[i].z;
		//	vertex.tangent = temp_vec3;
		//	// bi tangent
		//	temp_vec3.x = mesh->mBitangents[i].x;
		//	temp_vec3.y = mesh->mBitangents[i].y;
		//	temp_vec3.z = mesh->mBitangents[i].z;
		//	vertex.bi_tangent = temp_vec3;
		//}
		//else {
			vertex.tex_coords = glm::vec2{ 0.0f, 0.0f };
		//}
		vertices.push_back(vertex);
	}

	// loop through each face of mesh and retrieve indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// process materials
	//aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	//// sampler name convention in shaders: texture_diffuseN, texture_specularN, etc
	//// diffuse maps
	//std::vector<Texture> diffuse_maps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
	//textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
	//// specular maps
	//std::vector<Texture> specular_maps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
	//textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
	//// normal maps
	//std::vector<Texture> normal_maps = LoadMaterialTextures(material, aiTextureType_NORMALS, "normal");
	//textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
	//// height maps
	//std::vector<Texture> height_maps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "height");
	//textures.insert(textures.end(), height_maps.begin(), height_maps.end());

	// bone weight
	ExtractBoneWeightForVertices(vertices, mesh, scene);

	return Mesh(vertices, indices, textures);
}

std::vector<Texture*> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string type_name) {
	std::vector<Texture*> textures;
	for (uint32_t i = 0; i < material->GetTextureCount(type); ++i) {
		aiString str;
		material->GetTexture(type, i, &str);
		// skip if texture was loaded before
		bool skip = false;
		for (uint32_t j = 0; j < loaded_textures.size(); ++j) {
			if (std::strcmp(loaded_textures[j]->path, str.C_Str()) == 0) {
				textures.push_back(loaded_textures[j]);
				skip = true;
				break;
			}
		}
		// texture has not been loaded
		if (!skip) {
			std::string texture_path = this->directory.append(str.C_Str());
			Texture* texture = new Texture(texture_path.c_str());
			textures.push_back(texture);
			loaded_textures.push_back(texture);
		}
	}

	return textures;
}