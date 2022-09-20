#include "rpi_pico_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hidapi/hidapi.h"

#define VENDOR_ID   0xcafe
#define PRODUCT_ID  0x4004

static hid_device * handle;

static void led_set(bool value) {

    uint8_t buf[3];
    buf[0] = 0;
    buf[1] = 0x00;
    buf[2] = (value ? 1:0);

//    printf("led_set [");
//    for(int i = 0; i < sizeof(buf); i++) {
//        printf("%02x, ", buf[i]);
//    }
//    printf("]\n");

    hid_write(handle, buf, sizeof(buf));
}

static void pin_set_direction(uint8_t pin, bool direction) {
    const uint32_t mask = (1<<pin);
    const uint32_t val = ((direction?1:0)<<pin);

    uint8_t buf[10];
    buf[0] = 0;
    buf[1] = 0x10;
    buf[2] = (mask >> 24) & 0xff;
    buf[3] = (mask >> 16) & 0xff;
    buf[4] = (mask >> 8) & 0xff;
    buf[5] = (mask >> 0) & 0xff;
    buf[6] = (val >> 24) & 0xff;
    buf[7] = (val >> 16) & 0xff;
    buf[8] = (val >> 8) & 0xff;
    buf[9] = (val >> 0) & 0xff;

//    printf("pin_set_direction [");
//    for(int i = 0; i < sizeof(buf); i++) {
//        printf("%02x, ", buf[i]);
//    }
//    printf("]\n");

    hid_write(handle, buf, sizeof(buf));
}

static void pinmask_write(uint32_t mask, uint32_t val) {
    uint8_t buf[10];
    buf[0] = 0;
    buf[1] = 0x20;
    buf[2] = (mask >> 24) & 0xff;
    buf[3] = (mask >> 16) & 0xff;
    buf[4] = (mask >> 8) & 0xff;
    buf[5] = (mask >> 0) & 0xff;
    buf[6] = (val >> 24) & 0xff;
    buf[7] = (val >> 16) & 0xff;
    buf[8] = (val >> 8) & 0xff;
    buf[9] = (val >> 0) & 0xff;

//    printf("pin_write [");
//    for(int i = 0; i < sizeof(buf); i++) {
//        printf("%02x, ", buf[i]);
//    }
//    printf("]\n");

    hid_write(handle, buf, sizeof(buf));

}

static void pin_write(uint8_t pin, bool value) {
    const uint32_t mask = (1<<pin);
    const uint32_t val = ((value?1:0)<<pin);

    pinmask_write(mask, val);
}

static bool pin_read(uint8_t pin) {
  int response;
  uint8_t buf[2];
    buf[0] = 0;
    buf[1] = 0x30;

//    printf("pin_read [");
//    for(int i = 0; i < sizeof(buf); i++) {
//        printf("%02x, ", buf[i]);
//    }
//    printf("]\n");

    response = hid_write(handle, buf, sizeof(buf));
    if (response != sizeof(buf)) {
      printf("write buf size not ok %d\n", response);
      exit(1);
      }

    uint8_t ret_buf[5];
    const int bytes_read = hid_read_timeout(handle, ret_buf, sizeof(ret_buf), 40000);
    if(bytes_read != sizeof(ret_buf)) {
        printf("error reading, got:%i\n", bytes_read);
        printf("hiderror %ls\n", hid_error(handle));
        exit(1);
    }

//    printf("  ret=[");
//    for(int i = 0; i < sizeof(ret_buf); i++) {
//        printf("%02x, ", ret_buf[i]);
//    }
//    printf("]\n");

    uint32_t pins =
        (ret_buf[1] << 24)
        | (ret_buf[2] << 16)
        | (ret_buf[3] << 8)
        | (ret_buf[4] << 0);

    return (pins & (1<<pin));
}

#define MAX_BYTES_PER_TRANSFER (64-8)

static void bitbang_spi(
    uint8_t sck_pin,
    uint8_t mosi_pin,
    uint8_t miso_pin,
    uint32_t bit_count,
    uint8_t* buffer) {

    const int byte_count = ((bit_count + 7) / 8);
    if(byte_count > MAX_BYTES_PER_TRANSFER) {
        printf("bit count too high\n");
        exit(1);
    }

//    printf("bitbang_spi byte_count:%i bit_count:%i\n", byte_count, bit_count);

    uint8_t buf[9+MAX_BYTES_PER_TRANSFER];
    memset(buf, 0xFF, sizeof(buf));

    buf[0] = 0;
    buf[1] = 0x40;
    buf[2] = sck_pin;
    buf[3] = mosi_pin;
    buf[4] = miso_pin;
    buf[5] = (bit_count >> 24) & 0xff;
    buf[6] = (bit_count >> 16) & 0xff;
    buf[7] = (bit_count >> 8) & 0xff;
    buf[8] = (bit_count >> 0) & 0xff;


    memcpy(&buf[9], buffer, byte_count); // TODO: memory length

//    printf("  send:[");
//    for(int i = 0; i < (8+byte_count); i++) {
//        printf("%02x, ", buf[i]);
//    }
//    printf("]\n");

    uint8_t ret_buf[64];

    hid_write(handle, buf, sizeof(buf));

    const int bytes_read = hid_read_timeout(handle, ret_buf, 64, 4000);
    if(bytes_read != 64) {
        printf("error reading, got:%i\n", bytes_read);
        exit(1);
    }

//    printf("  receive:[");
//    for(int i = 0; i < (5+byte_count); i++) {
//        printf("%02x, ", ret_buf[i]);
//    }
//    printf("]\n");
    memcpy(buffer, &ret_buf[5], byte_count);
}

#define PIN_POWER 7
#define PIN_SCK 10
#define PIN_MOSI 13
#define PIN_SS 12
#define PIN_MISO 11
#define PIN_CRESET 14
#define PIN_CDONE 15

// ********* iceprog API ****************

// TODO
static void close() {
//    pin_set_direction(PIN_POWER, true);
    pin_set_direction(PIN_SCK, false);
    pin_set_direction(PIN_MOSI, false);
    pin_set_direction(PIN_SS, false);
    pin_set_direction(PIN_MISO, false);
    pin_set_direction(PIN_CRESET, false);
    pin_set_direction(PIN_CDONE, false);

    led_set(false);
    printf("closing\n");

    hid_close(handle);
    handle = NULL;
}

// TODO
static void error(int status) {
    close();
    exit(status);
}

// TODO
static void set_cs_creset(int cs_b, int creset_b) {
    pinmask_write(
        (1<<PIN_SS) | (1<PIN_CRESET),
        ((cs_b>0?1:0)<<PIN_SS) | ((creset_b>0?1:0)<<PIN_CRESET)
    );
}

// TODO
static bool get_cdone(void) {
    return pin_read(PIN_CDONE);
}

// TODO
static uint8_t xfer_spi_bits(uint8_t data, int n) {
    uint8_t buf = data;
    bitbang_spi(PIN_SCK, PIN_MOSI, PIN_MISO, n, &buf);

    return buf;
}

// TODO
static void xfer_spi(uint8_t *data, int n) {
//    printf("xfer_spi %i\n",n);
    for(int byte_index = 0; byte_index < n;) {
        int bytes_to_transfer = n - byte_index;
        if(bytes_to_transfer > MAX_BYTES_PER_TRANSFER) {
            bytes_to_transfer = MAX_BYTES_PER_TRANSFER;
        }
//      printf(" byte_index:%i bytes_to_transfer:%i\n", byte_index, bytes_to_transfer);

        bitbang_spi(PIN_SCK, PIN_MOSI, PIN_MISO, bytes_to_transfer*8, data+byte_index);
        byte_index += bytes_to_transfer;
    }
}

// TODO
static void send_spi(uint8_t *data, int n) {
    uint8_t buf[n];
    memcpy(buf,data,sizeof(buf));
    xfer_spi(buf,n);
}

// TODO
static void send_dummy_bytes(uint8_t n) {
    uint8_t buf[n];
    memset(buf, 0, sizeof(buf));
    xfer_spi(buf, sizeof(buf));
}


// TODO
static void send_dummy_bit(void) {
    xfer_spi_bits(0, 1);
}


const interface_t rpi_pico_interface = {
	.close = close,
	.error = error,

	.set_cs_creset = set_cs_creset,
	.get_cdone = get_cdone,

	.send_spi = send_spi,
	.xfer_spi = xfer_spi,
	.xfer_spi_bits = xfer_spi_bits,

	.send_dummy_bytes = send_dummy_bytes,
	.send_dummy_bit = send_dummy_bit,
};

void rpi_pico_interface_init() {
    hid_init();

    handle = hid_open( VENDOR_ID, PRODUCT_ID, NULL);
    if(handle == NULL) {
        printf("Failure!\n");

        hid_exit();
        exit(-1);
    }

    led_set(true);


    pin_set_direction(PIN_POWER, true);
    pin_set_direction(PIN_SCK, true);
    pin_set_direction(PIN_MOSI, true);
    pin_set_direction(PIN_SS, true);
    pin_set_direction(PIN_MISO, false);
    pin_set_direction(PIN_CRESET, true);
    pin_set_direction(PIN_CDONE, false);

    pin_write(PIN_POWER, true);
}

// ********* API ****************
