#ifndef FILE_H
#define FILE_H

#include "util.h"
#include <stddef.h>
#include <stdbool.h>
#define TESTS_BASE_PATH "resources/tests"

typedef enum
{
    TEST_SIMPLE200,

    TEST_TYPE_COUNT
} TestType;

typedef struct TestVocabulary
{
    char **list;
    size_t count;
    char  *_raw_buffer;
} TestVocabulary;

bool    InitTestVocabulary(TestVocabulary *voc, const char *lang, TestType type);
void    FreeTestVocabulary(TestVocabulary *voc, bool free_voc_struct);
char**  GetVocabularyChunk(TestVocabulary *voc, size_t chunk);

#endif /* FILE_H */
