/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 4808 $
 */ 

/** @file
@brief Implementation of the ACI transport layer module
*/

#include "system.h"
#include <stdio.h>
#include <string.h>
#include <SPI.h>
#include <PPS.h>
#include "hardware.h"
#include "util/util.h"

#include "hal_platform.h"
#include "hal/hal_aci_tl.h"
#include "hal/hal_io.h"
#include "ble_system.h"
//#include <avr/sleep.h>

extern int8_t HAL_IO_RADIO_RESET, HAL_IO_RADIO_REQN, HAL_IO_RADIO_RDY, HAL_IO_RADIO_IRQ;


static void           m_print_aci_data(hal_aci_data_t *p_data);

typedef struct {
 hal_aci_data_t           aci_data[ACI_QUEUE_SIZE];
 uint8_t                  head;
 uint8_t                  tail;
} aci_queue_t;

static hal_aci_data_t received_data;
static uint8_t        spi_readwrite(uint8_t aci_byte);
static bool           aci_debug_print;




static aci_queue_t    aci_tx_q;
static aci_queue_t    aci_rx_q;

static void m_aci_q_init(aci_queue_t *aci_q)
{
  uint8_t loop;
  
  aci_debug_print = false;
  aci_q->head = 0;
  aci_q->tail = 0;
  for(loop=0; loop<ACI_QUEUE_SIZE; loop++)
  {
    aci_tx_q.aci_data[loop].buffer[0] = 0x00;
    aci_tx_q.aci_data[loop].buffer[1] = 0x00;
  }
}

void hal_aci_debug_print(bool enable)
{
	aci_debug_print = enable;
}

static bool m_aci_q_enqueue(aci_queue_t *aci_q, hal_aci_data_t *p_data)
{
  const uint8_t next = (aci_q->tail + 1) % ACI_QUEUE_SIZE;
  const uint8_t length = p_data->buffer[0];
  
  if (next == aci_q->head)
  {
    /* full queue */
    return false;
  }
  aci_q->aci_data[aci_q->tail].status_byte = 0;
  
  memcpy((uint8_t *)&(aci_q->aci_data[aci_q->tail].buffer[0]), (uint8_t *)&p_data->buffer[0], length + 1);
  aci_q->tail = next;
  
  return true;
}

//@comment after a port have test for the queue states, esp. the full and the empty states
static bool m_aci_q_dequeue(aci_queue_t *aci_q, hal_aci_data_t *p_data)
{
  if (aci_q->head == aci_q->tail)
  {
    /* empty queue */
    return false;
  }
  
  memcpy((uint8_t *)p_data, (uint8_t *)&(aci_q->aci_data[aci_q->head]), sizeof(hal_aci_data_t));
  aci_q->head = (aci_q->head + 1) % ACI_QUEUE_SIZE;
  
  return true;
}

static bool m_aci_q_is_empty(aci_queue_t *aci_q)
{
  return (aci_q->head == aci_q->tail);
}

static bool m_aci_q_is_full(aci_queue_t *aci_q)
{
  uint8_t next;
  bool state;
  
  //This should be done in a critical section
  noInterrupts();
  next = (aci_q->tail + 1) % ACI_QUEUE_SIZE;  
  
  if (next == aci_q->head)
  {
    state = true;
  }
  else
  {
    state = false;
  }
  
  interrupts();
  //end
  
  return state;
}



void m_print_aci_data(hal_aci_data_t *p_data)
{
  const uint8_t length = p_data->buffer[0];
  uint8_t i;
  printf("%d :", length);
  for (i=0; i<=length; i++)
  {
      printf("%.2x ", p_data->buffer[i]);
  }
  printf("\n");
}

#if 0
void toggle_eimsk(bool state)
{
  /* ToDo: This will currently only work with the UNO/ATMega48/88/128/328 */
  /*       due to EIMSK. Abstract this away to something MCU nuetral! */
  uint8_t eimsk_bit = 0xFF;
  uint8_t i;
  for (i=0; i<sizeof(dreqinttable); i+=2) {
    if (HAL_IO_RADIO_RDY == dreqinttable[i]) {
      eimsk_bit = dreqinttable[i+1];
    }
  }
  if (eimsk_bit != 0xFF) 
  {
    if (state)
      EIMSK |= (1 << eimsk_bit);
    else
      EIMSK &= ~(1 << eimsk_bit);
  }
  else
  {
    /* RDY isn't a valid HW INT pin! */
    while(1);
  }         
}
#endif

void m_rdy_line_handle(void)
{
  hal_aci_data_t *p_aci_data;
  
  sleep_disable();
  detachInterrupt(HAL_IO_RADIO_IRQ);
  
  // Receive or transmit data
  p_aci_data = hal_aci_tl_poll_get();
  
  // Check if we received data
  if (p_aci_data->buffer[0] > 0)
  {
    if (!m_aci_q_enqueue(&aci_rx_q, p_aci_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
         */
       while(1);
    }
    if (m_aci_q_is_full(&aci_rx_q))
    {
      /* Disable RDY line interrupt.
         Will latch any pending RDY lines, so when enabled it again this
         routine should be taken again */
      toggle_eimsk(false);
    }    
  }
}

bool hal_aci_tl_event_get(hal_aci_data_t *p_aci_data)
{
  bool was_full = m_aci_q_is_full(&aci_rx_q);
  
  if (m_aci_q_dequeue(&aci_rx_q, p_aci_data))
  {
    if (true == aci_debug_print)
    {
        printf(" E");
      m_print_aci_data(p_aci_data);
    }
    
    if (was_full)
    {
      toggle_eimsk(true);
    }
    return true;
  }
  else
  {
    return false;
  }
}

void hal_aci_tl_init()
{
  received_data.buffer[0] = 0;

    _SPI1MD = 0; // Enable SPI peripheral

    // Initialize IO
    _LAT(BT_REQN) = 1;      // Active-low
    _TRIS(BT_REQN) = OUTPUT;
    _TRIS(BT_RDYN) = INPUT;

    // Set up the SPI interface..
    // Output data is clocked out on the rising edge
    // Input data is sampled on the falling edge
    // Clock is active-high
    OpenSPI1(ENABLE_SCK_PIN | ENABLE_SDO_PIN | SPI_MODE8_ON | SPI_SMP_OFF |
            SPI_CKE_ON | SLAVE_ENABLE_OFF | CLK_POL_ACTIVE_HIGH | MASTER_ENABLE_ON |
            //PRI_PRESCAL_1_1 | SEC_PRESCAL_1_1, // Fastest
            PRI_PRESCAL_16_1 | SEC_PRESCAL_1_1, // Slower, for debugging
            FRAME_ENABLE_OFF | SPI_ENH_BUFF_DISABLE,
            SPI_IDLE_CON | SPI_ENABLE);

  
  /* initialize aci cmd queue */
  m_aci_q_init(&aci_tx_q);  
  m_aci_q_init(&aci_rx_q);

  //Configure the IO lines
  pinMode(HAL_IO_RADIO_RESET, OUTPUT);
  pinMode(HAL_IO_RADIO_RDY,   INPUT_PULLUP);
  pinMode(HAL_IO_RADIO_REQN,  OUTPUT);
  
  digitalWrite(HAL_IO_RADIO_RESET, 1);
  delay(100);
  digitalWrite(HAL_IO_RADIO_RESET, 0);
  digitalWrite(HAL_IO_RADIO_RESET, 1);
  
  digitalWrite(SCK,  0);
  digitalWrite(MOSI, 0);
  digitalWrite(HAL_IO_RADIO_REQN,   1);
  digitalWrite(SCK,  0);  
  
  HAL_IO_RADIO_IRQ = 0xFF;
  for (uint8_t i=0; i<sizeof(dreqinttable); i+=2) {
    if (HAL_IO_RADIO_RDY == dreqinttable[i]) {
      HAL_IO_RADIO_IRQ = dreqinttable[i+1];
    }
  }

  delay(30); //Wait for the nRF8001 to get hold of its lines - the lines float for a few ms after the reset
  if (HAL_IO_RADIO_IRQ != 0xFF) 
    attachInterrupt(HAL_IO_RADIO_IRQ, m_rdy_line_handle, LOW); 
  // We use the LOW level of the RDYN line as the atmega328 can wakeup from sleep only on LOW
}

bool hal_aci_tl_send(hal_aci_data_t *p_aci_cmd)
{
  const uint8_t length = p_aci_cmd->buffer[0];
  bool ret_val = false;

  if (length > HAL_ACI_MAX_LENGTH)
  {
    return false;
  }
  else
  {
    if (m_aci_q_enqueue(&aci_tx_q, p_aci_cmd))
    {
      ret_val = true;
    }
  }

  if (true == aci_debug_print)
  {
      printf("C");
    m_print_aci_data(p_aci_cmd);
  }
  
  HAL_IO_SET_STATE(HAL_IO_RADIO_REQN, 0);
  return ret_val;
}



hal_aci_data_t * hal_aci_tl_poll_get(void)
{
  uint8_t byte_cnt;
  uint8_t byte_sent_cnt;
  uint8_t max_bytes;
  hal_aci_data_t data_to_send;


  //SPI.begin();  
    
  HAL_IO_SET_STATE(HAL_IO_RADIO_REQN, 0);
  
  // Receive from queue
  if (m_aci_q_dequeue(&aci_tx_q, &data_to_send) == false)
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }
  
  //Change this if your mcu has DMA for the master SPI
  
  // Send length, receive header
  byte_sent_cnt = 0;
  received_data.status_byte = spi_readwrite(data_to_send.buffer[byte_sent_cnt++]);
  // Send first byte, receive length from slave
  received_data.buffer[0] = spi_readwrite(data_to_send.buffer[byte_sent_cnt++]);
  if (0 == data_to_send.buffer[0])
  {
    max_bytes = received_data.buffer[0];
  }
  else
  {
    // Set the maximum to the biggest size. One command byte is already sent
    max_bytes = (received_data.buffer[0] > (data_to_send.buffer[0] - 1)) 
      ? received_data.buffer[0] : (data_to_send.buffer[0] - 1);
  }

  if (max_bytes > HAL_ACI_MAX_LENGTH)
  {
    max_bytes = HAL_ACI_MAX_LENGTH;
  }

  // Transmit/receive the rest of the packet 
  for (byte_cnt = 0; byte_cnt < max_bytes; byte_cnt++)
  {
    received_data.buffer[byte_cnt+1] =  spi_readwrite(data_to_send.buffer[byte_sent_cnt++]);
  }

  HAL_IO_SET_STATE(HAL_IO_RADIO_REQN, 1);
  //SPI.end()
  //RDYN should follow the REQN line in approx 100ns
  
  sleep_enable();
  attachInterrupt(HAL_IO_RADIO_IRQ, m_rdy_line_handle, LOW);  


  
  if (false == m_aci_q_is_empty(&aci_tx_q))
  {
    //Lower the REQN line to start a new ACI transaction         
    HAL_IO_SET_STATE(HAL_IO_RADIO_REQN, 0); 
  }
  
  /* valid Rx available or transmit finished*/
  return (&received_data);
}

static uint8_t spi_readwrite(const uint8_t aci_byte)
{
    WriteSPI1(bitreverse[aci_byte]);

    while (!DataRdySPI1()) continue;

    byte rb = ReadSPI1() & 0xFF;
    return bitreverse[rb];
}

void m_aci_q_flush(void)
{
  noInterrupts();
  /* re-initialize aci cmd queue and aci event queue to flush them*/
  m_aci_q_init(&aci_tx_q);
  m_aci_q_init(&aci_rx_q);
  interrupts();
}
