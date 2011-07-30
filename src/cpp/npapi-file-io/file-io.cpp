#include "file-io.h"

#include "third_party/build/build_config.h"

#include <stdio.h>

#if defined(OS_WIN)
#include <io.h>
#include <windows.h>
#include <direct.h>
#endif
#include <sys/stat.h>

#include <cstring>
#include <string>

#if defined(OS_LINUX)
const mode_t DEFAULT_FOLDER_PERMISSIONS = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP;
#endif

bool removeDirectory(const char *filename);

const char *dropTrailingSlash(const char *filenameWithTrailingSlash) {
  char *filenameWithoutTrailingSlash = new char[strlen(filenameWithTrailingSlash) + 1];
  strcpy(filenameWithoutTrailingSlash, filenameWithTrailingSlash);
  for (int i = strlen(filenameWithoutTrailingSlash) - 1; i >= 0; --i) {
    if (filenameWithoutTrailingSlash[i] == '\\') {
      filenameWithoutTrailingSlash[i] = '\0';
    } else {
      break;
    }
  }
  return filenameWithoutTrailingSlash;
}

bool fileExists(const char *filename) {
  const char *filenameWithoutTailingSlash = dropTrailingSlash(filename);
  struct stat s;
  bool fileExists = stat(filenameWithoutTailingSlash, &s) == 0;
  delete[] filenameWithoutTailingSlash;
  return fileExists;
}

bool isDirectory(const char *filename) {
  const char *filenameWithoutTailingSlash = dropTrailingSlash(filename);
  struct stat s;
  bool isDirectory = stat(filenameWithoutTailingSlash, &s) == 0 && (s.st_mode & S_IFDIR);
  delete[] filenameWithoutTailingSlash;
  return isDirectory;
}

bool getFile(const char *filename, char *&value, size_t &len, const bool isBinary) {
  FILE *file = fopen(filename, isBinary ? "rb" : "r");
  if (!file) {
    return false;
  }

  fseek(file, 0, SEEK_END);
  size_t fileLength = ftell(file);
  rewind(file);

  value = new char[fileLength + 1];
  if (!value) {
    fclose(file);
    return false;
  }

  len = fread(value, 1, fileLength, file);
  fclose(file);
  return true;
}

bool saveText(const char *filename, const char *value, size_t len) {
  if (fileExists(filename)) {
    return false;
  }
  FILE *file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  size_t written = 0;
  while (written < len) {
    const char *start = value + written;
    if (fputs(start, file) == EOF) {
      fclose(file);
      return false;
    }
    written += strlen(start);
    while (written < len && value[written] == '\0') {
      if (fputc('\0', file) == EOF) {
        fclose(file);
        return false;
      }
      ++written;
    }
  }
  fclose(file);
  return true;
}

bool saveBinaryFile(const char *filename, const char *bytes, const size_t len) {
  if (fileExists(filename)) {
    return false;
  }
  FILE *file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  const size_t written = fwrite(bytes, 1, len, file);
  fclose(file);

  return (written == len);
}

bool createDirectory(const char *filename) {
  const char *filenameWithoutTrailingSlash = dropTrailingSlash(filename);
  std::string filenameToSplit(filenameWithoutTrailingSlash);
  bool lastSucceeded = false;
  bool first = true;
  size_t lastSlash = -1;
  while (first || lastSlash != std::string::npos) {
    first = false;
    lastSlash = filenameToSplit.find("\\", lastSlash + 1);
    std::string substr = lastSlash == std::string::npos ? filenameToSplit : filenameToSplit.substr(0, lastSlash + 1);
    const char *subdir = substr.c_str();
    if (!fileExists(subdir)) {
#if defined(OS_WIN)
      lastSucceeded = _mkdir(subdir) == 0;
#elif defined(OS_LINUX)
      lastSucceeded = mkdir(subdir, DEFAULT_FOLDER_PERMISSIONS);
#endif
    }
  }

  delete[] filenameWithoutTrailingSlash;
  return lastSucceeded;
}

bool removeFile(const char *filename) {
  if (isDirectory(filename)) {
    return removeDirectory(filename);
  }
#if defined(OS_WIN)
  DWORD newAttributes = GetFileAttributesA(filename) & (((DWORD)-1) & ~FILE_ATTRIBUTE_READONLY);
  return SetFileAttributesA(filename, newAttributes) && (DeleteFile(filename) != 0);
#else
  return false;
#endif
}

bool removeDirectory(const char *filename) {
  bool success = true;

  std::vector<FileEntry *> *subfiles;
  listFiles(filename, subfiles);
  std::vector<FileEntry *>::iterator file;
  for (file = subfiles->begin() ; file < subfiles->end(); ++file) {
    char *fullName = new char[strlen(filename) + strlen((*file)->name) + 2];
    sprintf(fullName, "%s\\%s", filename, (*file)->name);
    if (!removeFile(fullName)) {
      success = false;
      break;
    }
  }
  subfiles->clear();
  delete subfiles;
#if defined(OS_WIN)
  return success && RemoveDirectoryA(filename);
#else
  return false;
#endif
}

bool getTempPath(char *&value, size_t &len) {
#if defined(OS_WIN)
  const size_t bufferSize = FILENAME_MAX + 1;
  value = new char[bufferSize];
  len = GetTempPathA(bufferSize, value);
  return len != 0;
#else
  return false;
#endif
}

#if defined(OS_WIN)
void pushFile(std::vector<FileEntry *> *&files, WIN32_FIND_DATAA &file) {
  if (strcmp(".", file.cFileName) && strcmp("..", file.cFileName)) {
    files->push_back(new FileEntry(file.cFileName, (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));
  }
}
#endif

//Assumes normalisedDirectoryName ends with the directory name, e.g. "c:\foo" NOT "c:\foo\"
bool listFiles(const char *normalisedDirectoryName, std::vector<FileEntry *> *&files) {
  if (!isDirectory(normalisedDirectoryName)) {
    return false;
  }
#if defined(OS_WIN)
  files = new std::vector<FileEntry *>();

  char *filenameSlashStar = new char[strlen(normalisedDirectoryName) + 3];
  sprintf(filenameSlashStar, "%s\\*", normalisedDirectoryName);

  WIN32_FIND_DATAA file;
  HANDLE handle = FindFirstFileA(filenameSlashStar, &file);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  } else {
    pushFile(files, file);
  }
  bool filesRemaining = true;
  while (filesRemaining) {
    if (FindNextFileA(handle, &file)) {
      pushFile(files, file);
    } else {
      FindClose(handle);
      filesRemaining = false;
    }
  }

  delete[] filenameSlashStar;
  return true;
#else
  return false;
#endif
}
