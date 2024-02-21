// From https://github.com/DNedic/lockfree
/**************************************************************
 * @file queue.hpp
 * @brief A queue implementation written in standard c++11
 * suitable for both low-end microcontrollers all the way
 * to HPC machines. Lock-free for all scenarios.
 **************************************************************/

/**************************************************************
 * Copyright (c) 2023 Djordje Nedic
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of lockfree
 *
 * Author:          Djordje Nedic <nedic.djordje2@gmail.com>
 * Version:         v2.0.6
 **************************************************************/

/************************** INCLUDE ***************************/
#ifndef LOCKFREE_MPMC_QUEUE_HPP
#define LOCKFREE_MPMC_QUEUE_HPP

#include <atomic>
#include <cstddef>
#include <type_traits>

#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#include <optional>
#endif

namespace lockfree {
namespace mpmc {
/*************************** TYPES ****************************/

template <typename T, size_t size> class Queue {
    static_assert(std::is_trivial<T>::value, "The type T must be trivial");
    static_assert(size > 2, "Buffer size must be bigger than 2");

    /********************** PUBLIC METHODS ************************/
  public:
    Queue();

    /**
     * @brief Adds an element into the queue.
     * @param[in] element
     * @retval Operation success
     */
    bool Push(const T &element);

    /**
     * @brief Removes an element from the queue.
     * @param[out] element
     * @retval Operation success
     */
    bool Pop(T &element);

#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    /**
     * @brief Removes an element from the queue.
     * @retval Either the element or nothing
     */
    std::optional<T> PopOptional();
#endif

    /*********************** PRIVATE TYPES ************************/
  private:
    struct Slot {
        T val;
        std::atomic_size_t pop_count;
        std::atomic_size_t push_count;

        Slot() : pop_count(0U), push_count(0U) {}
    };

    /********************** PRIVATE MEMBERS ***********************/
  private:
    Slot _data[size]; /**< Data array */
#if LOCKFREE_CACHE_COHERENT
    alignas(LOCKFREE_CACHELINE_LENGTH)
        std::atomic_size_t _r_count; /**< Read monotonic counter */
    alignas(LOCKFREE_CACHELINE_LENGTH)
        std::atomic_size_t _w_count; /**< Write monotonic counter */
#else
    std::atomic_size_t _r_count; /**< Read monotonic counter */
    std::atomic_size_t _w_count; /**< Write monotonic counter */
#endif
};

} /* namespace mpmc */
} /* namespace lockfree */

/************************** INCLUDE ***************************/

namespace lockfree {
namespace mpmc {
/********************** PUBLIC METHODS ************************/

template <typename T, size_t size>
Queue<T, size>::Queue() : _r_count(0U), _w_count(0U) {}

template <typename T, size_t size> bool Queue<T, size>::Push(const T &element) {
    size_t w_count = _w_count.load(std::memory_order_relaxed);

    while (true) {
        const size_t index = w_count % size;

        const size_t push_count =
            _data[index].push_count.load(std::memory_order_acquire);
        const size_t pop_count =
            _data[index].pop_count.load(std::memory_order_relaxed);

        if (push_count > pop_count) {
            return false;
        }

        const size_t revolution_count = w_count / size;
        const bool our_turn = revolution_count == push_count;

        if (our_turn) {
            /* Try to acquire the slot by bumping the monotonic write counter */
            if (_w_count.compare_exchange_weak(w_count, w_count + 1U,
                                               std::memory_order_relaxed)) {
                _data[index].val = element;
                _data[index].push_count.store(push_count + 1U,
                                              std::memory_order_release);
                return true;
            }
        } else {
            w_count = _w_count.load(std::memory_order_relaxed);
        }
    }
}

template <typename T, size_t size> bool Queue<T, size>::Pop(T &element) {
    size_t r_count = _r_count.load(std::memory_order_relaxed);

    while (true) {
        const size_t index = r_count % size;

        const size_t pop_count =
            _data[index].pop_count.load(std::memory_order_acquire);
        const size_t push_count =
            _data[index].push_count.load(std::memory_order_relaxed);

        if (pop_count == push_count) {
            return false;
        }

        const size_t revolution_count = r_count / size;
        const bool our_turn = revolution_count == pop_count;

        if (our_turn) {
            /* Try to acquire the slot by bumping the monotonic read counter. */
            if (_r_count.compare_exchange_weak(r_count, r_count + 1U,
                                               std::memory_order_relaxed)) {
                element = _data[index].val;
                _data[index].pop_count.store(pop_count + 1U,
                                             std::memory_order_release);
                return true;
            }
        } else {
            r_count = _r_count.load(std::memory_order_relaxed);
        }
    }
}

/********************* std::optional API **********************/
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
template <typename T, size_t size>
std::optional<T> Queue<T, size>::PopOptional() {
    T element;
    bool result = Pop(element);

    if (result) {
        return element;
    } else {
        return {};
    }
}
#endif

} /* namespace mpmc */
} /* namespace lockfree */

#endif /* LOCKFREE_MPMC_QUEUE_HPP */
