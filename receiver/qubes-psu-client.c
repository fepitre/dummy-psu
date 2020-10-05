#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <json.h>

#include "dummy_psu.h"

#define BUFSIZE 4096

int sigint_received = 0;
int sigterm_received = 0;
int cleaned_power_supplies = 0;

struct psu {
	int fd;
	struct ioctl_psu specs;
};

void sig_handler(int s)
{
	if (s == SIGINT) {
		fprintf(stderr, "INFO: SIGINT received\n");
		sigint_received = 1;
	}
	if (s == SIGTERM) {
		fprintf(stderr, "INFO: SIGTERM received\n");
		sigterm_received = 1;
	}
}

void clean_power_supplies(struct psu *power_supplies)
{
	int i;
	for (i = 0; i < MAX_PSU; i++) {
		if (power_supplies[i].fd >= 0) {
			close(power_supplies[i].fd);
		}
	}
	free(power_supplies);
	cleaned_power_supplies = 1;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	int i, ret = 0;
	struct psu *power_supplies = calloc(MAX_PSU, sizeof(struct psu));
	char buf[BUFSIZE];
	struct json_object *jobj;
	struct json_object *tmp;
	char curr_type[MAX_KEYLENGTH];
	char curr_name[MAX_KEYLENGTH];
	int dev_num;
	struct ioctl_pspval propval;
	int status;
	FILE *fp;
	char cmd[79];

	strncpy(cmd, "/usr/bin/qrexec-client-vm ", 27);

	if (argc == 2) {
		if (!strncmp("default", argv[1], 9)) {
			strncat(cmd, "@default", 9);
		} else {
			strncat(cmd, argv[1], 33);
		}
	} else if (argc > 2) {
		printf("ERROR: too many arguments provided\n");
		return -1;
	} else {
		strncat(cmd, "@default", 9);
	}

	strncat(cmd, " qubes.PowerSupply", 19);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("ERROR: failed to open pipe\n");
		return -1;
	}

	while (fgets(buf, BUFSIZE, fp) != NULL) {
		if (sigint_received || sigterm_received) {
			break;
		}
		setvbuf(stdout, NULL, _IONBF, 0);
		jobj = json_tokener_parse(buf);

		json_object_object_get_ex(jobj, "NAME", &tmp);
		strncpy(curr_name, json_object_get_string(tmp), MAX_KEYLENGTH - 1);

		json_object_object_get_ex(jobj, "TYPE", &tmp);
		strncpy(curr_type, json_object_get_string(tmp), MAX_KEYLENGTH - 1);

		dev_num = -1;
		if (strcmp(curr_name, "\0") && strcmp(curr_name, "\0")) {
			for (i = 0; i < MAX_PSU; i++) {
				if (!strcmp(power_supplies[i].specs.dev_name,
					    "\0")) {
					strncpy(power_supplies[i].specs.dev_type,
						curr_type, MAX_KEYLENGTH);
					strncpy(power_supplies[i].specs.dev_name,
						curr_name, MAX_KEYLENGTH);
					dev_num = i;
				} else if (!strcmp(power_supplies[i]
							   .specs.dev_name,
						   curr_name)) {
					dev_num = i;
				}

				if (dev_num >= 0) {
					break;
				}
			}

			if (dev_num >= 0) {
				if (power_supplies[dev_num].fd <= 0) {
					power_supplies[dev_num].fd =
						open("/dev/dummy_psu", O_RDWR);
					if (power_supplies[dev_num].fd < 0) {
						fprintf(stderr,
							"ERROR: failed to open file descriptor\n");
						return -1;
					}

					ret = ioctl(
						power_supplies[dev_num].fd,
						IOCTL_PSU_CREATE,
						&power_supplies[dev_num].specs);
					// printf("IOCTL_PSU_CREATE: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));

					json_object_object_foreach(jobj, key,
								   val)
					{
						if (!strncmp(
							    key,
							    "POWER_SUPPLY_PROP_",
							    18)) {
							ret = ioctl(
								power_supplies[dev_num]
									.fd,
								IOCTL_PSU_ADD_PSP,
								key);
							// printf("IOCTL_PSU_ADD_PSP: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
						}
					}

					ret = ioctl(power_supplies[dev_num].fd,
						    IOCTL_PSU_REGISTER);
					// printf("IOCTL_PSU_REGISTER: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
				}
				json_object_object_foreach(jobj, key, val)
				{
					if (!strncmp(key, "POWER_SUPPLY_PROP_",
						     18)) {
						strncpy(propval.psp, key,
							MAX_KEYLENGTH - 1);
						strncpy(propval.val,
							json_object_get_string(
								val),
							MAX_KEYLENGTH - 1);
						ret = ioctl(
							power_supplies[dev_num]
								.fd,
							IOCTL_PSU_UPDATE_PROPVAL,
							&propval);
						// fprintf(stderr, "IOCTL_PSU_UPDATE_PROPVAL: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
					}
				}
			} else {
				fprintf(stderr,
					"WARNING: maximum number of device reached\n");
			}
		}
	}

	status = pclose(fp);
	if (status == -1) {
		fprintf(stderr, "ERROR: an error occurred while running cmd\n");
		return -1;
	}

	if (!cleaned_power_supplies) {
		clean_power_supplies(power_supplies);
		fprintf(stderr, "INFO: terminating\n");
	}

	return ret;
}