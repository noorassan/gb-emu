#include <memory>

#include "../src/bus.h"

int main() {
    Bus bus;
    
    std::shared_ptr<Cartridge> cart = std::make_shared<Cartridge>("test_roms/tetris.gb");
    bus.insertCartridge(cart);
    bus.reset(); 
    while(true) {
        bus.clock();
    }
}
