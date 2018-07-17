#pragma once
#include <string>
#include <vector>

class File;

class FileHandle
{
public:
	bool Read(uint8_t* Buffer, int32_t Size);
	bool Read(std::string& Buffer, int32_t Size = 0);
	bool Write(const uint8_t* Buffer, int32_t Size, bool Append = false);
	bool Write(const std::string& Buffer, bool Append = false);

private:
	void* mHandle;
	int32_t mRead;

	FileHandle(void* Handle);
	friend File;

};

class File
{
public:
	static File& Get()
	{
		static File* instance = new File();
		return *instance;
	}

	bool Startup() { return true; };
	bool Shutdown() { return true; }

	class FileHandle* OpenRead(const std::string& Path);
	class FileHandle* OpenWrite(const std::string& Path);
	class FileHandle* OpenReadWrite(const std::string& Path);
	void CloseFile(FileHandle* Handle);

	bool Exists(const std::string& File);
	bool DirectoryExists(const std::string& Directory);
	int64_t Size(FileHandle* Handle);
	int64_t Size(const std::string& Path);
	std::string CurrentDirectory();
	bool Delete(const std::string& File);
	bool DeleteDirectory(const std::string& Directory);
	bool MakeDirectory(const std::string& Directory);

	bool GetFiles(std::vector<std::string>& Files, std::string Directory);
	bool GetDirectories(std::vector<std::string>& Directories, std::string Directory);

private:

	friend FileHandle;
	int32_t Read(void* Handle, void* Buffer, int32_t Size, int32_t Read);
	bool Write(void* Handle, const void* Buffer, int32_t Size, bool Append = false);

};