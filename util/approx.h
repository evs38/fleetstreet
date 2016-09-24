/* Approx.h */

void Fuzz_init(char *pattern, char *text, int degree);
void Fuzz_next(char **start, char **end, int *howclose);
void Fuzz_term(void);
