#include "pch.h"
#include "tar32api.h"

TEST(dll, version)
{
	ASSERT_EQ(247, TarGetVersion());
}

TEST(dll, TarGetFileCount)
{
	EXPECT_EQ(2099 + 1, TarGetFileCount((PROJECT_DIR() + "/test_2099.tar").c_str()));
}

TEST(dll, TarGetArchiveType)
{
	EXPECT_EQ(ARCHIVETYPE_TAR, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tar").c_str()));
	EXPECT_EQ(ARCHIVETYPE_TARGZ, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tgz").c_str()));
	EXPECT_EQ(ARCHIVETYPE_TARBZ2, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tbz").c_str()));
	EXPECT_EQ(ARCHIVETYPE_TARLZMA, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tar.lzma").c_str()));
	EXPECT_EQ(ARCHIVETYPE_TARXZ, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tar.xz").c_str()));
	EXPECT_EQ(ARCHIVETYPE_TARZSTD, TarGetArchiveType((PROJECT_DIR() + "/test_2099.tar.zst").c_str()));
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

TEST(dll, list_tar_lzma)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tar.lzma").c_str(), 0);
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

TEST(dll, list_tar_xz)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tar.xz").c_str(), 0);
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


TEST(dll, list_tar_zstd)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/test_2099.tar.zst").c_str(), 0);
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

TEST(dll, list_multistream_bz)
{
	auto tempDir = std::filesystem::temp_directory_path() / "tar_unit_test";
	std::filesystem::create_directories(tempDir);
	char cwd[_MAX_PATH] = {};
	_getcwd(cwd, _MAX_PATH);
	{
		_chdir(tempDir.string().c_str());
		std::string cmd = "x " + (PROJECT_DIR() + "/multistream.txt.bz2");
		char output[256];
		int ret = Tar(nullptr, cmd.c_str(), output, 256);
		ASSERT_EQ(0, ret);

		FILE* fp = fopen("multistream.txt", "r");
		ASSERT_NE(nullptr, fp);
		char content[256];
		fgets(content, 256, fp);
		fclose(fp);
		EXPECT_STREQ("helloworld", content);
	}
	_chdir(cwd);

	std::filesystem::remove_all(tempDir);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}


TEST(dll, list_tar_utf8)
{
	HARC hArc = TarOpenArchive(nullptr, (PROJECT_DIR() + "/utf8.tar").c_str(), 0);
	ASSERT_NE((HARC)0, hArc);

	INDIVIDUALINFO info = {};
	int ret = TarFindFirst(hArc, "*.*", &info);
	EXPECT_EQ(0, ret);
	for (; ret == 0; ret = TarFindNext(hArc, &info)) {
		int attrib = TarGetAttribute(hArc);
		EXPECT_FALSE(attrib & FA_DIREC);
		EXPECT_STREQ("日本語.txt", info.szFileName);
	}

	TarCloseArchive(hArc);
}
