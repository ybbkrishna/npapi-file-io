bool fileExists(const char *name);
bool getFile(const char *filename, char *&value, size_t &len, const bool issBinary);
bool myMkdir(const char *filename);
bool saveText(const char *filename, const char *value, size_t len);