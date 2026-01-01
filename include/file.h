#ifndef FILE_H
#define FILE_H

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

typedef struct LanguageList
{
    char **langs;
    size_t count;
    size_t curr_lang;
} LanguageList;

typedef struct TestTypeList
{
    char **types;
    size_t count;
    size_t curr_type;
} TestTypeList;

bool    InitTestVocabulary(TestVocabulary *voc, const char *lang, TestType type);
void    FreeTestVocabulary(TestVocabulary *voc, bool free_voc_struct);
char**  GetVocabularyChunk(TestVocabulary *voc, size_t chunk);
bool    ChangeTestLanguage(TestVocabulary *voc, const char *new_lang, TestType type);

bool    InitLanguageList(LanguageList *list);
void    FreeLanguageList(LanguageList *list);

bool    InitTestTypeList(TestTypeList *list);
void    FreeTestTypeList(TestTypeList *list);

#endif /* FILE_H */
