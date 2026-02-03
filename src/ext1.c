#include "llextc/container_api/c_api.h"
#include "zephyr/sys/printk.h"
#include <stdint.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/kernel.h>

struct llext_container_env env;



// TODO: bug rarísimo??? Si hago LOG_INF dos veces
// salta hard fault
int start() {
    printk("Hola desde contenedor %s", env.name);
    return 0;
}

EXPORT_SYMBOL(env);
EXPORT_SYMBOL(start);

