#ifndef MOCK_FS_H
#define MOCK_FS_H
typedef struct { int (*close)(void *); } fs_descriptor_type_t;
typedef struct { const fs_descriptor_type_t *type; void *data; } fs_descriptor_t;
const fs_descriptor_t *fs_get_descriptor(int fd);
void fs_free_descriptor(int fd);
#endif
