/**
 * @file Cosa/SPI/Driver/SR3W.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef COSA_SPI_DRIVER_SR3W_HH
#define COSA_SPI_DRIVER_SR3W_HH

#include "Cosa/OutputPin.hh"
#include "Cosa/SPI.hh"

/**
 * N-Shift Register 3-Wire SPI device driver. The shift registers
 * (74HC595) should be cascaded (see circuit below). The pins are
 * numbered from the first connect shift register (Q0..Q7) and
 * updwards in the chain (Q8..Q15) and so on. 
 *
 * @section Circuit
 * @code
 *                         74HC595    (VCC)
 *                       +----U----+    |
 * (Q1)----------------1-|Q1    VCC|-16-+
 * (Q2)----------------2-|Q2     Q0|-15-----------(Q0)
 * (Q3)----------------3-|Q3    SER|-14-----------(MOSI/D11)
 * (Q4)----------------4-|Q4    /OE|-13-----------(GND)
 * (Q5)----------------5-|Q5   RCLK|-12-----------(EN/D10)---+
 * (Q6)----------------6-|Q6   SCLK|-11-----------(SCK/D13)+ |
 * (Q7)----------------7-|Q7    /MR|-10-----------(VCC)    | |
 *                   +-8-|GND   Q7S|--9------------------+ | |
 *                   |   +---------+                     | | |
 *                   |      0.1uF                        | | |
 *                 (GND)---- ||-------(VCC)              | | |
 *                                      |                | | |
 *                         74HC595      |                | | |
 *                       +----U----+    |                | | |
 * (Q9)----------------1-|Q1    VCC|-16-+                | | |
 * (Q10)---------------2-|Q2     Q0|-15-----------(Q8)   | | |
 * (Q11)---------------3-|Q3    SER|-14------------------+ | |
 * (Q12)---------------4-|Q4    /OE|-13-----------(GND)    | |
 * (Q13)---------------5-|Q5   RCLK|-12--------------------(-+
 * (Q14)---------------6-|Q6   SCLK|-11--------------------+ |
 * (Q15)---------------7-|Q7    /MR|-10-----------(VCC)    | |
 *                   +-8-|GND   Q7S|--9------------------+ | |
 *                   |   +---------+                     | | |
 *                   |      0.1uF                        | | |
 *                 (GND)---- ||-------(VCC)              | | |
 *                                      |                | | |
 *                                      V                V V V
 * @endcode
 * 
 * @param[in] N number of shift registers (N * 8 output pins).
 */
template<uint8_t N>
class SR3W : public SPI::Driver {
public:
  /**
   * Construct N-shift register connected to SPI (MOSI, SCL) and given
   * chip select pin.
   * @param[in] cs chip select pin (Default Board::D10/D3).
   * @param[in] clock SPI hardware setting (default DIV4_CLOCK).
   */
#if !defined(BOARD_ATTINY)
  SR3W(Board::DigitalPin cs = Board::D10, 
       SPI::Clock rate = SPI::DEFAULT_CLOCK) : 
    SPI::Driver(cs, SPI::PULSE_HIGH, rate)
  {
    clear();
    update();
  }
#else
  SR3W(Board::DigitalPin en = Board::D3,
       SPI::Clock rate = SPI::DEFAULT_RATE) : 
    SPI::Driver(cs, SPI::PULSE_HIGH, rate)
  {
    clear();
    update();
  }
#endif

  /**
   * Set given pin in shadow register. Call update() to write to shift
   * register. 
   * @param[in] pin pin number.
   */
  void set(uint8_t pin) __attribute__((always_inline))
  {
    uint8_t ix = (pin >> 3);
    m_port[ix] |= _BV(pin & 0x7);
  }

  /**
   * Clear given pin in shadow register. Call update() to write to shift
   * register. 
   * @param[in] pin pin number.
   */
  void clear(uint8_t pin) __attribute__((always_inline))
  {
    uint8_t ix = (pin >> 3);
    m_port[ix] &= ~_BV(pin & 0x7);
  }

  /**
   * Set the shadow registers. Call update() to write to shift
   * register.
   */
  void set() __attribute__((always_inline))
  {
    memset(m_port, 0xff, N);
  }

  /**
   * Clear the shadow registers. Call update() to write to shift
   * register.
   */
  void clear() __attribute__((always_inline))
  {
    memset(m_port, 0, N);
  }

  /**
   * Update shift register with value of shadow registers. 
   */
  void update()
  {
    spi.begin(this);
    uint8_t ix = N - 1;
    spi.transfer_start(m_port[ix]);
    while (ix--) {
      uint8_t data = m_port[ix];
      spi.transfer_await();
      spi.transfer_start(data);
    }
    spi.transfer_await();
    spi.end();
  }
  
protected:
  uint8_t m_port[N];
};

#endif