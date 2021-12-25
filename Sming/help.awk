# Extract commented target information from makefiles and display
# Based on code from https://suva.sh/posts/well-documented-makefiles/
BEGIN {
    FS = "(:.*##)|(##@)"
    printf "Usage:\n  make \033[1;36m<target>\033[0m\n"
} /^##@/ {
    group = $2
    groups[group] = group
} /^[a-zA-Z0-9_-]+:.*?##/ {
    targets[$1, group] = $2
}
END {
    for (g in groups) {
        printf "\n\033[1m%s\033[0m\n", g
        for (t in targets) {
            split(t, sep, SUBSEP)
            if (sep[2] == g)
                printf "  \033[1;36m%-20s\033[0m %s\n", sep[1], targets[t]
        }
    }
}
