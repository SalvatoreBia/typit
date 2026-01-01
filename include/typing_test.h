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
size_t      GetRemainingCharsInCurrentWord(TypingSession *ts);


#endif /* TYPING_TEST_H */
