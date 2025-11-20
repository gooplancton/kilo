#include "ForthParser.h"

// int main(int argc, char **argv)
int main(void)
{
    ForthParser parser = {
        .string = "[-123 23.543 \"dio\" hey]"};

    ForthObject *obj = ForthParser__parse_list(&parser);

    ForthObject__print(obj);

    ForthObject__drop(obj);
    return 0;
}
