#include "typing_test.h"
#include "file.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


bool
InitTypingSession(TypingSession *ts, TestVocabulary *voc, size_t test_chunk, size_t row_count)
{
    if (ts == NULL || voc == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }
    if (test_chunk == 0 || row_count == 0 || test_chunk % row_count != 0)
    {
        ErrorMsg("Invalid chunk size configuration");
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

    ts->voc           = voc;
    ts->curr          = curr;
    ts->next          = next;
    ts->count         = test_chunk;
    ts->row_count     = row_count;
    ts->curr_idx      = 0;
    ts->curr_word_idx = 0;

    ts->correct_key_pressed = 0;
    ts->total_correct_words = 0;
    ts->total_key_pressed   = 0;
    
    ts->start_time    = 0.0;
    ts->elapsed_cache = 0.0;
    ts->is_running    = false;

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
    ts->voc           = NULL;
    ts->curr          = NULL;
    ts->next          = NULL;
    ts->count         = 0;
    ts->row_count     = 0;
    ts->curr_idx      = 0;
    ts->curr_word_idx = 0;

    ts->correct_key_pressed = 0;
    ts->total_correct_words = 0;
    ts->total_key_pressed   = 0;
    
    ts->start_time    = 0.0;
    ts->elapsed_cache = 0.0;
    ts->is_running    = false;

    return false;
}

void
FreeTypingSession(TypingSession *ts, bool free_ts_struct)
{
    if (ts)
    {
        if (ts->curr)
        {
            for (size_t i = 0; i < ts->count; ++i)
                if (ts->curr[i]) free(ts->curr[i]);
            free(ts->curr);
        }
        if (ts->next)
        {
            for (size_t i = 0; i < ts->count; ++i)
                if (ts->next[i]) free(ts->next[i]);
            free(ts->next);
        }
        
        ts->voc           = NULL;
        ts->curr          = NULL;
        ts->next          = NULL;
        ts->count         = 0;
        ts->row_count     = 0;
        ts->curr_idx      = 0;
        ts->curr_word_idx = 0;

        ts->correct_key_pressed = 0;
        ts->total_correct_words = 0;
        ts->total_key_pressed   = 0;
        
        ts->start_time   = 0.0;
        ts->is_running   = false;
        
        if (free_ts_struct)
        {
            free(ts);
        }
    }
}


bool
SwapAndSetNext(TypingSession *ts)
{
    if (ts == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }

    char **new_next = GetVocabularyChunk(ts->voc, ts->count);
    if (new_next == NULL)
    {
        ErrorMsg("Failed to request new chunk");
        return false;
    }
    
    if (ts->curr)
    {
        for (size_t i = 0; i < ts->count; ++i)
            if (ts->curr[i]) free(ts->curr[i]);
        free(ts->curr);
    }
    ts->curr          = ts->next;
    ts->next          = new_next;
    ts->curr_idx      = 0;
    ts->curr_word_idx = 0;
    return true;
}

bool
StartTest(TypingSession *ts)
{
    ts->is_running = true;
    ts->start_time = (double)clock() / CLOCKS_PER_SEC;
    return true;
}

void
StopTest(TypingSession *ts)
{
    ts->is_running = false;
    ts->start_time = 0.0;
}

bool
ValidateInput(TypingSession *ts, char input)
{
    if (ts == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }
    
    if (!ts->is_running) StartTest(ts);

    ts->total_key_pressed++;
    
    const char *curr_word = ts->curr[ts->curr_idx];
    bool is_valid = curr_word[ts->curr_word_idx] == input;
    
    if (is_valid)
    {
        ts->correct_key_pressed++;
        ts->curr_word_idx++;
        if (curr_word[ts->curr_word_idx] == '\0')
        {
            ts->total_correct_words++;
            ts->curr_idx++;
            ts->curr_word_idx = 0;
            if (ts->curr_idx >= ts->count)
            {
                if (!SwapAndSetNext(ts)) return false;
            }
        }
    }
    
    return is_valid;
}

double
GetElapsedTime(TypingSession *ts)
{
    if (!ts->is_running) return 0.0;
    return ts->elapsed_cache;
}

int
CalculateWPM(TypingSession *ts)
{
    if (ts == NULL) return 0;
    
    double elapsed = ts->elapsed_cache;
    if (elapsed <= 0.0) return 0;

    double wpm = (ts->total_key_pressed / 5.0) * (60.0 / elapsed);
    return (int)wpm;
}

float
CalculateAccuracy(TypingSession *ts)
{
    if (ts == NULL || ts->total_key_pressed == 0) return 0.0f;
    return (float)ts->correct_key_pressed / (float)ts->total_key_pressed;
}

const char*
GetCurrentWord(TypingSession *ts)
{
    if (ts == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return NULL;
    }
    return ts->curr[ts->curr_idx];
}

size_t
GetRemainingCharsInCurrentWord(TypingSession *ts)
{
    if (ts == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return 0;
    }
    return strlen(ts->curr[ts->curr_idx]) - ts->curr_word_idx;
}

bool
ResetTest(TypingSession *ts)
{
    if (ts == NULL)
    {
        ErrorMsg("Passed null pointers to function call");
        return false;
    }

    if (ts->curr)
    {
        for (size_t i = 0; i < ts->count; ++i)
            if (ts->curr[i]) free(ts->curr[i]);
        free(ts->curr);
    }
    if (ts->next)
    {
        for (size_t i = 0; i < ts->count; ++i)
            if (ts->next[i]) free(ts->next[i]);
        free(ts->next);
    }

    char **curr = GetVocabularyChunk(ts->voc, ts->count);
    if (curr == NULL)
    {
        ErrorMsg("Failed to allocate new current chunk");
        return false;
    }

    char **next = GetVocabularyChunk(ts->voc, ts->count);
    if (next == NULL)
    {
        ErrorMsg("Failed to allocate new next chunk");
        goto error;
    }

    ts->curr          = curr;
    ts->next          = next;
    ts->curr_idx      = 0;
    ts->curr_word_idx = 0;

    ts->total_key_pressed   = 0;
    ts->correct_key_pressed = 0;
    ts->total_correct_words = 0;

    ts->start_time = 0.0;
    ts->is_running = false;

    return true;

error:
    if (curr)
    {
        for (size_t i = 0; i < ts->count; ++i)
            if (curr[i]) free(curr[i]);
        free(curr);
    }
    return false;
}

const char*
GetWordAt(TypingSession *ts, size_t global_idx)
{
    if (ts == NULL) return NULL;
    
    if (global_idx < ts->count)
        return ts->curr[global_idx];
    else if (global_idx < ts->count * 2)
        return ts->next[global_idx - ts->count];
    return NULL;
}

bool
HandleCharInput(TypingSession *ts, char c, char *input_buffer, size_t *input_len, size_t max_len)
{
    if (ts == NULL || input_buffer == NULL || input_len == NULL) return false;
    if (*input_len >= max_len - 1) return false;
    
    const char *current_word = GetWordAt(ts, ts->curr_word_idx);
    if (!current_word) return false;
    
    size_t word_len = strlen(current_word);
    
    ts->total_key_pressed++;
    if (*input_len < word_len && c == current_word[*input_len])
    {
        ts->correct_key_pressed++;
    }
    
    input_buffer[*input_len] = c;
    (*input_len)++;
    input_buffer[*input_len] = '\0';
    
    return true;
}

bool
HandleSpaceInput(TypingSession *ts, char *input_buffer, size_t *input_len, bool *word_was_correct)
{
    if (ts == NULL || input_buffer == NULL || input_len == NULL) return false;
    
    const char *current_word = GetWordAt(ts, ts->curr_word_idx);
    if (!current_word) return false;
    
    size_t word_len = strlen(current_word);
    
    bool word_correct = (*input_len == word_len);
    if (word_correct)
    {
        for (size_t i = 0; i < word_len; i++)
        {
            if (input_buffer[i] != current_word[i])
            {
                word_correct = false;
                break;
            }
        }
    }
    
    if (word_was_correct) *word_was_correct = word_correct;
    
    if (word_correct)
    {
        ts->total_correct_words++;
        ts->correct_key_pressed++;
    }
    
    ts->total_key_pressed++;
    ts->curr_word_idx++;
    *input_len = 0;
    input_buffer[0] = '\0';
    
    if (ts->curr_word_idx >= ts->count)
    {
        SwapAndSetNext(ts);
        return true;
    }
    
    return false;
}

bool
HandleBackspace(TypingSession *ts, char *input_buffer, size_t *input_len)
{
    if (ts == NULL || input_buffer == NULL || input_len == NULL) return false;
    if (!ts->is_running || *input_len == 0) return false;
    
    (*input_len)--;
    input_buffer[*input_len] = '\0';
    
    return true;
}
