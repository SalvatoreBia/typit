#include "file.h"
#include "typing_test.h"
#include "gui.h"
#include "util.h"
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

int
main(void)
{
    srand(time(NULL));

    TestVocabulary voc;
    if (!InitTestVocabulary(&voc, "en", TEST_SIMPLE200))
    {
        ErrorMsg("Failed to initialize vocabulary");
        exit(1);
    }

    TypingSession ts;
    if (!InitTypingSession(&ts, &voc, 50, 5))
    {
        ErrorMsg("Failed to initialize typing session");
        exit(1);
    }

    GUIConfig gui_cfg = {
        .screen_width = 1200,
        .screen_height = 800,
        .target_fps = 144,
        .font_size = 36,
        .font_size_stats = 24,
        .bg_color = (Color){26, 26, 26, 255},
        .text_color = (Color){150, 150, 150, 255},
        .text_correct_color = (Color){150, 200, 150, 255},
        .text_wrong_color = (Color){200, 100, 100, 255},
        .text_completed_color = (Color){100, 100, 100, 255},
        .cursor_color = (Color){200, 200, 200, 255},
        .stats_color = (Color){180, 180, 180, 255},
        .words_y_position = 300,
        .stats_y_position = 100,
        .word_spacing = 15,
        .line_height = 50
    };

    InitGUI(&gui_cfg);
    RunTypingTest(&ts, &voc);
    CloseGUI();

    FreeTypingSession(&ts, false);
    FreeTestVocabulary(&voc, false);
    
    return 0;
}