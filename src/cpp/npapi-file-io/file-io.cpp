#include "file-io.h"

#include "third_party/build/build_config.h"

#include <stdio.h>

#if defined(OS_WIN)
#include <io.h>
#endif
#include <sys/stat.h>

//Returns true if successful, false otherwise
//Sets exists to true if fileExists, false otherwise
bool fileExists(const char *filename, bool &exists) {
#if defined(OS_WIN)
  exists = _access(filename, 0) != -1;
  return true;
#elif defined(OS_LINUX) || defined(OS_MAC)
  struct stat s;
  *exists = stat(path, &s) == 0;
  return true;
#endif
  return false;
}

bool getFile(const char *filename, char *&value, size_t &len, const bool isBinary) {
  FILE *file;
  fopen_s(&file, filename, (isBinary ? "rb" : "r"));
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

  if (!isBinary) {
    for (size_t i = 0; i < len; ++i) {
      if (value[i] == 0) {
        return false;
      }
    }
  }
  return true;
}