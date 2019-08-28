/*-
 * Copyright (c) <2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2019 by Keith Wiles @ intel.com */

#ifndef _PKTGEN_VERSION_H_
#define _PKTGEN_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PKTGEN_VER_PREFIX	"Pktgen"
#define PKTGEN_VER_CREATED_BY	"Keith Wiles"

#define STRINGIFY(x)	#x
#define TOSTRING(x)	STRINGIFY(x)

#define PKTGEN_VER_YEAR		__YEAR
#define PKTGEN_VER_MONTH	__MONTH
#define PKTGEN_VER_MINOR	__MINOR
#define PKTGEN_VER_SUFFIX	__SUFFIX
#define PKTGEN_VER_RELEASE	__RELEASE

#define PKTGEN_VERSION		pktgen_version_str()

static inline const char *
pktgen_version_str(void)
{
	static char version[64];

	if (version[0] != 0)
		return version;
	if (strlen(PKTGEN_VER_SUFFIX) == 0)
		snprintf(version, sizeof(version), "%s %d.%02d.%d",
				PKTGEN_VER_PREFIX,
				PKTGEN_VER_YEAR,
				PKTGEN_VER_MONTH,
				PKTGEN_VER_MINOR);
	else
		snprintf(version, sizeof(version), "%s %d.%02d.%d%s%d",
				PKTGEN_VER_PREFIX,
				PKTGEN_VER_YEAR,
				PKTGEN_VER_MONTH,
				PKTGEN_VER_MINOR,
				(strlen(PKTGEN_VER_SUFFIX) != 0)? "-rc" : TOSTRING(PKTGEN_VER_SUFFIX),
				PKTGEN_VER_RELEASE);
	return version;
}

#ifdef __cplusplus
}
#endif

#endif /* PKTGEN_VERSION_H_ */
