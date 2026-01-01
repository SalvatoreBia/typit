#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "file.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    TestVocabulary *voc;

    char **curr;
    char **next;

    size_t count;
    size_t curr_idx;
} TypingSession;


bool    InitTypingSession(TypingSession *ts, TestVocabulary *voc, size_t test_chunk);
void    FreeTypingSession(TypingSession *ts);
bool    SwapAndSetNext(TypingSession *ts);



#endif /* TYPING_TEST_H */
