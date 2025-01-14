/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * Copyright (c) 2014 by Matthijs Kooijman <matthijs@stdin.nl> (SPISettings AVR)
 * Copyright (c) 2021 by Timo Sandmann (Teensy 4.x port for FreeRTOS)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#pragma once

#include "Arduino.h"

#if defined(__arm__) && defined(TEENSYDUINO)
#if defined(__has_include) && __has_include("EventResponder.h")
// SPI_HAS_TRANSFER_ASYNC - Defined to say that the SPI supports an ASYNC version
// of the SPI_HAS_TRANSFER_BUF
#define SPI_HAS_TRANSFER_ASYNC 1
#include "DMAChannel.h"
#include "EventResponder.h"
#endif
#endif

#include <cstdint>
#include <concepts>


// SPI_HAS_TRANSACTION means SPI has beginTransaction(), endTransaction(),
// usingInterrupt(), and SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// Uncomment this line to add detection of mismatched begin/end transactions.
// A mismatch occurs if other libraries fail to use SPI.endTransaction() for
// each SPI.beginTransaction().  Connect a LED to this pin.  The LED will turn
// on if any mismatch is ever detected.
//#define SPI_TRANSACTION_MISMATCH_LED 5

// SPI_HAS_TRANSFER_BUF - is defined to signify that this library supports
// a version of transfer which allows you to pass in both TX and RX buffer
// pointers, either of which could be NULL
#define SPI_HAS_TRANSFER_BUF 1


#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

#define SPI_CLOCK_DIV4 0x00
#define SPI_CLOCK_DIV16 0x01
#define SPI_CLOCK_DIV64 0x02
#define SPI_CLOCK_DIV128 0x03
#define SPI_CLOCK_DIV2 0x04
#define SPI_CLOCK_DIV8 0x05
#define SPI_CLOCK_DIV32 0x06

#define SPI_MODE_MASK 0x0C // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK 0x03 // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK 0x01 // SPI2X = bit 0 on SPSR


#if defined(__arm__) && defined(TEENSYDUINO) && defined(KINETISK)
/**********************************************************/
/*     32 bit Teensy 3.x                                  */
/**********************************************************/

#define SPI_HAS_NOTUSINGINTERRUPT 1
#define SPI_ATOMIC_VERSION 1

template <typename T>
concept SPITransferType = std::same_as<uint8_t, T> || std::same_as<uint16_t, T>;


class SPISettings {
public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
        if (__builtin_constant_p(clock)) {
            init_AlwaysInline(clock, bitOrder, dataMode);
        } else {
            init_MightInline(clock, bitOrder, dataMode);
        }
    }
    SPISettings() {
        init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
    }

private:
    void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
        init_AlwaysInline(clock, bitOrder, dataMode);
    }
    void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) {
        uint32_t t, c = SPI_CTAR_FMSZ(7);
        if (bitOrder == LSBFIRST)
            c |= SPI_CTAR_LSBFE;
        if (__builtin_constant_p(clock)) {
            if (clock >= F_BUS / 2) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 3) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 4) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 5) {
                t = SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_DBR | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 6) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 8) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1);
            } else if (clock >= F_BUS / 10) {
                t = SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 12) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1);
            } else if (clock >= F_BUS / 16) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2);
            } else if (clock >= F_BUS / 20) {
                t = SPI_CTAR_PBR(2) | SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(0);
            } else if (clock >= F_BUS / 24) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2);
            } else if (clock >= F_BUS / 32) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(4) | SPI_CTAR_CSSCK(3);
            } else if (clock >= F_BUS / 40) {
                t = SPI_CTAR_PBR(2) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2);
            } else if (clock >= F_BUS / 56) {
                t = SPI_CTAR_PBR(3) | SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(2);
            } else if (clock >= F_BUS / 64) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4);
            } else if (clock >= F_BUS / 96) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(4);
            } else if (clock >= F_BUS / 128) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5);
            } else if (clock >= F_BUS / 192) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(5);
            } else if (clock >= F_BUS / 256) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6);
            } else if (clock >= F_BUS / 384) {
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6);
            } else if (clock >= F_BUS / 512) {
                t = SPI_CTAR_PBR(0) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7);
            } else if (clock >= F_BUS / 640) {
                t = SPI_CTAR_PBR(2) | SPI_CTAR_BR(7) | SPI_CTAR_CSSCK(6);
            } else { /* F_BUS / 768 */
                t = SPI_CTAR_PBR(1) | SPI_CTAR_BR(8) | SPI_CTAR_CSSCK(7);
            }
        } else {
            for (uint32_t i = 0; i < 23; i++) {
                t = ctar_clock_table[i];
                if (clock >= F_BUS / ctar_div_table[i])
                    break;
            }
        }
        if (dataMode & 0x08) {
            c |= SPI_CTAR_CPOL;
        }
        if (dataMode & 0x04) {
            c |= SPI_CTAR_CPHA;
            t = (t & 0xFFFF0FFF) | ((t & 0xF000) >> 4);
        }
        ctar = c | t;
    }
    static const uint16_t ctar_div_table[23];
    static const uint32_t ctar_clock_table[23];
    uint32_t ctar;
    friend class SPIClass;
};


class SPIClass { // Teensy 3.x
public:
#if defined(__MK20DX128__) || defined(__MK20DX256__)
    static const uint8_t CNT_MISO_PINS = 2;
    static const uint8_t CNT_MOSI_PINS = 2;
    static const uint8_t CNT_SCK_PINS = 2;
    static const uint8_t CNT_CS_PINS = 9;
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
    static const uint8_t CNT_MISO_PINS = 4;
    static const uint8_t CNT_MOSI_PINS = 4;
    static const uint8_t CNT_SCK_PINS = 3;
    static const uint8_t CNT_CS_PINS = 11;
#endif
    typedef struct {
        volatile uint32_t& clock_gate_register;
        uint32_t clock_gate_mask;
        uint8_t queue_size;
        uint8_t spi_irq;
        uint32_t max_dma_count;
        uint8_t tx_dma_channel;
        uint8_t rx_dma_channel;
        void (*dma_rxisr)();
        uint8_t miso_pin[CNT_MISO_PINS];
        uint32_t miso_mux[CNT_MISO_PINS];
        uint8_t mosi_pin[CNT_MOSI_PINS];
        uint32_t mosi_mux[CNT_MOSI_PINS];
        uint8_t sck_pin[CNT_SCK_PINS];
        uint32_t sck_mux[CNT_SCK_PINS];
        uint8_t cs_pin[CNT_CS_PINS];
        uint32_t cs_mux[CNT_CS_PINS];
        uint8_t cs_mask[CNT_CS_PINS];
    } SPI_Hardware_t;
    static const SPI_Hardware_t spi0_hardware;
    static const SPI_Hardware_t spi1_hardware;
    static const SPI_Hardware_t spi2_hardware;

    enum DMAState { notAllocated, idle, active, completed };

public:
    constexpr SPIClass(uintptr_t myport, uintptr_t myhardware) : port_addr(myport), hardware_addr(myhardware) {}
    // Initialize the SPI library
    void begin();

    // If SPI is to used from within an interrupt, this function registers
    // that interrupt with the SPI library, so beginTransaction() can
    // prevent conflicts.  The input interruptNumber is the number used
    // with attachInterrupt.  If SPI is used from a different interrupt
    // (eg, a timer), interruptNumber should be 255.
    void usingInterrupt(uint8_t n) {
        if (n == 3 || n == 4 || n == 24 || n == 33) {
            usingInterrupt(IRQ_PORTA);
        } else if (n == 0 || n == 1 || (n >= 16 && n <= 19) || n == 25 || n == 32) {
            usingInterrupt(IRQ_PORTB);
        } else if ((n >= 9 && n <= 13) || n == 15 || n == 22 || n == 23 || (n >= 27 && n <= 30)) {
            usingInterrupt(IRQ_PORTC);
        } else if (n == 2 || (n >= 5 && n <= 8) || n == 14 || n == 20 || n == 21) {
            usingInterrupt(IRQ_PORTD);
        } else if (n == 26 || n == 31) {
            usingInterrupt(IRQ_PORTE);
        }
    }
    void usingInterrupt(IRQ_NUMBER_t interruptName);
    void notUsingInterrupt(IRQ_NUMBER_t interruptName);

    // Before using SPI.transfer() or asserting chip select pins,
    // this function is used to gain exclusive access to the SPI bus
    // and configure the correct settings.
    void beginTransaction(SPISettings settings) {
        if (interruptMasksUsed) {
            __disable_irq();
            if (interruptMasksUsed & 0x01) {
                interruptSave[0] = NVIC_ICER0 & interruptMask[0];
                NVIC_ICER0 = interruptSave[0];
            }
#if NVIC_NUM_INTERRUPTS > 32
            if (interruptMasksUsed & 0x02) {
                interruptSave[1] = NVIC_ICER1 & interruptMask[1];
                NVIC_ICER1 = interruptSave[1];
            }
#endif
#if NVIC_NUM_INTERRUPTS > 64 && defined(NVIC_ISER2)
            if (interruptMasksUsed & 0x04) {
                interruptSave[2] = NVIC_ICER2 & interruptMask[2];
                NVIC_ICER2 = interruptSave[2];
            }
#endif
#if NVIC_NUM_INTERRUPTS > 96 && defined(NVIC_ISER3)
            if (interruptMasksUsed & 0x08) {
                interruptSave[3] = NVIC_ICER3 & interruptMask[3];
                NVIC_ICER3 = interruptSave[3];
            }
#endif
            __enable_irq();
        }
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 1;
#endif
        if (port().CTAR0 != settings.ctar) {
            port().MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x3F);
            port().CTAR0 = settings.ctar;
            port().CTAR1 = settings.ctar | SPI_CTAR_FMSZ(8);
            port().MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x3F);
        }
    }

    template <SPITransferType T>
    T transfer(const T data) {
        port().SR = SPI_SR_TCF;
        port().PUSHR = data | SPI_PUSHR_CTAS(sizeof(T) - 1);
        while (!(port().SR & SPI_SR_TCF)) { // wait
        }
        return static_cast<T>(port().POPR);
    }

    template <typename T>
    std::enable_if<std::is_same<T, uint32_t>::value, uint32_t>::type transfer(const T data) {
        return transfer32(data);
    }

    uint32_t transfer(const uint32_t data) {
        uint32_t ret { static_cast<uint32_t>(transfer<uint16_t>(data << 16U) << 16U) };
        ret |= transfer<uint16_t>(static_cast<uint16_t>(data));
        return ret;
    }

    uint8_t transfer(const int data) {
        return transfer(static_cast<uint8_t>(data));
    }

    uint16_t transfer16(const uint16_t data) {
        return transfer(data);
    }

    uint32_t transfer32(const uint32_t data) {
        return transfer(data);
    }

    void inline transfer(void* buf, size_t count) {
        transfer(reinterpret_cast<uint8_t*>(buf), count);
    }

    template <SPITransferType T>
    void transfer(T* buf, size_t count) {
        transfer(buf, buf, count);
    }

    void setTransferWriteFill(uint16_t ch) {
        _transferWriteFill = ch;
    }

    template <SPITransferType T>
    void transfer(const T* buf, T* retbuf, size_t count);

    // Asynch support (DMA )
#ifdef SPI_HAS_TRANSFER_ASYNC
    template <SPITransferType T>
    bool transfer(const T* txBuffer, T* rxBuffer, size_t count, EventResponderRef event_responder);

    template <SPITransferType T>
    bool transfer_os(const T* txBuffer, T* rxBuffer, size_t count);

    friend void _spi_dma_rxISR0();
    friend void _spi_dma_rxISR1();
    friend void _spi_dma_rxISR2();

    inline void dma_rxisr();
#endif


    // After performing a group of transfers and releasing the chip select
    // signal, this function allows others to access the SPI bus
    void endTransaction() {
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (!inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 0;
#endif
        if (interruptMasksUsed) {
            if (interruptMasksUsed & 0x01) {
                NVIC_ISER0 = interruptSave[0];
            }
#if NVIC_NUM_INTERRUPTS > 32
            if (interruptMasksUsed & 0x02) {
                NVIC_ISER1 = interruptSave[1];
            }
#endif
#if NVIC_NUM_INTERRUPTS > 64 && defined(NVIC_ISER2)
            if (interruptMasksUsed & 0x04) {
                NVIC_ISER2 = interruptSave[2];
            }
#endif
#if NVIC_NUM_INTERRUPTS > 96 && defined(NVIC_ISER3)
            if (interruptMasksUsed & 0x08) {
                NVIC_ISER3 = interruptSave[3];
            }
#endif
        }
    }

    // Disable the SPI bus
    void end();

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setBitOrder(uint8_t bitOrder);

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setDataMode(uint8_t dataMode);

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setClockDivider(uint8_t clockDiv) {
        if (clockDiv == SPI_CLOCK_DIV2) {
            setClockDivider_noInline(SPISettings(12000000, MSBFIRST, SPI_MODE0).ctar);
        } else if (clockDiv == SPI_CLOCK_DIV4) {
            setClockDivider_noInline(SPISettings(4000000, MSBFIRST, SPI_MODE0).ctar);
        } else if (clockDiv == SPI_CLOCK_DIV8) {
            setClockDivider_noInline(SPISettings(2000000, MSBFIRST, SPI_MODE0).ctar);
        } else if (clockDiv == SPI_CLOCK_DIV16) {
            setClockDivider_noInline(SPISettings(1000000, MSBFIRST, SPI_MODE0).ctar);
        } else if (clockDiv == SPI_CLOCK_DIV32) {
            setClockDivider_noInline(SPISettings(500000, MSBFIRST, SPI_MODE0).ctar);
        } else if (clockDiv == SPI_CLOCK_DIV64) {
            setClockDivider_noInline(SPISettings(250000, MSBFIRST, SPI_MODE0).ctar);
        } else { /* clockDiv == SPI_CLOCK_DIV128 */
            setClockDivider_noInline(SPISettings(125000, MSBFIRST, SPI_MODE0).ctar);
        }
    }
    void setClockDivider_noInline(uint32_t clk);

    // These undocumented functions should not be used.  SPI.transfer()
    // polls the hardware flag which is automatically cleared as the
    // AVR responds to SPI's interrupt
    void attachInterrupt() {}
    void detachInterrupt() {}

    // Teensy 3.x can use alternate pins for these 3 SPI signals.
    void setMOSI(uint8_t pin);
    void setMISO(uint8_t pin);
    void setSCK(uint8_t pin);

    // return true if "pin" has special chip select capability
    uint8_t pinIsChipSelect(uint8_t pin);
    bool pinIsMOSI(uint8_t pin);
    bool pinIsMISO(uint8_t pin);
    bool pinIsSCK(uint8_t pin);
    // return true if both pin1 and pin2 have independent chip select capability
    bool pinIsChipSelect(uint8_t pin1, uint8_t pin2);
    // configure a pin for chip select and return its SPI_MCR_PCSIS bitmask
    // setCS() is a special function, not intended for use from normal Arduino
    // programs/sketches.  See the ILI3941_t3 library for an example.
    uint8_t setCS(uint8_t pin);

private:
    KINETISK_SPI_t& port() {
        return *(KINETISK_SPI_t*) port_addr;
    }
    const SPI_Hardware_t& hardware() {
        return *(const SPI_Hardware_t*) hardware_addr;
    }
    void updateCTAR(uint32_t ctar);
    uintptr_t port_addr;
    uintptr_t hardware_addr;
    uint8_t miso_pin_index = 0;
    uint8_t mosi_pin_index = 0;
    uint8_t sck_pin_index = 0;
    uint8_t interruptMasksUsed = 0;
    uint32_t interruptMask[(NVIC_NUM_INTERRUPTS + 31) / 32] = {};
    uint32_t interruptSave[(NVIC_NUM_INTERRUPTS + 31) / 32] = {};
#ifdef SPI_TRANSACTION_MISMATCH_LED
    uint8_t inTransactionFlag = 0;
#endif

    alignas(4) uint16_t _transferWriteFill = 0;

    // DMA Support
#ifdef SPI_HAS_TRANSFER_ASYNC
    bool initDMAChannels();
    DMAState _dma_state = DMAState::notAllocated;
    uint32_t _dma_count_remaining = 0; // How many bytes left to output after current DMA completes
    DMAChannel* _dmaTX = nullptr;
    DMAChannel* _dmaRX = nullptr;
    EventResponder* _dma_event_responder = nullptr;
#endif
};


#elif defined(__arm__) && defined(TEENSYDUINO) && defined(KINETISL)
/**********************************************************/
/*     32 bit Teensy-LC                                   */
/**********************************************************/

#define SPI_ATOMIC_VERSION 1

class SPISettings {
public:
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
        if (__builtin_constant_p(clock)) {
            init_AlwaysInline(clock, bitOrder, dataMode);
        } else {
            init_MightInline(clock, bitOrder, dataMode);
        }
    }
    SPISettings() {
        init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
    }

private:
    void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
        init_AlwaysInline(clock, bitOrder, dataMode);
    }
    void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) {
        uint8_t c = SPI_C1_MSTR | SPI_C1_SPE;
        if (dataMode & 0x04)
            c |= SPI_C1_CPHA;
        if (dataMode & 0x08)
            c |= SPI_C1_CPOL;
        if (bitOrder == LSBFIRST)
            c |= SPI_C1_LSBFE;
        c1 = c;
        if (__builtin_constant_p(clock)) {
            if (clock >= F_BUS / 2) {
                c = SPI_BR_SPPR(0) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 4) {
                c = SPI_BR_SPPR(1) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 6) {
                c = SPI_BR_SPPR(2) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 8) {
                c = SPI_BR_SPPR(3) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 10) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 12) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 14) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 16) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(0);
            } else if (clock >= F_BUS / 20) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(1);
            } else if (clock >= F_BUS / 24) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(1);
            } else if (clock >= F_BUS / 28) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(1);
            } else if (clock >= F_BUS / 32) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(1);
            } else if (clock >= F_BUS / 40) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(2);
            } else if (clock >= F_BUS / 48) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(2);
            } else if (clock >= F_BUS / 56) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(2);
            } else if (clock >= F_BUS / 64) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(2);
            } else if (clock >= F_BUS / 80) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(3);
            } else if (clock >= F_BUS / 96) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(3);
            } else if (clock >= F_BUS / 112) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(3);
            } else if (clock >= F_BUS / 128) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(3);
            } else if (clock >= F_BUS / 160) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(4);
            } else if (clock >= F_BUS / 192) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(4);
            } else if (clock >= F_BUS / 224) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(4);
            } else if (clock >= F_BUS / 256) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(4);
            } else if (clock >= F_BUS / 320) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(5);
            } else if (clock >= F_BUS / 384) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(5);
            } else if (clock >= F_BUS / 448) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(5);
            } else if (clock >= F_BUS / 512) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(5);
            } else if (clock >= F_BUS / 640) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(6);
            } else /* F_BUS / 768 */ {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(6);
            }
        } else {
            for (uint32_t i = 0; i < 30; i++) {
                c = br_clock_table[i];
                if (clock >= F_BUS / br_div_table[i])
                    break;
            }
        }
        br[0] = c;
        if (__builtin_constant_p(clock)) {
            if (clock >= (F_PLL / 2) / 2) {
                c = SPI_BR_SPPR(0) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 4) {
                c = SPI_BR_SPPR(1) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 6) {
                c = SPI_BR_SPPR(2) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 8) {
                c = SPI_BR_SPPR(3) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 10) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 12) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 14) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 16) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(0);
            } else if (clock >= (F_PLL / 2) / 20) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(1);
            } else if (clock >= (F_PLL / 2) / 24) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(1);
            } else if (clock >= (F_PLL / 2) / 28) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(1);
            } else if (clock >= (F_PLL / 2) / 32) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(1);
            } else if (clock >= (F_PLL / 2) / 40) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(2);
            } else if (clock >= (F_PLL / 2) / 48) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(2);
            } else if (clock >= (F_PLL / 2) / 56) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(2);
            } else if (clock >= (F_PLL / 2) / 64) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(2);
            } else if (clock >= (F_PLL / 2) / 80) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(3);
            } else if (clock >= (F_PLL / 2) / 96) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(3);
            } else if (clock >= (F_PLL / 2) / 112) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(3);
            } else if (clock >= (F_PLL / 2) / 128) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(3);
            } else if (clock >= (F_PLL / 2) / 160) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(4);
            } else if (clock >= (F_PLL / 2) / 192) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(4);
            } else if (clock >= (F_PLL / 2) / 224) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(4);
            } else if (clock >= (F_PLL / 2) / 256) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(4);
            } else if (clock >= (F_PLL / 2) / 320) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(5);
            } else if (clock >= (F_PLL / 2) / 384) {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(5);
            } else if (clock >= (F_PLL / 2) / 448) {
                c = SPI_BR_SPPR(6) | SPI_BR_SPR(5);
            } else if (clock >= (F_PLL / 2) / 512) {
                c = SPI_BR_SPPR(7) | SPI_BR_SPR(5);
            } else if (clock >= (F_PLL / 2) / 640) {
                c = SPI_BR_SPPR(4) | SPI_BR_SPR(6);
            } else /* (F_PLL/2) / 768 */ {
                c = SPI_BR_SPPR(5) | SPI_BR_SPR(6);
            }
        } else {
            for (uint32_t i = 0; i < 30; i++) {
                c = br_clock_table[i];
                if (clock >= (F_PLL / 2) / br_div_table[i])
                    break;
            }
        }
        br[1] = c;
    }
    static const uint8_t br_clock_table[30];
    static const uint16_t br_div_table[30];
    uint8_t c1, br[2];
    friend class SPIClass;
};


class SPIClass { // Teensy-LC
public:
    static const uint8_t CNT_MISO_PINS = 2;
    static const uint8_t CNT_MMOSI_PINS = 2;
    static const uint8_t CNT_SCK_PINS = 2;
    static const uint8_t CNT_CS_PINS = 2;
    typedef struct {
        volatile uint32_t& clock_gate_register;
        uint32_t clock_gate_mask;
        uint8_t br_index;
        uint8_t tx_dma_channel;
        uint8_t rx_dma_channel;
        void (*dma_isr)();
        uint8_t miso_pin[CNT_MISO_PINS];
        uint32_t miso_mux[CNT_MISO_PINS];
        uint8_t mosi_pin[CNT_MMOSI_PINS];
        uint32_t mosi_mux[CNT_MMOSI_PINS];
        uint8_t sck_pin[CNT_SCK_PINS];
        uint32_t sck_mux[CNT_SCK_PINS];
        uint8_t cs_pin[CNT_CS_PINS];
        uint32_t cs_mux[CNT_CS_PINS];
        uint8_t cs_mask[CNT_CS_PINS];
    } SPI_Hardware_t;
    static const SPI_Hardware_t spi0_hardware;
    static const SPI_Hardware_t spi1_hardware;
    enum DMAState { notAllocated, idle, active, completed };

public:
    constexpr SPIClass(uintptr_t myport, uintptr_t myhardware) : port_addr(myport), hardware_addr(myhardware) {}
    // Initialize the SPI library
    void begin();

    // If SPI is to used from within an interrupt, this function registers
    // that interrupt with the SPI library, so beginTransaction() can
    // prevent conflicts.  The input interruptNumber is the number used
    // with attachInterrupt.  If SPI is used from a different interrupt
    // (eg, a timer), interruptNumber should be 255.
    void usingInterrupt(uint8_t n) {
        if (n == 3 || n == 4) {
            usingInterrupt(IRQ_PORTA);
        } else if ((n >= 2 && n <= 15) || (n >= 20 && n <= 23)) {
            usingInterrupt(IRQ_PORTCD);
        }
    }
    void usingInterrupt(IRQ_NUMBER_t interruptName) {
        uint32_t n = (uint32_t) interruptName;
        if (n < NVIC_NUM_INTERRUPTS)
            interruptMask |= (1 << n);
    }
    void notUsingInterrupt(IRQ_NUMBER_t interruptName) {
        uint32_t n = (uint32_t) interruptName;
        if (n < NVIC_NUM_INTERRUPTS)
            interruptMask &= ~(1 << n);
    }

    // Before using SPI.transfer() or asserting chip select pins,
    // this function is used to gain exclusive access to the SPI bus
    // and configure the correct settings.
    void beginTransaction(SPISettings settings) {
        if (interruptMask) {
            __disable_irq();
            interruptSave = NVIC_ICER0 & interruptMask;
            NVIC_ICER0 = interruptSave;
            __enable_irq();
        }
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 1;
#endif
        port().C1 = settings.c1;
        port().BR = settings.br[hardware().br_index];
    }

    // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
    uint8_t transfer(uint8_t data) {
        port().DL = data;
        while (!(port().S & SPI_S_SPRF))
            ; // wait
        return port().DL;
    }
    uint16_t transfer16(uint16_t data) {
        port().C2 = SPI_C2_SPIMODE;
        port().S;
        port().DL = data;
        port().DH = data >> 8;
        while (!(port().S & SPI_S_SPRF))
            ; // wait
        uint16_t r = port().DL | (port().DH << 8);
        port().C2 = 0;
        port().S;
        return r;
    }
    void transfer(void* buf, size_t count) {
        if (count == 0)
            return;
        uint8_t* p = (uint8_t*) buf;
        while (!(port().S & SPI_S_SPTEF))
            ; // wait
        port().DL = *p;
        while (--count > 0) {
            uint8_t out = *(p + 1);
            while (!(port().S & SPI_S_SPTEF))
                ; // wait
            __disable_irq();
            port().DL = out;
            while (!(port().S & SPI_S_SPRF))
                ; // wait
            uint8_t in = port().DL;
            __enable_irq();
            *p++ = in;
        }
        while (!(port().S & SPI_S_SPRF))
            ; // wait
        *p = port().DL;
    }

    void setTransferWriteFill(uint8_t ch) {
        _transferWriteFill = ch;
    }
    void transfer(const void* buf, void* retbuf, size_t count);

    // Asynch support (DMA )
#ifdef SPI_HAS_TRANSFER_ASYNC
    bool transfer(const void* txBuffer, void* rxBuffer, size_t count, EventResponderRef event_responder);

    friend void _spi_dma_rxISR0(void);
    friend void _spi_dma_rxISR1(void);
    inline void dma_isr(void);
#endif

    // After performing a group of transfers and releasing the chip select
    // signal, this function allows others to access the SPI bus
    void endTransaction(void) {
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (!inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 0;
#endif
        if (interruptMask) {
            NVIC_ISER0 = interruptSave;
        }
    }

    // Disable the SPI bus
    void end();

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setBitOrder(uint8_t bitOrder) {
        uint8_t c = port().C1 | SPI_C1_SPE;
        if (bitOrder == LSBFIRST)
            c |= SPI_C1_LSBFE;
        else
            c &= ~SPI_C1_LSBFE;
        port().C1 = c;
    }

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setDataMode(uint8_t dataMode) {
        uint8_t c = port().C1 | SPI_C1_SPE;
        if (dataMode & 0x04)
            c |= SPI_C1_CPHA;
        else
            c &= ~SPI_C1_CPHA;
        if (dataMode & 0x08)
            c |= SPI_C1_CPOL;
        else
            c &= ~SPI_C1_CPOL;
        port().C1 = c;
    }

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setClockDivider(uint8_t clockDiv) {
        unsigned int i = hardware().br_index;
        if (clockDiv == SPI_CLOCK_DIV2) {
            port().BR = (SPISettings(12000000, MSBFIRST, SPI_MODE0).br[i]);
        } else if (clockDiv == SPI_CLOCK_DIV4) {
            port().BR = (SPISettings(4000000, MSBFIRST, SPI_MODE0).br[i]);
        } else if (clockDiv == SPI_CLOCK_DIV8) {
            port().BR = (SPISettings(2000000, MSBFIRST, SPI_MODE0).br[i]);
        } else if (clockDiv == SPI_CLOCK_DIV16) {
            port().BR = (SPISettings(1000000, MSBFIRST, SPI_MODE0).br[i]);
        } else if (clockDiv == SPI_CLOCK_DIV32) {
            port().BR = (SPISettings(500000, MSBFIRST, SPI_MODE0).br[i]);
        } else if (clockDiv == SPI_CLOCK_DIV64) {
            port().BR = (SPISettings(250000, MSBFIRST, SPI_MODE0).br[i]);
        } else { /* clockDiv == SPI_CLOCK_DIV128 */
            port().BR = (SPISettings(125000, MSBFIRST, SPI_MODE0).br[i]);
        }
    }

    // These undocumented functions should not be used.  SPI.transfer()
    // polls the hardware flag which is automatically cleared as the
    // AVR responds to SPI's interrupt
    void attachInterrupt() {}
    void detachInterrupt() {}

    // Teensy LC can use alternate pins for these 3 SPI signals.
    void setMOSI(uint8_t pin);
    void setMISO(uint8_t pin);
    void setSCK(uint8_t pin);
    // return true if "pin" has special chip select capability
    bool pinIsChipSelect(uint8_t pin);
    bool pinIsMOSI(uint8_t pin);
    bool pinIsMISO(uint8_t pin);
    bool pinIsSCK(uint8_t pin);
    // return true if both pin1 and pin2 have independent chip select capability
    bool pinIsChipSelect(uint8_t pin1, uint8_t pin2) {
        return false;
    }
    // configure a pin for chip select and return its SPI_MCR_PCSIS bitmask
    // setCS() is a special function, not intended for use from normal Arduino
    // programs/sketches.  See the ILI3941_t3 library for an example.
    uint8_t setCS(uint8_t pin);

private:
    KINETISL_SPI_t& port() {
        return *(KINETISL_SPI_t*) port_addr;
    }
    const SPI_Hardware_t& hardware() {
        return *(const SPI_Hardware_t*) hardware_addr;
    }
    uintptr_t port_addr;
    uintptr_t hardware_addr;
    uint32_t interruptMask = 0;
    uint32_t interruptSave = 0;
    uint8_t mosi_pin_index = 0;
    uint8_t miso_pin_index = 0;
    uint8_t sck_pin_index = 0;
#ifdef SPI_TRANSACTION_MISMATCH_LED
    uint8_t inTransactionFlag = 0;
#endif
    uint8_t _transferWriteFill = 0;
#ifdef SPI_HAS_TRANSFER_ASYNC
    // DMA Support
    bool initDMAChannels();

    DMAState _dma_state = DMAState::notAllocated;
    uint32_t _dma_count_remaining = 0; // How many bytes left to output after current DMA completes
    DMAChannel* _dmaTX = nullptr;
    DMAChannel* _dmaRX = nullptr;
    EventResponder* _dma_event_responder = nullptr;
#endif
};


#elif defined(__arm__) && defined(TEENSYDUINO) && (defined(__IMXRT1052__) || defined(__IMXRT1062__))
/**********************************************************/
/*     32 bit Teensy 4.x                                  */
/**********************************************************/

#define SPI_ATOMIC_VERSION 1

template <typename T>
concept SPITransferType = std::same_as<uint8_t, T> || std::same_as<uint16_t, T> || std::same_as<uint32_t, T>;


class SPISettings {
public:
    SPISettings(uint32_t clockIn, uint8_t bitOrderIn, uint8_t dataModeIn) : _clock(clockIn) {
        init_AlwaysInline(bitOrderIn, dataModeIn);
    }

    SPISettings() : _clock(4'000'000) {
        init_AlwaysInline(MSBFIRST, SPI_MODE0);
    }

private:
    void init_AlwaysInline(uint8_t bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) {
        tcr = LPSPI_TCR_FRAMESZ(7); // TCR has polarity and bit order too

        // handle LSB setup
        if (bitOrder == LSBFIRST) {
            tcr |= LPSPI_TCR_LSBF;
        }

        // Handle Data Mode
        if (dataMode & 0x08) {
            tcr |= LPSPI_TCR_CPOL;
        }

        // Note: On T3.2 when we set CPHA it also updated the timing. It moved the
        // PCS to SCK Delay Prescaler into the After SCK Delay Prescaler
        if (dataMode & 0x04) {
            tcr |= LPSPI_TCR_CPHA;
        }
    }

    inline uint32_t clock() {
        return _clock;
    }

    uint32_t _clock;
    uint32_t tcr; // transmit command, pg 2664 (RT1050 ref, rev 2)
    friend class SPIClass;
};

class SPIClass { // Teensy 4
    static constexpr bool _DEBUG { false };

public:
#if defined(ARDUINO_TEENSY41)
    // T4.1 has SPI2 pins on memory connectors as well as SDCard
    static constexpr uint8_t CNT_MISO_PINS { 2 };
    static constexpr uint8_t CNT_MOSI_PINS { 2 };
    static constexpr uint8_t CNT_SCK_PINS { 2 };
    static constexpr uint8_t CNT_CS_PINS { 3 };
#else
    static constexpr uint8_t CNT_MISO_PINS { 1 };
    static constexpr uint8_t CNT_MOSI_PINS { 1 };
    static constexpr uint8_t CNT_SCK_PINS { 1 };
    static constexpr uint8_t CNT_CS_PINS { 1 };
#endif
    typedef struct {
        volatile uint32_t& clock_gate_register;
        const uint32_t clock_gate_mask;
        uint8_t tx_dma_channel;
        uint8_t rx_dma_channel;
        void (*dma_rxisr)();
        // MISO pins
        const uint8_t miso_pin[CNT_MISO_PINS];
        const uint32_t miso_mux[CNT_MISO_PINS];
        const uint8_t miso_select_val[CNT_MISO_PINS];
        volatile uint32_t& miso_select_input_register;

        // MOSI pins
        const uint8_t mosi_pin[CNT_MOSI_PINS];
        const uint32_t mosi_mux[CNT_MOSI_PINS];
        const uint8_t mosi_select_val[CNT_MOSI_PINS];
        volatile uint32_t& mosi_select_input_register;

        // SCK pins
        const uint8_t sck_pin[CNT_SCK_PINS];
        const uint32_t sck_mux[CNT_SCK_PINS];
        const uint8_t sck_select_val[CNT_SCK_PINS];
        volatile uint32_t& sck_select_input_register;

        // CS Pins
        const uint8_t cs_pin[CNT_CS_PINS];
        const uint32_t cs_mux[CNT_CS_PINS];
        const uint8_t cs_mask[CNT_CS_PINS];
        const uint8_t pcs_select_val[CNT_CS_PINS];
        volatile uint32_t* pcs_select_input_register[CNT_CS_PINS];

    } SPI_Hardware_t;
    static const SPI_Hardware_t spiclass_lpspi4_hardware;
#if defined(__IMXRT1062__)
    static const SPI_Hardware_t spiclass_lpspi3_hardware;
    static const SPI_Hardware_t spiclass_lpspi1_hardware;
#endif
public:
    constexpr SPIClass(uintptr_t myport, uintptr_t myhardware) : port_addr(myport), hardware_addr(myhardware) {}

    // Initialize the SPI library
    void begin();

    // If SPI is to used from within an interrupt, this function registers
    // that interrupt with the SPI library, so beginTransaction() can
    // prevent conflicts. The input interruptNumber is the number used
    // with attachInterrupt. If SPI is used from a different interrupt
    // (eg, a timer), interruptNumber should be 255.
    void usingInterrupt(uint8_t n) {
        if (n >= CORE_NUM_DIGITAL) {
            return;
        }
#if defined(__IMXRT1062__)
        usingInterrupt(IRQ_GPIO6789);
#elif defined(__IMXRT1052__)
        volatile uint32_t* gpio = portOutputRegister(n);
        switch ((uint32_t) gpio) {
            case (uint32_t) &GPIO1_DR:
                usingInterrupt(IRQ_GPIO1_0_15);
                usingInterrupt(IRQ_GPIO1_16_31);
                break;
            case (uint32_t) &GPIO2_DR:
                usingInterrupt(IRQ_GPIO2_0_15);
                usingInterrupt(IRQ_GPIO2_16_31);
                break;
            case (uint32_t) &GPIO3_DR:
                usingInterrupt(IRQ_GPIO3_0_15);
                usingInterrupt(IRQ_GPIO3_16_31);
                break;
            case (uint32_t) &GPIO4_DR:
                usingInterrupt(IRQ_GPIO4_0_15);
                usingInterrupt(IRQ_GPIO4_16_31);
                break;
        }
#endif
    }
    void usingInterrupt(IRQ_NUMBER_t interruptName);
    void notUsingInterrupt(IRQ_NUMBER_t interruptName);

    void beginTransaction(SPISettings&& settings) {
        SPISettings tmp { settings };
        beginTransaction(tmp);
    }

    // Before using SPI.transfer() or asserting chip select pins,
    // this function is used to gain exclusive access to the SPI bus
    // and configure the correct settings.
    void beginTransaction(SPISettings& settings) {
        if (interruptMasksUsed) {
            __disable_irq();
            if (interruptMasksUsed & 0x01) {
                interruptSave[0] = NVIC_ICER0 & interruptMask[0];
                NVIC_ICER0 = interruptSave[0];
            }
            if (interruptMasksUsed & 0x02) {
                interruptSave[1] = NVIC_ICER1 & interruptMask[1];
                NVIC_ICER1 = interruptSave[1];
            }
            if (interruptMasksUsed & 0x04) {
                interruptSave[2] = NVIC_ICER2 & interruptMask[2];
                NVIC_ICER2 = interruptSave[2];
            }
            if (interruptMasksUsed & 0x08) {
                interruptSave[3] = NVIC_ICER3 & interruptMask[3];
                NVIC_ICER3 = interruptSave[3];
            }
            if (interruptMasksUsed & 0x10) {
                interruptSave[4] = NVIC_ICER4 & interruptMask[4];
                NVIC_ICER4 = interruptSave[4];
            }
            __enable_irq();
        }
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 1;
#endif

        if (settings.clock() != _clock) {
            static constexpr uint32_t clk_sel[4] = { 664615384, // PLL3 PFD1
                720000000, // PLL3 PFD0
                528000000, // PLL2
                396000000 }; // PLL2 PFD2

            // First save away the new settings..
            _clock = settings.clock();

            uint32_t cbcmr = CCM_CBCMR;
            uint32_t clkhz = clk_sel[(cbcmr >> 4) & 0x03] / (((cbcmr >> 26) & 0x07) + 1); // LPSPI peripheral clock

            uint32_t d, div;
            d = _clock ? clkhz / _clock : clkhz;

            if (d && clkhz / d > _clock) {
                d++;
            }
            if (d > 257) {
                d = 257; // max div
            }
            if (d > 2) {
                div = d - 2;
            } else {
                div = 0;
            }

            _ccr = LPSPI_CCR_SCKDIV(div) | LPSPI_CCR_DBT(div / 2) | LPSPI_CCR_PCSSCK(div / 2);
            if (_DEBUG) {
                Serial.printf(PSTR("SPI::beginTransaction(%u)\r\n"), settings.clock());
                Serial.printf(PSTR("SPI::beginTransaction(): clkhz:%u MHz cbcmr:0x%x\r\n"), clkhz / 1'000'000UL, cbcmr);
                Serial.printf(PSTR("SPI::beginTransaction(): CCR:0x%x TCR:0x%x\r\n"), _ccr, settings.tcr);
                Serial.print(PSTR("SPI::beginTransaction(): SPI frequency set to: "));
                const uint32_t freq { clkhz / ((_ccr & 0xff) + 2U) };
                if (freq % 1'000'000U == 0) {
                    Serial.printf(PSTR("%u MHz\r\n"), freq / 1'000'000U);
                } else if (freq % 1'000U == 0) {
                    Serial.printf(PSTR("%u KHz\r\n"), freq / 1'000U);
                } else {
                    Serial.printf(PSTR("%u Hz\r\n"), freq);
                }
            }
        }
        port().CR = 0;
        port().CFGR1 = LPSPI_CFGR1_MASTER | LPSPI_CFGR1_SAMPLE;
        port().CCR = _ccr;
        port().TCR = settings.tcr;
        port().CR = LPSPI_CR_MEN;
    }

    template <SPITransferType T>
    T transfer(const T data) {
        uint32_t tcr;
        if (sizeof(T) > 1) {
            tcr = port().TCR;
            port().TCR = (tcr & 0xfffff000) | LPSPI_TCR_FRAMESZ(sizeof(T) * 8 - 1); // turn on N bit mode
        }

        port().TDR = data; // output data
        while ((port().RSR & LPSPI_RSR_RXEMPTY)) {
        } // wait while the RSR fifo is empty...

        if (sizeof(T) > 1) {
            port().TCR = tcr; // restore TCR
        }

        return static_cast<T>(port().RDR);
    }

    uint8_t transfer(const int data) {
        return transfer(static_cast<uint8_t>(data));
    }

    uint16_t transfer16(const uint16_t data) {
        return transfer(data);
    }

    uint32_t transfer32(const uint32_t data) {
        return transfer(data);
    }

    void transfer(void* buf, size_t count) {
        transfer(reinterpret_cast<uint8_t*>(buf), count);
    }

    template <SPITransferType T>
    void transfer(T* buf, size_t count) {
        transfer(buf, buf, count);
    }
    void setTransferWriteFill(uint32_t ch) {
        _transferWriteFill = ch;
    }
    template <SPITransferType T>
    void transfer(const T* buf, T* retbuf, size_t count);

    // Asynch support (DMA )
#ifdef SPI_HAS_TRANSFER_ASYNC
    template <SPITransferType T>
    bool transfer(const T* txBuffer, T* rxBuffer, size_t count, EventResponderRef event_responder);

    template <SPITransferType T>
    bool transfer_os(const T* txBuffer, T* rxBuffer, size_t count);

    friend void _spi_dma_rxISR0();
    inline void dma_rxisr();
#endif


    // After performing a group of transfers and releasing the chip select
    // signal, this function allows others to access the SPI bus
    void endTransaction() {
#ifdef SPI_TRANSACTION_MISMATCH_LED
        if (!inTransactionFlag) {
            pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
            digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
        }
        inTransactionFlag = 0;
#endif
        if (interruptMasksUsed) {
            if (interruptMasksUsed & 0x01)
                NVIC_ISER0 = interruptSave[0];
            if (interruptMasksUsed & 0x02)
                NVIC_ISER1 = interruptSave[1];
            if (interruptMasksUsed & 0x04)
                NVIC_ISER2 = interruptSave[2];
            if (interruptMasksUsed & 0x08)
                NVIC_ISER3 = interruptSave[3];
            if (interruptMasksUsed & 0x10)
                NVIC_ISER4 = interruptSave[4];
        }
        // Serial.printf("SPI.endTransaction CCR:%x TCR:%x\n", port().CCR, port().TCR);
    }

    // Disable the SPI bus
    void end();

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setBitOrder(uint8_t bitOrder);

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setDataMode(uint8_t dataMode);

    // This function is deprecated.	 New applications should use
    // beginTransaction() to configure SPI settings.
    void setClockDivider(uint8_t clockDiv) {
        if (clockDiv == SPI_CLOCK_DIV2) {
            setClockDivider_noInline(12000000);
        } else if (clockDiv == SPI_CLOCK_DIV4) {
            setClockDivider_noInline(4000000);
        } else if (clockDiv == SPI_CLOCK_DIV8) {
            setClockDivider_noInline(2000000);
        } else if (clockDiv == SPI_CLOCK_DIV16) {
            setClockDivider_noInline(1000000);
        } else if (clockDiv == SPI_CLOCK_DIV32) {
            setClockDivider_noInline(500000);
        } else if (clockDiv == SPI_CLOCK_DIV64) {
            setClockDivider_noInline(250000);
        } else { /* clockDiv == SPI_CLOCK_DIV128 */
            setClockDivider_noInline(125000);
        }
    }
    void setClockDivider_noInline(uint32_t clk);

    // These undocumented functions should not be used.  SPI.transfer()
    // polls the hardware flag which is automatically cleared as the
    // AVR responds to SPI's interrupt
    void attachInterrupt() {}
    void detachInterrupt() {}

    // Teensy 3.x can use alternate pins for these 3 SPI signals.
    void setMOSI(uint8_t pin);
    void setMISO(uint8_t pin);
    void setSCK(uint8_t pin);

    // return true if "pin" has special chip select capability
    uint8_t pinIsChipSelect(uint8_t pin);
    bool pinIsMOSI(uint8_t pin);
    bool pinIsMISO(uint8_t pin);
    bool pinIsSCK(uint8_t pin);
    // return true if both pin1 and pin2 have independent chip select capability
    bool pinIsChipSelect(uint8_t pin1, uint8_t pin2);
    // configure a pin for chip select and return its SPI_MCR_PCSIS bitmask
    // setCS() is a special function, not intended for use from normal Arduino
    // programs/sketches. See the ILI3941_t3 library for an example.
    uint8_t setCS(uint8_t pin);

private:
    IMXRT_LPSPI_t& port() {
        return *(IMXRT_LPSPI_t*) port_addr;
    }
    const SPI_Hardware_t& hardware() {
        return *(const SPI_Hardware_t*) hardware_addr;
    }
    uintptr_t port_addr;
    uintptr_t hardware_addr;

    uint32_t _clock = 0;
    uint32_t _ccr = 0;

    void updateCTAR(uint32_t ctar);
    uint8_t miso_pin_index = 0;
    uint8_t mosi_pin_index = 0;
    uint8_t sck_pin_index = 0;
    uint8_t interruptMasksUsed = 0;
    uint32_t interruptMask[(NVIC_NUM_INTERRUPTS + 31) / 32] = {};
    uint32_t interruptSave[(NVIC_NUM_INTERRUPTS + 31) / 32] = {};
#ifdef SPI_TRANSACTION_MISMATCH_LED
    uint8_t inTransactionFlag = 0;
#endif

    alignas(4) uint32_t _transferWriteFill = 0;

#ifdef SPI_HAS_TRANSFER_ASYNC // DMA Support
    bool initDMAChannels();
    enum DMAState { notAllocated, idle, active, completed };
    static constexpr size_t MAX_DMA_COUNT { 32'767 };
    DMAState _dma_state = DMAState::notAllocated;
    size_t _dma_count_remaining = 0; // How many bytes left to output after current DMA completes
    DMAChannel* _dmaTX = nullptr;
    DMAChannel* _dmaRX = nullptr;
    EventResponder* _dma_event_responder = nullptr;
    uint32_t _dma_tcr_saved;
#endif
};

#endif // __IMXRT1052__ || __IMXRT1062__


extern SPIClass SPI;
#if defined(__MKL26Z64__)
extern SPIClass SPI1;
#endif
#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__)
extern SPIClass SPI1;
extern SPIClass SPI2;
#endif
