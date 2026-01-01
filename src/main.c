#include "file.h"
#include "typing_test.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>


int
main(void)
{
    TestVocabulary voc;
    if (!InitTestVocabulary(&voc, "en", TEST_SIMPLE200))
    {
        exit(1);
    }

    TypingSession ts;
    if (!InitTypingSession(&ts, &voc, 25))
    {
        exit(1);
    }

    fprintf(stdout, "test chunk size: %zu\n", ts.count);
    for (size_t i = 0; i < 25; ++i)
    {
        fprintf(stdout, "%zu. CURR = %s, NEXT = %s\n", i+1, ts.curr[i], ts.next[i]);
    }

    FreeTestVocabulary(&voc, false);
    return 0;
}