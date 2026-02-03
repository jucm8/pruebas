#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include "./llextc/llextc.h"
#include "oci/parser.h"
#include "neta/wifi.h"
#include "neta/http.h"

	#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_core.h>

LOG_MODULE_REGISTER(app);


//void entry(void *ext_start, void *p2, void *p3) {
//
//	LOG_INF("ext start: %p, memoria :%p", ext_start, user_mem);
//	void (*start)() = ext_start; 
//	start();
//}

int main(void) {

	struct oci_ref_t oci;
	oci_error_t res = parse_oci_ref("gulis/temporal:2.0", &oci);
	LOG_INF("Nombre: %s", oci.namespace);
	LOG_INF("Digest: %s", oci.digest);
	LOG_INF("Tag: %s", oci.tag);
	LOG_INF("Registry: %s", oci.registry);
	LOG_INF("Repository: %s", oci.repository);

	configure_static_ip();
	k_msleep(5000);
	//run_queries();
	//
	
	 struct net_if *iface = net_if_get_default();
	struct net_if_addr *addr;
    char buf[NET_IPV4_ADDR_LEN];

	//LOG_INF("Hola!");
	//if (llextc_init()) {
	//	LOG_ERR("Failed to init llextc");
	//	return 1;
	//}
	//if (llextc_container_run("1_contenedor", "imagen_prueba")) {
	//	LOG_ERR("llextc_run_image failed");
	//}

	/* if (llextc_container_run("2_contenedor", "imagen_prueba")) { */
	/* 	LOG_ERR("llextc_run_image failed"); */
	/* } */
	//LOG_INF("Adios!");
	//   struct http_client_req req;


	return 0;
}
