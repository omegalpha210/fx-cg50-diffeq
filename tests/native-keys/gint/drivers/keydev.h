#ifndef TEST_KEYDEV_H
#define TEST_KEYDEV_H
typedef struct {int unused;} keydev_t;
typedef struct {int enabled;int (*repeater)(int,int,int);} keydev_transform_t;
keydev_t *keydev_std(void);
keydev_transform_t keydev_transform(keydev_t *device);
void keydev_set_transform(keydev_t *device,keydev_transform_t transform);
#endif
