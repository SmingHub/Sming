/*
 * Sming version information
 *
 * https://github.com/semver/semver/blob/master/semver.md
 *
 */

#define SMING_MAJOR_VERSION 4
#define SMING_MINOR_VERSION 0
#define SMING_PATCH_VERSION 0
#define SMING_PRE_RELEASE ""

#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

// Version string, e.g. 3.8.0-dev
#define SMING_VERSION                                                                                                  \
	MACROQUOTE(SMING_MAJOR_VERSION)                                                                                    \
	"." MACROQUOTE(SMING_MINOR_VERSION) "." MACROQUOTE(SMING_PATCH_VERSION) SMING_PRE_RELEASE
