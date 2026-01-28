#include <stdint.h>
#include <string.h>
#include <zephyr/llext/buf_loader.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/llext/llext.h>
#include <zephyr/logging/log.h>
#include "syscalls/kernel.h"
#include "zephyr/app_memory/mem_domain.h"
#include "zephyr/kernel.h"
#include "llextc.h"
#include "llextc_priv.h"
#include "buff.h"
#include "zephyr/sys/libc-hooks.h"
#include <zephyr/sys/uuid.h>
#include "llextc_container_api.h"
#include "llextc_utils.h"
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(llextc_priv);

// Mailbox
K_MBOX_DEFINE(llextc_mailbox);

struct k_thread llextc_main_thread;
K_THREAD_STACK_DEFINE(llextc_main_thread_stack, 1024);

char llext_msgq_buffer[10 * sizeof(struct llextc_message)];
struct k_msgq llextc_msgq;

/*
 * MEMORY DOMAIN
*/
k_mem_domain_t llextc_domain;
uint8_t __aligned(CODE_BUFFER_SIZE) llextc_code_buff[CODE_BUFFER_SIZE];
K_MEM_PARTITION_DEFINE(llextc_code_partition, llextc_code_buff, sizeof(llextc_code_buff), 
    				   K_MEM_PARTITION_P_RX_U_RX); // Importante que sea RX


// ZBUS
ZBUS_CHAN_DEFINE(llextc_zbus_chan, 
	struct llextc_zbus_msg,
	NULL,
	NULL,
	ZBUS_OBSERVERS_EMPTY,
	ZBUS_MSG_INIT(.code = -1)
);


// TEMPORAL
struct llextc_image test_image = {
	.name = "imagen_prueba",
	.status = LLEXTC_IMAGE_STATUS_READY,
	.code_buffer = ext1_buf
};

ZBUS_MSG_SUBSCRIBER_DEFINE(sub1);
ZBUS_MSG_SUBSCRIBER_DEFINE(sub2);
ZBUS_MSG_SUBSCRIBER_DEFINE(sub3);
ZBUS_MSG_SUBSCRIBER_DEFINE(sub4);

const struct zbus_observer *observers[] = {&sub1, &sub2, &sub3, &sub4};

// For now, keep stopped containers in the array
// to save the state. TODO: store container state
// in flash?
//K_MUTEX_DEFINE(slot_mutex);
struct llextc_container_slot container_slots[MAX_CONTAINERS];
int num_containers = 0;


#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/container_exit_mrsh.c>
void z_vrfy_container_exit() {
	return z_impl_container_exit();
}
#endif /* CONFIG_USERSPACE */

void z_impl_container_exit(void) {

	struct llextc_message msg = {
		.type = LLEXTC_MSG_CONTAINER_END,
		.arg1 = k_current_get()
	};
	k_msgq_put(&llextc_msgq, &msg, K_FOREVER);
}

int llextc_init_priv() {

    k_msgq_init(&llextc_msgq, llext_msgq_buffer, sizeof(struct llextc_message), 10);
	struct k_mem_partition *user_parts[] = {
		&z_libc_partition,
		&z_malloc_partition,
		&llextc_code_partition
	};

	for (int i = 0; i < MAX_CONTAINERS; i++) {
		container_slots[i].obs = observers[i];
	}

	if (k_mem_domain_init(&llextc_domain, ARRAY_SIZE(user_parts), user_parts)) {
		LOG_ERR("Failed to init llextc memory domain");
		return 1;
	}

	if (llext_heap_init(llextc_code_buff, sizeof(llextc_code_buff))) {
		LOG_ERR("Failted to init llext heap for llextc");
		return 1;
	}

	k_thread_create(&llextc_main_thread, llextc_main_thread_stack, 
	             sizeof(llextc_main_thread_stack), llextc_main_loop, 
	             NULL, NULL, NULL, 2, 0, K_NO_WAIT);

	return 0;
}

void llextc_main_loop() {

    LOG_DBG("Entered llext main loop");
	while (1) {
		
		struct llextc_message msg;
		int ret = k_msgq_get(&llextc_msgq, &msg, K_FOREVER);
		if (ret != 0) {
			LOG_WRN("k_msqg_get returned with error");
			continue;
		}
		
		if (msg.type == LLEXTC_MSG_RUN_IMAGE) {
			char *name = (char* ) msg.arg1;
			char *image = (char*) msg.arg2;
		    LOG_INF("Run image msg received: %s", image);
			llextc_run_msg_handler(name, image);
		}

		if (msg.type == LLEXTC_MSG_CONTAINER_END) {
			LOG_INF("Received end container message");
			k_tid_t tid = msg.arg1;
			llextc_container_end_handler(tid);
		}
    }
}

int llextc_container_end_handler(k_tid_t tid) {

	struct llextc_container_slot *slot = get_slot_by_tid(tid);
	k_thread_join(tid, K_FOREVER);
	k_thread_stack_free(slot->stack);
	slot->status = LLEXTC_CONTAINER_STATUS_FINISHED;
	zbus_chan_rm_obs(&llextc_zbus_chan, slot->obs, K_FOREVER);
	LOG_INF("Container %s finished", slot->name);
	return 0;
}

// Message handlers.
//
int llextc_run_msg_handler(char *container_name, char *image_name) {

	int ret = 0;
	//k_mutex_lock(&slot_mutex, K_FOREVER);
	if (num_containers == MAX_CONTAINERS) {
		LOG_ERR("Maximum number of containers reached");
		ret = LLEXTC_ERROR_MAX_CONTAINERS;
		goto name_free;
	}

	if (strcmp(image_name, test_image.name)) {
		LOG_ERR("Unknown image: %s", image_name);
		ret = LLEXTC_ERROR_UNKNOWN_IMAGE;
		goto name_free;
	}
	
	int free_ind = 0;
	while (container_slots[free_ind].used)
		free_ind++;

	struct llextc_container_slot *free_slot = &container_slots[free_ind];

	struct llext_buf_loader ext1_loader = LLEXT_TEMPORARY_BUF_LOADER(ext1_buf, sizeof(ext1_buf));
	struct llext_load_param load_params = LLEXT_LOAD_PARAM_DEFAULT;
	// TODO: hacer unload cuando se borre el contenedor.
	if (llext_load(&ext1_loader.loader, container_name, &free_slot->llext, &load_params)) {
		LOG_ERR("Failed to load extension of image %s", image_name);
		ret = LLEXTC_ERROR_UNKNOWN;
		goto name_free;
	}
	
	free_slot->name = container_name;
	free_slot->image = &test_image;
	if (start_container(free_slot)) {
		LOG_ERR("Failed to start container");
		ret = LLEXTC_ERROR_UNKNOWN;
		goto extension_unload;
	}
	free_slot->used = 1;
	goto end;

// Error handling.
// Cuidado, que si los nombres no están en el heap esto peta.
extension_unload:
	llext_unload(&free_slot->llext);

name_free:
	k_free(container_name);
end:
	k_free(image_name);
	//k_mutex_unlock(&slot_mutex);
	return ret;
}

void thread_entry(void *ext_entry, void *p2, void *p3) {

	//struct llextc_container_slot *slot = (struct llextc_container_slot*) p1;
	LOG_INF("Entered user thread (%p)", k_current_get());

	void (*ext_start)() = ext_entry; 
	ext_start();

	//arch_syscall_invoke0(uintptr_t call_id)
	container_exit();
	LOG_INF("Thread acabando");
}

// Must be called with mutex locked.
int start_container(struct llextc_container_slot *slot) {
	
	void (*ext_start)() = llext_find_sym(&slot->llext->exp_tab, "start");
	if (ext_start == NULL) {
		LOG_ERR("Failed to find entrypoint for container %s", slot->name);
		return LLEXTC_ERROR_INVALID_IMAGE;
	}

	struct llext_container_env *container_env = (struct llext_container_env*) llext_find_sym(&slot->llext->exp_tab, "env");
	if (container_env != NULL) {
		LOG_INF("Setting envs for container %s", slot->name);
		strncpy(container_env->name, slot->name, sizeof(container_env->name));
	}

	if (zbus_chan_add_obs(&llextc_zbus_chan, slot->obs, K_FOREVER)) {
		LOG_ERR("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
		return 1;
	}

	k_thread_stack_t *stack = k_thread_stack_alloc(1024, 0);
	if (stack == NULL) {
	    LOG_ERR("Failed to allocate stack for container %s", slot->name);
		return 1;
	}

	slot->stack = stack;

	k_tid_t tid = k_thread_create(&slot->thread, stack, 1024, 
							   thread_entry, ext_start, NULL, NULL, 8, K_USER, K_FOREVER);

	if (k_mem_domain_add_thread(&llextc_domain, tid)) {
		LOG_INF("Failed to add thread to llextc memory domain");
		return 1;
	}

	k_thread_start(tid);
	slot->status = LLEXTC_CONTAINER_STATUS_RUNNING;
	LOG_INF("Started container thread with id %p", tid);
	return 0;
}

struct llextc_container_slot *get_container_slots() {
	return container_slots;
}

int llextc_send_message(char *dst, void *msg, uint32_t msg_size) {

	LOG_INF("thread %p enter send_message syscall", k_current_get());
	struct llextc_container_slot *dst_slot = get_slot_by_container_name(dst);
	if (dst_slot == NULL) {
		LOG_ERR("send_message: invalid destination 1");
		return LLEXTC_ERROR_INVALID_DESTINATION;
	}
	if (dst_slot->status != LLEXTC_CONTAINER_STATUS_RUNNING && dst_slot->status != LLEXTC_CONTAINER_STATUS_PAUSED) {
		LOG_ERR("send_message: invalid destination 2");
		return LLEXTC_ERROR_INVALID_DESTINATION;
	}
	
	if (msg_size > LLEXTC_CONTAINER_MESSAGE_MAX_SIZE) {
		LOG_ERR("send_message: message too big");
		return LLEXTC_ERROR_NOT_ENOUGH_SPACE;
	}

	k_tid_t dst_tid = &dst_slot->thread;
	char *msg_buff = k_malloc(msg_size);
	memcpy(msg_buff, msg, msg_size);
	struct k_mbox_msg mbox_msg = {
		.info = (uint32_t) k_current_get(), // Sender tid in the info field
		.tx_data = msg_buff,
		.size = msg_size,
		.tx_target_thread = dst_tid
	};

	LOG_INF("Sending message from thread %p, to thread %p", k_current_get(), dst_tid);
	k_mbox_async_put(&llextc_mailbox, &mbox_msg, NULL);
	LOG_INF("lkjlkjlkjlkjlkj");
	return LLEXTC_OK;
}


int llextc_receive_message(uint8_t *buff, char *sender, k_timeout_t timeout) {

	LOG_INF("thread %p enter receive_message syscall", k_current_get());
	struct k_mbox_msg msg = {
		.info = 100,
		.size = LLEXTC_CONTAINER_MESSAGE_MAX_SIZE,
		.rx_source_thread = K_ANY
	};

	int ret = k_mbox_get(&llextc_mailbox, &msg, buff, timeout);
	if (ret)
		return 0;

	LOG_INF("FALLO MIO\n");
	LOG_INF("MSG info received: %p", (k_tid_t) msg.info);
	struct llextc_container_slot *slot = get_slot_by_tid((k_tid_t) msg.info);
	strcpy(sender, slot->name);
	return msg.size;	
}

int llextc_chan_pub(struct llextc_zbus_msg *msg, k_timeout_t timeout) {
	LOG_INF("LLEXTCPUB MSG: %s", msg->msg);
	return zbus_chan_pub(&llextc_zbus_chan, msg, timeout);
}

int llextc_chan_read(struct llextc_zbus_msg *msg, k_timeout_t timeout) {

	struct llextc_container_slot *slot = get_slot_by_tid(k_current_get());
	const struct zbus_channel *chan;
	zbus_sub_wait_msg(slot->obs, &chan, msg, K_FOREVER);
	return zbus_chan_read(&llextc_zbus_chan, msg, K_FOREVER);
	//return zbus_sub_wait_msg(slot->obs, &chan, msg, K_FOREVER);
}
