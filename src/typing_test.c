#include "typing_test.h"
#include "file.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool
InitTypingSession(TypingSession *ts, TestVocabulary *voc, size_t test_chunk)
{
    if (ts == NULL || voc == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }
    
    char **curr = GetVocabularyChunk(voc, test_chunk);
    if (curr == NULL)
    {
        ErrorMsg("Failed to assign current chunk");
        return false;
    }

    char **next = GetVocabularyChunk(voc, test_chunk);
    if (next == NULL)
    {
        ErrorMsg("Failed to assign next chunk");
        goto error;
    }

    ts->voc      = voc;
    ts->curr     = curr;
    ts->next     = next;
    ts->count    = test_chunk;
    ts->curr_idx = 0;

    return true;

error:
    if (curr)
    {
        for (size_t i = 0; i < test_chunk; ++i)
        {
            if (curr[i]) free(curr[i]);
        }
        free(curr);
    }
    ts->voc      = NULL;
    ts->curr     = NULL;
    ts->next     = NULL;
    ts->count    = 0;
    ts->curr_idx = 0;
    return false;
}
