#pragma once

#include <vector>
#include <string>

namespace JMD {
    struct RenderPass {};
    struct SMPipeline {};   // TEMP: SM prefix to avoid name clashes
    struct SMMesh {         // TEMP: SM prefix to avoid name clashes
        unsigned int    num_vertices_;
        unsigned int    stride_;
        unsigned int    num_indices_;
        unsigned int    num_indices_bytes_;
        char*           vertices_;
        char*           indices_;
        
        SMMesh()
        :
        num_vertices_(0),
        stride_(0),
        num_indices_(0),
        num_indices_bytes_(0),
        vertices_(nullptr),
        indices_(nullptr)
        {}
        SMMesh(char* vertices,
               unsigned int num_vertices,
               unsigned int stride,
               char* indices,
               unsigned int num_indices,
               unsigned int num_indices_bytes)
        :
        num_vertices_(num_vertices),
        stride_(stride),
        num_indices_(num_indices),
        num_indices_bytes_(num_indices_bytes)
        {
            vertices_ = new char[num_vertices];
            memcpy(vertices_,vertices,num_vertices);
            indices_ = new char[num_indices];
            memcpy(indices_,indices,num_indices);
        }
        ~SMMesh() {
        }
        void ReleaseData() {
            delete[] vertices_;vertices_=nullptr;
            delete[] indices_;indices_=nullptr;
        }
    };
    struct Model {
        unsigned int number_of_meshes;
        SMMesh* mesh_array;
        Model()
        :
        number_of_meshes(0),
        mesh_array(nullptr)
        {}
        ~Model(){
            delete []mesh_array;
        }
    };
    struct Actor {
        std::string name;
    };
class SceneMan {
public:
    ~SceneMan();
    void Load(const std::string& scene_json_name);
    std::vector<Actor*> GetActorPtrs(std::string regex_string);
    void Update();
    void Draw();
private:
    std::vector<RenderPass> render_passes_;
    std::vector<SMPipeline> pipelines_;
    std::vector<Model> models_;
    std::vector<Actor> actors_;
    
    void ReleaseData();
    void BakeRenderPass(unsigned int index);
};
}