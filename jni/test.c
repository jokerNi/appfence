/*
 * test.c
 * For unit test
 */

#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "zygote_helper.h"
#include "ptraceaux.h"
#include "sandbox_helper.h"
#include "config_sync.h"

#define APP_FLAG "app"
#define ZYGOTE_FLAG "zygote"

int main(int argc, char *argv[])
{
	/* init_ptrace_tool(ARCH_ARM); */
	if (create_link(SANDBOX_STORAGE_PATH, SANDBOX_LINK) < 0) {
		printf("warnning: link existed\n");
	}

	init_sandbox_state();

	if (fork() == 0) {
		if (fork() == 0) {
			pid_t pid = ptrace_zygote(zygote_find_process());
			if (pid > 0) {
				/* wait for receiving sandbox setting.*/
				if (is_sandbox_enabled()) {
					set_sandbox_enabled(false);
					ptrace_app_process(pid, SANDBOX_ENABLED);
				} else {
					ptrace_app_process(pid, 0);
				}
			}
		} else {
			config_sync();
		}
	} else {
		printf("press [Ctrl - c] to stop sandboxing\n");
		while (1) {
			waitpid(-1, NULL, __WALL);
		}
	}
	return 0;
}

void blind_cont(pid_t pid)
{
	int status;
	ptrace_setopt(pid, 0);
	while (1) {
		/* wait for all child threads/processes */
		int pid = waitpid(-1, &status, __WALL);
		printf("app pid=%d, status %x\n", pid, status);
		ptrace(PTRACE_CONT, pid, NULL, NULL);
		ptrace_detach(pid);
	}

}
