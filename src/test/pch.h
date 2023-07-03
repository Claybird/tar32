//
// pch.h
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <filesystem>
#include <gtest/gtest.h>
inline std::string PROJECT_DIR() {
	return std::filesystem::path(__FILE__).parent_path().string();
}

