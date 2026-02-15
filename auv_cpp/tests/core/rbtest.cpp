  #include "../../src/core/ring_buffer.hpp"
  #include <iostream>
  #include <cassert>

  int main() {
      SPSC<int, 8> buf;

      // push 3 items
      assert(buf.push(10));
      assert(buf.push(20));
      assert(buf.push(30));

      // pop in order (FIFO)
      assert(buf.pop() == 10);
      assert(buf.pop() == 20);
      assert(buf.pop() == 30);

      // empty
      assert(!buf.pop().has_value());

      // fill it up (7 items in 8-slot buffer)
      for (int i = 0; i < 7; i++) {
          assert(buf.push(i));
      }
      assert(!buf.push(99));  // 8th push fails, full

      std::cout << "All tests passed!" << std::endl;
      return 0;
  }
