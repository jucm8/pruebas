#pragma once

#include "container_api/c_api.h"
#include "zephyr/kernel.h"
#include "zephyr/llext/llext.h"
#include "llextc.h"
#include "zephyr/zbus/zbus.h"

typedef enum {
    LLEXTC_MSG_RUN_IMAGE,
    LLEXTC_MSG_STOP_CONTAINER,
    LLEXTC_MSG_PAUSE_CONTAINER,
    LLEXTC_MSG_CONTAINER_END
}llextc_message_type;

struct llextc_message {
    llextc_message_type type;
    void *arg1;
    void *arg2;
    void *arg3;
};

struct llextc_container_slot {
    int used;
    char *name;
    llextc_container_status status;
    k_thread_stack_t *stack;
    struct llext *llext;
    struct k_thread thread;
    struct llextc_image *image;
    const struct zbus_observer *obs;
    struct k_heap heap;
};



//extern struct llextc_container_slot container_slots[];

extern char llext_msgq_buffer[10 * sizeof(struct llextc_message)];
extern struct k_msgq llextc_msgq;

char *llextc_container_status_to_string(llextc_container_status status);
int llextc_run_msg_handler(char *container_name, char *image_name);
void llextc_main_loop();
void thread_entry(void *ext_entry, void *p1, void *p2);
int start_container(struct llextc_container_slot *slot);
int llextc_init_priv();
int llextc_container_end_handler(k_tid_t tid);

struct llextc_container_slot *get_container_slots();
int llextc_chan_pub(struct llextc_zbus_msg *msg, k_timeout_t timeout);
int llextc_chan_read(struct llextc_zbus_msg *msg, k_timeout_t timeout);
int llextc_send_message(char *dst, void *msg, uint32_t msg_size, k_timeout_t timeout);
int llextc_receive_message(uint8_t *buff, char *sender, k_timeout_t timeout);

__syscall void container_exit(void);
#include <zephyr/syscalls/llextc_priv.h>
