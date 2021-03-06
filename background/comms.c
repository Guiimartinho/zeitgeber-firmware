/*
 * File:   comms.c
 * Author: Jared Sanson
 *
 * Created on 13 December 2013, 6:55 PM
 */

////////// Includes ////////////////////////////////////////////////////////////

#include <system.h>
#include <string.h>
#include "hardware.h"
#include "drivers/usb/usb.h"
#include "background/comms.h"
#include "core/kernel.h"

#include "usb_config.h"
#include "./USB/usb.h"
#include "./USB/usb_function_hid.h"

#include "api/clock.h"
#include "api/calendar.h"
#include "background/power_monitor.h"
#include "api/graphics/gfx.h"
#include "drivers/ssd1351.h"
#include "core/printf.h"
#include "core/os.h"

////////// Defines /////////////////////////////////////////////////////////////

// Run this as fast as possible, since usually it's only
// checking if data is received and nothing else.
#define PROCESS_COMMS_INTERVAL 25

#define SetTxErrorCode(code) (tx_buffer[1] = code)

////////// Variables ///////////////////////////////////////////////////////////

//unsigned char rx_usb_buffer[64];
//unsigned char tx_usb_buffer[64];

//USB_HANDLE USBOutHandle = 0; //USB handle.  Must be initialized to 0 at startup.
//USB_HANDLE USBInHandle = 0; //USB handle.  Must be initialized to 0 at startup.
bool usb_connected = false;
comms_status_t comms_status = cmDisconnected;

char tx_buffer[PACKET_SIZE];

static task_t* comms_task;


////////// Prototypes //////////////////////////////////////////////////////////

void ProcessComms();
void comms_ReceivedPacket(unsigned char* packet);
void comms_SendPacket(unsigned char* buffer);
void comms_sleep();
void comms_wake();

////////// Methods /////////////////////////////////////////////////////////////

void InitializeComms() {
    msg_init();

    InitializeUSB(&comms_sleep, &comms_wake);
    
    // Communications, only needs to be run when USB is connected
    comms_task = RegisterTask("Comms", ProcessComms);
    comms_task->state = tsRun;

    usb_connected = false;
    comms_status = cmDisconnected;
}

void comms_sleep() {
    // Called by the USB module when the USB becomes disconnected
    comms_task->state = tsStop;
    usb_connected = false;
    comms_status = cmDisconnected;

    //os.h
    auto_screen_off = true;
}

void comms_wake() {
    // Called by the USB module when the USB becomes connected
    comms_task->state = tsRun;
    usb_connected = true;
    comms_status = cmIdle;

    //os.h
    auto_screen_off = false; // Prevent screen from turning off
}

void ProcessComms() {
    while (1) {
        USBProcess(&comms_ReceivedPacket);

        //TODO: Implement some sort of variable delay that waits for data transmission
        Delay(1);
    }
}

void comms_set_led(byte led, byte value) {
    switch (led) {
        case 1:
            _LAT(LED1) = value;
            break;
        case 2:
            _LAT(LED2) = value;
            break;
    }
}

// Called when a packet is received
void comms_ReceivedPacket(unsigned char* packet) {
    // packet is 64 bytes

    //NOTE: If transferring multiple packets,
    // it might pay to temporarily reduce the task interval.

    SetTxErrorCode(ERR_OK);

    switch (packet[0]) {

        ////////// Basic System Commands //////////

        case CMD_PING:
            break;

        case CMD_RESET:
            Reset(); // Causes immediate reset of the MCU
            break;

        case CMD_SET_LED:
            comms_set_led(packet[1], packet[2]);
            break;

        ////////// Diagnostics //////////

        case CMD_GET_BATTERY_INFO:
        {
            battery_info_t* tx_packet = (battery_info_t*)tx_buffer;

            // power_monitor.h
            tx_packet->level = battery_level;
            tx_packet->voltage = battery_voltage;
            tx_packet->charge_status = charge_status;
            tx_packet->power_status = power_status;
            tx_packet->battery_status = battery_status;
            tx_packet->bq25010_status = bq25010_status;

            break;
        }

        case CMD_GET_CPU_INFO:
        {
            cpu_info_t* tx_packet = (cpu_info_t*)tx_buffer;

            // systick.h
            tx_packet->systick = systick;

            break;
        }

        case CMD_GET_NEXT_MESSAGE:
        {
            message_packet_t* tx_packet = (message_packet_t*)tx_buffer;

            if (!msg_isempty()) {
                msg_pop(tx_packet->message);
                tx_packet->len = strlen(tx_packet->message);
            } else {
                tx_packet->len = 0;
            }

            break;
        }

        ////////// Display Interface //////////

        case CMD_QUERY_DISPLAY:
        {
            display_query_t* tx_packet = (display_query_t*)tx_buffer;

            // gfx.h
            tx_packet->width = DISPLAY_WIDTH;
            tx_packet->height = DISPLAY_HEIGHT;
            tx_packet->bpp = DISPLAY_BPP;

            tx_packet->display_on = display_power;

            break;
        }

        case CMD_SET_DISPLAY_POWER:
        {
            bool on = packet[1];
            if (on)
                ssd1351_DisplayOn();
            else
                ssd1351_DisplayOff();

            break;
        }

        case CMD_DISPLAY_LOCK:
        {
            lock_display = true;
            break;
        }

        case CMD_DISPLAY_UNLOCK:
        {
            lock_display = false;
            break;
        }

        case CMD_DISPLAY_WRITEBUF:
        {
            //TODO: Will require multiple RX packets
            SetTxErrorCode(ERR_NOT_IMPLEMENTED);
            break;
        }

        case CMD_DISPLAY_READBUF:
        {
            display_chunk_t* request = (display_chunk_t*)packet;
            display_chunk_t* chunk = (display_chunk_t*)tx_buffer;

            // Make sure the display has a full frame first
            if (display_frame_ready) {
                chunk->state = 1;
                
                // Send the next chunk
                chunk->offset = request->offset;
                ReadScreenBuffer(chunk->buf, request->offset, DISP_CHUNK_SIZE);

            } else {
                chunk->state = 0;
            }

            //comms_read_display_buf(tx_packet);
            break;
        }

        ////////// Sensors //////////

        case CMD_QUERY_SENSORS:
        {
            sensor_query_t* tx_packet = (sensor_query_t*)tx_buffer;

            tx_packet->count = 0;
            //TODO: Dynamically populate with known system sensors
            SetTxErrorCode(ERR_NOT_IMPLEMENTED);
            break;
        }

        case CMD_SET_SENSOR_ENABLE:
        {
            uint16 index = packet[1];

            // TODO: Enable/disable the specified sensor/
            // may already be enabled by the system,
            // and may be re-enabled by the system as required.
            SetTxErrorCode(ERR_NOT_IMPLEMENTED);
            break;
        }

        case CMD_GET_SENSOR_DATA:
        {
            uint16 index = packet[1];

            // TODO: Return the data for the specified sensor index.
            // Won't return the data until the sensor has been updated,
            // will return 0 if the sensor is currently disabled.
            SetTxErrorCode(ERR_NOT_IMPLEMENTED);
            break;
        }

        ////////// Time & Date //////////

        case CMD_GET_DATETIME:
        {
            datetime_packet_t* tx_packet = (datetime_packet_t*)tx_buffer;
            timestamp_t ts = ClockNow();

            tx_packet->hour = ts.hour;
            tx_packet->minute = ts.min;
            tx_packet->second = ts.sec;

            tx_packet->day_of_week = ts.dow;
            tx_packet->day = ts.day;
            tx_packet->month = ts.month;
            tx_packet->year = ts.year;
            break;
        }

        case CMD_SET_DATETIME:
        {
            datetime_packet_t* rx_packet = (datetime_packet_t*)packet;

            ClockSetTime(
                rx_packet->hour,
                rx_packet->minute,
                rx_packet->second
            );

            ClockSetDate(
                rx_packet->day_of_week,
                rx_packet->day,
                rx_packet->month,
                rx_packet->year
            );
            break;
        }

        ////////// Calendar //////////
        
        case CMD_CLEAR_CALENDAR:
        {
            CalendarClear();
            break;
        }

        case CMD_ADD_CALENDAR_EVT:
        {
            calendar_event_packet_t* rx_packet = (calendar_event_packet_t*)packet;
            event_t event;

            event.event_type = rx_packet->event_type;

            strncpy(event.label, rx_packet->label, MAX_LABEL_LEN);
            strncpy(event.location, rx_packet->location, MAX_LOCATION_LEN);

            event.color = rx_packet->color;

            // Timetable events
            event.dow = rx_packet->dow;
            event.hr = rx_packet->hr;
            event.min = rx_packet->min;

            if (CalendarAddEvent(&event) == NULL) {
                SetTxErrorCode(ERR_OUT_OF_RAM);
                break;
            }

            break;
        }

        case CMD_GET_CALENDAR_INFO:
        {
            calendar_info_packet_t* tx_packet = (calendar_info_packet_t*)tx_buffer;
            tx_packet->num_events = CalendarGetNumEvents();
            break;
        }

        case CMD_GET_CALENDAR_EVT:
        {
            calendar_event_packet_t* rx_packet = (calendar_event_packet_t*)packet;
            calendar_event_packet_t* tx_packet  = (calendar_event_packet_t*)tx_buffer;

            int16 index = rx_packet->index;
            event_t* event = CalendarGetEvent(index);

            if (event == NULL) {
                SetTxErrorCode(ERR_INVALID_INDEX);
                break;
            }

            tx_packet->index = index;
            tx_packet->event_type = event->event_type;

            strncpy(tx_packet->label, event->label, MAX_LABEL_LEN);
            strncpy(tx_packet->location, event->location, MAX_LOCATION_LEN);

            tx_packet->color = event->color;

            tx_packet->dow = event->dow;
            tx_packet->hr = event->hr;
            tx_packet->min = event->min;

            break;
        }


        default:
            return; // Don't send any response
    }

    tx_buffer[0] = packet[0]; // Set command field
    USBSendPacket(tx_buffer);
}