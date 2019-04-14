#include <stdlib.h>
#include <stdio.h>
#include <yuarel.h>

int main(void)
{
	int p;
	struct yuarel url;
	char *parts[3];
	struct yuarel_param params[3];
	char url_string[] = "http://localhost:8989/path/to/test?query=yes&param1=no#frag=1";

	if (-1 == yuarel_parse(&url, url_string)) {
		fprintf(stderr, "Could not parse url!\n");
		return 1;
	}

	printf("Struct values:\n");
	printf("\tscheme:\t\t%s\n", url.scheme);
	printf("\thost:\t\t%s\n", url.host);
	printf("\tport:\t\t%d\n", url.port);
	printf("\tpath:\t\t%s\n", url.path);
	printf("\tquery:\t\t%s\n", url.query);
	printf("\tfragment:\t%s\n", url.fragment);

	if (3 != yuarel_split_path(url.path, parts, 3)) {
		fprintf(stderr, "Could not split path!\n");
		return 1;
	}

	printf("\nPath parts: '%s', '%s', '%s'\n\n", parts[0], parts[1], parts[2]);

	printf("Query string parameters:\n");

	p = yuarel_parse_query(url.query, '&', params, 3);
	while (p-- > 0) {
		printf("\t%s: %s\n", params[p].key, params[p].val);
	}
}
