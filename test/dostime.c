#include "dostime.h"
#include "util.h"

int main() {
    time_t ts = getTS();

    printf("%d\n", getDOSTime(ts));
    printf("%d\n", getDOSDate(ts));

    return 0;
}
