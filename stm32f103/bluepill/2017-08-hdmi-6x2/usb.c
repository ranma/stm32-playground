#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>

#include "stm32f103.h"
#include "usb.h"
#include "jtag.h"
#include "tasklet.h"

struct line_coding {
	uint32_t	dwDTERate;
	uint8_t		bCharFormat;
	uint8_t		bParityType;
	uint8_t		bDataBits;
	uint8_t		lineState;
};

static struct line_coding line_coding = { 115200, 0x00, 0x00, 0x00, 0x00 };


enum string_descriptor_indexes {
	STRING_DESCRIPTOR_LANGUAGE = 0,
	STRING_DESCRIPTOR_MANUFACTURER,
	STRING_DESCRIPTOR_PRODUCT,
};

enum descriptor_type {
	DEVICE_DESCRIPTOR = 1,
	CONFIGURATION_DESCRIPTOR,
	STRING_DESCRIPTOR,
	INTERFACE_DESCRIPTOR,
	ENDPOINT_DESCRIPTOR,
	CDC_DESCRIPTOR = 11,
	HID_DESCRIPTOR = 0x21,
	HID_REPORT_DESCRIPTOR,
};

struct device_descriptor {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint16_t	bcdUSB;
	uint8_t		bDeviceClass;
	uint8_t		bDeviceSubClass;
	uint8_t		bDeviceProtocol;
	uint8_t		bMaxPacketSize0;
	uint16_t	idVendor;
	uint16_t	idProduct;
	uint16_t	bcdDevice;
	uint8_t		iManufacturer;
	uint8_t		iProduct;
	uint8_t		iSerialNumber;
	uint8_t		bNumConfigurations;
};

struct interface_descriptor {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint8_t		bInterfaceNumber;
	uint8_t		bAlternateSetting;
	uint8_t		bNumEndpoints;
	uint8_t		bInterfaceClass;
	uint8_t		bInterfaceSubClass;
	uint8_t		bInterfaceProtocol;
	uint8_t		iInterface;
} __attribute__((packed));

// Endpoint descriptor
struct endpoint_descriptor {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint8_t		bEndpointAddress;
	uint8_t		bmAttributes;
	uint16_t	wMaxPacketSize;
	uint8_t		bInterval;
} __attribute__((packed));

struct iad_descriptor {
	uint8_t len;		// 8
	uint8_t dtype;		// 11
	uint8_t firstInterface;
	uint8_t interfaceCount;
	uint8_t functionClass;
	uint8_t functionSubClass;
	uint8_t functionProtocol;
	uint8_t iInterface;
};

struct cdc_cs_interface_descriptor {
	uint8_t len;	// 5
	uint8_t dtype;	// 0x24
	uint8_t subtype;
	uint8_t d0;
	uint8_t d1;
} __attribute__((packed));

struct cdc_cs_interface_descriport4 {
	uint8_t len;		// 4
	uint8_t dtype;		// 0x24
	uint8_t subtype;
	uint8_t d0;
};

struct cm_functional_descriptor {
	uint8_t	len;
	uint8_t	dtype;		// 0x24
	uint8_t	subtype;	// 1
	uint8_t	bmCapabilities;
	uint8_t	bDataInterface;
} __attribute__((packed));

struct acm_functional_descriptor {
	uint8_t	len;
	uint8_t	dtype;		// 0x24
	uint8_t	subtype;	// 1
	uint8_t	bmCapabilities;
};

struct cdc_descriptor {
	struct iad_descriptor			iad;	// Only needed on compound device

	//	Control
	struct interface_descriptor		cif;
	struct cdc_cs_interface_descriptor	header;
	struct cm_functional_descriptor		callManagement;
	struct acm_functional_descriptor	controlManagement;
	struct cdc_cs_interface_descriptor	functionalDescriptor;
	struct endpoint_descriptor		ctrl;

	//	Data
	struct interface_descriptor		dif;
	struct endpoint_descriptor		in;
	struct endpoint_descriptor		out;
} __attribute__((packed));

struct usbdm_descriptor {
	struct interface_descriptor		dif;
	struct endpoint_descriptor		in;
	struct endpoint_descriptor		out;
} __attribute__((packed));

struct configuration_descriptor {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint16_t	wTotalLength;
	uint8_t		bNumInterfaces;
	uint8_t		bConfigurationValue;
	uint8_t		iConfiguration;
	uint8_t		bmAttributes;
	uint8_t		bMaxPower;
	struct usbdm_descriptor usbdm_interface;
	struct cdc_descriptor cdc_interface;
} __attribute__((packed));


struct ep_ctx {
	volatile uint32_t *rx_pm;
	volatile uint32_t *tx_pm;
	uint32_t (*rx_fn)(struct ep_ctx *ctx, int ep);
	uint32_t (*tx_fn)(struct ep_ctx *ctx, int ep);
};

struct ep_ctx ep_ctx[USB_MAX_ENDPOINTS];
int usb_configuration;
static int set_address;

static const void* setup_response;
static int setup_response_remain;

const struct device_descriptor device_descriptor =
{
	.bLength            = sizeof(struct device_descriptor),
	.bDescriptorType    = DEVICE_DESCRIPTOR,
	.bcdUSB             = 0x0200, // USB 2.0
	.bDeviceClass       = 0,      // Defined at interface level
	.bDeviceSubClass    = 0,
	.bDeviceProtocol    = 0,
	.bMaxPacketSize0    = 64,     // EP0 max packet size
	.idVendor           = 0x15a2, // Freescale Semiconductor, Inc.
	.idProduct          = 0x0042, // OSBDM debug port
	.bcdDevice          = 0x0100, // Device version number
	.iManufacturer      = STRING_DESCRIPTOR_MANUFACTURER,
	.iProduct           = STRING_DESCRIPTOR_PRODUCT,
	.iSerialNumber      = 0,
	.bNumConfigurations = 1,      // Configuration count
};

const struct configuration_descriptor configuration_descriptor =
{
  .bLength                = 9,     // Size of the configuration descriptor
  .bDescriptorType        = CONFIGURATION_DESCRIPTOR,
  .wTotalLength           = sizeof(configuration_descriptor),  // Total size of the configuration descriptor and EP/interface descriptors
  .bNumInterfaces         = 3,     // Interface count
  .bConfigurationValue    = 1,     // Configuration identifer
  .iConfiguration         = 0,
  .bmAttributes           = 0x80,  // Bus powered
  .bMaxPower              = 50,    // Max power of 50*2mA = 100mA
  .usbdm_interface = {
    .dif = {
      .bLength            = 9,    // Size of the interface descriptor
      .bDescriptorType    = INTERFACE_DESCRIPTOR,
      .bInterfaceNumber   = 0,    // Interface index
      .bAlternateSetting  = 0,
      .bNumEndpoints      = 2,    // 2 endpoints (bulk in, bulk out)
      .bInterfaceClass    = 0xff, // Vendor specific
      .bInterfaceSubClass = 0xff,
      .bInterfaceProtocol = 0xff,
      .iInterface         = 0,
    },
    .in = {
      .bLength            = 7,    // Size of the endpoint descriptor
      .bDescriptorType    = ENDPOINT_DESCRIPTOR,
      .bEndpointAddress   = 0x82, // EP2 IN
      .bmAttributes       = 0x02, // Bulk EP
      .wMaxPacketSize     = 64,   // 64 byte packet buffer
      .bInterval          = 0,
    },
    .out = {
      .bLength            = 7,    // Size of the endpoint descriptor
      .bDescriptorType    = ENDPOINT_DESCRIPTOR,
      .bEndpointAddress   = 0x01, // EP1 OUT
      .bmAttributes       = 0x02, // Bulk EP
      .wMaxPacketSize     = 64,   // 64 byte packet buffer
      .bInterval          = 0,
    },
  },
  .cdc_interface = {
      .iad = {
          .len                = 8,
          .dtype              = CDC_DESCRIPTOR,  // 11
          .firstInterface     = 1,
          .interfaceCount     = 2,
          .functionClass      = 2,    // CDC
          .functionSubClass   = 2,    // Abstract control model
          .functionProtocol   = 1,    // AT-commands
          .iInterface         = 0,
        },
      .cif = {
          .bLength            = 9,    // Size of the interface descriptor
          .bDescriptorType    = INTERFACE_DESCRIPTOR,
          .bInterfaceNumber   = 1,    // Interface index
          .bAlternateSetting  = 0,
          .bNumEndpoints      = 1,    // 1 endpoint (EP2 in)
          .bInterfaceClass    = 2,    // CDC
          .bInterfaceSubClass = 2,    // Abstract control model
          .bInterfaceProtocol = 0,    // No class specific protocolrequired
          .iInterface         = 0,
        },
      .header = {
          .len                = 5,
          .dtype              = 0x24, // CS interface
          .subtype            = 0x00, // CDC header
          .d0                 = 0x10, // minor version
          .d1                 = 0x01, // major version
        },
      .callManagement = {
          .len                = 5,
          .dtype              = 0x24, // CS interface
          .subtype            = 0x01, // CDC call management
          .bmCapabilities     = 1, // Supports call management
          .bDataInterface     = 1,
        },
      .controlManagement = {
          .len                = 4,
          .dtype              = 0x24, // CS interface
          .subtype            = 2, // CDC abstract control management
          .bmCapabilities     = 6, // SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE
        },
      .functionalDescriptor = {
          .len                = 5,
          .dtype              = 0x24, // CS interface
          .subtype            = 6, // CDC union
          .d0                 = 1, // ACM interface idx
          .d1                 = 2, // Data interface idx
        },
      .ctrl = {
          .bLength            = 7,    // Size of the endpoint descriptor
          .bDescriptorType    = ENDPOINT_DESCRIPTOR,
          .bEndpointAddress   = 0x84, // EP4 IN
          .bmAttributes       = 0x03, // Interrupt EP
          .wMaxPacketSize     = 64,   // 64 byte packet buffer
          .bInterval          = 100,  // poll every 100ms
        },
      .dif = {
          .bLength            = 9,    // Size of the interface descriptor
          .bDescriptorType    = INTERFACE_DESCRIPTOR,
          .bInterfaceNumber   = 2,    // Interface index
          .bAlternateSetting  = 0,
          .bNumEndpoints      = 2,    // 2 endpoints (bulk in, bulk out)
          .bInterfaceClass    = 0xa,  // CDC data
          .bInterfaceSubClass = 0,    // n/a
          .bInterfaceProtocol = 0,    // n/a
          .iInterface         = 0,
        },
    .in = {
        .bLength            = 7,    // Size of the endpoint descriptor
        .bDescriptorType    = ENDPOINT_DESCRIPTOR,
        .bEndpointAddress   = 0x83, // EP3 IN
        .bmAttributes       = 0x02, // Bulk EP
        .wMaxPacketSize     = 64,   // 64 byte packet buffer
        .bInterval          = 0,
      },
      .out = {
        .bLength            = 7,    // Size of the endpoint descriptor
        .bDescriptorType    = ENDPOINT_DESCRIPTOR,
        .bEndpointAddress   = 0x03, // EP3 OUT
        .bmAttributes       = 0x02, // Bulk EP
        .wMaxPacketSize     = 64,   // 64 byte packet buffer
        .bInterval          = 0,
      },
  },
};

static const char *string_descriptors[] = {
	"\x04\x03\x09\x04",
	"\x0e\x03r\0a\0n\0m\0a\0@\0",
	"\x0a\x03T\0e\0s\0t\0",
};

static void usb2mem(void *mem_dst, volatile uint32_t *usb_pm_src, int n)
{
	uint16_t *dst = mem_dst;
	while (n >= 8) {
		dst[0] = usb_pm_src[0];
		dst[1] = usb_pm_src[1];
		dst[2] = usb_pm_src[2];
		dst[3] = usb_pm_src[3];
		dst += 4;
		usb_pm_src += 4;
		n -= 8;
	}
	while (n > 1) {
		*(dst++) = *(usb_pm_src++);
		n -= 2;
	}
	if (n) {
		*((uint8_t*)dst) = *usb_pm_src;
	}
}

static void usbdma2mem(void *mem_dst, volatile uint32_t *usb_pm_src, int n)
{
	DMA1->CH[0].CCR = 0;
	DMA1->CH[0].CNDTR = n / 2;
	DMA1->CH[0].CPAR = (uint32_t) usb_pm_src;
	DMA1->CH[0].CMAR = (uint32_t) mem_dst;
	DMA1->CH[0].CCR = 0x000046c1;
	while (DMA1->CH[0].CNDTR);
	DMA1->CH[0].CCR = 0;
}

static void mem2mem(void *mem_dst, void *mem_src, int n)
{
	uint32_t *dst = mem_dst;
	uint32_t *src = mem_src;
	while (n > 0) {
		*(dst++) = *(src++);
		n -= 4;
	}
}

static void mem2usb(volatile uint32_t *usb_pm_dst, const void *mem_src, int n)
{
	const uint16_t *src = mem_src;
	while (n > 0) {
		*(usb_pm_dst++) = *(src++);
		n -= 2;
	}
}

static void usb_reset(void)
{
	usb_configuration = 0;
	set_address = 0;

	// Set up EP0 (Control)
	// Clear toggle bits.
	USB->EP[0] ^= 0;
	// Clear interrupt status and set state.
	USB->EP[0] = USB_EP_RX_VALID | USB_EP_TX_NAK | USB_EP_CONTROL;
	// Set up EP1
	USB->EP[1] ^= 0;
	USB->EP[1] = USB_EP_RX_VALID | USB_EP_TX_NAK | USB_EP_BULK | 1;
	// Set up EP2
	USB->EP[2] ^= 0;
	USB->EP[2] = USB_EP_RX_NAK | USB_EP_TX_NAK | USB_EP_BULK | 2;
	// Set up EP3
	USB->EP[3] ^= 0;
	USB->EP[3] = USB_EP_RX_VALID | USB_EP_TX_NAK | USB_EP_BULK | 3;
	// Set up EP4
	USB->EP[4] ^= 0;
	USB->EP[4] = USB_EP_RX_VALID | USB_EP_TX_NAK | USB_EP_INTERRUPT | 4;
	// Set address to 0 (unconfigured)
	USB->DADDR = USB_DADDR_EF;
}

struct usb_request {
	uint8_t type;
	uint8_t request;
	uint16_t value;
	uint16_t index;
	uint16_t length;
};

static int usb_write_descriptor(struct usb_request *request)
{
	const void *src = NULL;
	uint32_t tx_count = 0;

	switch (request->value >> 8) {
	case DEVICE_DESCRIPTOR:
		src = &device_descriptor;
		break;
	case CONFIGURATION_DESCRIPTOR:
		src = &configuration_descriptor;
		tx_count = sizeof(configuration_descriptor);
		break;
	case STRING_DESCRIPTOR:
		src = string_descriptors[request->value & 0xff];
		break;
	default:
		break;
	}

	if (src == NULL) {
		tx_count = -1;
	} else {
		if (tx_count == 0) {
			tx_count = *((uint8_t*)src);
		}
		tx_count = min(tx_count, request->length);
		setup_response = src;
		setup_response_remain = tx_count;
	}
	return tx_count;
}

static int usb_setup(int ep, void* setup_buf, int nrx)
{
	struct usb_request *request = setup_buf;
	int tx_count = 0;
	volatile uint32_t *tx_pm = ep_ctx[0].tx_pm;

	switch (request->request) {
	case 0: // Get status
		*tx_pm = 0x0000;
		tx_count = 2;
		break;
	case 5: // Set address. Deferred until status phase.
		set_address = request->value | USB_DADDR_EF;
		break;
	case 6: {// Get descriptor
		int towrite = usb_write_descriptor(request);
		tx_count = min(64, towrite);
		if (tx_count > 0) {
			mem2usb(tx_pm, setup_response, min(64, tx_count));
			if (setup_response_remain <= 64) {
				setup_response = NULL;
				setup_response_remain = 0;
			} else {
				setup_response += 64;
				setup_response_remain -= 64;
			}
		}
		}
		break;
	case 8: // Get configuration
		*tx_pm = usb_configuration;
		tx_count = 1;
		break;
	case 9: // Set configuration
		usb_configuration = request->value;
		break;
	case 0x20: // CDC set line coding
		// ignored
		break;
	case 0x21: // CDC get line coding
		tx_count = sizeof(struct line_coding);
		mem2usb(tx_pm, &line_coding, tx_count);
		break;
	case 0x22: // CDC set control line state
		line_coding.lineState = request->value;
		break;
	default:
		printf("SETUP t=%x r=%x v=%x i=%x l=%x\n",
			request->type,
			request->request,
			request->value,
			request->index,
			request->length);
	}

	return tx_count;
}

volatile int usb_irq;
int usb_irqs = 0;
int usb_errs = 0;

static struct usb_request setup_request;
int setup_nrx;

static uint32_t ep0_rx(struct ep_ctx *ctx, int unused_ep)
{
	uint32_t ep_status = USB->EP[0];
	uint32_t ep_status_wb = USB_EP_RX_VALID;
	int nrx = USB_BTABLE[0].RX_COUNT & USB_RX_COUNT_MASK;

	if (ep_status & USB_EP_SETUP) {
		usb2mem(&setup_request, ep_ctx[0].rx_pm, min(sizeof(setup_request), nrx));
		setup_nrx = nrx;
		int tx_count = usb_setup(0, &setup_request, nrx);
		if (tx_count >= 0) {
			ep_status_wb |= (ep_status & USB_EPTX_STAT) ^ USB_EP_TX_VALID;
			USB_BTABLE[0].TX_COUNT = tx_count;
			if (setup_response_remain != 0) {
				ep_status_wb |= USB_EP_STATUS_OUT;
			}
		}
	} else {
		if (nrx > 0) {
			printf("ep0r%x\n", nrx);
		}
	}

	return ep_status_wb;
}

static uint32_t ep0_tx(struct ep_ctx *ctx, int unused_ep)
{
	uint32_t ep_status_wb = USB_EP_TX_VALID;
	volatile uint32_t *tx_pm = ep_ctx[0].tx_pm;

	if (set_address) {
		// Execute deferred address change
		USB->DADDR = set_address;
		set_address = 0;
	}
	if (setup_response_remain > 0) {
		int txlen = min(64, setup_response_remain);
		mem2usb(tx_pm, setup_response, txlen);
		setup_response += txlen;
		setup_response_remain -= txlen;
		USB_BTABLE[0].TX_COUNT = txlen;
		if (setup_response_remain != 0) {
			ep_status_wb |= USB_EP_STATUS_OUT;
		}
	} else {
		ep_status_wb = USB_EP_TX_STALL;
	}

	return ep_status_wb;
}

extern int usb_try_getc(char *c);
extern int usb_try_putc(char c);
int usb_busy = 0;

static int maybe_send_serial_data(volatile uint32_t *pm)
{
	int maxwords = 32;
	int ntx = 0;
	while (--maxwords) {
		uint8_t c1, c2;
		if (!usb_try_getc((char*)&c1))
			break;
		ntx++;
		if (usb_try_getc((char*)&c2)) {
			ntx++;
		} else {
			c2 = 0;
		}
		*(pm++) = c1 | (c2 << 8);
		// If we had a short read, break so we don't end up with
		// holes in the buffer.
		if (ntx & 1)
			break;
	}
	return ntx;
}

static uint32_t ep3_rx(struct ep_ctx *ctx, int unused_ep)
{
	int nrx = USB_BTABLE[3].RX_COUNT & USB_RX_COUNT_MASK;
	char buf[64];
	usb2mem(buf, ctx->rx_pm, min(sizeof(buf), nrx));

	for (int i = 0; i < nrx; i++) {
		usb_try_putc(buf[i]);
	}

	return USB_EP_RX_VALID;
}

static uint32_t ep3_tx(struct ep_ctx *ctx, int unused_ep)
{
	int ntx;
	if ((ntx = maybe_send_serial_data(ctx->tx_pm)) > 0) {
		usb_busy = 1;
		USB_BTABLE[3].TX_COUNT = ntx;
		return USB_EP_TX_VALID;
	}

	usb_busy = 0;
	return USB_EP_TX_NAK;
}

void poll_usb_console(void)
{
	if (!usb_busy) {
		uint32_t ep_status = USB->EP[3];
		uint32_t ep_status_wb = (ep_status & USB_EP_TX_WB_MASK) | USB_EP_IRQ_MASK;
		ep_status_wb |= (ep_status & USB_EPTX_STAT) ^ ep3_tx(&ep_ctx[3], 3);
		USB->EP[3] = ep_status_wb;
	}
}

static uint32_t ep4_rx(struct ep_ctx *ctx, int unused_ep)
{
	int nrx = USB_BTABLE[4].RX_COUNT & USB_RX_COUNT_MASK;
	printf("ep4rx %x\n", nrx);

	return USB_EP_RX_VALID;
}

static uint32_t ep4_tx(struct ep_ctx *ctx, int unused_ep)
{
	printf("ep4tx\n");

	return USB_EP_TX_NAK;
}

static uint8_t jtag_rx[64];
static uint8_t jtag_tx[64];

static uint32_t ep1_rx(struct ep_ctx *ctx, int unused_ep)
{
	int nrx = USB_BTABLE[1].RX_COUNT & USB_RX_COUNT_MASK;

	usb2mem(jtag_rx, ctx->rx_pm, nrx);

	int ntx = jtag_rxtx(jtag_rx, jtag_tx);
	// Send response
	if (ntx) {
		mem2usb(ep_ctx[2].tx_pm, jtag_tx, ntx);
		USB_BTABLE[2].TX_COUNT = ntx;
		uint32_t ep_status = USB->EP[2];
		uint32_t ep_status_wb = ep_status & USB_EP_TX_WB_MASK;
		ep_status_wb |= (ep_status & USB_EPTX_STAT) ^ USB_EP_TX_VALID;
		USB->EP[2] = ep_status_wb;
	}

	return USB_EP_RX_VALID;
}

static struct tasklet usb_tasklet;

void poll_usb(void);

static void usb_bh(struct tasklet *ctx)
{
	poll_usb();
}

void USB_LP_CAN_RX0_IRQHandler(void)
{
	usb_irq = 1;
	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	tasklet_schedule(&usb_tasklet);
}

void poll_usb(void)
{
	if (!usb_irq) {
		return;
	}

	uint32_t istr = USB->ISTR;
	uint32_t ep = istr & 7;
	usb_irqs++;

	if (istr & USB_ISTR_ERR) {
		uint32_t ep_status = USB->EP[ep];
		usb_errs++;
		printf("ERR %lx %lx\n", istr, ep_status);
	}
	if (istr & USB_ISTR_RESET) {
		usb_reset();
	} else if (istr & USB_ISTR_CTR) {
		struct ep_ctx *ctx = &ep_ctx[ep];
		uint32_t ep_status = USB->EP[ep];
		uint32_t ep_status_wb = (ep_status & USB_EP_WB_MASK) | USB_EP_IRQ_MASK;
		if (ep == 0) {
			ep_status_wb &= ~USB_EP_STATUS_OUT;
		}
		if (ep_status & USB_EP_CTR_RX) {
			ep_status_wb &= ~USB_EP_CTR_RX;
			if (ctx->rx_fn) {
				ep_status_wb |= (ep_status & USB_EPRX_STAT) ^ ctx->rx_fn(ctx, ep);
			}
		}
		if (ep_status & USB_EP_CTR_TX) {
			ep_status_wb &= ~USB_EP_CTR_TX;
			if (ctx->tx_fn) {
				ep_status_wb |= (ep_status & USB_EPTX_STAT) ^ ctx->tx_fn(ctx, ep);
			}
		}
		USB->EP[ep] = ep_status_wb;
	}

	usb_irq = 0;
	// Clear only interrupt status bits that were set when we
	// polled it.
	USB->ISTR = ~istr;
	NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

static uint8_t test_buf[512];
static uint8_t test_buf2[512];

void usb_benchmark(void)
{
	uint32_t ta, tb;

	ta = SYSTICK->CVR;
	mem2mem(test_buf, test_buf2, sizeof(test_buf));
	tb = SYSTICK->CVR;
	printf("mem2mem %lx (%lx %lx)\n", ta - tb, ta, tb);

	ta = SYSTICK->CVR;
	usb2mem(test_buf, USB_PM, sizeof(test_buf));
	tb = SYSTICK->CVR;
	printf("usb2mem %lx (%lx %lx)\n", ta - tb, ta, tb);

	ta = SYSTICK->CVR;
	usbdma2mem(test_buf2, USB_PM, sizeof(test_buf2));
	tb = SYSTICK->CVR;
	printf("usbdma2mem %lx (%lx %lx)\n", ta - tb, ta, tb);
}

void usb_init(void)
{
	tasklet_register(&usb_tasklet, "usb_irq", usb_bh);

	// Enable alternate function IO, GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// Output LOW on D+ (PA12) to reset USB when
	// the controller is disabled
	GPIOA->CRH &= ~(0xF << 16);
	GPIOA->CRH |= 0x1 << 16; // Push-pull output
	GPIOA->BRR = 1 << 12; // Pull PA12 low

	mdelay(10);

	GPIOA->CRH &= ~(0xF << 16);
	GPIOA->CRH |= 0x4 << 16; // Default input

	// Enable USB
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;

	// Power up USB, this automatically connects the PA11/PA12 as
	// DP-/DP+ lines without further GPIO configuration
	USB->CNTR = USB_CNTR_FRES;

	// tStartup is 1us
	udelay(1);

	// Release USB reset
	USB->CNTR = 0;

	// Set buffer descriptor table offset
	USB->BTABLE = 0;
	ep_ctx[0].tx_fn = ep0_tx;
	ep_ctx[0].rx_fn = ep0_rx;
	ep_ctx[0].tx_pm = &USB_PM[0x40 / 2];
	ep_ctx[0].rx_pm = &USB_PM[0x80 / 2];
	USB_BTABLE[0].TX_ADDR  = 0x40;
	USB_BTABLE[0].TX_COUNT = 0x0;
	USB_BTABLE[0].RX_ADDR  = 0x80;
	USB_BTABLE[0].RX_COUNT = USB_RX_SIZE64;
	ep_ctx[3].tx_fn = ep3_tx;
	ep_ctx[3].rx_fn = ep3_rx;
	ep_ctx[3].tx_pm = &USB_PM[0xc0 / 2];
	ep_ctx[3].rx_pm = &USB_PM[0x100 / 2];
	USB_BTABLE[3].TX_ADDR  = 0xc0;
	USB_BTABLE[3].TX_COUNT = 0x0;
	USB_BTABLE[3].RX_ADDR  = 0x100;
	USB_BTABLE[3].RX_COUNT = USB_RX_SIZE64;
	ep_ctx[4].tx_fn = ep4_tx;
	ep_ctx[4].rx_fn = ep4_rx;
	ep_ctx[4].tx_pm = &USB_PM[0x140 / 2];
	ep_ctx[4].rx_pm = &USB_PM[0x180 / 2];
	USB_BTABLE[4].TX_ADDR  = 0x140;
	USB_BTABLE[4].TX_COUNT = 0x0;
	USB_BTABLE[4].RX_ADDR  = 0x180;
	USB_BTABLE[4].RX_COUNT = USB_RX_SIZE64;
	ep_ctx[1].tx_fn = NULL;
	ep_ctx[1].rx_fn = ep1_rx;
	ep_ctx[1].tx_pm = &USB_PM[0x1c0 / 2];
	ep_ctx[1].rx_pm = &USB_PM[0x1c0 / 2];
	USB_BTABLE[1].TX_ADDR  = 0x1c0;
	USB_BTABLE[1].TX_COUNT = 0x0;
	USB_BTABLE[1].RX_ADDR  = 0x1c0;
	USB_BTABLE[1].RX_COUNT = USB_RX_SIZE64;
	ep_ctx[2].tx_fn = NULL;
	ep_ctx[2].rx_fn = NULL;
	ep_ctx[2].tx_pm = &USB_PM[0x1c0 / 2];
	ep_ctx[2].rx_pm = &USB_PM[0x1c0 / 2];
	USB_BTABLE[2].TX_ADDR  = 0x1c0;
	USB_BTABLE[2].TX_COUNT = 0x0;
	USB_BTABLE[2].RX_ADDR  = 0x1c0;
	USB_BTABLE[2].RX_COUNT = USB_RX_SIZE64;

	usb_reset();

	// Clear interrupt status
	USB->ISTR = 0;
	usb_irqs = 0;
	usb_errs = 0;
	setup_nrx = 0;

	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

	// Enable transfer / error / reset interrupts
	USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM;
}
