#include "scene_parser.hpp"

namespace JMD {
    // Code nabbed from here: https://ysonggit.github.io/coding/2014/12/16/split-a-string-using-c.html
    std::vector<std::string> StringSplit(const std::string &s, char delim) {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> tokens;
        while (getline(ss, item, delim)) {
            tokens.push_back(item);
        }
        return tokens;
    }
    void SceneParser::ProcessEffects(const Json::Value &effects_node) {
        if(!effects_node.isArray()) return;
        for(unsigned int index=0; index < effects_node.size(); ++index) {
            const Json::Value value_object(effects_node[index]);
            if(!value_object.isObject()) continue;
            ParsedEffect parsed_effect;
            parsed_effect.name = value_object["name"].asString();
            parsed_effect.vert_shader_name = value_object["vert_shader"].asString();
            parsed_effect.frag_shader_name = value_object["frag_shader"].asString();
            effects.push_back(parsed_effect);
        }
    }
    void SceneParser::ProcessActors(const Json::Value &actors_node) {
        if(!actors_node.isArray()) return;
        for(unsigned int index=0; index < actors_node.size(); ++index) {
            const Json::Value value_object(actors_node[index]);
            if(!value_object.isObject()) continue;
            ParsedActor parsed_actor;
            parsed_actor.name = value_object["name"].asString();
            parsed_actor.effect_name = value_object["effect"].asString();
            parsed_actor.model_name = value_object["model"].asString();
            auto world_position_string = StringSplit(value_object["world_position"].asString(),',');
            parsed_actor.world_position = glm::vec4(atof(world_position_string[0].c_str()),
                                                    atof(world_position_string[1].c_str()),
                                                    atof(world_position_string[2].c_str()),
                                                    1.0);
            actors.push_back(parsed_actor);
        }
    }
    void SceneParser::Parse(const std::string &scene_file) {
        Json::Value root, default_node;
        Json::Reader json_reader;json_reader.parse(scene_file, root);
        ProcessEffects(root.get("effects", default_node));
        ProcessActors(root.get("actors", default_node));
    }
}