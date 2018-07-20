//------------------------------------------------------------------------------
//
// Copyright 2013-2017 BBC Research and Development
//
// Author: Chris Needham
//
// This file is part of Audio Waveform Image Generator.
//
// Audio Waveform Image Generator is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// Audio Waveform Image Generator is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Audio Waveform Image Generator.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include "WaveformBuffer.h"
#include "Streams.h"

#include <boost/format.hpp>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

//------------------------------------------------------------------------------

static int32_t readInt32(std::istream& stream)
{
    int32_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));

    return value;
}

//------------------------------------------------------------------------------

static uint32_t readUInt32(std::istream& stream)
{
    uint32_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));

    return value;
}

//------------------------------------------------------------------------------

static int16_t readInt16(std::istream& stream)
{
    int16_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));

    return value;
}

//------------------------------------------------------------------------------

static int8_t readInt8(std::istream& stream)
{
    int8_t value;
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));

    return value;
}

//------------------------------------------------------------------------------

static void reportReadError(const char* filename, const char* message)
{
    error_stream << "Failed to read data file: " << filename << '\n'
                 << message << '\n';
}

//------------------------------------------------------------------------------

static void reportWriteError(const char* filename, const char* message)
{
    error_stream << "Failed to write data file: " << filename << '\n'
                 << message << '\n';
}

//------------------------------------------------------------------------------

const uint32_t FLAG_8_BIT = 0x00000001U;

//------------------------------------------------------------------------------

WaveformBuffer::WaveformBuffer() :
    sample_rate_(0),
    samples_per_pixel_(0),
    bits_(16)
{
	// Must always have at least one channel.
	channels_.push_back(vector_type());
}

//------------------------------------------------------------------------------

bool WaveformBuffer::load(const char* filename)
{
    bool success = true;

    std::ifstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);

    uint32_t size = 0;

    try {
        file.open(filename, std::ios::in | std::ios::binary);

        output_stream << "Reading waveform data file: " << filename << std::endl;

        const int32_t version = readInt32(file);

        if (version != 1) {
            reportReadError(
                filename,
                boost::str(boost::format("Cannot load data file version: %1%") % version).c_str()
            );

            return false;
        }

        const uint32_t flags = readUInt32(file);

        sample_rate_       = readInt32(file);
        samples_per_pixel_ = readInt32(file);

        size = readUInt32(file);

        if ((flags & FLAG_8_BIT) != 0) {
            bits_ = 8;

            for (uint32_t i = 0; i < size; ++i) {
                int8_t min_value = readInt8(file);
                channels_[0].push_back(static_cast<short>(
				                       static_cast<int16_t>(min_value) * 256));

                int8_t max_value = readInt8(file);
                channels_[0].push_back(static_cast<short>(
				                       static_cast<int16_t>(max_value) * 256));
            }
        }
        else {
            bits_ = 16;

            for (uint32_t i = 0; i < size; ++i) {
                int16_t min_value = readInt16(file);
                channels_[0].push_back(min_value);

                int16_t max_value = readInt16(file);
                channels_[0].push_back(max_value);
            }
        }

        output_stream << "Sample rate: " << sample_rate_ << " Hz"
                      << "\nBits: " << bits_
                      << "\nSamples per pixel: " << samples_per_pixel_
                      << "\nLength: " << getSize() << " points" << std::endl;

        if (samples_per_pixel_ < 2) {
            reportReadError(
                filename,
                boost::str(
                    boost::format("Invalid samples per pixel: %1%, minimum 2") % samples_per_pixel_
                ).c_str()
            );

            success = false;
        }
        else if (sample_rate_ < 1) {
            reportReadError(
                filename,
                boost::str(
                    boost::format("Invalid sample rate: %1% Hz, minimum 1 Hz") % sample_rate_
                ).c_str()
            );

            success = false;
        }

        file.clear();
    }
    catch (const std::exception& e) {

        // Note: Catching std::exception instead of std::ios::failure is a
        // workaround for a g++ v5 / v6 libstdc++ ABI bug.
        //
        // See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
        // and http://stackoverflow.com/questions/38471518

        if (!file.eof()) {
            reportReadError(filename, strerror(errno));
            success = false;
        }
    }

    const int actual_size = getSize();

    if (size != static_cast<uint32_t>(actual_size)) {
        error_stream << "Expected " << size << " points, read "
                     << actual_size << " min and max points\n";
    }

    return success;
}

//------------------------------------------------------------------------------

bool WaveformBuffer::saveAsText(const char* filename, int bits) const
{
    bool success = true;

    std::ofstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);

    try {
        file.open(filename);

        output_stream << "Writing output file: " << filename << std::endl;

        const int size = getSize();

        if (bits == 8) {
            for (int i = 0; i < size; ++i) {
                const int min_value = getMinSample(i) / 256;
                const int max_value = getMaxSample(i) / 256;

                file << min_value << ',' << max_value<< '\n';
            }
        }
        else {
            for (int i = 0; i < size; ++i) {
                file << getMinSample(i) << ','
                     << getMaxSample(i) << '\n';
            }
        }
    }
    catch (const std::exception&) {
        reportWriteError(filename, strerror(errno));
        success = false;
    }

    return success;
}

//------------------------------------------------------------------------------

