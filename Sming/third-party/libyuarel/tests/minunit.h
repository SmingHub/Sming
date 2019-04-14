#define KNRM  "\x1B[0m"
#define KBLU  "\x1B[34m"
#define KGRN  "\x1B[32m"
#define KERR  "\x1B[5;31;50m"

/* macro to print out the header for a new group of tests */
#define mu_group(name) printf("%s • %s%s\n", KBLU, name, KNRM)

/* macro for asserting a statement */
#define mu_assert(message, test) do { \
		if (!(test)) { \
			printf("\t%s× %s%s\n", KERR, message, KNRM); \
			return message; \
		} \
		printf("\t%s• %s%s\n", KGRN, message, KNRM); \
	} while (0)

/* macro for asserting a statement without printing it unless it is a failure */
#define mu_silent_assert(message, test) do { \
		if (!(test)) { \
			printf("\t\t%s× %s%s\n", KERR, message, KNRM); \
			return message; \
		} \
	} while (0)

/* run a test function and return result */
#define mu_run_test(test) do { \
		char *message = test(); tests_run++; \
		if (message) { return message; } \
	} while (0)
