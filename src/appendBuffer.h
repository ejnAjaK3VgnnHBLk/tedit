#ifndef APPEND_BUFFER_C_
#define APPEND_BUFFER_C_

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

#endif