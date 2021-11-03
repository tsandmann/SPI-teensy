# SPI Library for Teensy

http://www.pjrc.com/teensy/td_libs_SPI.html

## Updated version for Teensy 4.x with FreeRTOS

This is an experimental fork with optimizations for teensy 4.x boards that adds a DMA-based transfer for [FreeRTOS](https://github.com/tsandmann/freertos-teensy): 
```C++
template <SPITransferType T> bool transfer_os(const T* txBuffer, T* rxBuffer, size_t count)
```

See [github.com/PaulStoffregen/SPI](https://github.com/PaulStoffregen/SPI) for the original version.
