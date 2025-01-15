/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

#include <QString>

#include <cstdint>

class QDebug;

/**
 * @brief Debug stack trace generation.
 *
 * This implementation is a little different from most others in that it won't
 * leak memory layout information in debug logs. Instead, it'll use the base
 * address of functions inside the binary. This is useful for debugging crashes
 * without local memory layout information, which we don't need when debugging
 * crash reports.
 */
namespace Stacktrace {
// This can be used in a no-heap context, like a signal handler.
struct Frame
{
    /** Frame number. */
    int number;
    /** Base address of the object containing the function. */
    uint64_t base;
    /** Name of the object (binary/library) containing the function. */
    const char* object;
    /** Name of the function. */
    const char* function; // This is ephemeral, do not store it.
    /** Offset of the program counter from the start of the function. */
    uint64_t offset;
};

/** Print the frame to a debug stream (potential memory allocation). */
QDebug operator<<(QDebug dbg, const Frame& frame);

namespace detail {
using Callback = void(void* userdata, const Frame& frame);

void process(Callback* callback, void* userdata);
} // namespace detail

template <typename F>
void process(F&& callback)
{
    detail::process([](void* userdata, const Frame& frame) { (*static_cast<F*>(userdata))(frame); },
                    &callback);
}
} // namespace Stacktrace
