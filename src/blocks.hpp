#ifndef BLOCKS_H
#define BLOCKS_H

#include "c4/substr_fwd.hpp"
#include "c4/yml/node.hpp"
#include "materials.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <ios>
#include <memory>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <c4/yml/parse.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// Lua
extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/detail/LuaRef.h>

class BlocksManager {
public:
    /*
     * @brief A struct used to define each global block.
     * Contains info shared across all block types
     */
    struct Block {
        /*
        * @brief The textures of the block.
        * Each texture represents an index into a Material Manager
        */
        struct {
            uint top;
            uint bottom;
            uint north;
            uint east;
            uint south;
            uint west;
        }   textures;

        /*
         * @brief Physical properties of the block
         */
        struct {
            bool affectedByGravity;
            bool solid;
            bool flammable;
            float friction;
        } physics;

        /*
        * @brief The Lua functions executed at each interaction
        */
        // struct {
        //     luabridge::LuaRef onInteract;   /* When right-click */
        //     luabridge::LuaRef onPunch;      /* When left-click */
        //     luabridge::LuaRef onBreak;      /* When block broken */
        //     luabridge::LuaRef onPlace;      /* When block placed */
        //     luabridge::LuaRef onTouch;      /* When entity touches block */
        //     luabridge::LuaRef onLook;       /* When player looks at block */
        // }   functions;

        /*
         * @brief Tags are used to categorize blocks.
         * Example: `wooden`, `can_be_smelted`
         */
        std::vector<std::string> tags;
    };

    BlocksManager(MaterialManager* MaterialManager) : thisMaterialManager(MaterialManager){}

    /*
     * @brief Loads in all the blocks from the provided mod
     */
    void loadBlocks(const char* modName) {
        // Build path to YAML file
        fs::path blocksFolder = fs::path("game_resources") / modName / "blocks";
        fs::path yamlPath = blocksFolder / "blocks.yaml";

        // Read the YAML file
        std::ifstream yamlFile(yamlPath, std::ios::binary | std::ios::ate);
        std::streamsize yamlSize = yamlFile.tellg();
        yamlFile.seekg(0, std::ios::beg);
        std::vector<char> yamlBuffer(yamlSize);

        // Parse YAML
        if (!yamlFile.read(yamlBuffer.data(), yamlSize)) return;
        ryml::Tree yamlTree = ryml::parse_in_place(ryml::to_substr(yamlBuffer));

        // Iterate over blocks
        auto root = yamlTree.rootref();
        for (ryml::NodeRef child : root.children()) {
            std::unique_ptr<Block> thisBlock = std::make_unique<Block>();
            std::string blockName(child.key().data(), child.key().size());

            // Check for inheritance
            if (child.has_child("inherit")) {
                std::string nameInherit(child["inherit"].val().data(), child["inherit"].val().size());
                if (blockRegistry.contains(nameInherit)) {
                    *thisBlock = *blockRegistry[nameInherit];
                }
            }

            // Check for visuals
            if (child.has_child("visuals")) {
                // Check for textures
                if (child["visuals"].has_child("texture"))
                    loadTexturesFromYAMLChild(child["visuals"]["texture"], thisBlock.get(), blocksFolder);
            }

            // Add to the block registry
            blockRegistry[blockName] = std::move(thisBlock);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Block>> blockRegistry;
    MaterialManager* thisMaterialManager;

    //! Written by Gemini
    void loadTexturesFromYAMLChild(ryml::NodeRef child, Block* block, fs::path blocksFolder) {
        using TextureMemberPtr = uint decltype(block->textures)::*;

        struct Mapping {
            const char* key;
            TextureMemberPtr ptr;
        };

        // Static so it's not rebuilt on the stack every call
        static const Mapping mappings[] = {
            {"top",    &decltype(block->textures)::top},
            {"bottom", &decltype(block->textures)::bottom},
            {"north",  &decltype(block->textures)::north},
            {"east",   &decltype(block->textures)::east},
            {"south",  &decltype(block->textures)::south},
            {"west",   &decltype(block->textures)::west}
        };

        // Optimization: Pre-convert path to string to avoid repeated internal processing
        std::string folderStr = blocksFolder.string();
        if (!folderStr.empty() && folderStr.back() != '/' && folderStr.back() != '\\') {
            folderStr += "/";
        }

        for (const auto& mapping : mappings) {
            // Look up the child once. If it doesn't exist, it returns an invalid node.
            if (child.has_child(mapping.key)) {
                ryml::NodeRef sideNode = child[mapping.key];

                // Extract as a csubstr (a pointer + length) - NO HEAP ALLOCATION
                ryml::csubstr fname;
                sideNode >> fname;

                // Manual concatenation to avoid fs::path overhead if newMaterial can take a string
                // Otherwise, use a reusable buffer to avoid allocations
                std::string fullPath;
                fullPath.reserve(folderStr.size() + fname.len);
                fullPath.append(folderStr).append(fname.str, fname.len);

                block->textures.*(mapping.ptr) = thisMaterialManager->newMaterial(fullPath);
            }
        }
    }
};

#endif
