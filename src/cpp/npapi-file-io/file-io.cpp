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
  FILE *file;
  if (fopen_s(&file, filename, "wb") || !file) {
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
      lastSucceeded = _mkdir(subdir) == 0;
    }
  }

  delete[] filenameWithoutTrailingSlash;
  return lastSucceeded;
}

bool removeFile(const char *filename) {
  if (isDirectory(filename)) {
    return removeDirectory(filename);
  }

  DWORD newAttributes = GetFileAttributesA(filename) & (((DWORD)-1) & ~FILE_ATTRIBUTE_READONLY);
  return SetFileAttributesA(filename, newAttributes) && (DeleteFile(filename) != 0);
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
  return success && RemoveDirectoryA(filename);
}

bool getTempPath(char *&value, size_t &len) {
  const size_t bufferSize = MAX_PATH + 1;
  value = new char[bufferSize];
  len = GetTempPathA(bufferSize, value);
  return len != 0;
}

void pushFile(std::vector<FileEntry *> *&files, WIN32_FIND_DATAA &file) {
  if (strcmp(".", file.cFileName) && strcmp("..", file.cFileName)) {
    files->push_back(new FileEntry(file.cFileName, (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY));
  }
}

//Assumes normalisedDirectoryName ends with the directory name, e.g. c:\foo NOT c:\foo\ 
bool listFiles(const char *normalisedDirectoryName, std::vector<FileEntry *> *&files) {
  if (!isDirectory(normalisedDirectoryName)) {
    return false;
  }
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
}
