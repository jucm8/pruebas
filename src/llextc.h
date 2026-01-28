#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAX_CONTAINERS 4
#define CODE_BUFFER_SIZE 64 * MAX_CONTAINERS * 1024
#define MAX_IMAGE_SIZE CODE_BUFFER_SIZE / MAX_CONTAINERS

typedef enum {
    LLEXTC_CONTAINER_STATUS_UNKNOWN,
    LLEXTC_CONTAINER_STATUS_NOT_EXISTS,
    LLEXTC_CONTAINER_STATUS_NO_IMAGE,
    LLEXTC_CONTAINER_STATUS_RUNNING,
    LLEXTC_CONTAINER_STATUS_STOPPED,
    LLEXTC_CONTAINER_STATUS_PAUSED,
    LLEXTC_CONTAINER_STATUS_FINISHED
}llextc_container_status;

typedef enum {
    LLEXTC_IMAGE_STATUS_NO_BINARY,
    LLEXTC_IMAGE_SATUS_DOWNLOADING,
    LLEXTC_IMAGE_STATUS_READY
}llextc_image_status;

struct llextc_image {
    char *name;
    // TODO int num_containers;
    uint8_t *code_buffer;
    llextc_image_status status;
};


/**
* @brief Initialize the llextc system.
 *
 * ABC
 *
 * @retval 0 if successful
 * @retval 1 otherwise
 */
int llextc_init();
int llextc_container_run(char *container_name, char *image_name);
int llextc_container_get_status(char *name);



