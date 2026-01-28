#include "zephyr/sys/clock.h"
#include <stdint.h>
#include "llextc_container_api.h"
#include "llextc_priv.h"
#include <zephyr/logging/log.h>
#include "llextc_utils.h"

LOG_MODULE_REGISTER(llextc_api);

int z_impl_send_message(char *dst, void *msg, uint32_t msg_size) {
	return llextc_send_message(dst, msg, msg_size);
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
int z_vrfy_send_message(char *dst, void *msg, uint32_t msg_size) {
	return z_impl_send_message(dst, msg, msg_size);
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
