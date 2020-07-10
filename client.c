#include "client.h"

int main()
{
	int i, ac_fd, battery_fd, usb_fd, ret = 0;
	struct ioctl_psu ac_spec, battery_spec, usb_spec;

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

	static enum power_supply_property usb_props[] = {
		POWER_SUPPLY_PROP_STATUS,
		POWER_SUPPLY_PROP_PRESENT,
		POWER_SUPPLY_PROP_ONLINE,
		POWER_SUPPLY_PROP_MODEL_NAME,
		POWER_SUPPLY_PROP_MANUFACTURER,
		POWER_SUPPLY_PROP_SERIAL_NUMBER,
	};

	ac_spec.dev_type = POWER_SUPPLY_TYPE_MAINS;
	strncpy(ac_spec.dev_name, "ac0", sizeof(ac_spec.dev_name));

	battery_spec.dev_type = POWER_SUPPLY_TYPE_BATTERY;
	strncpy(battery_spec.dev_name, "bat0", sizeof(battery_spec.dev_name));

	usb_spec.dev_type = POWER_SUPPLY_TYPE_USB;
	strncpy(usb_spec.dev_name, "usb0", sizeof(usb_spec.dev_name));

	static const struct ioctl_propval battery_val[] = {
		{ .psp = POWER_SUPPLY_PROP_ONLINE, .intval = 1 },
		{ .psp = POWER_SUPPLY_PROP_STATUS, .intval = POWER_SUPPLY_STATUS_DISCHARGING },
		{ .psp = POWER_SUPPLY_PROP_HEALTH, .intval = POWER_SUPPLY_HEALTH_GOOD },
		{ .psp = POWER_SUPPLY_PROP_PRESENT, .intval = 1 },
		{ .psp = POWER_SUPPLY_PROP_TECHNOLOGY, .intval = POWER_SUPPLY_TECHNOLOGY_LION }, 
		{ .psp = POWER_SUPPLY_PROP_CAPACITY, .intval = 42 },
		{ .psp = POWER_SUPPLY_PROP_CAPACITY_LEVEL, .intval = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL },
		{ .psp = POWER_SUPPLY_PROP_TEMP, .intval = 10 },
		{ .psp = POWER_SUPPLY_PROP_VOLTAGE_NOW, .intval = 10000000 },
		{ .psp = POWER_SUPPLY_PROP_MODEL_NAME, .strval = "Dummy battery" },
		{ .psp = POWER_SUPPLY_PROP_MANUFACTURER, .strval = "Linux" },
		{ .psp = POWER_SUPPLY_PROP_SERIAL_NUMBER, .strval = "Azerty12346" },
	};

	static const struct ioctl_propval usb_val[] = {
		{ .psp = POWER_SUPPLY_PROP_ONLINE, .intval = 1 },
		{ .psp = POWER_SUPPLY_PROP_STATUS, .intval = POWER_SUPPLY_STATUS_FULL },
		{ .psp = POWER_SUPPLY_PROP_PRESENT, .intval = 1 },
		{ .psp = POWER_SUPPLY_PROP_MODEL_NAME, .strval = "Dummy USB" },
		{ .psp = POWER_SUPPLY_PROP_MANUFACTURER, .strval = "Linux" },
		{ .psp = POWER_SUPPLY_PROP_SERIAL_NUMBER, .strval = "Azerty12346" },
	};

	ac_fd = open("/dev/dummy_psu", O_RDWR);

	printf("File descriptor ac_fd(%d)\n", ac_fd);
	if (ac_fd < 0) {
		printf("File open error\n");
		return -1;
	}

	battery_fd = open("/dev/dummy_psu", O_RDWR);
	printf("File descriptor battery_fd(%d)\n", battery_fd);
	if (battery_fd < 0) {
		printf("File open error\n");
		return -1;
	}

	usb_fd = open("/dev/dummy_psu", O_RDWR);
	printf("File descriptor usb_fd(%d)\n", usb_fd);
	if (usb_fd < 0) {
		printf("File open error\n");
		return -1;
	}

	// ac0
	ret = ioctl(ac_fd, IOCTL_PSU_CREATE, &ac_spec);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	for (i = 0; i < sizeof(ac_props) / sizeof(ac_props[0]); i++) {
		ret = ioctl(ac_fd, IOCTL_PSU_ADD_PSP, &ac_props[i]);
		printf("ioctl: return (%d) errno (%d)\n", ret, errno);
	}

	ret = ioctl(ac_fd, IOCTL_PSU_ADD_SUPPLIED_TO, &battery_spec.dev_name);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	ret = ioctl(ac_fd, IOCTL_PSU_REGISTER);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	// bat0
	ret = ioctl(battery_fd, IOCTL_PSU_CREATE, &battery_spec);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	for (i = 0; i < sizeof(battery_props) / sizeof(battery_props[0]); i++) {
		ret = ioctl(battery_fd, IOCTL_PSU_ADD_PSP, &battery_props[i]);
		printf("ioctl: return (%d) errno (%d)\n", ret, errno);
	}

	ret = ioctl(battery_fd, IOCTL_PSU_REGISTER);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	for (i = 0; i < sizeof(battery_val) / sizeof(battery_val[0]); i++) {
		ret = ioctl(battery_fd, IOCTL_PSU_UPDATE_PROPVAL, &battery_val[i]);
		printf("ioctl: return (%d) errno (%d)\n", ret, errno);
	}

	// usb0
	ret = ioctl(usb_fd, IOCTL_PSU_CREATE, &usb_spec);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	for (i = 0; i < sizeof(usb_props) / sizeof(usb_props[0]); i++) {
		ret = ioctl(ac_fd, IOCTL_PSU_ADD_PSP, &usb_props[i]);
		printf("ioctl: return (%d) errno (%d)\n", ret, errno);
	}

	ret = ioctl(usb_fd, IOCTL_PSU_ADD_SUPPLIED_TO, &battery_spec.dev_name);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	ret = ioctl(usb_fd, IOCTL_PSU_REGISTER);
	printf("ioctl: return (%d) errno (%d)\n", ret, errno);

	sleep(30);

	close(ac_fd);
	close(battery_fd);
	close(usb_fd);
}