# Scan RP2040 SDK board header file and extract variables for use in makefiles
BEGIN {
	FS = "#define "
}
/^#define +[^_]+/ {
	n = index($2, " ")
	if (n == 0) {
		vars[$2] = ""
	} else {
		name = substr($2, 1, n-1)
		value = substr($2, n + 1)
		if (index(value, " ") == 0) {
			vars[name] = value
		} else {
			vars[name] = "$$((" value "))"
		}
	}
}
END {
	for (v in vars) {
		printf "%s=%s\\n", v, vars[v]
	}
	printf "RP2040_BOARD_VARS := "
	for (v in vars) {
		printf v " "
	}
}
