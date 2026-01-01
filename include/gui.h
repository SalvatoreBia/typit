#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "typing_test.h"
#include "file.h"

typedef struct GUIConfig
{
    int screen_width;
    int screen_height;
    int target_fps;

    int font_size;
    int font_size_stats;

    Color bg_color;
    Color text_color;
    Color text_correct_color;
    Color text_wrong_color;
    Color text_completed_color;
    Color cursor_color;
    Color stats_color;
    
    int words_y_position;
    int stats_y_position;
    int word_spacing;
    int line_height;
} GUIConfig;

typedef struct TestDurationList
{
    int *durations;
    size_t count;
    size_t curr_duration;
} TestDurationList;


void    InitGUI(GUIConfig *cfg);
void    RunTypingTest(TypingSession *ts, TestVocabulary *voc);
void    CloseGUI(void);

bool    InitTestDurationList(TestDurationList *list);
void    FreeTestDurationList(TestDurationList *list);

#endif /* GUI_H */
