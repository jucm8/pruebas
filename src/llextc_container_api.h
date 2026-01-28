#pragma once
#include "zephyr/kernel/thread.h"
#include "zephyr/sys/clock.h"
#include <stdint.h>

#define LLEXTC_CONTAINER_MESSAGE_MAX_SIZE 32

typedef enum {
    LLEXTC_OK,
    LLEXTC_ERROR_UNKNOWN,
    LLEXTC_ERROR_MAX_CONTAINERS,
    LLEXTC_ERROR_UNKNOWN_IMAGE,
    LLEXTC_ERROR_INVALID_IMAGE,
    LLEXTC_ERROR_ALREADY_EXISTS,
    LLEXTC_ERROR_INVALID_DESTINATION,
    LLEXTC_ERROR_TIMEOUT,
    LLEXTC_ERROR_NOT_ENOUGH_SPACE
}llextc_ret_status;

struct llext_container_env {
    char name[32];
    // TODO
    // int argc;
    // char *argv[];
};

struct llextc_zbus_msg {
    int code;
    uint8_t msg[64];
};

__syscall int send_message(char *dst, void *msg, uint32_t msg_size);
__syscall int receive_message(uint8_t *buff, char *sender, k_timeout_t timeout);

__syscall int zbuss_publish(struct llextc_zbus_msg *msg, k_timeout_t timeout);
__syscall int zbuss_receive(struct llextc_zbus_msg *msg, k_timeout_t timeout);
#include <zephyr/syscalls/llextc_container_api.h>
