#include "stdafx.h"

#include "Mesh.h"

#include "Texture.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture*> textures) :
    vertices{ vertices },
    indices{ indices },
    textures{ textures } {
    SetupMesh();
}

// currently not drawing textures
void Mesh::Draw(Shader& shader, DrawMode draw_mode) {
    //uint32_t diffuse_num = 1;
    //uint32_t specular_num = 1;
    //uint32_t normal_num = 1;
    //uint32_t height_num = 1;

    //for (uint32_t i = 0; i < textures.size(); ++i) {
    //    glActiveTexture(GL_TEXTURE0 + i);

    //    std::string name = textures[i].type;
    //    //// retrieve texture num
    //    //std::string number;
    //    //std::string name = textures[i].type;
    //    //if (name == "texture_diffuse") {
    //    //    number = std::to_string(diffuse_num++);
    //    //}
    //    //else if (name == "texture_specular") {
    //    //    number = std::to_string(specular_num++);
    //    //}
    //    //else if (name == "texture_normal") {
    //    //    number = std::to_string(normal_num++);
    //    //}
    //    //else if (name == "texture_height") {
    //    //    number = std::to_string(height_num++);
    //    //}

    //    // set sampler to correct texture unit
    //    //shader.SetInt(("material." + name).c_str(), i);
    //    // bind texture
    //    glBindTexture(GL_TEXTURE_2D, textures[i].id);
    //}

    // draw mesh
    glBindVertexArray(vao);

    switch (draw_mode)
    {
    case DrawMode::POINTS:
        break;
    case DrawMode::LINE_STRIP:
        break;
    case DrawMode::LINE_LOOP:
        break;
    case DrawMode::LINES:
        break;
    case DrawMode::LINE_STRIP_ADJACENCY:
        break;
    case DrawMode::LINES_ADJACENCY:
        break;
    case DrawMode::TRIANGLE_STRIP:
        glDrawElements(GL_TRIANGLE_STRIP, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT, 0);
        break;
    case DrawMode::TRIANGLE_FAN:
        break;
    case DrawMode::TRIANGLES:
        glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(indices.size()), GL_UNSIGNED_INT, 0);
        break;
    case DrawMode::TRIANGLE_STRIP_ADJACENCY:
        break;
    case DrawMode::TRIANGLES_ADJACENCY:
        break;
    case DrawMode::PATCHES:
        break;
    default:
        break;
    }

    glBindVertexArray(0);

    // set back to default
    glActiveTexture(GL_TEXTURE0);
}

// HELPERS
void Mesh::SetupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // load data into vertex buffers
    // structs have sequetial memory layout
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    //// texture coords attribute
    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    //// tangent attribute
    //glEnableVertexAttribArray(3);
    //glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    //// bi tangent attribute
    //glEnableVertexAttribArray(4);
    //glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bi_tangent));
    //// ids
    //glEnableVertexAttribArray(2);
    //glVertexAttribIPointer(2, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, bone_ids));
    //// weights
    //glEnableVertexAttribArray(3);
    //glVertexAttribPointer(3, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

    glBindVertexArray(0);
}