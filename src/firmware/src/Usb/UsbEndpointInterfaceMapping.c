#include <xc.h>
#include <stdint.h>

#include "Usb.h"

uint8_t usbMapEndpointToInterface(uint8_t endpoint)
{
	switch (endpoint)
	{
		case CORE_EP_ID:
			return CORE_INTERFACE_ID;

		case TIMER_EP_ID:
			return TIMER_INTERFACE_ID;

		case CCP_EP_ID:
			return CCP_INTERFACE_ID;

		case USB_EP_ID:
			return USB_INTERFACE_ID;

		case I2C_EP_ID:
			return I2C_INTERFACE_ID;

		case UART_EP_ID:
			return UART_INTERFACE_ID;

		case SPI_EP_ID:
			return SPI_INTERFACE_ID;

		case ADC_EP_ID:
			return ADC_INTERFACE_ID;

		case DAC_EP_ID:
			return DAC_INTERFACE_ID;

		case COMPARATOR_EP_ID:
			return COMPARATOR_INTERFACE_ID;

		case CLC_EP_ID:
			return CLC_INTERFACE_ID;

		default:
			return 0xff;
	}
}

uint8_t usbMapInterfaceToEndpoint(uint8_t interface)
{
	switch (interface)
	{
		case CORE_INTERFACE_ID:
			return CORE_EP_ID;

		case TIMER_INTERFACE_ID:
			return TIMER_EP_ID;

		case CCP_INTERFACE_ID:
			return CCP_EP_ID;

		case USB_INTERFACE_ID:
			return USB_EP_ID;

		case I2C_INTERFACE_ID:
			return I2C_EP_ID;

		case UART_INTERFACE_ID:
			return UART_EP_ID;

		case SPI_INTERFACE_ID:
			return SPI_EP_ID;

		case ADC_INTERFACE_ID:
			return ADC_EP_ID;

		case DAC_INTERFACE_ID:
			return DAC_EP_ID;

		case COMPARATOR_INTERFACE_ID:
			return COMPARATOR_EP_ID;

		case CLC_INTERFACE_ID:
			return CLC_EP_ID;

		default:
			return 0xff;
	}
}
