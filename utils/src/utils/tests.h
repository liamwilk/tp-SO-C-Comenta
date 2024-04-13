#define COLOR_BLUE "\x1b[34m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"

#define test(expr)                                                                                           \
    do                                                                                                       \
    {                                                                                                        \
        if (!(expr))                                                                                         \
        {                                                                                                    \
            printf(COLOR_RED "✗ Test fallo: %s, file %s, line %d\n" COLOR_RESET, #expr, __FILE__, __LINE__); \
            return 1;                                                                                        \
        }                                                                                                    \
        else                                                                                                 \
        {                                                                                                    \
            printf(COLOR_GREEN "✔ Test correcto: %s\n" COLOR_RESET, #expr);                                  \
        }                                                                                                    \
    } while (0)

void describe(const char *description);
