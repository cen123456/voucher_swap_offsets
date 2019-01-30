/*
 * parameters.c
 * Brandon Azad
 */
#define PARAMETERS_EXTERN
#include "parameters.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>

#include "log.h"
#include "platform.h"
#include "platform_match.h"

// ---- Initialization routines -------------------------------------------------------------------

// A struct describing an initialization.
struct initialization {
	const char *devices;
	const char *builds;
	void (*init)(void);
};

// Run initializations matching this platform.
static size_t
run_initializations(struct initialization *inits, size_t count) {
	size_t match_count = 0;
	for (size_t i = 0; i < count; i++) {
		struct initialization *init = &inits[i];
		if (platform_matches(init->devices, init->builds)) {
			init->init();
			match_count++;
		}
	}
	return match_count;
}

// A helper macro to get the number of elements in a static array.
#define ARRAY_COUNT(x)	(sizeof(x) / sizeof((x)[0]))

// ---- General system parameters -----------------------------------------------------------------

// Initialization for general system parameters.
static void
init__system_parameters() {
	STATIC_ADDRESS(kernel_base) = 0xFFFFFFF007004000;
	kernel_slide_step = 0x200000;
	message_size_for_kmsg_zone = 76;
	kmsg_zone_size = 256;
	max_ool_ports_per_message = 16382;
	gc_step = 2 * MB;
}

// A list of general system parameter initializations by platform.
static struct initialization system_parameters[] = {
	{ "*", "*", init__system_parameters },
};

// ---- Offset initialization ---------------------------------------------------------------------

// Initialization for iPhone11,8 16C50 (and similar devices).
static void
offsets__iphone11_8__16C50() {
	SIZE(ipc_entry)               = 0x18;
	OFFSET(ipc_entry, ie_object)  =  0;
	OFFSET(ipc_entry, ie_bits)    =  8;
	OFFSET(ipc_entry, ie_request) = 16;

	SIZE(ipc_port)                  =   0xa8;
	BLOCK_SIZE(ipc_port)            = 0x4000;
	OFFSET(ipc_port, ip_bits)       =   0;
	OFFSET(ipc_port, ip_references) =   4;
	OFFSET(ipc_port, waitq_flags)   =  24;
	OFFSET(ipc_port, imq_messages)  =  64;
	OFFSET(ipc_port, imq_msgcount)  =  80;
	OFFSET(ipc_port, imq_qlimit)    =  82;
	OFFSET(ipc_port, ip_receiver)   =  96;
	OFFSET(ipc_port, ip_kobject)    = 104;
	OFFSET(ipc_port, ip_nsrequest)  = 112;
	OFFSET(ipc_port, ip_requests)   = 128;
	OFFSET(ipc_port, ip_mscount)    = 156;
	OFFSET(ipc_port, ip_srights)    = 160;

	SIZE(ipc_port_request)                = 0x10;
	OFFSET(ipc_port_request, ipr_soright) = 0;

	OFFSET(ipc_space, is_table_size) = 0x14;
	OFFSET(ipc_space, is_table)      = 0x20;

	SIZE(ipc_voucher)       =   0x50;
	BLOCK_SIZE(ipc_voucher) = 0x4000;

	OFFSET(proc, p_pid)   = 0x60;
	OFFSET(proc, p_ucred) = 0xf8;

	SIZE(sysctl_oid)                = 0x50;
	OFFSET(sysctl_oid, oid_parent)  =  0x0;
	OFFSET(sysctl_oid, oid_link)    =  0x8;
	OFFSET(sysctl_oid, oid_kind)    = 0x14;
	OFFSET(sysctl_oid, oid_handler) = 0x30;
	OFFSET(sysctl_oid, oid_version) = 0x48;
	OFFSET(sysctl_oid, oid_refcnt)  = 0x4c;

	OFFSET(task, lck_mtx_type) =   0xb;
	OFFSET(task, ref_count)    =  0x10;
	OFFSET(task, active)       =  0x14;
	OFFSET(task, map)          =  0x20;
	OFFSET(task, itk_space)    = 0x300;
	OFFSET(task, bsd_info)     = 0x368;
}

// Initialization for iPhone10,1 16B92 (and similar devices).
static void
offsets__iphone10_1__16B92() {
	offsets__iphone11_8__16C50();

	OFFSET(task, bsd_info) = 0x358;
}

// Initialize offset parameters whose values are computed from other parameters.
static void
initialize_computed_offsets() {
	COUNT_PER_BLOCK(ipc_port) = BLOCK_SIZE(ipc_port) / SIZE(ipc_port);
	COUNT_PER_BLOCK(ipc_voucher) = BLOCK_SIZE(ipc_voucher) / SIZE(ipc_voucher);
}

// Initialize offset paramaters for iPhone 7
int kstruct_offsets_12_all[] = {
  0xb,   // KSTRUCT_OFFSET_TASK_LCK_MTX_TYPE,
  0x10,  // KSTRUCT_OFFSET_TASK_REF_COUNT,
  0x14,  // KSTRUCT_OFFSET_TASK_ACTIVE,
  0x20,  // KSTRUCT_OFFSET_TASK_VM_MAP,
  0x28,  // KSTRUCT_OFFSET_TASK_NEXT,
  0x30,  // KSTRUCT_OFFSET_TASK_PREV,
  0x308, // KSTRUCT_OFFSET_TASK_ITK_SPACE
  0x368, // KSTRUCT_OFFSET_TASK_BSD_INFO,
  
  0x0,   // KSTRUCT_OFFSET_IPC_PORT_IO_BITS,
  0x4,   // KSTRUCT_OFFSET_IPC_PORT_IO_REFERENCES,
  0x40,  // KSTRUCT_OFFSET_IPC_PORT_IKMQ_BASE,
  0x50,  // KSTRUCT_OFFSET_IPC_PORT_MSG_COUNT,
  0x60,  // KSTRUCT_OFFSET_IPC_PORT_IP_RECEIVER,
  0x68,  // KSTRUCT_OFFSET_IPC_PORT_IP_KOBJECT,
  0x88,  // KSTRUCT_OFFSET_IPC_PORT_IP_PREMSG,
  0x90,  // KSTRUCT_OFFSET_IPC_PORT_IP_CONTEXT,
  0xa0,  // KSTRUCT_OFFSET_IPC_PORT_IP_SRIGHTS,
  
  0x10,  // KSTRUCT_OFFSET_PROC_PID,
  0x108, // KSTRUCT_OFFSET_PROC_P_FD
  
  0x0,   // KSTRUCT_OFFSET_FILEDESC_FD_OFILES
  
  0x8,   // KSTRUCT_OFFSET_FILEPROC_F_FGLOB
  
  0x38,  // KSTRUCT_OFFSET_FILEGLOB_FG_DATA
  
  0x10,  // KSTRUCT_OFFSET_SOCKET_SO_PCB
  
  0x10,  // KSTRUCT_OFFSET_PIPE_BUFFER
  
  0x14,  // KSTRUCT_OFFSET_IPC_SPACE_IS_TABLE_SIZE
  0x20,  // KSTRUCT_OFFSET_IPC_SPACE_IS_TABLE
  
  0x7c,  // KFREE_ADDR_OFFSET
};
// iPhone10,1 should work on all devices pre X
// iPhone11,8 should work on all devices After X
// offsets should work on all versions (12.0-12.1.2)

// A list of offset initializations by platform.
static struct initialization offsets[] = {

  // Organized by device.
  
  // iPhone Offsets                                               // Format: {device} - {model number}
	{ "iPhone11,*", "16A366-16C104", offsets__iphone11_8__16C50  }, // iPhone 7 and 7 Plus - global offset
	{ "iPhone10,3", "16A366-16C104", offsets__iphone11_8__16C50  }, // iPhone X - A1865/A1902
	{ "iPhone10,6", "16A366-16C104", offsets__iphone11_8__16C50  }, // iPhone X - A1901
	{ "iPhone10,1", "16A9366-16C104", offsets__iphone10_1__16B92 }, // iPhone 8 - A1863/A1906/A1907
	{ "iPhone10,4", "16A9366-16C104", offsets__iphone10_1__16B92 }, // iPhone 8 - A1905
	{ "iPhone10,2", "16A9366-16C104", offsets__iphone10_1__16B92 }, // iPhone 8 Plus - A1864/A1898/A1899
	{ "iPhone10,5", "16A9366-16C104", offsets__iphone10_1__16B92 }, // iPhone 8 Plus - A1897
  { "iPhone9,*", "16A366-16C104", offsets__iphone10_1__16B92  },  // iPhone 7 and 7 Plus - global offset
  
  // iPad Offsets                                                 // Format: {device} - {model number}
  {"iPad7,5", "16C50-16C104", offsets__iphone10_1__16B92 },

    //Aperently A9 will get tfp0 but wont be able to creat IOAudio2DeviceUserCilent (maybe?) so we will leave the offsets as is for noe
   { "iPhone8,*", "16A366-16C104", offsets__iphone10_1__16B92  },
    
	{ "*",          "*",            initialize_computed_offsets },
};

// The minimum number of offsets that must match in order to declare a platform initialized.
static const size_t min_offsets = 2;

// ---- Public API --------------------------------------------------------------------------------

bool
parameters_init() {
	// Get general platform info.
	platform_init();
	// Initialize general system parameters.
	run_initializations(system_parameters, ARRAY_COUNT(system_parameters));
	// Initialize offsets.
	size_t count = run_initializations(offsets, ARRAY_COUNT(offsets));
	if (count < min_offsets) {
		ERROR("no offsets for %s %s", platform.machine, platform.osversion);
		return false;
	}
	return true;
}
