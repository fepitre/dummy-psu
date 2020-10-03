#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <json.h>

// Header waiting for upstream
#include "power_supply.h"
#include "dummy_psu.h"

#define BUFSIZE 4096

int sigint_received = 0;
int sigterm_received = 0;
int cleaned_power_supplies = 0;

struct psu {
	int fd;
	struct ioctl_psu specs;
};

void sig_handler(int s) {
	if (s == SIGINT) {
		fprintf(stderr, "INFO: SIGINT received\n");
		sigint_received = 1;
	}
	if (s == SIGTERM) {
		fprintf(stderr, "INFO: SIGTERM received\n");
		sigterm_received = 1;
	}
}

void clean_power_supplies(struct psu *power_supplies) {
	int i;
	for (i = 0; i < MAX_PSU; i++) {
		if (power_supplies[i].fd >= 0) {
			close(power_supplies[i].fd);
		}
	}
	free(power_supplies);
	cleaned_power_supplies = 1;
}

// battery_property_map related picked and adapted from test_power.c
struct battery_property_map {
	int value;
	char const *key;
};

static struct battery_property_map map_status[] = {
	{ POWER_SUPPLY_STATUS_CHARGING,     "charging"     },
	{ POWER_SUPPLY_STATUS_DISCHARGING,  "discharging"  },
	{ POWER_SUPPLY_STATUS_NOT_CHARGING, "not-charging" },
	{ POWER_SUPPLY_STATUS_FULL,         "full"         },
	{ -1,                               NULL           },
};

static struct battery_property_map map_health[] = {
	{ POWER_SUPPLY_HEALTH_GOOD,           "good"        },
	{ POWER_SUPPLY_HEALTH_OVERHEAT,       "overheat"    },
	{ POWER_SUPPLY_HEALTH_DEAD,           "dead"        },
	{ POWER_SUPPLY_HEALTH_OVERVOLTAGE,    "overvoltage" },
	{ POWER_SUPPLY_HEALTH_UNSPEC_FAILURE, "failure"     },
	{ -1,                                 NULL          },
};

static struct battery_property_map map_technology[] = {
	{ POWER_SUPPLY_TECHNOLOGY_NiMH, "NiMH" },
	{ POWER_SUPPLY_TECHNOLOGY_LION, "Li-ion" },
	{ POWER_SUPPLY_TECHNOLOGY_LIPO, "Li-poly" },
	{ POWER_SUPPLY_TECHNOLOGY_LiFe, "LiFe" },
	{ POWER_SUPPLY_TECHNOLOGY_NiCd, "NiCd" },
	{ POWER_SUPPLY_TECHNOLOGY_LiMn, "LiMn" },
	{ -1,							NULL   },
};

static int map_get_value(struct battery_property_map *map, const char *key)
{
	char buf[MAX_KEYLENGTH];
	int cr;

	strncpy(buf, key, MAX_KEYLENGTH);
	buf[MAX_KEYLENGTH-1] = '\0';

	cr = strnlen(buf, MAX_KEYLENGTH) - 1;
	if (cr < 0)
		return -1;
	if (buf[cr] == '\n')
		buf[cr] = '\0';

	while (map->key) {
		if (strncasecmp(map->key, buf, MAX_KEYLENGTH) == 0)
			return map->value;
		map++;
	}

	return -1;
}

static int strtoenum(const char *str_enum) {
	if (!strcmp("POWER_SUPPLY_PROP_STATUS", str_enum)) { return POWER_SUPPLY_PROP_STATUS; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_TYPE", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_TYPE; };
	if (!strcmp("POWER_SUPPLY_PROP_HEALTH", str_enum)) { return POWER_SUPPLY_PROP_HEALTH; };
	if (!strcmp("POWER_SUPPLY_PROP_PRESENT", str_enum)) { return POWER_SUPPLY_PROP_PRESENT; };
	if (!strcmp("POWER_SUPPLY_PROP_ONLINE", str_enum)) { return POWER_SUPPLY_PROP_ONLINE; };
	if (!strcmp("POWER_SUPPLY_PROP_AUTHENTIC", str_enum)) { return POWER_SUPPLY_PROP_AUTHENTIC; };
	if (!strcmp("POWER_SUPPLY_PROP_TECHNOLOGY", str_enum)) { return POWER_SUPPLY_PROP_TECHNOLOGY; };
	if (!strcmp("POWER_SUPPLY_PROP_CYCLE_COUNT", str_enum)) { return POWER_SUPPLY_PROP_CYCLE_COUNT; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_NOW", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_AVG", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_OCV", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_OCV; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_BOOT", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_BOOT; };
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_MAX", str_enum)) { return POWER_SUPPLY_PROP_CURRENT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_NOW", str_enum)) { return POWER_SUPPLY_PROP_CURRENT_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_AVG", str_enum)) { return POWER_SUPPLY_PROP_CURRENT_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_CURRENT_BOOT", str_enum)) { return POWER_SUPPLY_PROP_CURRENT_BOOT; };
	if (!strcmp("POWER_SUPPLY_PROP_POWER_NOW", str_enum)) { return POWER_SUPPLY_PROP_POWER_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_POWER_AVG", str_enum)) { return POWER_SUPPLY_PROP_POWER_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_EMPTY_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_EMPTY_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_FULL", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_FULL; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_EMPTY", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_EMPTY; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_NOW", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_AVG", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_COUNTER", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_COUNTER; };
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT", str_enum)) { return POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT; };
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX", str_enum)) { return POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE", str_enum)) { return POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE; };
	if (!strcmp("POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE_MAX", str_enum)) { return POWER_SUPPLY_PROP_CONSTANT_CHARGE_VOLTAGE_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_START_THRESHOLD", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_CONTROL_START_THRESHOLD; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_CONTROL_END_THRESHOLD; };
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT", str_enum)) { return POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT; };
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT", str_enum)) { return POWER_SUPPLY_PROP_INPUT_VOLTAGE_LIMIT; };
	if (!strcmp("POWER_SUPPLY_PROP_INPUT_POWER_LIMIT", str_enum)) { return POWER_SUPPLY_PROP_INPUT_POWER_LIMIT; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_FULL; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_EMPTY", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_EMPTY; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_NOW", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_AVG", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY_ALERT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_LEVEL", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY_LEVEL; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP", str_enum)) { return POWER_SUPPLY_PROP_TEMP; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_MAX", str_enum)) { return POWER_SUPPLY_PROP_TEMP_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_MIN", str_enum)) { return POWER_SUPPLY_PROP_TEMP_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_ALERT_MIN", str_enum)) { return POWER_SUPPLY_PROP_TEMP_ALERT_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_ALERT_MAX", str_enum)) { return POWER_SUPPLY_PROP_TEMP_ALERT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT", str_enum)) { return POWER_SUPPLY_PROP_TEMP_AMBIENT; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN", str_enum)) { return POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MAX", str_enum)) { return POWER_SUPPLY_PROP_TEMP_AMBIENT_ALERT_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW", str_enum)) { return POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG", str_enum)) { return POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_FULL_NOW", str_enum)) { return POWER_SUPPLY_PROP_TIME_TO_FULL_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_TIME_TO_FULL_AVG", str_enum)) { return POWER_SUPPLY_PROP_TIME_TO_FULL_AVG; };
	if (!strcmp("POWER_SUPPLY_PROP_TYPE", str_enum)) { return POWER_SUPPLY_PROP_TYPE; };
	if (!strcmp("POWER_SUPPLY_PROP_USB_TYPE", str_enum)) { return POWER_SUPPLY_PROP_USB_TYPE; };
	if (!strcmp("POWER_SUPPLY_PROP_SCOPE", str_enum)) { return POWER_SUPPLY_PROP_SCOPE; };
	if (!strcmp("POWER_SUPPLY_PROP_PRECHARGE_CURRENT", str_enum)) { return POWER_SUPPLY_PROP_PRECHARGE_CURRENT; };
	if (!strcmp("POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT", str_enum)) { return POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT; };
	if (!strcmp("POWER_SUPPLY_PROP_CALIBRATE", str_enum)) { return POWER_SUPPLY_PROP_CALIBRATE; };
	if (!strcmp("POWER_SUPPLY_PROP_MODEL_NAME", str_enum)) { return POWER_SUPPLY_PROP_MODEL_NAME; };
	if (!strcmp("POWER_SUPPLY_PROP_MANUFACTURER", str_enum)) { return POWER_SUPPLY_PROP_MANUFACTURER; };
	if (!strcmp("POWER_SUPPLY_PROP_SERIAL_NUMBER", str_enum)) { return POWER_SUPPLY_PROP_SERIAL_NUMBER; };
	if (!strcmp("POWER_SUPPLY_TYPE_MAINS", str_enum)) { return POWER_SUPPLY_TYPE_MAINS; };
	if (!strcmp("POWER_SUPPLY_TYPE_BATTERY", str_enum)) { return POWER_SUPPLY_TYPE_BATTERY; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB", str_enum)) { return POWER_SUPPLY_TYPE_USB; };
	if (!strcmp("POWER_SUPPLY_TYPE_UNKNOWN", str_enum)) { return POWER_SUPPLY_TYPE_UNKNOWN; };
	if (!strcmp("POWER_SUPPLY_TYPE_BATTERY", str_enum)) { return POWER_SUPPLY_TYPE_BATTERY; };
	if (!strcmp("POWER_SUPPLY_TYPE_UPS", str_enum)) { return POWER_SUPPLY_TYPE_UPS; };
	if (!strcmp("POWER_SUPPLY_TYPE_MAINS", str_enum)) { return POWER_SUPPLY_TYPE_MAINS; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB", str_enum)) { return POWER_SUPPLY_TYPE_USB; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_DCP", str_enum)) { return POWER_SUPPLY_TYPE_USB_DCP; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_CDP", str_enum)) { return POWER_SUPPLY_TYPE_USB_CDP; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_ACA", str_enum)) { return POWER_SUPPLY_TYPE_USB_ACA; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_TYPE_C", str_enum)) { return POWER_SUPPLY_TYPE_USB_TYPE_C; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_PD", str_enum)) { return POWER_SUPPLY_TYPE_USB_PD; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB_PD_DRP", str_enum)) { return POWER_SUPPLY_TYPE_USB_PD_DRP; };
	if (!strcmp("POWER_SUPPLY_TYPE_APPLE_BRICK_ID", str_enum)) { return POWER_SUPPLY_TYPE_APPLE_BRICK_ID; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_UNKNOWN", str_enum)) { return POWER_SUPPLY_USB_TYPE_UNKNOWN; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_SDP", str_enum)) { return POWER_SUPPLY_USB_TYPE_SDP; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_DCP", str_enum)) { return POWER_SUPPLY_USB_TYPE_DCP; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_CDP", str_enum)) { return POWER_SUPPLY_USB_TYPE_CDP; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_ACA", str_enum)) { return POWER_SUPPLY_USB_TYPE_ACA; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_C", str_enum)) { return POWER_SUPPLY_USB_TYPE_C; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD", str_enum)) { return POWER_SUPPLY_USB_TYPE_PD; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD_DRP", str_enum)) { return POWER_SUPPLY_USB_TYPE_PD_DRP; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_PD_PPS", str_enum)) { return POWER_SUPPLY_USB_TYPE_PD_PPS; };
	if (!strcmp("POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID", str_enum)) { return POWER_SUPPLY_USB_TYPE_APPLE_BRICK_ID; };

	return -1;
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
	int curr_type;
	char curr_name[MAX_KEYLENGTH];
	int dev_num;
	int psp_prop;
	struct ioctl_propval propval;
	int status;
    FILE *fp;
    char cmd[79];

	strncpy(cmd, "/usr/bin/qrexec-client-vm ", 27);

	if( argc == 2 ) {
		if (!strncmp("default", argv[1], 9)) {
			strncat(cmd, "@default", 9);
		} else {
			strncat(cmd, argv[1], 33);
		}
	} else if ( argc > 2 ) {
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
		strncpy(curr_name, json_object_get_string(tmp), MAX_KEYLENGTH);

		json_object_object_get_ex(jobj, "TYPE", &tmp);
		curr_type = strtoenum(json_object_get_string(tmp));

		dev_num = -1;
		if (strcmp(curr_name, "\0") && (curr_type >= 0)) {
			for (i = 0; i < MAX_PSU; i++) {
				if (!strcmp(power_supplies[i].specs.dev_name, "\0")) {
					strncpy(power_supplies[i].specs.dev_name, curr_name, MAX_KEYLENGTH);
					power_supplies[i].specs.dev_type = curr_type;
					dev_num = i;
				} else if (!strcmp(power_supplies[i].specs.dev_name, curr_name)) {
					dev_num = i;
				}

				if (dev_num >= 0) {
					break;
				}
			}

			if (dev_num >= 0) {
				if (power_supplies[dev_num].fd <= 0) {
					power_supplies[dev_num].fd = open("/dev/dummy_psu", O_RDWR);
					if (power_supplies[dev_num].fd < 0) {
						fprintf(stderr, "ERROR: failed to open file descriptor\n");
						return -1;
					}

					ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_CREATE, &power_supplies[dev_num].specs);
					// printf("IOCTL_PSU_CREATE: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));

					json_object_object_foreach(jobj, key, val) {
						psp_prop = strtoenum(key);
						if (!strncmp(key, "POWER_SUPPLY_PROP_", 18) && (psp_prop >= 0)) {
							ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_ADD_PSP, psp_prop);
							// printf("IOCTL_PSU_ADD_PSP: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
						}
					}

					ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_REGISTER);
					// printf("IOCTL_PSU_REGISTER: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
				}
				// printf("File descriptor %s(%d)\n", curr_name, power_supplies[dev_num].fd);
				json_object_object_foreach(jobj, key, val) {
					if (!strncmp(key, "POWER_SUPPLY_PROP_", 18) && (strtoenum(key) >= 0)) {
						propval.psp = strtoenum(key);
						if (power_supply_is_str_property(propval.psp)) {
							strncpy(propval.strval, json_object_get_string(val), MAX_KEYLENGTH);
							propval.intval = 0;
						} else if (!strcmp(key, "POWER_SUPPLY_PROP_STATUS")) {
							propval.intval = map_get_value(map_status, json_object_get_string(val));
							propval.strval[0] = '\0';
						} else if (!strcmp(key, "POWER_SUPPLY_PROP_HEALTH")) {
							propval.intval = map_get_value(map_health, json_object_get_string(val));
							propval.strval[0] = '\0';
						} else if (!strcmp(key, "POWER_SUPPLY_PROP_TECHNOLOGY")) {
							propval.intval = map_get_value(map_technology, json_object_get_string(val));
							propval.strval[0] = '\0';
						} else {
							propval.intval = json_object_get_int(val);
							propval.strval[0] = '\0';
						}
						ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_UPDATE_PROPVAL, &propval);
						// fprintf(stderr, "IOCTL_PSU_UPDATE_PROPVAL: return (%d) errno (%d): %s\n", ret, errno, strerror(errno));
					}
				}
			} else {
				fprintf(stderr, "WARNING: maximum number of device reached\n");
			}
		}
    }

    status = pclose(fp);
    if (status == -1)  {
        fprintf(stderr, "ERROR: an error occurred while running cmd\n");
        return -1;
    }

	if (!cleaned_power_supplies) {
		clean_power_supplies(power_supplies);
		fprintf(stderr, "INFO: terminating\n");
	}
}