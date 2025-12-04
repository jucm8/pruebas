#include "zephyr/llext/symbol.h"
#include "zephyr/logging/log.h"

LOG_MODULE_REGISTER(ext1);

int start() {

    while (1) {
        LOG_INF("Hola desde ext 1");
        k_msleep(500);
    }
    return 0;
}

EXPORT_SYMBOL(start);
