#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <stdint.h>
#include <zephyr/logging/log.h>
#include "c_api.h"
#include "../llextc_priv.h"

LOG_MODULE_REGISTER(llextc_api);

int z_impl_send_message(char *dst, void *msg, uint32_t msg_size, k_timeout_t timeout) {
	return llextc_send_message(dst, msg, msg_size, timeout);
}
EXPORT_SYMBOL(z_impl_send_message);

#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/receive_message_mrsh.c>
int z_vrfy_receive_message(uint8_t *buff, char *sender, k_timeout_t timeout) {
	return z_impl_receive_message(buff, sender, timeout);
}
#endif

#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/send_message_mrsh.c>
int z_vrfy_send_message(char *dst, void *msg, uint32_t msg_size, k_timeout_t timeout) {
	return z_impl_send_message(dst, msg, msg_size, timeout);
}
#endif

int z_impl_receive_message(uint8_t *buff, char *sender, k_timeout_t timeout) {
	return llextc_receive_message(buff, sender, timeout);	
}
EXPORT_SYMBOL(z_impl_receive_message);

#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/zbuss_publish_mrsh.c>
int z_vrfy_zbuss_publish(struct llextc_zbus_msg *msg, k_timeout_t timeout) {
	return z_impl_zbuss_publish(msg, timeout);
}
#endif


int z_impl_zbuss_publish(struct llextc_zbus_msg *msg, k_timeout_t timeout) {
	llextc_chan_pub(msg, timeout);
	return 0;
}
EXPORT_SYMBOL(z_impl_zbuss_publish);

#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/zbuss_receive_mrsh.c>
int z_vrfy_zbuss_receive(struct llextc_zbus_msg *msg, k_timeout_t timeout) {
	return z_impl_zbuss_receive(msg, timeout);
}
#endif


int z_impl_zbuss_receive(struct llextc_zbus_msg *msg, k_timeout_t timeout) {
	llextc_chan_read(msg, timeout);
	return 0;
}
EXPORT_SYMBOL(z_impl_zbuss_receive);

#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/llextc_heap_init_mrsh.c>
void z_vrfy_llextc_heap_init(struct k_heap *heap, void *mem, size_t size) {
	z_impl_llextc_heap_init(heap, mem, size);
}
#endif
void z_impl_llextc_heap_init(struct k_heap *heap, void *mem, size_t size) {
	k_heap_init(heap, mem, size);
}
EXPORT_SYMBOL(z_impl_llextc_heap_init);


#ifdef CONFIG_USERSPACE
#include <zephyr/syscalls/llextc_heap_alloc_mrsh.c>
void *z_vrfy_llextc_heap_alloc(struct k_heap *heap, size_t size, k_timeout_t timeout) {
	return z_impl_llextc_heap_alloc(heap, size, timeout);
}
#endif
void *z_impl_llextc_heap_alloc(struct k_heap *heap,  size_t size, k_timeout_t timeout) {
	return k_heap_alloc(heap, size, timeout);
}
EXPORT_SYMBOL(z_impl_llextc_heap_alloc);





