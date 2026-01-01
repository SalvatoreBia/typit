#ifndef TYPING_TEST_H
#define TYPING_TEST_H

#include "file.h"
#include <stddef.h>
#include <stdbool.h>


typedef struct TypingSession
{
    TestVocabulary *voc;
    char **curr;
    char **next;
    size_t count;
    size_t row_count;
    size_t curr_idx;
    size_t curr_word_idx;

    int total_key_pressed;
    int correct_key_pressed;
    int total_correct_words;

    double start_time;
    double elapsed_cache;
    bool   is_running;
} TypingSession;


bool    InitTypingSession(TypingSession *ts, TestVocabulary *voc, size_t test_chunk, size_t row_count);
void    FreeTypingSession(TypingSession *ts, bool free_ts_struct);
bool    SwapAndSetNext(TypingSession *ts);
bool    ValidateInput(TypingSession *ts, char input);
int     CalculateWPM(TypingSession *ts);
float   CalculateAccuracy(TypingSession *ts);
double  GetElapsedTime(TypingSession *ts);
bool    StartTest(TypingSession *ts);
void    StopTest(TypingSession *ts);
bool    ResetTest(TypingSession *ts);

const char* GetCurrentWord(TypingSession *ts);
const char* GetWordAt(TypingSession *ts, size_t global_idx);
size_t      GetRemainingCharsInCurrentWord(TypingSession *ts);

bool    HandleCharInput(TypingSession *ts, char c, char *input_buffer, size_t *input_len, size_t max_len);
bool    HandleSpaceInput(TypingSession *ts, char *input_buffer, size_t *input_len, bool *word_was_correct);
bool    HandleBackspace(TypingSession *ts, char *input_buffer, size_t *input_len);


#endif /* TYPING_TEST_H */
