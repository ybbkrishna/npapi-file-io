#include "file-io.h"

#include "third_party/build/build_config.h"

#include <stdio.h>

#if defined(OS_WIN)
#include <io.h>
#include <windows.h>
#endif
#include <sys/stat.h>

#include <direct.h>
#include <cstring>
#include <string>

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
  FILE *file;
  if (fopen_s(&file, filename, (isBinary ? "rb" : "r")) || !file) {
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
  FILE *file;
  if (fopen_s(&file, filename, "w") || !file) {
    return false;
  }
  if (fputs(value, file) == EOF) {
    fclose(file);
    return false;
  }
  fclose(file);
  return true;
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
      lastSucceeded = _mkdir(subdir) == 0;
    }
  }

  delete[] filenameWithoutTrailingSlash;
  return lastSucceeded;
}

bool removeFile(const char *filename) {
  if (isDirectory(filename)) {
    return RemoveDirectoryA(filename);
  }
  return DeleteFile(filename) != 0;
}

bool getTempPath(char *&value, size_t &len) {
  const size_t bufferSize = 1024;
  value = new char[bufferSize];
  len = GetTempPathA(bufferSize, value);
  return len != 0;
}
