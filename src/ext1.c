#include "zephyr/llext/symbol.h"
#include "llextc_container_api.h"
#include "llextc_utils.h"
#include "zephyr/sys/printk.h"
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/kernel.h>

struct llext_container_env env;
uint8_t msg_buff[LLEXTC_CONTAINER_MESSAGE_MAX_SIZE];

// TODO: bug rarísimo??? Si hago LOG_INF dos veces
// salta hard fault

int start() {

    printk("Hola desde contenedor: %s\n", env.name);
    if (env.name[0] == '1') {

        /* struct llextc_zbus_msg msg = { */
        /*     .code = 11, */
        /* }; */
        /* memset(msg.msg, 0, sizeof(msg.msg)); */
        /* memcpy(msg.msg, "Hola", sizeof("Hola")); */
        /* int ret = zbuss_publish(&msg, K_FOREVER); */
        /* if (ret) */
        /*     printk("Failed to publish message\n"); */
        /* else */
        /*     printk("Published message\n"); */
        char original[] = "Caca";
        int ret = send_message("2_contenedor", original, strlen(original) + 1);
        if (ret) 
            printk("Send message failed\n");
        else
            printk("Succesfulyl sent message\n");
    }
    else {
        k_msleep(5000);
        char sender[64];
        int ret = receive_message(msg_buff, sender, K_FOREVER);
        if (ret)
            printk("Failed to receive message \n");
        else
            printk("Received message from %s: %s\n", sender, msg_buff);

        /* struct llextc_zbus_msg msg2 = { */
        /*     .code = 0, */
        /* }; */
        /* int ret = zbuss_receive(&msg2, K_FOREVER); */
        /* if (ret) */
        /*     printk("Failed to receive message\n"); */
        /* else */
        /*     printk("Received - code: %d, message: %s\n", msg2.code, msg2.msg); */
    }
    return 0;
}

EXPORT_SYMBOL(env);
EXPORT_SYMBOL(start);
