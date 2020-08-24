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
	if (!strcmp("POWER_SUPPLY_PROP_TECHNOLOGY", str_enum)) { return POWER_SUPPLY_PROP_TECHNOLOGY; };
	if (!strcmp("POWER_SUPPLY_PROP_CYCLE_COUNT", str_enum)) { return POWER_SUPPLY_PROP_CYCLE_COUNT; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MAX; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MIN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_VOLTAGE_NOW", str_enum)) { return POWER_SUPPLY_PROP_VOLTAGE_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_POWER_NOW", str_enum)) { return POWER_SUPPLY_PROP_POWER_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_FULL", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_FULL; };
	if (!strcmp("POWER_SUPPLY_PROP_ENERGY_NOW", str_enum)) { return POWER_SUPPLY_PROP_ENERGY_NOW; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY; };
	if (!strcmp("POWER_SUPPLY_PROP_CAPACITY_LEVEL", str_enum)) { return POWER_SUPPLY_PROP_CAPACITY_LEVEL; };
	if (!strcmp("POWER_SUPPLY_PROP_TEMP", str_enum)) { return POWER_SUPPLY_PROP_TEMP; };
	if (!strcmp("POWER_SUPPLY_PROP_MODEL_NAME", str_enum)) { return POWER_SUPPLY_PROP_MODEL_NAME; };
	if (!strcmp("POWER_SUPPLY_PROP_MANUFACTURER", str_enum)) { return POWER_SUPPLY_PROP_MANUFACTURER; };
	if (!strcmp("POWER_SUPPLY_PROP_SERIAL_NUMBER", str_enum)) { return POWER_SUPPLY_PROP_SERIAL_NUMBER; };
	if (!strcmp("POWER_SUPPLY_TYPE_MAINS", str_enum)) { return POWER_SUPPLY_TYPE_MAINS; };
	if (!strcmp("POWER_SUPPLY_TYPE_BATTERY", str_enum)) { return POWER_SUPPLY_TYPE_BATTERY; };
	if (!strcmp("POWER_SUPPLY_TYPE_USB", str_enum)) { return POWER_SUPPLY_TYPE_USB; };

	return -1;
}

int main()
{
	int i, ret = 0;
	struct psu *power_supplies = calloc(MAX_PSU, sizeof(struct psu));

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	static enum power_supply_property ac_props[] = {
		POWER_SUPPLY_PROP_ONLINE,
	};

	static enum power_supply_property battery_props[] = {
		POWER_SUPPLY_PROP_STATUS,
		POWER_SUPPLY_PROP_CHARGE_TYPE,
		POWER_SUPPLY_PROP_HEALTH,
		POWER_SUPPLY_PROP_PRESENT,
		POWER_SUPPLY_PROP_ONLINE,
		POWER_SUPPLY_PROP_TECHNOLOGY,
		POWER_SUPPLY_PROP_CYCLE_COUNT,
		POWER_SUPPLY_PROP_VOLTAGE_MAX,
		POWER_SUPPLY_PROP_VOLTAGE_MIN,
		POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
		POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
		POWER_SUPPLY_PROP_VOLTAGE_NOW,
		POWER_SUPPLY_PROP_POWER_NOW,
		POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
		POWER_SUPPLY_PROP_ENERGY_EMPTY_DESIGN,
		POWER_SUPPLY_PROP_ENERGY_FULL,
		POWER_SUPPLY_PROP_ENERGY_NOW,
		POWER_SUPPLY_PROP_CAPACITY,
		POWER_SUPPLY_PROP_CAPACITY_LEVEL,
		POWER_SUPPLY_PROP_TEMP,
		POWER_SUPPLY_PROP_MODEL_NAME,
		POWER_SUPPLY_PROP_MANUFACTURER,
		POWER_SUPPLY_PROP_SERIAL_NUMBER,
	};

    #define BUFSIZE 4096
	char buf[BUFSIZE];
    struct json_object *jobj;
	struct json_object *tmp;

	int status;
    FILE *fp;
    char *cmd = "/usr/bin/qrexec-client-vm @default qubes.PowerSupply";

	int curr_type;
	char curr_name[MAX_KEYLENGTH];

	int dev_num;

	enum power_supply_property *psp_props;
	int psp_num;

	struct ioctl_propval propval;

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
					// printf("ioctl: return (%d) errno (%d)\n", ret, errno);

					if (curr_type == POWER_SUPPLY_TYPE_MAINS) {
						psp_props = ac_props;
						psp_num = sizeof(ac_props) / sizeof(ac_props[0]);
					} else if (curr_type == POWER_SUPPLY_TYPE_BATTERY) {
						psp_props = battery_props;
						psp_num = sizeof(battery_props) / sizeof(battery_props[0]);
					}

					for (i = 0; i < psp_num; i++) {
						ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_ADD_PSP, &psp_props[i]);
						// printf("ioctl: return (%d) errno (%d)\n", ret, errno);
					}

					ret = ioctl(power_supplies[dev_num].fd, IOCTL_PSU_REGISTER);
					// printf("ioctl: return (%d) errno (%d)\n", ret, errno);
				}
				// printf("File descriptor %s(%d)\n", curr_name, power_supplies[dev_num].fd);
				json_object_object_foreach(jobj, key, val) {
					if (!strncmp(key, "POWER_SUPPLY_PROP_", 18) && (strtoenum(key) >= 0)) {
						propval.psp = strtoenum(key);
						if (power_supply_is_str_property(strtoenum(key))) {
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
						// fprintf(stderr, "ioctl: return (%d) errno (%d)\n", ret, errno);
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