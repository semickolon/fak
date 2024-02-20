#include "usb.h"
#include "ch552.h"
#include "math.h"

#include <string.h>

#define MIN(a, b) ((a > b) ? b : a)
#define MSB(u16) (u16 >> 8)
#define LSB(u16) (u16 & 0xFF)

#ifdef USB_MANUFACTURER_STR
#define USB_MANUFACTURER_STR_IDX 1
#else
#define USB_MANUFACTURER_STR_IDX 0
#endif

#ifdef USB_PRODUCT_STR
#define USB_PRODUCT_STR_IDX 2
#else
#define USB_PRODUCT_STR_IDX 0
#endif

#ifdef USB_SERIAL_NO_STR
#define USB_SERIAL_NO_STR_IDX 3
#else
#define USB_SERIAL_NO_STR_IDX 0
#endif

#if defined(USB_MANUFACTURER_STR) || defined(USB_PRODUCT_STR) || defined(USB_SERIAL_NO_STR)
#define USB_STRINGS_ENABLE
#endif

#define ITF_NUM_KEYBOARD 0

#ifdef CONSUMER_KEYS_ENABLE
#define ITF_NUM_CONSUMER 1
#endif

#ifdef MOUSE_KEYS_ENABLE
#ifdef CONSUMER_KEYS_ENABLE
#define ITF_NUM_MOUSE 2
#else
#define ITF_NUM_MOUSE 1
#endif
#endif

typedef struct {
    USB_CFG_DESCR cfg_descr;
    USB_ITF_DESCR itf_keyboard_descr;
    USB_HID_DESCR hid_keyboard_descr;
    USB_ENDP_DESCR endp1_in_descr;
#ifdef CONSUMER_KEYS_ENABLE
    USB_ITF_DESCR itf_consumer_descr;
    USB_HID_DESCR hid_consumer_descr;
    USB_ENDP_DESCR endp2_in_descr;
#endif
#ifdef MOUSE_KEYS_ENABLE
    USB_ITF_DESCR itf_mouse_descr;
    USB_HID_DESCR hid_mouse_descr;
    USB_ENDP_DESCR endp3_in_descr;
#endif
} USB_CFG1_DESCR;

__code uint8_t *p_usb_tx;
__xdata __at(XADDR_USB_TX_LEN) uint8_t usb_tx_len;

__bit hid_protocol_keyboard;
#ifdef MOUSE_KEYS_ENABLE
__bit hid_protocol_mouse;
#endif

__xdata __at(XADDR_USB_EP0) uint8_t EP0_buffer[USB_EP0_SIZE];
__xdata __at(XADDR_USB_EP1) uint8_t EP1I_buffer[USB_EP1_SIZE];

#ifdef CONSUMER_KEYS_ENABLE
__xdata __at(XADDR_USB_EP2) uint16_t EP2I_buffer[USB_EP2_SIZE / 2];
#endif

#ifdef MOUSE_KEYS_ENABLE
__xdata __at(XADDR_USB_EP3) uint8_t EP3I_buffer[USB_EP3_SIZE];
#endif

__code USB_DEV_DESCR USB_DEVICE_DESCR = {
    .bLength = sizeof(USB_DEV_DESCR),
    .bDescriptorType = USB_DESCR_TYP_DEVICE,
    .bcdUSBL = 0x10,
    .bcdUSBH = 0x01,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = USB_EP0_SIZE,
    .idVendorL = LSB(USB_VENDOR_ID),
    .idVendorH = MSB(USB_VENDOR_ID),
    .idProductL = LSB(USB_PRODUCT_ID),
    .idProductH = MSB(USB_PRODUCT_ID),
    .bcdDeviceL = LSB(USB_PRODUCT_VER),
    .bcdDeviceH = MSB(USB_PRODUCT_VER),
    .iManufacturer = USB_MANUFACTURER_STR_IDX,
    .iProduct = USB_PRODUCT_STR_IDX,
    .iSerialNumber = USB_SERIAL_NO_STR_IDX,
    .bNumConfigurations = 1
};

__code USB_CFG1_DESCR USB_CONFIG1_DESCR = {
    .cfg_descr = {
        .bLength = sizeof(USB_CFG_DESCR),
        .bDescriptorType = USB_DESCR_TYP_CONFIG,
        .wTotalLengthL = LSB(sizeof(USB_CONFIG1_DESCR)),
        .wTotalLengthH = MSB(sizeof(USB_CONFIG1_DESCR)),
        .bNumInterfaces = USB_NUM_INTERFACES,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = 0xC0,
        .bMaxPower = 50
    },
    .itf_keyboard_descr = {
        .bLength = sizeof(USB_ITF_DESCR),
        .bDescriptorType = USB_DESCR_TYP_INTERF,
        .bInterfaceNumber = ITF_NUM_KEYBOARD,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_DEV_CLASS_HID,
        .bInterfaceSubClass = 1, // Boot interface
        .bInterfaceProtocol = 1, // Keyboard
        .iInterface = 0
    },
    .hid_keyboard_descr = {
        .bLength = sizeof(USB_HID_DESCR),
        .bDescriptorType = USB_DESCR_TYP_HID,
        .bcdHIDL = 0x11,
        .bcdHIDH = 0x01,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
        .bDescriptorTypeX = USB_DESCR_TYP_REPORT,
        .wDescriptorLengthL = LSB(sizeof(USB_HID_REPORT_DESCR)),
        .wDescriptorLengthH = MSB(sizeof(USB_HID_REPORT_DESCR))
    },
    .endp1_in_descr = {
        .bLength = sizeof(USB_ENDP_DESCR),
        .bDescriptorType = USB_DESCR_TYP_ENDP,
        .bEndpointAddress = USB_ENDP_DIR_MASK | 1, // IN 1
        .bmAttributes = USB_ENDP_TYPE_INTER,
        .wMaxPacketSizeL = LSB(USB_EP1_SIZE),
        .wMaxPacketSizeH = MSB(USB_EP1_SIZE),
        .bInterval = 1
    },
#ifdef CONSUMER_KEYS_ENABLE
    .itf_consumer_descr = {
        .bLength = sizeof(USB_ITF_DESCR),
        .bDescriptorType = USB_DESCR_TYP_INTERF,
        .bInterfaceNumber = ITF_NUM_CONSUMER,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_DEV_CLASS_HID,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0
    },
    .hid_consumer_descr = {
        .bLength = sizeof(USB_HID_DESCR),
        .bDescriptorType = USB_DESCR_TYP_HID,
        .bcdHIDL = 0x11,
        .bcdHIDH = 0x01,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
        .bDescriptorTypeX = USB_DESCR_TYP_REPORT,
        .wDescriptorLengthL = LSB(sizeof(USB_HID_CONSUMER_REPORT_DESCR)),
        .wDescriptorLengthH = MSB(sizeof(USB_HID_CONSUMER_REPORT_DESCR))
    },
    .endp2_in_descr = {
        .bLength = sizeof(USB_ENDP_DESCR),
        .bDescriptorType = USB_DESCR_TYP_ENDP,
        .bEndpointAddress = USB_ENDP_DIR_MASK | 2, // IN 2
        .bmAttributes = USB_ENDP_TYPE_INTER,
        .wMaxPacketSizeL = LSB(USB_EP2_SIZE),
        .wMaxPacketSizeH = MSB(USB_EP2_SIZE),
        .bInterval = 1
    },
#endif
#ifdef MOUSE_KEYS_ENABLE
    .itf_mouse_descr = {
        .bLength = sizeof(USB_ITF_DESCR),
        .bDescriptorType = USB_DESCR_TYP_INTERF,
        .bInterfaceNumber = ITF_NUM_MOUSE,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_DEV_CLASS_HID,
        .bInterfaceSubClass = 1, // Boot interface
        .bInterfaceProtocol = 2, // Mouse
        .iInterface = 0
    },
    .hid_mouse_descr = {
        .bLength = sizeof(USB_HID_DESCR),
        .bDescriptorType = USB_DESCR_TYP_HID,
        .bcdHIDL = 0x11,
        .bcdHIDH = 0x01,
        .bCountryCode = 0,
        .bNumDescriptors = 1,
        .bDescriptorTypeX = USB_DESCR_TYP_REPORT,
        .wDescriptorLengthL = LSB(sizeof(USB_HID_MOUSE_REPORT_DESCR)),
        .wDescriptorLengthH = MSB(sizeof(USB_HID_MOUSE_REPORT_DESCR))
    },
    .endp3_in_descr = {
        .bLength = sizeof(USB_ENDP_DESCR),
        .bDescriptorType = USB_DESCR_TYP_ENDP,
        .bEndpointAddress = USB_ENDP_DIR_MASK | 3, // IN 3
        .bmAttributes = USB_ENDP_TYPE_INTER,
        .wMaxPacketSizeL = LSB(USB_EP3_SIZE),
        .wMaxPacketSizeH = MSB(USB_EP3_SIZE),
        .bInterval = 1
    },
#endif
};

__code uint8_t USB_HID_REPORT_DESCR[] = {
    0x05, 0x01,
    0x09, 0x06,
    0xA1, 0x01,
    0x05, 0x07,
    0x19, 0xE0,
    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,
    0xC0
};

#ifdef CONSUMER_KEYS_ENABLE
__code uint8_t USB_HID_CONSUMER_REPORT_DESCR[] = {
    0x05, 0x0C,                     // Usage Page (Consumer Devices)
    0x09, 0x01,                     // Usage (Consumer Control)
    0xA1, 0x01,                     // Collection (Application)
    0x75, 0x10,                     //      Report Size (16)
    0x95, 0x04,                     //      Report Count (4)
    0x26, 0xFF, 0x03,               //      Logical Maximum (1023)
    0x19, 0x00,                     //      Usage Minimum (0)
    0x2A, 0xFF, 0x03,               //      Usage Maximum (1023)
    0x81, 0x00,                     //      Input (Data, Ary, Abs)
    0xC0
};
#endif

#ifdef MOUSE_KEYS_ENABLE
__code uint8_t USB_HID_MOUSE_REPORT_DESCR[] = {
    0x05, 0x01,     // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,     // USAGE (Mouse)
    0xA1, 0x01,     // COLLECTION (Application)
    0x09, 0x01,     //   USAGE (Pointer)
    0xA1, 0x00,     //   COLLECTION (Physical)
    0x05, 0x09,     //     USAGE_PAGE (Button)
    0x19, 0x01,     //     USAGE_MINIMUM (Button 1)
    0x29, 0x08,     //     USAGE_MAXIMUM (Button 8)
    0x15, 0x00,     //     LOGICAL_MINIMUM (0)
    0x25, 0x01,     //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,     //     REPORT_SIZE (1)
    0x95, 0x08,     //     REPORT_COUNT (8)
    0x81, 0x02,     //     INPUT (Data,Var,Abs)
    0x05, 0x01,     //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,     //     USAGE (X)
    0x09, 0x31,     //     USAGE (Y)
    0x09, 0x38,     //     USAGE (Wheel)
    0x15, 0x81,     //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,     //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,     //     REPORT_SIZE (8)
    0x95, 0x03,     //     REPORT_COUNT (3)
    0x81, 0x06,     //     INPUT (Data,Var,Rel)
    0xC0,           //   END_COLLECTION
    0xC0            // END_COLLECTION
};
#endif

#ifdef USB_STRINGS_ENABLE
__code uint8_t USB_STR0_DESCR[] = {
    sizeof(USB_STR0_DESCR),
    USB_DESCR_TYP_STRING,
    0x09, 0x04
};

#ifdef USB_MANUFACTURER_STR
__code uint8_t USB_STR1_DESCR[] = {
    sizeof(USB_STR1_DESCR),
    USB_DESCR_TYP_STRING,
    USB_MANUFACTURER_STR
};
#endif

#ifdef USB_PRODUCT_STR
__code uint8_t USB_STR2_DESCR[] = {
    sizeof(USB_STR2_DESCR),
    USB_DESCR_TYP_STRING,
    USB_PRODUCT_STR
};
#endif

#ifdef USB_SERIAL_NO_STR
__code uint8_t USB_STR3_DESCR[] = {
    sizeof(USB_STR3_DESCR),
    USB_DESCR_TYP_STRING,
    USB_SERIAL_NO_STR
};
#endif
#endif

static void USB_EP0_tx() {
    UEP0_T_LEN = MIN(usb_tx_len, USB_EP0_SIZE);

    if (UEP0_T_LEN) {
        memcpy(EP0_buffer, p_usb_tx, UEP0_T_LEN);
        usb_tx_len -= UEP0_T_LEN;
        p_usb_tx += UEP0_T_LEN;
    }
}

inline static void USB_EP0_SETUP() {
    UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG;

    USB_SETUP_REQ *setupPacket = (USB_SETUP_REQ *) EP0_buffer;
    usb_tx_len = 0;

    switch (setupPacket->bRequest) {
        case USB_GET_DESCRIPTOR:
            switch (setupPacket->wValueH) {
                case USB_DESCR_TYP_DEVICE:
                    usb_tx_len = sizeof(USB_DEVICE_DESCR);
                    p_usb_tx = (__code uint8_t *) &USB_DEVICE_DESCR;
                    break;
                case USB_DESCR_TYP_CONFIG:
                    usb_tx_len = sizeof(USB_CONFIG1_DESCR);
                    p_usb_tx = (__code uint8_t *) &USB_CONFIG1_DESCR;
                    break;
#ifdef USB_STRINGS_ENABLE
                case USB_DESCR_TYP_STRING:
                    switch (setupPacket->wValueL) {
                        case 0:
                            usb_tx_len = sizeof(USB_STR0_DESCR);
                            p_usb_tx = (__code uint8_t *) &USB_STR0_DESCR;
                            break;
#ifdef USB_MANUFACTURER_STR
                        case 1:
                            usb_tx_len = sizeof(USB_STR1_DESCR);
                            p_usb_tx = (__code uint8_t *) &USB_STR1_DESCR;
                            break;
#endif
#ifdef USB_PRODUCT_STR
                        case 2:
                            usb_tx_len = sizeof(USB_STR2_DESCR);
                            p_usb_tx = (__code uint8_t *) &USB_STR2_DESCR;
                            break;
#endif
#ifdef USB_SERIAL_NO_STR
                        case 3:
                            usb_tx_len = sizeof(USB_STR3_DESCR);
                            p_usb_tx = (__code uint8_t *) &USB_STR3_DESCR;
                            break;
#endif
                    }
                    break;
#endif
                case USB_DESCR_TYP_REPORT:
                    switch (setupPacket->wIndexL) {
                    case ITF_NUM_KEYBOARD:
                        usb_tx_len = sizeof(USB_HID_REPORT_DESCR);
                        p_usb_tx = (__code uint8_t *) &USB_HID_REPORT_DESCR;
                        break;
#ifdef CONSUMER_KEYS_ENABLE
                    case ITF_NUM_CONSUMER:
                        usb_tx_len = sizeof(USB_HID_CONSUMER_REPORT_DESCR);
                        p_usb_tx = (__code uint8_t *) &USB_HID_CONSUMER_REPORT_DESCR;
                        break;
#endif
#ifdef MOUSE_KEYS_ENABLE
                    case ITF_NUM_MOUSE:
                        usb_tx_len = sizeof(USB_HID_MOUSE_REPORT_DESCR);
                        p_usb_tx = (__code uint8_t *) &USB_HID_MOUSE_REPORT_DESCR;
                        break;
#endif
                    }
                    break;
            }

            if (usb_tx_len) {
                if (usb_tx_len > setupPacket->wLengthL) usb_tx_len = setupPacket->wLengthL;
                UDEV_CTRL |= bUD_GP_BIT;
                USB_EP0_tx();
                return;
            }
            break;
        
        case USB_SET_ADDRESS:
            usb_tx_len = setupPacket->wValueL;
            UDEV_CTRL &= ~bUD_GP_BIT;
            return;
        
        case USB_SET_CONFIGURATION:
            USB_DEV_AD = (USB_DEV_AD & MASK_USB_ADDR) | (setupPacket->wValueL << 7); // bUDA_GP_BIT
            return;
        
        case USB_GET_CONFIGURATION:
            EP0_buffer[0] = USB_DEV_AD >> 7; // bUDA_GP_BIT
            UEP0_T_LEN = 1;
            return;

        case USB_GET_STATUS:
            EP0_buffer[0] = 0;
            EP0_buffer[1] = 0;
            UEP0_T_LEN = 2;
            return;
        
        case HID_GET_REPORT:
            if (setupPacket->bRequestType == (USB_REQ_TYP_IN | USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF)) {
                switch (setupPacket->wIndexL) {
                case ITF_NUM_KEYBOARD:
                    for (uint8_t i = 0; i < USB_EP1_SIZE; i++) {
                        EP0_buffer[i] = EP1I_buffer[i];
                    }
                    UEP0_T_LEN = USB_EP1_SIZE;
                    return;
#ifdef CONSUMER_KEYS_ENABLE
                case ITF_NUM_CONSUMER:
                    for (uint8_t i = 0; i < USB_EP2_SIZE; i++) {
                        EP0_buffer[i] = ((uint8_t*) EP2I_buffer)[i];
                    }
                    UEP0_T_LEN = USB_EP2_SIZE;
                    return;
#endif
#ifdef MOUSE_KEYS_ENABLE
                case ITF_NUM_MOUSE:
                    for (uint8_t i = 0; i < USB_EP3_SIZE; i++) {
                        EP0_buffer[i] = EP3I_buffer[i];
                    }
                    UEP0_T_LEN = USB_EP3_SIZE;
                    return;
#endif
                }
            }
            break;
        
        case HID_GET_PROTOCOL:
            if (setupPacket->bRequestType == (USB_REQ_TYP_IN | USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF)) {
                switch (setupPacket->wIndexL) {
                case ITF_NUM_KEYBOARD:
                    EP0_buffer[0] = hid_protocol_keyboard;
                    UEP0_T_LEN = 1;
                    return;
#ifdef MOUSE_KEYS_ENABLE
                case ITF_NUM_MOUSE:
                    EP0_buffer[0] = hid_protocol_mouse;
                    UEP0_T_LEN = 1;
                    return;
#endif
                }
             }
             break;
        
        case HID_SET_PROTOCOL:
            if (setupPacket->bRequestType == (USB_REQ_TYP_OUT | USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF)) {
                switch (setupPacket->wIndexL) {
                case ITF_NUM_KEYBOARD:
                    hid_protocol_keyboard = setupPacket->wValueL;
                    return;
#ifdef MOUSE_KEYS_ENABLE
                case ITF_NUM_MOUSE:
                    hid_protocol_mouse = setupPacket->wValueL;
                    return;
#endif
                }
             }
             break;
    }

    UEP0_CTRL |= UEP_R_RES_STALL | UEP_T_RES_STALL;
}

inline static void USB_EP0_IN() {
    if (!usb_tx_len) return;
    
    if (UDEV_CTRL & bUD_GP_BIT) {
        // USB_GET_DESCRIPTOR
        USB_EP0_tx();
        UEP0_CTRL ^= bUEP_T_TOG;
    } else {
        // USB_SET_ADDRESS
        USB_DEV_AD = USB_DEV_AD & ~MASK_USB_ADDR | usb_tx_len;
    }
}

inline static void USB_EP0_OUT() {}

uint8_t USB_EP1I_read(uint8_t idx) {
    IE_USB = 0;
    uint8_t value = EP1I_buffer[idx];
    IE_USB = 1;
    return value;
}

void USB_EP1I_write(uint8_t idx, uint8_t value) {
    IE_USB = 0;
    EP1I_buffer[idx] = value;
    IE_USB = 1;
    USB_EP1I_ready_send();
}

inline void USB_EP1I_ready_send() {
    UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
}

inline void USB_EP1I_send_now() {
    USB_EP1I_ready_send();
    while (!(UEP1_CTRL & UEP_T_RES_NAK));
}

inline static void USB_EP1_IN() {
    UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
}

#ifdef CONSUMER_KEYS_ENABLE
void USB_EP2I_write_now(uint8_t idx, uint16_t value) {
    IE_USB = 0;
    EP2I_buffer[idx] = value;
    IE_USB = 1;

    UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
    while (!(UEP2_CTRL & UEP_T_RES_NAK));
}

inline static void USB_EP2_IN() {
    UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
}
#endif

#ifdef MOUSE_KEYS_ENABLE
uint8_t USB_EP3I_read(uint8_t idx) {
    IE_USB = 0;
    uint8_t value = EP3I_buffer[idx];
    IE_USB = 1;
    return value;
}

void USB_EP3I_write(uint8_t idx, uint8_t value) {
    IE_USB = 0;
    EP3I_buffer[idx] = value;
    IE_USB = 1;
    USB_EP3I_ready_send();
}

inline void USB_EP3I_ready_send() {
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
}

inline void USB_EP3I_send_now() {
    USB_EP3I_ready_send();
    while (!(UEP3_CTRL & UEP_T_RES_NAK));
}

inline static void USB_EP3_IN() {
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
}
#endif

inline void USB_reset() {
    usb_tx_len = 0;
    hid_protocol_keyboard = 1;
#ifdef MOUSE_KEYS_ENABLE
    hid_protocol_mouse = 1;
#endif
}

#pragma save
#pragma nooverlay
void USB_interrupt() {
    if (UIF_TRANSFER) {
        UEP0_T_LEN = 0;
        uint8_t endp = USB_INT_ST & MASK_UIS_ENDP;

        switch (USB_INT_ST & MASK_UIS_TOKEN) {
            case UIS_TOKEN_SETUP:
                if (endp == 0) USB_EP0_SETUP();
                break;
            
            case UIS_TOKEN_IN:
                switch (endp) {
                    case 0: USB_EP0_IN(); break;
                    case 1: USB_EP1_IN(); break;
#ifdef CONSUMER_KEYS_ENABLE
                    case 2: USB_EP2_IN(); break;
#endif
#ifdef MOUSE_KEYS_ENABLE
                    case 3: USB_EP3_IN(); break;
#endif
                }
                break;
            
            case UIS_TOKEN_OUT:
                switch (endp) {
                    case 0: USB_EP0_OUT(); break;
                }
                break;
        }

        UIF_TRANSFER = 0;
    }

    if (UIF_BUS_RST) {
        USB_reset();

        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD   = 0;
        UIF_SUSPEND  = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST  = 0;
    }
        
    if (UIF_SUSPEND) {
        UIF_SUSPEND = 0;
        if (!(USB_MIS_ST & bUMS_SUSPEND)) USB_INT_FG = 0xFF;
    }
}
#pragma restore

void USB_init() {
    // Reset USB
    USB_CTRL |= bUC_RESET_SIE | bUC_CLR_ALL;
    USB_CTRL &= ~bUC_CLR_ALL;

    // Flush endpoints
    for (uint8_t i = 8; i;) {
        i--;
        EP0_buffer[i] = 0;
        EP1I_buffer[i] = 0;
#ifdef CONSUMER_KEYS_ENABLE
        EP2I_buffer[i / 2] = 0;
#endif
#ifdef MOUSE_KEYS_ENABLE
        EP3I_buffer[i / 2] = 0;
#endif
    }

    USB_reset();

    // Main init
    USB_CTRL = bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN; 
    UDEV_CTRL = bUD_PD_DIS | bUD_PORT_EN;

    UEP0_T_LEN = 0;
    UEP0_DMA = XADDR_USB_EP0;

    UEP1_T_LEN = USB_EP1_SIZE;
    UEP1_DMA = XADDR_USB_EP1;
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
    UEP4_1_MOD = bUEP1_TX_EN;

#ifdef CONSUMER_KEYS_ENABLE
    UEP2_T_LEN = USB_EP2_SIZE;
    UEP2_DMA = XADDR_USB_EP2;
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;
#endif

#ifdef MOUSE_KEYS_ENABLE
    UEP3_T_LEN = USB_EP3_SIZE;
    UEP3_DMA = XADDR_USB_EP3;
    UEP3_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;
#endif

#if defined(CONSUMER_KEYS_ENABLE) && defined(MOUSE_KEYS_ENABLE)
    UEP2_3_MOD = bUEP2_TX_EN | bUEP3_TX_EN;
#elif defined(CONSUMER_KEYS_ENABLE)
    UEP2_3_MOD = bUEP2_TX_EN;
#elif defined(MOUSE_KEYS_ENABLE)
    UEP2_3_MOD = bUEP3_TX_EN;
#endif

    USB_INT_EN = bUIE_TRANSFER | bUIE_SUSPEND | bUIE_BUS_RST;
    IE_USB = 1;
}
