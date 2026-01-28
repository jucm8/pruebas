#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include "llextc.h"

LOG_MODULE_REGISTER(app);


//void entry(void *ext_start, void *p2, void *p3) {
//
//	LOG_INF("ext start: %p, memoria :%p", ext_start, user_mem);
//	void (*start)() = ext_start; 
//	start();
//}


int main(void) {

	LOG_INF("Hola!");
	if (llextc_init()) {
		LOG_ERR("Failed to init llextc");
		return 1;
	}
	if (llextc_container_run("1_contenedor", "imagen_prueba")) {
		LOG_ERR("llextc_run_image failed");
	}

	if (llextc_container_run("2_contenedor", "imagen_prueba")) {
		LOG_ERR("llextc_run_image failed");
	}
	//LOG_INF("Adios!");
	
	return 0;
}
