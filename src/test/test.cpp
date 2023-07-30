#include "pch.h"
#include "tar32api.h"

TEST(dll, version)
{
	ASSERT_EQ(248, TarGetVersion());	//need to update manually
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

void sub_tar_list(const std::string& fname)
{
	//---test for Tar() command line interface
	// get list of filename and filesize by Tar()
	// compare the list with those by TarFindFirst/Next() API

	//Tar()
	std::vector<char> output;
	output.resize(1024 * 1024);
	int retCmd = Tar(NULL, ("-l \"" + fname + "\"").c_str(), &output[0], (DWORD)output.size());
	EXPECT_EQ(0, retCmd);

	//FindFirst()
	HARC hArc = TarOpenArchive(nullptr, fname.c_str(), 0);
	ASSERT_NE((HARC)0, hArc);

	INDIVIDUALINFO info = {};
	int ret = TarFindFirst(hArc, "*.*", &info);
	EXPECT_EQ(0, ret);

	//Compare
	char* next_token = NULL;
	char* token = strtok_s(&output[0], "\n", &next_token);
	int lineCount = 0;
	for (;token && ret == 0;) {
		if (lineCount == 0) {
			ASSERT_STREQ("filename" "\t" "filesize", token);
		} else {
			char line[256];
			snprintf(line, 256, "%s\t%d", info.szFileName, info.dwOriginalSize);
			ASSERT_STREQ(line, token);
			ret = TarFindNext(hArc, &info);
		}
		token = strtok_s(NULL, "\n", &next_token);
		lineCount++;
	}
	//make sure both method finishes scan
	EXPECT_NE(0, ret);
	EXPECT_EQ(NULL, token);

	TarCloseArchive(hArc);
}

TEST(dll, Tar_list)
{
	sub_tar_list((PROJECT_DIR() + "/test_2099.tar").c_str());
	sub_tar_list((PROJECT_DIR() + "/test_2099.tgz").c_str());
	sub_tar_list((PROJECT_DIR() + "/test_2099.tbz").c_str());
	sub_tar_list((PROJECT_DIR() + "/test_2099.tar.lzma").c_str());
	sub_tar_list((PROJECT_DIR() + "/test_2099.tar.xz").c_str());
	sub_tar_list((PROJECT_DIR() + "/test_2099.tar.zst").c_str());
}

void sub_extract_create(const std::string& fname, const std::string& format_arg, int64_t acceptable_diff)
{
	auto prevWD = std::filesystem::current_path();	//getcwd
	//get temp directory
	char randomName[256];
	srand((unsigned)time(nullptr));
	snprintf(randomName, 256, "tar32_test_%d", rand());
	auto tempDir = std::filesystem::temp_directory_path() / randomName;
	ASSERT_FALSE(std::filesystem::exists(tempDir));
	auto targetDir = tempDir / "extract";
	std::filesystem::create_directories(targetDir);

	std::filesystem::current_path(targetDir);	//chdir
	ASSERT_EQ(targetDir, std::filesystem::current_path());

	//---extract existing archive
	int ret = Tar(NULL, ("-x \"" + fname + "\"").c_str(), nullptr, 0);
	ASSERT_EQ(0, ret);

	//---create archive from extracted files
	auto ext = std::filesystem::path(fname).extension().string();
	auto archiveName = "..\\test" + ext;
	ret = Tar(NULL, ("-c " + format_arg + " " + archiveName + " --store-in-utf8=0 *.*").c_str(), nullptr, 0);
	ASSERT_EQ(0, ret);

	//---check
	EXPECT_TRUE(TarCheckArchive(archiveName.c_str(), 0));
	EXPECT_EQ(
		TarGetArchiveType(fname.c_str()),
		TarGetArchiveType(archiveName.c_str())
	);
	EXPECT_EQ(
		TarGetFileCount(fname.c_str()),
		TarGetFileCount(archiveName.c_str())
	);

	HARC hArcA = TarOpenArchive(nullptr, fname.c_str(), 0);
	ASSERT_NE((HARC)0, hArcA);
	HARC hArcB = TarOpenArchive(nullptr, archiveName.c_str(), 0);
	ASSERT_NE((HARC)0, hArcB);

	INDIVIDUALINFO infoA = {}, infoB = {};
	int retA = TarFindFirst(hArcA, "*.*", &infoA);
	EXPECT_EQ(0, retA);
	int retB = TarFindFirst(hArcB, "*.*", &infoB);
	EXPECT_EQ(0, retB);

	for (;retA==0 && retB==0;) {
		ASSERT_EQ(infoA.dwOriginalSize, infoB.dwOriginalSize);
		ASSERT_STREQ(infoA.szFileName, infoB.szFileName);
		retA = TarFindNext(hArcA, &infoA);
		retB = TarFindNext(hArcB, &infoB);
	}
	EXPECT_EQ(retA, retB);

	TarCloseArchive(hArcA);
	TarCloseArchive(hArcB);

	//re-created archive size might differ a bit...
	auto diff = std::filesystem::file_size(fname) - std::filesystem::file_size(archiveName);
	EXPECT_LE(abs(int64_t(diff)), acceptable_diff);

	//---cleanup
	std::filesystem::current_path(prevWD);	//chdir
	std::filesystem::remove_all(tempDir);
	ASSERT_FALSE(std::filesystem::exists(tempDir));
}

TEST(dll, Tar_extract_and_create_tar)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tar").c_str(), "", 0);
}
TEST(dll, Tar_extract_and_create_targz)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tgz").c_str(), "-z6", 50);
}
TEST(dll, Tar_extract_and_create_tarbz)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tbz").c_str(), "-B", 100);
}
TEST(dll, Tar_extract_and_create_tarlzma)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tar.lzma").c_str(), "--lzma=9", 100);
}
TEST(dll, Tar_extract_and_create_tarxz)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tar.xz").c_str(), "-J", 100);
}
TEST(dll, Tar_extract_and_create_tarzstd)
{
	sub_extract_create((PROJECT_DIR() + "/test_2099.tar.zst").c_str(), "--zstd", 300);
}

/*TEST(dll, TarExtractMem_TarCompressMem)
{
}*/

TEST(dll, TarCheckArchive)
{
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tar").c_str(), 0));
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tgz").c_str(), 0));
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tbz").c_str(), 0));
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tar.lzma").c_str(), 0));
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tar.xz").c_str(), 0));
	EXPECT_TRUE(TarCheckArchive((PROJECT_DIR() + "/test_2099.tar.zst").c_str(), 0));

	EXPECT_FALSE(TarCheckArchive(PROJECT_DIR().c_str(), 0));
	EXPECT_FALSE(TarCheckArchive(__FILE__, 0));
}

/*TEST(dll, TarConfigDialog)
{
	TODO;
	No test is planned for GUI
}*/

TEST(dll, TarGetFileCount)
{
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tar").c_str()));
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tgz").c_str()));
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tbz").c_str()));
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tar.lzma").c_str()));
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tar.xz").c_str()));
	EXPECT_EQ(2100, TarGetFileCount((PROJECT_DIR() + "/test_2099.tar.zst").c_str()));

	EXPECT_EQ(-1, TarGetFileCount(PROJECT_DIR().c_str()));
	EXPECT_EQ(-1, TarGetFileCount(__FILE__));
}

void sub_test_tar(const std::string &name)
{
	/*
	TarOpenArchive,
	TarCloseArchive,
	TarFindFirst,
	TarFindNext,
	TarGetFileName,
	TarGetOriginalSizeEx
	TarGetCompressedSizeEx
	TarGetRatio
	*/
	HARC hArc = TarOpenArchive(nullptr, name.c_str(), 0);
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
				__int64 llsize;
				EXPECT_TRUE(TarGetOriginalSizeEx(hArc, &llsize));
				EXPECT_EQ(__int64(48), llsize);
				EXPECT_EQ(info.wRatio, TarGetRatio(hArc));
			} else {
				EXPECT_EQ(44, info.dwOriginalSize);
				char buf[256] = {};
				ASSERT_EQ(0, TarGetFileName(hArc, buf, 256));
				EXPECT_STREQ(info.szFileName, buf);
				__int64 llsize;
				EXPECT_TRUE(TarGetCompressedSizeEx(hArc, &llsize));
				EXPECT_EQ(__int64(info.dwCompressedSize), llsize);
			}
		}
		count++;
	}

	EXPECT_EQ(2099 + 1, count);

	TarCloseArchive(hArc);
}

TEST(dll, list_tar)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tar");
}

TEST(dll, list_tar_gz)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tgz");
}

TEST(dll, list_tar_bz)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tbz");
}

TEST(dll, list_tar_lzma)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tar.lzma");
}

TEST(dll, list_tar_xz)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tar.xz");
}

TEST(dll, list_tar_zstd)
{
	sub_test_tar(PROJECT_DIR() + "/test_2099.tar.zst");
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
