char *encodeFile(char *target, char *address);
void populateDict(char *dict[], char *filename, char *address);
char *infoDict(char *filename, char *ans);
char *encodeString(char *input, char *ans);
char *encodeInt(int *input, char *ans);
char *encodeListString(char *input, char *ans);
char *encodeListInt(char *input, char *ans);
char *encodeListList(char *input, char *ans);
char *encodeListDict(char *input, char *ans);
char *encodeDict(char *input, int size, char *ans);