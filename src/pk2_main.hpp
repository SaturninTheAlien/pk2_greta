#pragma once
#include "engine/PDLL.hpp"
#include <string>


std::string PK2_EXPORT pk2_get_version();
void PK2_EXPORT pk2_init();

void PK2_EXPORT pk2_main(bool _dev_mode, bool _show_fps, bool test_level, const std::string& test_path);
bool PK2_EXPORT pk2_convertToNewFormat(const std::string& filename_in, const std::string& filename_out);
bool PK2_EXPORT pk2_setAssetsPath(const std::string& path);

/**
 * @brief 
 * A temporary function for updating sprites.
 */
void PK2_EXPORT pk2_updateSprites(const std::string& dir);