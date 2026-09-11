#include "PelcoDFrame.h"

#include <numeric>

namespace FujinonSX800 {

std::uint8_t PelcoDFrame::calculateChecksum(const std::vector<std::uint8_t>& bytes) noexcept
{
    return calculateChecksum(bytes.data(), bytes.size());
}

std::uint8_t PelcoDFrame::calculateChecksum(const std::uint8_t* data, std::size_t length) noexcept
{
    if (data == nullptr || length == 0U) {
        return 0x00U;
    }

    std::uint32_t sum { 0U };
    for (std::size_t i { 0U }; i < length; ++i) {
        sum += data[i];
    }

    return static_cast<std::uint8_t>(sum & 0xFFU);
}

std::vector<std::uint8_t> PelcoDFrame::createFrame(
    std::uint8_t address, std::uint8_t cmd1, std::uint8_t cmd2, std::uint8_t data1, std::uint8_t data2)
{
    std::vector<std::uint8_t> frame { SyncByte, address, cmd1, cmd2, data1, data2, 0x00U };

    frame[6] = calculateChecksum(&frame[1], 5U);
    return frame;
}

bool PelcoDFrame::isValidOpcode(std::uint8_t cmd1) noexcept
{
    return cmd1 == 0x00U || cmd1 == 0x01U || cmd1 == 0x02U || cmd1 == 0x04U || cmd1 == 0x08U || cmd1 == 0x10U
        || cmd1 == 0xF0U || cmd1 == 0xF1U;
}

bool PelcoDFrame::isValidFrame(const std::vector<std::uint8_t>& frame) noexcept
{
    if (frame.empty() || frame[0] != SyncByte) {
        return false;
    }

    const std::size_t size { frame.size() };
    if (size != GeneralResponseSize && size != StandardFrameSize && size != QueryResponseSize) {
        return false;
    }

    // 4-byte general response: byte 2 must be 0x00
    if (size == GeneralResponseSize && frame[2] != 0x00U) {
        return false;
    }

    // 7-byte standard / extended frame: byte 2 must be a valid opcode prefix
    if (size == StandardFrameSize && !isValidOpcode(frame[2])) {
        return false;
    }

    // Checksum covers bytes from index 1 up to size - 2
    const std::uint8_t expectedChecksum { frame[size - 1U] };
    const std::uint8_t calculatedChecksum { calculateChecksum(&frame[1], size - 2U) };

    return expectedChecksum == calculatedChecksum;
}

std::vector<std::vector<std::uint8_t>> PelcoDFrame::splitStream(const std::vector<std::uint8_t>& stream)
{
    std::vector<std::vector<std::uint8_t>> frames;
    if (stream.empty()) {
        return frames;
    }

    std::size_t idx { 0U };
    const std::size_t length { stream.size() };

    while (idx < length) {
        // Find next SyncByte (0xFF)
        if (stream[idx] != SyncByte) {
            ++idx;
            continue;
        }

        const std::size_t remaining { length - idx };
        bool matched { false };

        // Test QueryResponseSize (18) first, then StandardFrameSize (7), then GeneralResponseSize (4)
        const std::size_t candidateSizes[] { QueryResponseSize, StandardFrameSize, GeneralResponseSize };

        for (const std::size_t candidateSize : candidateSizes) {
            if (remaining >= candidateSize) {
                if (candidateSize == GeneralResponseSize && stream[idx + 2U] != 0x00U) {
                    continue;
                }
                if (candidateSize == StandardFrameSize && !isValidOpcode(stream[idx + 2U])) {
                    continue;
                }

                const std::uint8_t expectedCksm { stream[idx + candidateSize - 1U] };
                const std::uint8_t computedCksm { calculateChecksum(&stream[idx + 1U], candidateSize - 2U) };

                if (expectedCksm == computedCksm) {
                    frames.emplace_back(stream.begin() + static_cast<std::ptrdiff_t>(idx),
                        stream.begin() + static_cast<std::ptrdiff_t>(idx + candidateSize));
                    idx += candidateSize;
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            // Not a recognized frame starting here, advance one byte
            ++idx;
        }
    }

    return frames;
}

} // namespace FujinonSX800
