#include "util.h"
#include <file.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>


#define CHUNK_SIZE 65536

static const char *test_type_table[TEST_TYPE_COUNT] = {
    [TEST_SIMPLE200] = "simple.txt"
};

static const char *test_type_name[TEST_TYPE_COUNT] = {
    [TEST_SIMPLE200] = "default"
};

static size_t
GetFileRowsCount(const char *file)
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL) return 0;

    char buf[CHUNK_SIZE];
    long lines = 0;
    size_t bytes_read;
    char last_char = '\n';
    while ((bytes_read = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        char *p   = buf;
        char *end = p + bytes_read;
        while (p < end)
        {
            if (*p == '\n') lines++;
            p++;
        }
        last_char = buf[bytes_read - 1];
    }

    if (last_char != '\n' && bytes_read == 0 && lines > 0)     lines++;
    else if (last_char != '\n' && lines == 0 && ftell(fp) > 0) lines = 1;
    
    fclose(fp);
    return (size_t)lines;
}

static char**
GetFileContentAsList(const char *file, size_t nrows, char **out_raw_buf)
{
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *content = malloc(fsize + 1);
    if (content == NULL)
    {
        fclose(fp);
        return NULL;
    }

    if (fread(content, 1, fsize, fp) != (size_t)fsize)
    {
        if (!feof(fp))
        { 
            free(content);
            fclose(fp);
            return NULL;
        }
    }
    content[fsize] = '\0';
    fclose(fp);

    char **lines = malloc(nrows * sizeof(char*));
    if (lines == NULL)
    {
        free(content);
        return NULL;
    }

    size_t line_idx = 0;
    char *p = content;
    if (*p != '\0' && line_idx < nrows) lines[line_idx++] = p;

    while (*p != '\0' && line_idx < nrows)
    {
        if (*p == '\r') *p = '\0';
        else if (*p == '\n')
        {
            *p = '\0';
            if (*(p + 1) != '\0') lines[line_idx++] = p + 1;
        }
        p++;
    }

    if (out_raw_buf) *out_raw_buf = content;
    
    return lines;
}

static char*
StringDup(const char *src)
{
    if (src == NULL) return NULL;
    
    size_t len = 0;
    while (src[len] != '\0') len++;
    char *dest = (char *)malloc(len + 1);
    if (dest == NULL) return NULL;
    for (size_t i = 0; i < len; ++i) dest[i] = src[i];
    dest[len] = '\0';
    return dest;
}

bool
InitTestVocabulary(TestVocabulary *voc, const char *lang, TestType type)
{
    if (voc == NULL || lang == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }
    if (strlen(lang) == 0)
    {
        ErrorMsg("Vocabulary language provided is empty");
        return false;
    }

    char path[4096];
    int written = snprintf(path, sizeof(path), "%s/%s/%s", TESTS_BASE_PATH, lang, test_type_table[type]);
    if (written >= (int)sizeof(path))
    {
        ErrorMsg("Path name too long");
        return false;
    }

    voc->count = GetFileRowsCount(path);
    if (voc->count == 0)
    {
        ErrorMsg("File empty or not found -> %s", path);
        return false;
    }

    voc->list = GetFileContentAsList(path, voc->count, &voc->_raw_buffer);
    if (voc->list == NULL)
    {
        ErrorMsg("Failed to load file content into memory");
        goto error;
    }

    return true;

error:
    FreeTestVocabulary(voc, false);
    return false;
}

void
FreeTestVocabulary(TestVocabulary *voc, bool free_voc_struct)
{
    if (voc)
    {
        if (voc->list) { free(voc->list); voc->list = NULL; }
        if (voc->_raw_buffer) { free(voc->_raw_buffer); voc->_raw_buffer = NULL; }
        voc->count = 0;
        if (free_voc_struct) { free(voc); voc = NULL; }
    }
}

bool
ChangeTestLanguage(TestVocabulary *voc, const char *new_lang, TestType type)
{
    if (voc == NULL || new_lang == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }
    
    if (voc->list) { free(voc->list); voc->list = NULL; }
    if (voc->_raw_buffer) { free(voc->_raw_buffer); voc->_raw_buffer = NULL; }
    voc->count = 0;
    
    return InitTestVocabulary(voc, new_lang, type);
}

char**
GetVocabularyChunk(TestVocabulary *voc, size_t chunk)
{
    if (voc == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return NULL;
    }
    if (chunk == 0 || chunk > voc->count)
    {
        ErrorMsg("Invalid request for chunk of size %zu", chunk);
        return NULL;
    }

    char **sublist = (char **)malloc(chunk * sizeof(char*));
    if (sublist == NULL)
    {
        ErrorMsg("Failed to allocate memory for vocabulary chunk");
        return NULL;
    }

    for (size_t i = 0; i < chunk; ++i)
    {
        size_t random_idx = rand() % voc->count;
        char *str = StringDup(voc->list[random_idx]);
        if (str == NULL)
        {
            ErrorMsg("Failed to fill vocabulary list");
            goto error;
        }
        sublist[i] = str;
    }

    return sublist;

error:
    if (sublist)
    {
        size_t i = 0;
        for (;;)
        {
            if (sublist[i] == NULL) break;
            free(sublist[i]);
        }
        free(sublist);
        sublist = NULL;
    }
    return NULL;
}

bool
InitLanguageList(LanguageList *list)
{
    if (list == NULL)
    {
        ErrorMsg("Passed null pointer to function call");
        return false;
    }

    DIR *dir = opendir(TESTS_BASE_PATH);
    if (dir == NULL)
    {
        ErrorMsg("Failed to open tests directory: %s", TESTS_BASE_PATH);
        return false;
    }

    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_name[0] == '.') continue;
        
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", TESTS_BASE_PATH, entry->d_name);
        
        struct stat s;
        if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
        {
            count++;
        }
    }

    if (count == 0)
    {
        ErrorMsg("No language directories found");
        closedir(dir);
        return false;
    }

    list->langs = malloc(count * sizeof(char*));
    if (list->langs == NULL)
    {
        ErrorMsg("Failed to allocate memory for language list");
        closedir(dir);
        return false;
    }
    list->count = count;

    rewinddir(dir);
    size_t idx = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", TESTS_BASE_PATH, entry->d_name);
        
        struct stat s;
        if (stat(path, &s) == 0 && S_ISDIR(s.st_mode))
        {
            list->langs[idx] = StringDup(entry->d_name);
            if (list->langs[idx] == NULL)
            {
                ErrorMsg("Failed to duplicate language name");
                goto error;
            }
            if (strcmp(list->langs[idx], "en") == 0) list->curr_lang = idx;
            idx++;
        }
    }

    closedir(dir);
    return true;

error:
    for (size_t i = 0; i < idx; i++)
    {
        if (list->langs[i]) free(list->langs[i]);
    }
    free(list->langs);
    list->langs     = NULL;
    list->count     = 0;
    list->curr_lang = 0;
    closedir(dir);
    return false;
}

void
FreeLanguageList(LanguageList *list)
{
    if (list == NULL) return;
    if (list->langs)
    {
        for (size_t i = 0; i < list->count; i++)
        {
            if (list->langs[i]) free(list->langs[i]);
        }
        free(list->langs);
        list->langs = NULL;
    }
    list->count     = 0;
    list->curr_lang = 0;
}

bool
InitTestTypeList(TestTypeList *list)
{
    if (list == NULL)
    {
        ErrorMsg("Passed null pointer to function call");
        return false;
    }

    list->types = malloc(TEST_TYPE_COUNT * sizeof(char*));
    if (list->types == NULL)
    {
        ErrorMsg("Failed to allocate memory for test type list");
        return false;
    }

    for (size_t i = 0; i < TEST_TYPE_COUNT; i++)
    {
        list->types[i] = StringDup(test_type_name[i]);
        if (list->types[i] == NULL)
        {
            ErrorMsg("Failed to duplicate test type name");
            for (size_t j = 0; j < i; j++)
            {
                free(list->types[j]);
            }
            free(list->types);
            list->types = NULL;
            return false;
        }
    }

    list->count = TEST_TYPE_COUNT;
    list->curr_type = 0;
    return true;
}

void
FreeTestTypeList(TestTypeList *list)
{
    if (list == NULL) return;
    if (list->types)
    {
        for (size_t i = 0; i < list->count; i++)
        {
            if (list->types[i]) free(list->types[i]);
        }
        free(list->types);
        list->types = NULL;
    }
    list->count = 0;
    list->curr_type = 0;
}
