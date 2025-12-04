#include <zephyr/logging/log.h>
#include "zephyr/app_memory/mem_domain.h"
#include "zephyr/kernel/thread.h"
#include "zephyr/llext/buf_loader.h"
#include "zephyr/llext/llext.h"
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include "buff.h"
#include "zephyr/sys/libc-hooks.h"
#include "zephyr/sys/util.h"

LOG_MODULE_REGISTER(app);

struct k_mem_domain user_domain;

#define XD 1024 * 32
uint8_t __aligned(XD) user_mem[XD];
K_MEM_PARTITION_DEFINE(user_part, user_mem, sizeof(user_mem), 
    K_MEM_PARTITION_P_RWX_U_RX);


struct k_thread thread1;
k_thread_stack_t *t1_stack;

void entry(void *ext_start, void *p2, void *p3) {

	LOG_INF("ext start: %p, memoria :%p", ext_start, user_mem);
	void (*start)() = ext_start; 
	start();
}

int load_ext() {

	struct llext *ext1;
	struct llext_buf_loader ext1_loader = LLEXT_TEMPORARY_BUF_LOADER(ext1_buf, sizeof(ext1_buf));
	struct llext_load_param load_params = LLEXT_LOAD_PARAM_DEFAULT;

	if (llext_load(&ext1_loader.loader, "ext1 instance", &ext1, &load_params)) {
		LOG_ERR("Failed to load extension");
		return 1;
	}
	void (*ext1_start)() = llext_find_sym(&ext1->exp_tab, "start");
	if (ext1_start == NULL) {
		LOG_ERR("Failed to find entrypoint for extension 1");
		return -1;
	}

	t1_stack = k_thread_stack_alloc(1024, 0);
	if (t1_stack == NULL) {
        LOG_ERR("Failed to allocate stack...");
        return 1;
    }

	k_tid_t tid = k_thread_create(&thread1, t1_stack, 1024, 
		entry, ext1_start, NULL, NULL, 5, K_USER, K_FOREVER
	);

	if (k_mem_domain_add_thread(&user_domain, tid)) {
		LOG_ERR("Failed to add thread to memory domain");
		return 1;
	}

	k_thread_start(tid);

	for (int i = 5; i > 0; i--) {
		LOG_INF("Terminando thread en %d...", i);
		k_msleep(1000);
	}

	k_thread_abort(tid);
	LOG_INF("Thread terminado");
	//LOG_INF("Waiting for thread %p to finish...", tid);
	//k_thread_join(tid, K_FOREVER);
	//LOG_INF("Thread %p finished", tid);
	return 0;
}

int main(void) {

	LOG_INF("Main thread: %p", k_current_get());
	struct k_mem_partition *user_parts[] = {
		&z_libc_partition,
		&z_malloc_partition,
		&user_part
	};

	if (k_mem_domain_init(&user_domain, ARRAY_SIZE(user_parts), user_parts)) {
		LOG_ERR("Failed to init memory domain");
		return 1;
	}
	if (llext_heap_init(user_mem, sizeof(user_mem))) {
		LOG_ERR("Failed to init llext heap");
		return 1;
	}

	k_tid_t tid = k_current_get();
	if (k_mem_domain_add_thread(&user_domain,tid)) {
		LOG_ERR("CAGASTE");
		return 1;
	}
	
	load_ext();
	return 0;
}
