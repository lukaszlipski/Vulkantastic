#include "file.h"
#define NOMINMAX
#include <windows.h>
#include <algorithm>

bool FileHandle::Read(uint8_t* Buffer, int32_t Size)
{
	int32_t ReadCount = File::Get().Read(mHandle, Buffer, Size, mRead);
	mRead = ReadCount;
	return ReadCount;
}

bool FileHandle::Read(std::string& Buffer, int32_t Size /*= 0*/)
{
	if (!Size)
	{
		Size = static_cast<int32_t>(File::Get().Size(this));
	}
	Buffer.resize(Size);
	int32_t ReadCount = File::Get().Read(mHandle, &Buffer[0], Size, mRead);
	mRead = ReadCount;
	return ReadCount;
}

bool FileHandle::Write(const uint8_t* Buffer, int32_t Size, bool Append /*= false*/)
{
	return File::Get().Write(mHandle, Buffer, Size, Append);
}

bool FileHandle::Write(const std::string& Buffer, bool Append /*= false*/)
{
	return File::Get().Write(mHandle, &Buffer[0], static_cast<int32_t>(Buffer.length()), Append);
}

FileHandle::FileHandle(void* Handle)
	: mHandle(Handle)
{

}

FileHandle* File::OpenRead(const std::string& Path)
{
	HANDLE Handle = CreateFile(Path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	return Handle == INVALID_HANDLE_VALUE ? nullptr : new FileHandle(Handle);
}

FileHandle* File::OpenWrite(const std::string& Path)
{
	HANDLE Handle = CreateFile(Path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, 0, nullptr);
	return Handle == INVALID_HANDLE_VALUE ? nullptr : new FileHandle(Handle);
}

FileHandle* File::OpenReadWrite(const std::string& Path)
{
	HANDLE Handle = CreateFile(Path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
	return Handle == INVALID_HANDLE_VALUE ? nullptr : new FileHandle(Handle);
}

void File::CloseFile(FileHandle* Handle)
{
	if (!Handle) { return; }
	CloseHandle(Handle->mHandle);
	delete Handle;
}

bool File::Exists(const std::string& File)
{
	DWORD dirAttrib = GetFileAttributes(File.c_str());
	return (dirAttrib != INVALID_FILE_ATTRIBUTES && !(dirAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool File::DirectoryExists(const std::string& Directory)
{
	DWORD dirAttrib = GetFileAttributes(Directory.c_str());
	return (dirAttrib != INVALID_FILE_ATTRIBUTES && (dirAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int64_t File::Size(FileHandle* Handle)
{
	LARGE_INTEGER size;
	GetFileSizeEx(Handle->mHandle, &size);
	return size.QuadPart;
}

int64_t File::Size(const std::string& Path)
{
	FileHandle* Handle = OpenRead(Path);
	if (Handle == INVALID_HANDLE_VALUE) { return -1; }

	int64_t FileSize = Size(Handle);
	CloseFile(Handle);

	return FileSize;
}

std::string File::CurrentDirectory()
{
	std::vector<char> CurrentDir(512);

	GetCurrentDirectory(512, CurrentDir.data());

	std::for_each(CurrentDir.begin(), CurrentDir.end(), [](auto& Elem)
	{
		if (Elem == '\\') { Elem = '/'; }
	});

	return std::string(CurrentDir.data());
}

bool File::Delete(const std::string& File)
{
	return DeleteFile(File.c_str());
}

bool File::DeleteDirectory(const std::string& Directory)
{
	return RemoveDirectory(Directory.c_str());
}

bool File::MakeDirectory(const std::string& Directory)
{
	return CreateDirectory(Directory.c_str(), nullptr);
}

bool File::GetFiles(std::vector<std::string>& Files, std::string Directory)
{
	Files.clear();

	if (Directory.empty()) return false;
	if (Directory.back() != '/') { Directory.push_back('/'); }
	Directory.push_back('*');

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(Directory.c_str(), &FindData);
	if (hFind == INVALID_HANDLE_VALUE) return false;

	do
	{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { Files.push_back(FindData.cFileName); }
	} while (FindNextFile(hFind, &FindData));

	FindClose(hFind);
	return true;
}

bool File::GetDirectories(std::vector<std::string>& Directories, std::string Directory)
{
	Directories.clear();

	if (Directory.empty()) return false;
	if (Directory.back() != '/') { Directory.push_back('/'); }
	Directory.push_back('*');

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(Directory.c_str(), &FindData);
	if (hFind == INVALID_HANDLE_VALUE) return false;

	do
	{
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { Directories.push_back(FindData.cFileName); }
	} while (FindNextFile(hFind, &FindData) != 0);

	FindClose(hFind);
	return true;
}

int32_t File::Read(void* Handle, void* Buffer, int32_t Size, int32_t Read)
{
	DWORD ReadCount = Read;
	bool Result = ReadFile(Handle, Buffer, Size, &ReadCount, nullptr);
	return Result ? ReadCount : 0;
}

bool File::Write(void* Handle, const void* Buffer, int32_t Size, bool Append /*= false*/)
{
	if (Append)
	{
		SetFilePointer(Handle, 0, nullptr, FILE_END);
	}

	DWORD Written = 0;
	bool result = WriteFile(Handle, Buffer, Size, &Written, nullptr);
	SetFilePointer(Handle, 0, nullptr, 0);

	return Written;
}

