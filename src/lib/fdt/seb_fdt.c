#include <lib/misc/string.h>
#include <fdt/secure_boot.h>

static int fdt_magic(const struct fdt_header *fdt) {
    return fdt->magic;
}

static int fdt_version(const struct fdt_header *fdt) {
    return fdt->version;
}

static int fdt_last_comp_version(const struct fdt_header *fdt) {
    return fdt->last_comp_version;
}

static int fdt_size_dt_struct(const struct fdt_header *fdt) {
    return fdt->size_dt_struct;
}

static uint32_t fdt_is_eos(const char s, const char ext_eos) {
    return (s == 0) || (s == ext_eos);
}

/* 0 for same; others for diff */
/* both '\0' and '/' stands for end of string */
_noinline_ uint32_t fdt_strcmp(const char *s1, const char *s2, const char ext_eos) {
    while (*s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
    };

    return !(fdt_is_eos(*s1, ext_eos) && fdt_is_eos(*s2, ext_eos));
}

static const char *fdt_path_next_name(const char *s) {
    const char *p = s;
    do {
        if (*p == '/') return p;
    } while (*(++p));

    return NULL;
}

static const char *fdt_node_name(const uint32_t *token) {
    return (const char *)(&token[1]);
}

static char *fdt_prop_name(const uint32_t *token, const char *string) {
    return (char *)(token[2] + string);
}

static uint32_t fdt_is_token_head(const uint32_t *token) {
    return ((*token >> 4) == 0);
}

/* return pointer to the first token in the node. */
static const uint32_t *
fdt_find_node(const uint32_t *token, const char *string, const char *name) {
    uint32_t depth = 0;

    while (*token != FDT_END) {
        switch (*token) {
        case FDT_NODE_START:
            if (depth == 0) {
                if (fdt_strcmp(fdt_node_name(token), name, '/')) {
                    /* not found */
                    token++;
                    while (!fdt_is_token_head(++token));
                } else {
                    /* found */
                    token++;
                    while (!fdt_is_token_head(++token));
                    return token;
                }
            }
            depth++;
            break;
        case FDT_NODE_END:
            token++;
            depth--;
            break;
        case FDT_PROP:
            token += 3 + ((token[1]+3)/4);
            break;
        case FDT_NOP:
            token++;
            break;
        default:
            printf("EE: unknown token: %08x @ %p\n", *token, token);
            while (1);
        }
    }

    return NULL;
}

/* return pointer to property value */
static const uint32_t *
fdt_find_prop(const uint32_t *token, const char *string, const char *name) {
    uint32_t depth = 0;

    while (*token != FDT_END) {
        switch (*token) {
        case FDT_NODE_START:
            token++;
            while (!fdt_is_token_head(++token));
            depth++;
            break;
        case FDT_NODE_END:
            if (depth) {
                token++;
                depth--;
            } else {
                return NULL;
            }
            break;
        case FDT_PROP:
            if (depth == 0) {
                if (fdt_strcmp(fdt_prop_name(token, string), name, (const char)0) == 0) {
                    /* match */
                    return (const uint32_t *)&token[3];
                } else {
                    token += 3 + ((token[1]+3)/4);
                }
            } else {
                token += 3 + ((token[1]+3)/4);
            }
            break;
        case FDT_NOP:
            token++;
            break;
        default:
            printf("EE: unknown token: %08x @ %p\n", *token, token);
            while (1);
        }
    }

    return NULL;
}

//path, e.g., /images/uboot/data-size
void *fdt_get_prop(const struct fdt_header *fdt, const char *path) {
    const char *ptr = path;
    const char *next;

    const uint32_t *dt_node = fdt->dt_struct;
    char *dt_string = fdt->dt_strings;

    /* into root node */
    dt_node = fdt_find_node(dt_node, dt_string, "\0");

    while ((*ptr) == '/') {
        ptr++;
        if ((next = fdt_path_next_name(ptr))) {
            if ((dt_node = fdt_find_node(dt_node, dt_string, ptr))) {
                ptr = next;
                continue;
            } else {
                goto fdt_bad_path;
            }
        }

        if ((dt_node = fdt_find_prop(dt_node, dt_string, ptr))) {
            break;
        } else {
            goto fdt_bad_prop;
        }
    }

    return (void *)dt_node;

fdt_bad_path:
fdt_bad_prop:
    return NULL;
}

int fdt_check_header(const void *fdt) {
    if (fdt_magic(fdt) == FDT_MAGIC) {
        /* Complete tree */
        if (fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION)
            return -FDT_ERR_BADVERSION;
        if (fdt_last_comp_version(fdt) > FDT_LAST_SUPPORTED_VERSION)
            return -FDT_ERR_BADVERSION;
    } else if (fdt_magic(fdt) == FDT_SW_MAGIC) {
        /* Unfinished sequential-write blob */
        if (fdt_size_dt_struct(fdt) == 0)
            return -FDT_ERR_BADSTATE;
    } else {
        return -FDT_ERR_BADMAGIC;
    }

    return 0;
}

int fdt_get_sig_algo(const struct fdt_header *fdt, image_t *info) {
    char *hash_algo;
    char *crypto_algo;
    char *value;

    printf("II: getting FDT:/images/uboot/signature/algo...\n");
    hash_algo = (char *)fdt_get_prop(fdt, "/images/uboot/signature/algo");

    if (hash_algo) {
        if ((fdt_strcmp("sha1", hash_algo, ',')) == 0) {
            info->sig_hash_algo = SEB_HASH_SHA1;
            crypto_algo = hash_algo + 5;
        } else {
            crypto_algo = hash_algo;
        }

        if ((fdt_strcmp("rsa2048", crypto_algo, 0)) == 0) {
            info->sig_crypto_algo = SEB_CRYPTO_RSA2048;
            info->sig_len = 256;

            printf("II: getting FDT:/images/uboot/signature/value...\n");
            value = fdt_get_prop(fdt, "/images/uboot/signature/value");
            if (value) {
                memcpy((void *)&info->sig_signature, (void *)value, info->sig_len);
            } else {
                printf("WW: missing signature value\n");
            }
        }
    } else {
        info->sig_hash_algo = SEB_HASH_NONE;
        info->sig_crypto_algo = SEB_CRYPTO_NONE;
    }

    return 0;
}
