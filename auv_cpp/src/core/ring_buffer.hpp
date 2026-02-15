#pragma once
#include <atomic>
#include <array>
#include <optional>
#include <type_traits>
//creating a universal ring buffer for any type and size

//size_t is a argument used by std:array, used as std::size_t. Its only positive, so better to use than integers IMO
template<typename T, size_t Capacity> 
struct SPSC {
    static_assert(Capacity > 0 && (Capacity & (Capacity-1)) == 0, "Capacity has to be a power of 2 for bit modulo");
    static_assert(std::is_trivially_copyable_v<T>,"T must be trivially copyable"); //for memcpy to be allowed
    private:
    alignas(64) std::array<T, Capacity> SPSC_Buffer; //alignas 64 is basically a way to stop false sharing
    alignas(64) std::atomic<size_t> head_index = 0; //It creates seperate cache lines for each variable
    alignas(64) std::atomic<size_t> tail_index = 0; //So that they dont invalidate eachother

    static constexpr size_t mask = Capacity - 1;
    
    
    public:

    bool push(const T& item){
        
        size_t h = head_index.load(std::memory_order_relaxed);
        size_t t = tail_index.load(std::memory_order_acquire);
        if (((h + 1) & mask) == t){
            return false;
        }

        SPSC_Buffer[h] = item;
        head_index.store((h+1) & mask, std::memory_order_release); //mask for wrapping around from 7 to 0 and not overshooting to 8. Basically modulo
        return true;
    };

    std::optional<T> pop(){
        size_t t = tail_index.load(std::memory_order_relaxed);
        size_t h = head_index.load(std::memory_order_acquire);
        if (t==h){
            return {};
        } else {
            const T tail_value = SPSC_Buffer[t];
            tail_index.store((t + 1)& mask, std::memory_order_release);
            return tail_value;
        }
    };
};