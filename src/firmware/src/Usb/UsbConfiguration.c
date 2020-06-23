#include <xc.h>
#include <stdint.h>
#include <string.h>

#include "../Configuration.h"
#include "../Pins.h"
#include "../FreeRtos.h"

#include "Usb.h"

#include "usb_config.h"

static const struct configuration_descriptor usbTemplateConfigurationDescriptor =
{
	.bLength = sizeof(struct configuration_descriptor),
	.bDescriptorType = DESC_CONFIGURATION,
	.wTotalLength = sizeof(struct ConfigurationDescriptor),
	.bNumInterfaces = NUMBER_OF_INTERFACES,
	.bConfigurationValue = 1,
	.iConfiguration = 4,
	.bmAttributes = 0x80,
	.bMaxPower = 100 / 2
};

static struct ConfigurationDescriptor usbConfigurations[NUMBER_OF_CONFIGURATIONS];
const struct ConfigurationDescriptor *usbCurrentConfigurationDescriptor;
struct Configuration usbCurrentConfiguration;

const struct configuration_descriptor *usbConfigurationDescriptors[] =
{
#if NUMBER_OF_CONFIGURATIONS <= 0
#error The value of NUMBER_OF_CONFIGURATIONS must be at least 1
#endif

#if NUMBER_OF_CONFIGURATIONS > 0
	&usbConfigurations[0].configuration,
#endif
#if NUMBER_OF_CONFIGURATIONS > 1
	&usbConfigurations[1].configuration,
#endif
#if NUMBER_OF_CONFIGURATIONS > 2
	&usbConfigurations[2].configuration,
#endif
#if NUMBER_OF_CONFIGURATIONS > 3
	&usbConfigurations[3].configuration
#endif

#if NUMBER_OF_CONFIGURATIONS > 4
#error The value of NUMBER_OF_CONFIGURATIONS must be at most 4
#endif
};

STATIC_SIZE_CHECK_EQUAL(USB_ARRAYLEN(USB_CONFIG_DESCRIPTOR_MAP), NUMBER_OF_CONFIGURATIONS);

static void usbLoadConfiguration(struct ConfigurationDescriptor *descriptor, uint8_t index);
static void unassignAllPinsFromTheirCurrentInterface(void);
static void assignAllPinsToTheirNewInterface(void);

void usbConfigurationInitialise(void)
{
	for (int i = 0; i < NUMBER_OF_CONFIGURATIONS; i++)
		usbLoadConfiguration(&usbConfigurations[i], i);

	uint8_t bootConfigurationIndex = configurationGetBootIndex();
	configurationLoad(&usbCurrentConfiguration, bootConfigurationIndex);
	usbCurrentConfigurationDescriptor = &usbConfigurations[bootConfigurationIndex];
}

static void usbLoadConfiguration(struct ConfigurationDescriptor *descriptor, uint8_t index)
{
	if (!descriptor)
		return;

	struct Configuration configuration;
	configurationLoad(&configuration, index);

	uint8_t numberOfEnabledInterfaces = 0;
	memcpy(descriptor, &usbTemplateConfigurationDescriptor, sizeof(usbTemplateConfigurationDescriptor));
	for (uint16_t i = 0, offset = sizeof(usbTemplateConfigurationDescriptor); i < NUMBER_OF_INTERFACES; i++)
	{
		uint16_t interfaceDescriptorSize = usbInterfaces[i]->memcpyInterfaceDescriptor(((uint8_t *) descriptor) + offset);
		if (interfaceDescriptorSize > 0)
		{
			offset += interfaceDescriptorSize;
			numberOfEnabledInterfaces++;
		}
	}

	descriptor->configuration.bNumInterfaces = numberOfEnabledInterfaces;
	descriptor->configuration.bConfigurationValue += index;
	descriptor->configuration.iConfiguration += index;
	descriptor->configuration.bMaxPower = (configuration.currentLimitMilliamps + 1) / 2;
	if (descriptor->configuration.bMaxPower > 250)
		descriptor->configuration.bMaxPower = 250;
	else if (descriptor->configuration.bMaxPower == 0)
		descriptor->configuration.bMaxPower = 1;

	usbDescriptorsSetConfigurationName(descriptor->configuration.iConfiguration, configuration.unicodeName);
}

void usbConfigurationSet(uint8_t configuration)
{
	struct UsbEvent event =
	{
		.header = EVENT_QUEUE_HEADER_INIT(USB_MODULE_ID, USB_EVENT_CONFIGURATION_CHANGED),
		.as =
		{
			.configurationChanged = { .configurationIndex = configuration }
		}
	};

	xQueueSendToBack(usbFlagsEventQueue, &event, portMAX_DELAY);
}

void usbOnSetConfiguration(uint8_t configuration)
{
	struct UsbEvent event =
	{
		.header = EVENT_QUEUE_HEADER_INIT(USB_MODULE_ID, USB_EVENT_CONFIGURATION_CHANGED),
		.as =
		{
			.configurationChanged = { .configurationIndex = configuration - 1 }
		}
	};

	BaseType_t wasHigherPriorityTaskWoken = pdFALSE;
	xQueueSendToBackFromISR(usbFlagsEventQueue, &event, &wasHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(wasHigherPriorityTaskWoken);
}

void usbConfigurationApply(const struct UsbEvent *event)
{
	uint8_t configurationIndex = configurationGetBootIndex();
	if (event && event->header.as.raw == EVENT_QUEUE_HEADER_WORD_FOR(USB_MODULE_ID, USB_EVENT_CONFIGURATION_CHANGED))
		configurationIndex = event->as.configurationChanged.configurationIndex;

	if (configurationIndex >= NUMBER_OF_CONFIGURATIONS)
		configurationIndex = configurationGetBootIndex();

	taskENTER_CRITICAL();
	configurationLoad(&usbCurrentConfiguration, configurationIndex);

	unassignAllPinsFromTheirCurrentInterface();
	configurationApply(&usbCurrentConfiguration);
	usbCurrentConfigurationDescriptor = &usbConfigurations[configurationIndex];
	assignAllPinsToTheirNewInterface();
	taskEXIT_CRITICAL();
}

static void unassignAllPinsFromTheirCurrentInterface(void)
{
	for (int i = 0; i < PINS_NUMBER_CONFIGURABLE; i++)
	{
		struct Pin pin =
		{
			.bank = (usbCurrentConfiguration.pins.assignments.map[i].pin >> 4) & 0x0f,
			.index = (usbCurrentConfiguration.pins.assignments.map[i].pin >> 0) & 0x0f
		};

		int interface = usbCurrentConfiguration.pins.assignments.map[i].interface;
		if (interface < NUMBER_OF_INTERFACES)
		{
			if (usbInterfaces[interface]->unassignPin)
				usbInterfaces[interface]->unassignPin(pin);
		}
	}
}

static void assignAllPinsToTheirNewInterface(void)
{
	for (int i = 0; i < PINS_NUMBER_CONFIGURABLE; i++)
	{
		struct Pin pin =
		{
			.bank = (usbCurrentConfiguration.pins.assignments.map[i].pin >> 4) & 0x0f,
			.index = (usbCurrentConfiguration.pins.assignments.map[i].pin >> 0) & 0x0f
		};

		int interface = usbCurrentConfiguration.pins.assignments.map[i].interface;
		if (interface < NUMBER_OF_INTERFACES && usbInterfaces[interface]->assignPin)
		{
			struct PinState pinState;
			if (usbInterfaces[interface]->assignPin(&pinState, pin, usbCurrentConfiguration.pins.assignments.map[i].args))
				pinsSetState(pin, &pinState);
		}
	}
}
