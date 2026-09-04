#pragma once
#include "Scattershot.h"
#include <vector>
#include <string>
std::vector<MK64Input> collectInputs(Block* best);
void writeMkr(const std::vector<MK64Input>& replay, const std::string& path);
void writeJson(const GlobalState& g, int totalFrames, const std::string& path, int characterId, float topSpeed, const std::string& course);
std::vector<MK64Input> readMkr(const std::string& path);
