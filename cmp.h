#include "options.h"

int
sortHelpArgs(const void *a, const void *b);

int
sortHelpFTS(const FTSENT **a, const FTSENT **b);

int
sortBySize(const FTSENT **a, const FTSENT **b);

int
sortByModifiedTime(const FTSENT **a, const FTSENT **b);