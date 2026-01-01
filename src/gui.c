#include "gui.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPTION_BAR_ITEMS    3
#define OPTION_BAR_PADDING  10

static GUIConfig *gui_config = NULL;
static TypingSession *current_session = NULL;
static TestVocabulary *current_vocabulary = NULL;
static LanguageList langs;
static TestTypeList test_types;
static TestDurationList test_durations;
static int dropdown_open = -1;

void
InitGUI(GUIConfig *cfg)
{
    if (cfg == NULL)
    {
        ErrorMsg("Passed null pointer to InitGUI");
        return;
    }
    if (!InitLanguageList(&langs)) return;
    if (!InitTestTypeList(&test_types)) return;
    if (!InitTestDurationList(&test_durations)) return;

    gui_config = cfg;
    
    InitWindow(cfg->screen_width, cfg->screen_height, "TYPIT - Typing Test");
    SetTargetFPS(cfg->target_fps);
    
    fprintf(stdout, "GUI initialized: %dx%d @ %d FPS\n", 
           cfg->screen_width, cfg->screen_height, cfg->target_fps);
}

void
CloseGUI(void)
{
    FreeLanguageList(&langs);
    FreeTestTypeList(&test_types);
    FreeTestDurationList(&test_durations);
    CloseWindow();
    gui_config = NULL;
}

static const char*
GetDropdownButtonText(int dropdown_index, char *buffer, size_t buffer_size)
{
    if (dropdown_index == 0)
    {
        return langs.langs[langs.curr_lang];
    }
    else if (dropdown_index == 1)
    {
        return test_types.types[test_types.curr_type];
    }
    else if (dropdown_index == 2)
    {
        snprintf(buffer, buffer_size, "%ds", test_durations.durations[test_durations.curr_duration]);
        return buffer;
    }
    return NULL;
}

static size_t
GetDropdownItemCount(int dropdown_index)
{
    if (dropdown_index == 0) return langs.count;
    else if (dropdown_index == 1) return test_types.count;
    else if (dropdown_index == 2) return test_durations.count;
    return 0;
}

static const char*
GetDropdownItemText(int dropdown_index, size_t item_index, char *buffer, size_t buffer_size)
{
    if (dropdown_index == 0)
    {
        return langs.langs[item_index];
    }
    else if (dropdown_index == 1)
    {
        return test_types.types[item_index];
    }
    else if (dropdown_index == 2)
    {
        snprintf(buffer, buffer_size, "%ds", test_durations.durations[item_index]);
        return buffer;
    }
    return NULL;
}

static void
SelectDropdownItem(int dropdown_index, size_t item_index)
{
    if (dropdown_index == 0)
    {
        langs.curr_lang = item_index;
        if (current_vocabulary && current_session)
        {
            const char *new_lang = langs.langs[item_index];
            TestType type = (TestType)test_types.curr_type;
            if (ChangeTestLanguage(current_vocabulary, new_lang, type))
            {
                ResetTest(current_session);
            }
        }
    }
    else if (dropdown_index == 1) test_types.curr_type = item_index;
    else if (dropdown_index == 2) test_durations.curr_duration = item_index;
}

static void
DrawDropdownButton(Rectangle rect, const char *text, bool is_hover)
{
    Color btn_color = is_hover ? Fade(GRAY, 0.5f) : Fade(GRAY, 0.3f);
    DrawRectangleRounded(rect, 0.3f, 10, btn_color);
    
    if (text)
    {
        int text_width = MeasureText(text, 20);
        float text_x = rect.x + (rect.width - text_width) / 2;
        DrawText(text, text_x, 40, 20, WHITE);
    }
}

static void
DrawDropdownMenu(int dropdown_index, float rect_x, float option_width, Vector2 mouse, bool clicked)
{
    size_t item_count = GetDropdownItemCount(dropdown_index);
    char item_buffer[32];
    
    for (size_t j = 0; j < item_count; j++)
    {
        Rectangle item = {
            .x = rect_x, .y = 80 + (j * 40),
            .width = option_width, .height = 35
        };
        
        bool item_hover = CheckCollisionPointRec(mouse, item);
        Color item_color = item_hover ? Fade(DARKGRAY, 1.0f) : Fade(DARKGRAY, 0.9f);
        DrawRectangleRounded(item, 0.2f, 10, item_color);
        
        const char *item_text = GetDropdownItemText(dropdown_index, j, item_buffer, sizeof(item_buffer));
        if (item_text)
        {
            DrawText(item_text, rect_x + 20, 85 + (j * 40), 18, WHITE);
        }
        
        if (clicked && item_hover)
        {
            SelectDropdownItem(dropdown_index, j);
            dropdown_open = -1;
        }
    }
}

void
DrawOptionBar()
{
    float option_bar_width = gui_config->screen_width - 50;
    float option_width     = (option_bar_width - (OPTION_BAR_PADDING * (OPTION_BAR_ITEMS + 1))) / OPTION_BAR_ITEMS;
    
    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    char text_buffer[32];
    
    for (int i = 0; i < OPTION_BAR_ITEMS; ++i)
    {
        float rect_x = 25 + OPTION_BAR_PADDING + (i * (option_width + OPTION_BAR_PADDING));
        Rectangle option = {
            .x = rect_x, .y = 25,
            .width = option_width, .height = 50
        };
        
        bool is_hover = CheckCollisionPointRec(mouse, option);
        const char *text = GetDropdownButtonText(i, text_buffer, sizeof(text_buffer));
        
        DrawDropdownButton(option, text, is_hover);
        
        if (clicked && is_hover)
        {
            dropdown_open = (dropdown_open == i) ? -1 : i;
        }
        if (dropdown_open == i)
        {
            DrawDropdownMenu(i, rect_x, option_width, mouse, clicked);
        }
    }
}

static void
DrawWord(const char *word, int x, int y, Color color)
{
    for (size_t c = 0; c < strlen(word); c++)
    {
        char ch[2] = {word[c], '\0'};
        DrawText(ch, x, y, gui_config->font_size, color);
        x += MeasureText(ch, gui_config->font_size) + 2;
    }
}

static int
GetWordDisplayWidth(const char *word)
{
    int width = 0;
    for (size_t c = 0; c < strlen(word); c++)
    {
        char ch[2] = {word[c], '\0'};
        width += MeasureText(ch, gui_config->font_size) + 2;
    }
    return width;
}

static void
DrawCurrentWordWithInput(const char *word, const char *typed, size_t typed_len, int x, int y)
{
    size_t word_len = strlen(word);
    int char_x = x;
    size_t max_extra = 5;
    
    for (size_t c = 0; c < word_len; c++)
    {
        char ch[2] = {word[c], '\0'};
        Color color = gui_config->text_color;
        
        if (c < typed_len)
        {
            if (typed[c] == word[c])
                color = gui_config->text_correct_color;
            else
                color = gui_config->text_wrong_color;
        }
        
        DrawText(ch, char_x, y, gui_config->font_size, color);
        char_x += MeasureText(ch, gui_config->font_size) + 2;
    }
    
    size_t extra = (typed_len > word_len) ? typed_len - word_len : 0;
    if (extra > max_extra) extra = max_extra;
    
    for (size_t c = 0; c < extra; c++)
    {
        char ch[2] = {typed[word_len + c], '\0'};
        DrawText(ch, char_x, y, gui_config->font_size, gui_config->text_wrong_color);
        char_x += MeasureText(ch, gui_config->font_size) + 2;
    }
}

static void
DrawCursor(int x, int y)
{
    double time = GetTime();
    if ((int)(time * 2) % 2 == 0)
    {
        DrawRectangle(x, y, 3, gui_config->font_size, gui_config->cursor_color);
    }
}

static void
DrawTestAreaInfinite(TypingSession *ts, size_t first_word_idx, const char *current_input, size_t input_len, bool *word_status, size_t status_count)
{
    int max_width = gui_config->screen_width - 200;
    int start_x = 100;
    int start_y = gui_config->words_y_position;
    
    int x = start_x;
    int y = start_y;
    int row = 0;
    int words_in_row = 0;
    
    size_t total_words = ts->count * 2;
    
    for (size_t word_idx = first_word_idx; word_idx < total_words && row < 3; word_idx++)
    {
        const char *word = GetWordAt(ts, word_idx);
        if (!word) break;
        
        int word_width = GetWordDisplayWidth(word);
        
        if (x + word_width > start_x + max_width && words_in_row > 0)
        {
            row++;
            x = start_x;
            y = start_y + row * gui_config->line_height;
            words_in_row = 0;
            if (row >= 3) break;
        }
        
        if (word_idx < ts->curr_word_idx)
        {
            bool was_correct = (word_idx < status_count) ? word_status[word_idx] : true;
            Color color = was_correct ? gui_config->text_correct_color : gui_config->text_wrong_color;
            DrawWord(word, x, y, color);
        }
        else if (word_idx == ts->curr_word_idx)
        {
            DrawCurrentWordWithInput(word, current_input, input_len, x, y);
            
            int cursor_x = x;
            size_t word_len = strlen(word);
            size_t cursor_pos = input_len;
            if (cursor_pos > word_len + 5) cursor_pos = word_len + 5;
            
            for (size_t c = 0; c < cursor_pos; c++)
            {
                char ch[2];
                if (c < word_len) ch[0] = word[c];
                else ch[0] = current_input[c];
                ch[1] = '\0';
                cursor_x += MeasureText(ch, gui_config->font_size) + 2;
            }
            DrawCursor(cursor_x, y);
        }
        else
        {
            DrawWord(word, x, y, gui_config->text_color);
        }
        
        x += word_width + gui_config->word_spacing;
        words_in_row++;
    }
}

static void
DrawCountdown(TypingSession *ts)
{
    int remaining = test_durations.durations[test_durations.curr_duration] - (int)GetElapsedTime(ts);
    if (remaining < 0) remaining = 0;
    
    char time_text[32];
    snprintf(time_text, sizeof(time_text), "%d", remaining);
    int text_width = MeasureText(time_text, gui_config->font_size_stats);
    int time_x = (gui_config->screen_width - text_width) / 2;
    
    DrawText(time_text, time_x, gui_config->stats_y_position, 
             gui_config->font_size_stats, gui_config->stats_color);
}

static void
DrawResultScreen(int wpm, float accuracy)
{
    char wpm_text[64];
    snprintf(wpm_text, sizeof(wpm_text), "%d", wpm);
    int wpm_width = MeasureText(wpm_text, 120);
    int center_x = (gui_config->screen_width - wpm_width) / 2;
    int center_y = gui_config->screen_height / 2 - 80;
    
    DrawText("wpm", center_x - 80, center_y + 20, 40, gui_config->stats_color);
    DrawText(wpm_text, center_x, center_y, 120, gui_config->text_correct_color);
    
    char acc_text[64];
    snprintf(acc_text, sizeof(acc_text), "%.1f%%", accuracy * 100.0f);
    int acc_width = MeasureText(acc_text, 60);
    int acc_x = (gui_config->screen_width - acc_width) / 2;
    
    DrawText("acc", acc_x - 60, center_y + 150, 30, gui_config->stats_color);
    DrawText(acc_text, acc_x, center_y + 140, 60, gui_config->text_correct_color);
    
    const char *restart_text = "press TAB to restart";
    int restart_width = MeasureText(restart_text, 20);
    DrawText(restart_text, (gui_config->screen_width - restart_width) / 2, 
             center_y + 250, 20, gui_config->text_color);
}

static void
DrawFooter(void)
{
    const char *tab_text = "[Tab to reset]";
    const char *esc_text = "[Esc to quit]";
    const char *repo_text = "[Source code: https://github.com/SalvatoreBia/typit.git]";
    
    int footer_y = gui_config->screen_height - 40;
    int pad = 20;
    
    int tab_width = MeasureText(tab_text, 16);
    int esc_width = MeasureText(esc_text, 16);
    
    DrawText(repo_text, pad, footer_y, 16, gui_config->text_completed_color);
    DrawText(esc_text, gui_config->screen_width - pad - tab_width - 10 - esc_width, footer_y, 16, gui_config->text_completed_color);
    DrawText(tab_text, gui_config->screen_width - pad - tab_width, footer_y, 16, gui_config->text_completed_color);
}

static size_t
CalculateFirstWordIdx(TypingSession *ts, size_t current_first)
{
    int max_width = gui_config->screen_width - 200;
    int start_x = 100;
    int x = start_x;
    int words_in_first_row = 0;
    size_t total_words = ts->count * 2;
    
    for (size_t i = current_first; i < total_words; i++)
    {
        const char *word = GetWordAt(ts, i);
        if (!word) break;
        
        int word_width = GetWordDisplayWidth(word);
        
        if (x + word_width > start_x + max_width && words_in_first_row > 0)
        {
            if (i <= ts->curr_word_idx)
                return i;
            break;
        }
        
        x += word_width + gui_config->word_spacing;
        words_in_first_row++;
    }
    
    return current_first;
}

void
RunTypingTest(TypingSession *ts, TestVocabulary *voc)
{
    if (ts == NULL || voc == NULL || gui_config == NULL)
    {
        ErrorMsg("Invalid parameters for RunTypingTest");
        return;
    }

    current_session = ts;
    current_vocabulary = voc;

    char input_buffer[256] = {0};
    size_t input_len = 0;
    
    bool word_status[512] = {0};
    size_t word_status_count = 0;
    
    size_t first_word_idx = 0;
    bool test_finished = false;
    int final_wpm = 0;
    float final_accuracy = 0.0f;

    while (!WindowShouldClose())
    {
        if (ts->is_running)
        {
            ts->elapsed_cache = GetTime() - ts->start_time;
            
            int duration = test_durations.durations[test_durations.curr_duration];
            if (ts->elapsed_cache >= duration)
            {
                ts->is_running = false;
                final_wpm = CalculateWPM(ts);
                final_accuracy = CalculateAccuracy(ts);
                test_finished = true;
            }
        }
        
        if (!test_finished)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if (!ts->is_running)
                {
                    ts->start_time = GetTime();
                    ts->is_running = true;
                }
                
                if (key == ' ')
                {
                    bool word_correct = false;
                    bool did_swap = HandleSpaceInput(ts, input_buffer, &input_len, &word_correct);
                    
                    if (word_status_count < 512)
                    {
                        word_status[word_status_count] = word_correct;
                        word_status_count++;
                    }
                    
                    if (did_swap)
                    {
                        first_word_idx = 0;
                        for (size_t i = 0; i < 512; i++) word_status[i] = false;
                        word_status_count = 0;
                    }
                    else
                    {
                        first_word_idx = CalculateFirstWordIdx(ts, first_word_idx);
                    }
                }
                else
                {
                    HandleCharInput(ts, (char)key, input_buffer, &input_len, 256);
                }
                
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                HandleBackspace(ts, input_buffer, &input_len);
            }
        }
        
        if (IsKeyPressed(KEY_TAB))
        {
            ResetTest(ts);
            first_word_idx = 0;
            input_len = 0;
            input_buffer[0] = '\0';
            for (size_t i = 0; i < 512; i++) word_status[i] = false;
            word_status_count = 0;
            test_finished = false;
            final_wpm = 0;
            final_accuracy = 0.0f;
        }

        BeginDrawing();
        ClearBackground(gui_config->bg_color);
        
        if (!test_finished)
        {
            DrawOptionBar();
            DrawTestAreaInfinite(ts, first_word_idx, input_buffer, input_len, word_status, word_status_count);
            DrawFooter();
            
            if (ts->is_running)
                DrawCountdown(ts);
        }
        else
        {
            DrawResultScreen(final_wpm, final_accuracy);
        }
        
        EndDrawing();
    }

    current_session = NULL;
    current_vocabulary = NULL;
}

bool
InitTestDurationList(TestDurationList *list)
{
    if (list == NULL)
    {
        ErrorMsg("Passed null pointer to function call");
        return false;
    }

    static int durations[] = {15, 30, 60, 120};
    size_t count = sizeof(durations) / sizeof(durations[0]);

    list->durations = malloc(count * sizeof(int));
    if (list->durations == NULL)
    {
        ErrorMsg("Failed to allocate memory for test duration list");
        return false;
    }

    for (size_t i = 0; i < count; i++)
    {
        list->durations[i] = durations[i];
    }

    list->count = count;
    list->curr_duration = 2;
    return true;
}

void
FreeTestDurationList(TestDurationList *list)
{
    if (list == NULL) return;
    if (list->durations)
    {
        free(list->durations);
        list->durations = NULL;
    }
    list->count = 0;
    list->curr_duration = 0;
}