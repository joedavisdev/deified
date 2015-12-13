#pragma once

#include <vector>
#include <string>

namespace JMD {
    struct RenderPass {};
    struct SMPipeline {};   // TEMP: SM prefix to avoid name clashes
    struct SMMesh {};       // TEMP: SM prefix to avoid name clashes
    struct Model {};
    struct Actor {
        std::string name;
    };
class SceneMan {
public:
    void Load(const std::string& scene_json_name);
    std::vector<Actor*> GetActorPtrs(std::string regex_string);
    void Update();
    void Draw();
private:
    std::vector<RenderPass> render_passes_;
    std::vector<SMPipeline> pipelines_;
    std::vector<SMMesh> meshes_;
    std::vector<Model> models_;
    std::vector<Actor> actors_;
    
    void BakeRenderPass(unsigned int index);
};
}