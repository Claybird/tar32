#include "pch.h"
#include "tar32api.h"

TEST(dll, version)
{
	ASSERT_EQ(247, TarGetVersion());
}

TEST(dll, list_tar)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tar").c_str(), 0);
	ASSERT_NE((HARC)0, hArc);

	INDIVIDUALINFO info = {};
	int ret = TarFindFirst(hArc, "*.*", &info);
	EXPECT_EQ(0, ret);
	int count = 0;
	for (;ret==0; ret = TarFindNext(hArc, &info)) {
		int attrib = TarGetAttribute(hArc);
		if (attrib & FA_DIREC) {
			EXPECT_STREQ("test_2099/", info.szFileName);
		} else {
			if (std::string(info.szFileName).find("pch.txt") != std::string::npos) {
				EXPECT_EQ(48, info.dwOriginalSize);
			} else {
				EXPECT_EQ(44, info.dwOriginalSize);
			}
		}
		count++;
	}

	EXPECT_EQ(2099 + 1, count);

	TarCloseArchive(hArc);
}

TEST(dll, list_tar_gz)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tgz").c_str(), 0);
	ASSERT_NE((HARC)0, hArc);

	INDIVIDUALINFO info = {};
	int ret = TarFindFirst(hArc, "*.*", &info);
	EXPECT_EQ(0, ret);
	int count = 0;
	for (; ret == 0; ret = TarFindNext(hArc, &info)) {
		int attrib = TarGetAttribute(hArc);
		if (attrib & FA_DIREC) {
			EXPECT_STREQ("test_2099/", info.szFileName);
		} else {
			if (std::string(info.szFileName).find("pch.txt") != std::string::npos) {
				EXPECT_EQ(48, info.dwOriginalSize);
			} else {
				EXPECT_EQ(44, info.dwOriginalSize);
			}
		}
		count++;
	}

	EXPECT_EQ(2099 + 1, count);

	TarCloseArchive(hArc);
}

TEST(dll, list_tar_bz)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tbz").c_str(), 0);
	ASSERT_NE((HARC)0, hArc);

	INDIVIDUALINFO info = {};
	int ret = TarFindFirst(hArc, "*.*", &info);
	EXPECT_EQ(0, ret);
	int count = 0;
	for (; ret == 0; ret = TarFindNext(hArc, &info)) {
		int attrib = TarGetAttribute(hArc);
		if (attrib & FA_DIREC) {
			EXPECT_STREQ("test_2099/", info.szFileName);
		} else {
			if (std::string(info.szFileName).find("pch.txt") != std::string::npos) {
				EXPECT_EQ(48, info.dwOriginalSize);
			} else {
				EXPECT_EQ(44, info.dwOriginalSize);
			}
		}
		count++;
	}

	EXPECT_EQ(2099 + 1, count);

	TarCloseArchive(hArc);
}
