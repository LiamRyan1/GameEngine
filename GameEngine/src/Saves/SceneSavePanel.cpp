#include <GL/glew.h>
#include "../include/Saves/SceneSavePanel.h"

#include "../include/Scene/Scene.h"
#include "../External/imgui/core/imgui.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

/*Scenes are stored as JSON files in:
../../assets/scenes/ */

void DrawSceneSaveLoadPanel(Scene& scene)
{
    // Buffer used when typing a name to SAVE a new scene
    static char sceneName[128] = "scene_test";
    // Cached list of scene filenames
    static std::vector<std::string> sceneFiles;
    // Currently selected scene in dropdown
    static int selectedIndex = -1;

    // Buffer used when typing a new name for RENAME
    static char renameBuffer[128] = "";

    // Folder where scene JSON files live
    const std::string sceneFolder = "../../assets/scenes/";

    ImGui::Begin("Scene Manager");

    // Auto-load scene list once
    static bool loadedOnce = false;

    // We scan the scene directory once when panel first opens.
    if (!loadedOnce)
    {
        sceneFiles.clear();

        if (std::filesystem::exists(sceneFolder))
        {
            for (auto& entry : std::filesystem::directory_iterator(sceneFolder))
            {
                if (entry.path().extension() == ".json")
                {
                    sceneFiles.push_back(entry.path().stem().string());
                }
            }
        }

        loadedOnce = true;
    }

    // =========================
    // SAVE
    // =========================
    // User types a name and saves the current scene.
    // After saving we refresh the dropdown list.
    ImGui::InputText("Scene Name", sceneName, sizeof(sceneName));

    if (ImGui::Button("Save Scene"))
    {
        std::string fullPath = sceneFolder + std::string(sceneName) + ".json";
        scene.saveToFile(fullPath);

        // AUTO REFRESH SCENE LIST AFTER SAVE
        sceneFiles.clear();

        if (std::filesystem::exists(sceneFolder))
        {
            for (auto& entry : std::filesystem::directory_iterator(sceneFolder))
            {
                if (entry.path().extension() == ".json")
                {
                    sceneFiles.push_back(entry.path().stem().string());
                }
            }
        }
    }

    ImGui::Separator();

    // =========================
    // DROPDOWN
    // =========================
    // Shows all available scenes found in the directory.
    if (!sceneFiles.empty())
    {
        std::vector<const char*> items;
        for (auto& s : sceneFiles)
            items.push_back(s.c_str());

        ImGui::Combo("Scenes", &selectedIndex, items.data(), items.size());
    }

    // =========================
    // RENAME
    // =========================
    // Renames selected scene file on disk.
    // Prevents overwriting an existing file.
    if (selectedIndex >= 0 && selectedIndex < sceneFiles.size())
    {
        ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));

        if (ImGui::Button("Rename Scene"))
        {
            std::string oldPath = sceneFolder + sceneFiles[selectedIndex] + ".json";
            std::string newPath = sceneFolder + std::string(renameBuffer) + ".json";

            // Only rename if destination doesn't already exist
            if (!std::filesystem::exists(newPath))
            {
                std::filesystem::rename(oldPath, newPath);

                // Update dropdown list immediately
                sceneFiles[selectedIndex] = renameBuffer;

                std::cout << "Scene renamed to " << renameBuffer << std::endl;
            }
            else
            {
                std::cout << "Rename failed: file already exists\n";
            }
        }
    }

    // =========================
    // LOAD
    // =========================
    // Loads selected scene into the engine.
    if (selectedIndex >= 0 && selectedIndex < sceneFiles.size())
    {
        if (ImGui::Button("Load Scene"))
        {
            std::string fullPath = sceneFolder + sceneFiles[selectedIndex] + ".json";
            scene.loadFromFile(fullPath);
        }
    }

    // =========================
    // DELETE
    // =========================
    // Opens confirmation popup before removing scene file.
    // Prevents accidental deletion.
    if (selectedIndex >= 0 && selectedIndex < sceneFiles.size())
    {
        if (ImGui::Button("Delete Scene"))
        {
            ImGui::OpenPopup("Confirm Delete");
        }
    }

    if (ImGui::BeginPopupModal("Confirm Delete", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete this scene?");
        ImGui::Separator();

        // ---- Confirm deletion ----
        if (ImGui::Button("Yes", ImVec2(120, 0)))
        {
            std::string fullPath = sceneFolder + sceneFiles[selectedIndex] + ".json";

            if (std::filesystem::exists(fullPath))
                std::filesystem::remove(fullPath);

            // Reset selection after deletion
            selectedIndex = -1;

            // Refresh scene list after deletion
            sceneFiles.clear();

            if (std::filesystem::exists(sceneFolder))
            {
                for (auto& entry : std::filesystem::directory_iterator(sceneFolder))
                {
                    if (entry.path().extension() == ".json")
                    {
                        sceneFiles.push_back(entry.path().stem().string());
                    }
                }
            }

            std::cout << "Scene deleted\n";

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Cancel deletion
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}