#include <stdio.h>
#include <libfdt_env.h>
#include <fdt.h>
#include <libfdt.h>
#include <libfdt_internal.h>
#include <util.h>
#include <assert.h>

int fdt_get_node(const void *fdt, const char *path)
{
    int node = fdt_path_offset(fdt, path);
    if (node < 0) {
        printf("EE: Cannot find \"%s\" node: %d\n", path, node);
        return -1;
    }
    return node;
}

int fdt_getprop_path_val(const void *fdt, const char *path, const char *name, void **out, int *len)
{
    int node = fdt_get_node(fdt, path);
    assert(node>=0);
    
    *out = (void *)fdt_getprop(fdt, node, name, len);
    if (!*out) {
        printf("EE: Cannot find \"%s\"\n", name);
        return -1;
    }

    return 0;
}

int fdt_getprop_path_u32(const void *fdt, const char *path, const char *name, uint32_t *out)
{
    int node = fdt_get_node(fdt, path);
    assert(node>=0);
    int len;
    const fdt32_t *val = fdt_getprop(fdt, node, name, &len);
    assert(len==sizeof(uint32_t));
    
    *out = fdt32_to_cpu(*val);
    return 0;
}

int fdt_setprop_path_val(void *fdt, const char *path, const char *name,
                         const void *val, int len)
{
    int node = fdt_get_node(fdt, path);
    assert(node>=0);
    return fdt_setprop(fdt, node, name, val, len);
}

int fdt_setprop_path_u32(void *fdt, const char *path,
                         const char *name, const uint32_t *val)
{
    uint32_t v = cpu_to_fdt32(*val);
    return fdt_setprop_path_val(fdt, path, name, &v, sizeof(uint32_t));
}