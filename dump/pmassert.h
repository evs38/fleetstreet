/* PMASSERT.H */

void _PMAssert(const char *pCond, const char *pFile, int Line,
               const char *pComment);

#define PMASSERT(cond,comm) do { \
                                 if (!(cond)) \
                                    _PMAssert(#cond, __FILE__, __LINE__, \
                                              comm); \
                            } while(0)

/* End of PMASSERT.H */
