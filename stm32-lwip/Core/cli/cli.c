#include "cli.h"

#include "microrl/src/microrl.h"
//#include "usbd_cdc_if.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include <sys/queue.h>
#include <assert.h>
#include <stdio.h>


typedef void (*callback_p)(int argc, const char * const * argv,
		void (*print)(const char * message));

struct command_item {
	const char * command;
	const char * description;
	callback_p callback;
	SLIST_ENTRY(command_item)
	entries;
};

microrl_t rl;
static SLIST_HEAD(command_item_head, command_item)
s_command_items = SLIST_HEAD_INITIALIZER(s_command_items);

void cli_register(const char * command, const char * description,
		void (*callback)(int argc, const char * const * argv,
				void (*print)(const char * message))) {
	struct command_item * item = malloc(sizeof(struct command_item));
	assert(item);
	item->command = command;
	item->description = description;
	item->callback = callback;
	SLIST_INSERT_HEAD(&s_command_items, item, entries);
}

static void print(const char * str) {
	printf("%s", str);
	fflush(stdout);
//	size_t len = strlen(str);
//	CDC_Transmit_FS(str, len);
}

// definition commands word
#define _CMD_HELP  "help"
#define _CMD_CLEAR "clear"
#define _CMD_LIST  "list"
#define _CMD_LISP  "lisp" // for demonstration completion on 'l + <TAB>'
#define _CMD_NAME  "name"
#define _CMD_VER   "version"
// sub commands for version command
#define _SCMD_MRL  "microrl"
#define _SCMD_DEMO "demo"

#define _NUM_OF_CMD 6
#define _NUM_OF_VER_SCMD 2

//available  commands
//static char * keyworld[] = { _CMD_HELP, _CMD_CLEAR, _CMD_LIST, _CMD_NAME,
//		_CMD_VER,
//		_CMD_LISP };
// version subcommands
//static char * ver_keyworld[] = { _SCMD_MRL, _SCMD_DEMO };

// array for comletion
//static char * compl_world[_NUM_OF_CMD + 1];

// 'name' var for store some string
#define _NAME_LEN 8
//static char name[_NAME_LEN];
//static int val;

static void print_help() {
	struct command_item *it = NULL;

	SLIST_FOREACH(it, &s_command_items, entries)
	{

//		print(it->command);
		print(it->description);
		print("\n");
	}

//	print("Use TAB key for completion\r\nCommand:\r\n");
//	print(
//			"\tversion {microrl | demo} - print version of microrl lib or version of this demo src\r\n");
//	print("\thelp  - this message\r\n");
//	print("\tclear - clear screen\r\n");
//	print("\tlist  - list all commands in tree\r\n");
//	print(
//			"\tname [string] - print 'name' value if no 'string', set name value to 'string' if 'string' present\r\n");
//	print(
//			"\tlisp - dummy command for demonstration auto-completion, while inputed 'l+<TAB>'\r\n");
}

//*****************************************************************************
// execute callback for microrl library
// do what you want here, but don't write to argv!!! read only!!
static int execute(int argc, const char * const * argv) {
	if (strcmp("help", argv[0]) == 0) {
		print_help();
		return 0;
	}
	struct command_item *it = NULL;
	SLIST_FOREACH(it, &s_command_items, entries)
	{
		if (strcmp(it->command, argv[0]) == 0) {
			it->callback(argc, argv, print);
			// one tag in the linked list matched, update the level

			// quit with it != NULL
//		            break;
		}
	}

	return 0;

//	int i = 0;
//	// just iterate through argv word and compare it with your commands
//	while (i < argc) {
//		if (strcmp(argv[i], _CMD_HELP) == 0) {
//			print("microrl library based shell v 1.0\r\n");
//			print_help();        // print help
//		} else if (strcmp(argv[i], _CMD_NAME) == 0) {
//			if ((++i) < argc) { // if value preset
//				if (strlen(argv[i]) < _NAME_LEN) {
//					strcpy(name, argv[i]);
//				} else {
//					print("name value too long!\r\n");
//				}
//			} else {
//				print(name);
//				print("\r\n");
//			}
//		} else if (strcmp(argv[i], _CMD_VER) == 0) {
//			if (++i < argc) {
//				if (strcmp(argv[i], _SCMD_DEMO) == 0) {
//					print("demo v 1.0\r\n");
//				} else if (strcmp(argv[i], _SCMD_MRL) == 0) {
//					print("microrl v 1.2\r\n");
//				} else {
//					print((char*) argv[i]);
//					print(" wrong argument, see help\r\n");
//				}
//			} else {
//				print("version needs 1 parametr, see help\r\n");
//			}
//		} else if (strcmp(argv[i], _CMD_CLEAR) == 0) {
//			print("\033[2J");    // ESC seq for clear entire screen
//			print("\033[H");     // ESC seq for move cursor at left-top corner
//		} else if (strcmp(argv[i], _CMD_LIST) == 0) {
//			print("available command:\n");     // print all command per line
//			for (int i = 0; i < _NUM_OF_CMD; i++) {
//				print("\t");
//				print(keyworld[i]);
//				print("\r\n");
//			}
//		} else {
//			print("command: '");
//			print((char*) argv[i]);
//			print("' Not found.\r\n");
//		}
//		i++;
//	}
	return 0;
}
//
//#ifdef _USE_COMPLETE
////*****************************************************************************
//// completion callback for microrl library
//static char ** complet(int argc, const char * const * argv) {
//	int j = 0;
//
//	compl_world[0] = NULL;
//
//	// if there is token in cmdline
//	if (argc == 1) {
//		// get last entered token
//		char * bit = (char*) argv[argc - 1];
//		// iterate through our available token and match it
//		for (int i = 0; i < _NUM_OF_CMD; i++) {
//			// if token is matched (text is part of our token starting from 0 char)
//			if (strstr(keyworld[i], bit) == keyworld[i]) {
//				// add it to completion set
//				compl_world[j++] = keyworld[i];
//			}
//		}
//	} else if ((argc > 1) && (strcmp(argv[0], _CMD_VER) == 0)) { // if command needs subcommands
//		// iterate through subcommand for command _CMD_VER array
//		for (int i = 0; i < _NUM_OF_VER_SCMD; i++) {
//			if (strstr(ver_keyworld[i], argv[argc - 1]) == ver_keyworld[i]) {
//				compl_world[j++] = ver_keyworld[i];
//			}
//		}
//	} else { // if there is no token in cmdline, just print all available token
//		for (; j < _NUM_OF_CMD; j++) {
//			compl_world[j] = keyworld[j];
//		}
//	}
//
//	// note! last ptr in array always must be NULL!!!
//	compl_world[j] = NULL;
//	// return set of variants
//	return compl_world;
//}
//#endif

//*****************************************************************************
static void sigint(void) {
	print("^C catched!\r\n");
}

static void cli_thread(void const * params) {
	microrl_init(&rl, print);
	microrl_set_execute_callback(&rl, execute);
//	microrl_set_complete_callback(&rl, complet);

// call init with ptr to microrl instance and print callback
	microrl_init(&rl, print);
	// set callback for execute
	microrl_set_execute_callback(&rl, execute);

#ifdef _USE_COMPLETE
	// set callback for completion
	microrl_set_complete_callback(&rl, complet);
#endif
	// set callback for Ctrl+C
	microrl_set_sigint_callback(&rl, sigint);
	while (1) {
		// put received char from stdin to microrl lib

		//TODO: add buffer in processing
			uint8_t buffer[1];
		int got_bytes = get_rx_data(buffer, 1);
		if (got_bytes > 0){
			microrl_insert_char(&rl, buffer[0]);
		}else{
			osDelay(10);
		}


//		int16_t value = ringbuffer_remove(&cdc_rxbuf);
//
//		if (value != rbEmpty) {
//			microrl_insert_char(&rl, value);

//		} else {
			osDelay(10);
//		}
	}
}

osThreadDef(clithread, cli_thread, osPriorityNormal, 0, 3 * 1024  / sizeof(StackType_t));
static osThreadId cli_thread_id;


// static void consoletest(void const * params) {
// 	while(1){
// 		const char * outp = "testing123\r\n";
// //		CDC_Transmit_FS((uint8_t*) outp, strlen(outp));
// 		printf("%s", outp);
// 		osDelay(10);
// 	}
// }
//
// osThreadDef(consoletest, consoletest, osPriorityNormal, 0, 4096  / sizeof(StackType_t));
// static osThreadId consoletest_id;

void cli_initialize(void) {
	cli_thread_id = osThreadCreate(osThread(clithread), NULL);
	if (cli_thread_id == NULL) {
		//could not create thread
		abort();
	}

//	 consoletest_id = osThreadCreate(osThread(consoletest), NULL);
//	 	if (consoletest_id == NULL) {
//	 		//could not create thread
//	 		abort();
//	 	}
}
