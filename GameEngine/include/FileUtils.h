#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
/**
 * @namespace FileUtils
 * @brief Utility functions for file system operations.
 *
 * Provides helper functions for scanning directories and filtering files
 * by extension. All functions are header-only (inline) for convenience.
 *
 * Common use cases:
 * - Loading texture assets for rendering
 * - Discovering available resources at runtime
 * - Building file picker UI elements
 * - Asset validation and management
 */
namespace FileUtils {
    /**
     * @brief Scans a directory and returns files matching specified extensions.
     *
     * Recursively searches the given directory for regular files (not subdirectories)
     * and filters them by file extension. Case-sensitive extension matching.
     *
     * Features:
     * - Non-recursive (only scans immediate directory, not subdirectories)
     * - Handles missing directories gracefully (returns empty vector)
     * - Supports extension filtering with or without leading dot
     * - Returns full file paths relative to working directory
     *
     * @param directory Path to directory to scan (e.g. "textures", "assets/models")
     * @param extensions List of file extensions to include (e.g. {".jpg", ".png"})
     *                   If empty, returns ALL files in directory (no filtering)
     *
     * @return Vector of file paths (full paths as strings). Empty if directory doesn't exist.
     *
     * @note Extensions are case-sensitive. ".jpg" and ".JPG" are treated differently.
     * @note Accepts extensions with or without leading dot: ".png" and "png" both work
     * @note Logs errors to stderr if directory doesn't exist or filesystem errors occur
     *
     * @example
     * // Get all image files
     * auto images = getFilesInDirectory("assets", {".png", ".jpg", ".bmp"});
     *
     * // Get all files (no filter)
     * auto allFiles = getFilesInDirectory("data");
     *
     * @example
     * // Use in ImGui dropdown
     * auto files = getFilesInDirectory("textures", {".png"});
     * for (const auto& file : files) {
     *     if (ImGui::Selectable(file.c_str())) {
     *         loadTexture(file);
     *     }
     * }
     */
	inline std::vector<std::string> getFilesInDirectory(
		const std::string& directory,
		const std::vector<std::string>& extensions = {})
	{
		std::vector<std::string> files;

		try {
			if (!std::filesystem::exists(directory)) {
				std::cerr << "Directory does not exist: " << directory << std::endl;
				return files;
			}
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string filepath = entry.path().string();
                    std::string extension = entry.path().extension().string();

                    // If no extensions specified, include all files
                    if (extensions.empty()) {
                        files.push_back(filepath);
                        continue;
                    }

                    // Check if file extension matches any of the specified extensions
                    for (const auto& ext : extensions) {
                        if (extension == ext || extension == ("." + ext)) {
                            files.push_back(filepath);
                            break;
                        }
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        return files;
    }

    /**
    * @brief Gets all texture files from a directory.
    *
    * Convenience function that scans a directory for common image file formats
    * used as textures in 3D rendering. Wraps getFilesInDirectory() with a
    * predefined list of texture extensions.
    *
    * Supported formats:
    * - .jpg, .jpeg (JPEG compressed images)
    * - .png (PNG with transparency support)
    * - .bmp (Bitmap images)
    * - .tga (Targa images)
    *
    * @param textureDirectory Path to texture directory (default: "textures")
    *
    * @return Vector of texture file paths. Empty if directory doesn't exist.
    *
    * @note Does not validate that files are actually valid images - only checks extension
    * @note Does not recurse into subdirectories
    *
    * @example
    * // Get all textures from default directory
    * auto textures = getTextureFiles();
    *
    * // Get textures from custom directory
    * auto uiTextures = getTextureFiles("assets/ui");
    *
    * @example
    * // Populate texture dropdown in editor
    * auto availableTextures = FileUtils::getTextureFiles("textures");
    * for (size_t i = 0; i < availableTextures.size(); i++) {
    *     if (ImGui::Selectable(availableTextures[i].c_str())) {
    *         object->setTexturePath(availableTextures[i]);
    *     }
    * }
    */
    inline std::vector<std::string> getTextureFiles(const std::string& textureDirectory = "textures") {
        return getFilesInDirectory(
            textureDirectory,
            { ".jpg", ".jpeg", ".png", ".bmp", ".tga" }
        );
    }
}