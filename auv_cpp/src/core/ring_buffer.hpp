#pragma once
#include <atomic>
#include <array>
#include <optional>
//creating a universal ring buffer for any type and size

//size_t is a argument used by std:array, used as std::size_t. Its only positive, so better to use than integers IMO
template<typename T, size_t Capacity> 
struct SPSC {
    private:
    std::array<T, Capacity> SPSC_Buffer;
    size_t head_index = 0;
    size_t tail_index = 0;

    static constexpr size_t mask = Capacity - 1;
    public:

    bool push(const T& item){
        if (((head_index + 1) & mask) == tail_index){
            return false;
        }

        SPSC_Buffer[head_index] = item;
        head_index = (head_index + 1) & mask; //for wrapping around from 7 to 0 and not overshooting to 8. Basically modulo
        return true;
    };

    std::optional<T> pop(){
        if (head_index == tail_index){
            return {};
        } else {
            const T tail_value = SPSC_Buffer[tail_index];
            tail_index = (tail_index + 1) & mask;
            return tail_value;
        }
    };
};