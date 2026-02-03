#include <string.h>
#include <stdbool.h>
#include "parser.h"

#define DEFAULT_REGISTRY  "docker.io"
#define DEFAULT_NAMESPACE "library"
#define DEFAULT_TAG       "latest"

static bool is_registry(const char *s) {
    return strchr(s, '.') || strchr(s, ':') || strcmp(s, "localhost") == 0;
}

static bool valid_digest(const char *d) {
    return strncmp(d, "sha256:", 7) == 0 && strlen(d) == 71;
}

oci_error_t parse_oci_ref(const char *input, struct oci_ref_t *out) {
    if (!input || !*input)
        return OCI_ERR_EMPTY;

    if (strlen(input) >= 1024)
        return OCI_ERR_TOO_LONG;

    char buf[1024];
    strncpy(buf, input, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    memset(out, 0, sizeof(*out));

    /* ---- Step 1: digest ---- */
    char *digest = strrchr(buf, '@');
    if (digest) {
        if (!valid_digest(digest + 1))
            return OCI_ERR_INVALID_DIGEST;

        strncpy(out->digest, digest + 1, sizeof(out->digest));
        *digest = '\0';
    }

    /* ---- Step 2: tag (':' after last '/') ---- */
    char *last_slash = strrchr(buf, '/');
    char *tag = strrchr(buf, ':');

    if (tag && (!last_slash || tag > last_slash)) {
        if (*(tag + 1) == '\0')
            return OCI_ERR_INVALID_REFERENCE;

        strncpy(out->tag, tag + 1, sizeof(out->tag));
        *tag = '\0';
    } else {
        strcpy(out->tag, DEFAULT_TAG);
    }

    /* ---- Step 3: split path ---- */
    char *parts[10];
    int count = 0;

    char *tok = strtok(buf, "/");
    while (tok && count < 10) {
        parts[count++] = tok;
        tok = strtok(NULL, "/");
    }

    if (count == 0)
        return OCI_ERR_INVALID_REFERENCE;

    int idx = 0;

    /* ---- Step 4: registry detection ---- */
    if (is_registry(parts[0])) {
        strncpy(out->registry, parts[0], sizeof(out->registry));
        idx = 1;
    } else {
        strcpy(out->registry, DEFAULT_REGISTRY);
    }

    /* ---- Step 5: namespace + repo ---- */
    int remaining = count - idx;

    if (remaining == 1) {
        if (strcmp(out->registry, DEFAULT_REGISTRY) == 0) {
            strcpy(out->namespace, DEFAULT_NAMESPACE);
            strncpy(out->repository, parts[idx], sizeof(out->repository));
        } else {
            return OCI_ERR_INVALID_REFERENCE;
        }
    } else if (remaining == 2) {
        strncpy(out->namespace, parts[idx], sizeof(out->namespace));
        strncpy(out->repository, parts[idx + 1], sizeof(out->repository));
    } else {
        return OCI_ERR_INVALID_REFERENCE;
    }

    return OCI_OK;
}

