#include <stdbool.h>

typedef enum {
    OCI_OK = 0,
    OCI_ERR_EMPTY,
    OCI_ERR_TOO_LONG,
    OCI_ERR_INVALID_DIGEST,
    OCI_ERR_INVALID_REFERENCE
} oci_error_t;

struct oci_ref_t {
    char registry[256];
    char namespace[256];
    char repository[256];
    char tag[128];
    char digest[128];
};

#define DEFAULT_REGISTRY  "docker.io"
#define DEFAULT_NAMESPACE "library"
#define DEFAULT_TAG       "latest"

//static bool is_registry(const char *s);
//static bool valid_digest(const char *d);
oci_error_t parse_oci_ref(const char *input, struct oci_ref_t *out);
