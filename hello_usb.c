#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

//#include "blink.pio.h"
#include "blink2.pio.h"
#include "ws2812byte.pio.h"

//#include "Adafruit_NeoPixel.h"

/**
 * NOTE:
 *  Take into consideration if your WS2812 is a RGB or RGBW variant.
 *
 *  If it is RGBW, you need to set IS_RGBW to true and provide 4 bytes per 
 *  pixel (Red, Green, Blue, White) and use urgbw_u32().
 *
 *  If it is RGB, set IS_RGBW to false and provide 3 bytes per pixel (Red,
 *  Green, Blue) and use urgb_u32().
 *
 *  When RGBW is used with urgb_u32(), the White channel will be ignored (off).
 *
 */
#define IS_RGBW false
#define NUM_PIXELS 1
#define WS2812_PIN 21
#define NEOPIXEL_BUS 21
#define NEOPIXEL_POWER 20

// default to pin 2 if the board doesn't have a default WS2812 pin defined


PIO pio[2];
uint sm[2];


// NEOPIXEL STUFF
static inline void put_pixel(uint32_t pixel_grb) {    
    pio_sm_put_blocking(pio[1], sm[1], pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    uint max = 100; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};
// END NEOPIXEL STUFF



/*
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}
*/


void set_neopixel_color(uint32_t color) {
    pio_sm_put_blocking(pio0, 0, color << 8u);
}


 int64_t alarm_callback(alarm_id_t id, void *user_data) {
    printf("alarm_id: %d\n", id);
    // Put your timeout handler code in here
    return -1;
}

bool repeating_timer_callback(struct repeating_timer *t) {
    printf("%d: repeating_timer_callback\n", to_ms_since_boot(get_absolute_time()));

    set_neopixel_color(0x00FF0000);

    return 1;
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Hello, world!\n");

    gpio_set_dir(NEOPIXEL_POWER, GPIO_OUT);
    gpio_put(NEOPIXEL_POWER, 1);

    //Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

    // ##### PIO Module #####
    // ##### PIO Module #####
    // ##### PIO Module #####

    static const uint led_pin = 13;
    static const float pio_freq[2] = {2000, 800000};
    

    // Choose PIO instance (0 or 1)

    pio[0] = pio1;
    pio[1] = pio0;

    // Get first free state machine in PIO 0

    sm[0] = pio_claim_unused_sm(pio[0], true);
    sm[1] = pio_claim_unused_sm(pio[1], true);

    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    uint offset[2];
    offset[0] = pio_add_program(pio[0], &blink_program);    
    offset[1] = pio_add_program(pio[1], &ws2812byte_program); 
    
    // Calculate the PIO clock divider
    float div[2];
    div[0] = (float)clock_get_hz(clk_sys) / pio_freq[0];
    div[1] = (float)clock_get_hz(clk_sys) / pio_freq[1];

    // Initialize the program using the helper function in our .pio file
    blink_program_init(pio[0], sm[0], offset[0], led_pin, div[0]);
    ws2812byte_program_init(pio[1], sm[1], offset[1], NEOPIXEL_BUS, 800000, 8);

    // Start running our PIO program in the state machine
    pio_sm_set_enabled(pio[0], sm[0], true);
    pio_sm_set_enabled(pio[1], sm[1], true);
    
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio

    printf("sm:%d, div: %f, clock_get_hz(clk_sys) %f, pio_freq %f\n", sm[0], div[0], (float)clock_get_hz(clk_sys), pio_freq[0]);
    printf("sm:%d, div: %f, clock_get_hz(clk_sys) %f, pio_freq %f\n", sm[1], div[1], (float)clock_get_hz(clk_sys), pio_freq[1]);

    // ##### PIO Module END #####
    // ##### PIO Module END #####
    // ##### PIO Module END #####

    // Timer example code - This example fires off the callback after 2000ms
    // add_alarm_in_ms(2000, alarm_callback, NULL, false);

    static repeating_timer_t timer;
    // negative timeout means exact delay (rather than delay between callbacks)
    if (!add_repeating_timer_ms(5000, repeating_timer_callback, NULL, &timer)) {
        printf("Failed to add timer\n");
        return 1;
    }
    
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    // Watchdog example code
    if (watchdog_caused_reboot()) {
        printf("Watchdog: Rebooted by Watchdog!\n");
        // Whatever action you may take if a watchdog caused a reboot
    } else {
        printf("Watchdog: Clean boot\n");
    }
    
    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    watchdog_enable(100, 1);
    


    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));

    put_pixel(0x0000FF00);
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    while (1)
    {
        // You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
        
        watchdog_update();
        uint8_t numBytes = 8;
        while(numBytes--) {
            // Bits for transmission must be shifted to top 8 bits
        
            pio_sm_put_blocking(pio[1], sm[1], 1<< 24);
        }
        

        
    }
    


}
