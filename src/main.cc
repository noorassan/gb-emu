#include <chrono>
#include <memory>
#include <thread>

#include "bus.h"

int main() {
    Bus bus;
    //std::chrono::high_resolution_clock::time_point t1, t2;
    //std::chrono::duration<float, std::milli> time_span;
    
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>("test_roms/09-op-r,r.gb");
    bus.insertCartridge(cart);
    bus.reset(); 

    while(true) {
        //t1 = std::chrono::high_resolution_clock::now();

        // Gameboy clock runs at 4.19 mHz.
        // For convenience, we do all the clocks in a one second interval at once
        // and then wait for the remainder of the second.
        // 4.19 mHz / 60 Hz = 69,833.33 -> 69,833
        for(uint32_t i = 0; i < 69833; i++) {
            bus.clock();
        }

        //t2 = std::chrono::high_resolution_clock::now();
        //time_span = std::chrono::seconds(1) - (t2 - t1);
        //std::this_thread::sleep_for(time_span);
    }
}
