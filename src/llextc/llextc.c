#include <string.h>
#include <zephyr/kernel.h>
#include <syscalls/kernel.h>
#include <zephyr/sys/uuid.h>
#include <zephyr/llext/buf_loader.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/llext/llext.h>
#include <zephyr/logging/log.h>
#include "llextc.h"
#include "llextc_priv.h"

LOG_MODULE_REGISTER(llextc);


int llextc_init() {
	return llextc_init_priv();
}

int llextc_container_run(char *container_name, char *image_name) {

	char *c_name = k_malloc(strlen(container_name) + 1);
	if (c_name == NULL) {
		LOG_ERR("Failed to alloc memory for container name");
		return 1;
	}
	char *i_name = k_malloc(strlen(image_name) + 1);	
	if (i_name == NULL) {
		k_free(i_name);
		LOG_ERR("Failed to alloc memory for image name");
		return 1;
	}

	// TODO: free later.
	strcpy(c_name, container_name);
	strcpy(i_name, image_name);

	struct llextc_message msg = {
		.type = LLEXTC_MSG_RUN_IMAGE,
		.arg1 = c_name,
		.arg2 = i_name
	};

	return k_msgq_put(&llextc_msgq, &msg, K_FOREVER);
}

//void llextc_list_containers() {
//
//	//k_mutex_lock(&slot_mutex, K_FOREVER);
//	for (int i = 0; i < MAX_CONTAINERS; i++) {
//		if (container_slots[i].used) {
//			char *name = container_slots[i].name;
//			char *image = container_slots[i].image->name;
//			char *status = llextc_container_status_to_string(container_slots[i].status);
//			LOG_INF("Name: %s - Image: %s - Status: %s", name, image, status);
//		}
//	}
//	//k_mutex_unlock(&slot_mutex);
//}
