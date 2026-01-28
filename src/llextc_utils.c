#include "llextc.h"
#include "llextc_priv.h"
#include "llextc_container_api.h"

char *llext_container_status_string[] = {
    "NO_IMAGE",
    "STOPPED",
    "PAUSED",
    "RUNNING",
    "FINISHED"
};

char *llextc_container_status_to_string(llextc_container_status status) {
    return llext_container_status_string[status];
}

struct llextc_container_slot* get_slot_by_container_name(char *name) {

    struct llextc_container_slot *slots = get_container_slots();
    for (int i = 0; i < MAX_CONTAINERS; i++) {
        if (!strcmp(name, slots[i].name)) {
            return &slots[i];
        }
    }

    return NULL;
}

struct llextc_container_slot* get_slot_by_tid(k_tid_t tid) {
    
    struct llextc_container_slot *slots = get_container_slots();
	for (int i = 0; i < MAX_CONTAINERS; i++) {
		if (&slots[i].thread == tid) 
            return &slots[i];
	}

	return NULL;
}
