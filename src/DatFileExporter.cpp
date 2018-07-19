//------------------------------------------------------------------------------
//
// Copyright 2013-2018 BBC Research and Development
//
// Author: Chris Chaffey
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

#include "DatFileExporter.h"


//------------------------------------------------------------------------------

static void writeInt32(std::ostream& stream, int32_t value)
{
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

//------------------------------------------------------------------------------

static void writeUInt32(std::ostream& stream, uint32_t value)
{
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

//------------------------------------------------------------------------------

static void writeInt8(std::ostream& stream, int8_t value)
{
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

//------------------------------------------------------------------------------

template <typename T>
static void writeVector(std::ostream& stream, const std::vector<T>& values)
{
    static_assert(std::is_integral<T>::value, "T must be integral type");

    stream.write(
        reinterpret_cast<const char*>(&values[0]),
        static_cast<std::streamsize>(values.size() * sizeof(T))
    );
}

//------------------------------------------------------------------------------

DatFileExporter::DatFileExporter(const Options &options,
                                 const std::string output_filename):
	FileExporter(options, output_filename)
{
}
	
void DatFileExporter::writeHeader(std::ofstream& stream,
                                  const std::uint32_t chan,
                                  const std::uint32_t size,
                                  const std::uint32_t sample_rate_,
                                  const std::uint32_t samples_per_pixel_)
{

}

void DatFileExporter::writeChannel(std::ostream &stream,
                                   WaveformBuffer *data,
                                   const std::uint32_t chan_num)
{

}

void DatFileExporter::writeFooter(std::ostream& stream)
{
	
}

/*
	if (bits != 8 && bits != 16) {
        error_stream << "Invalid bits: must be either 8 or 16\n";
        return false;
    }

    bool success = true;

    std::ofstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);

    try {
        file.open(filename, std::ios::out | std::ios::binary);

        output_stream << "Writing output file: " << filename
                      << "\nResolution: " << bits << " bits" << std::endl;

        const int32_t version = 1;
        writeInt32(file, version);

        uint32_t flags = 0;

        if (bits == 8) {
            flags |= FLAG_8_BIT;
        }

        writeUInt32(file, flags);
        writeInt32(file, sample_rate_);
        writeInt32(file, samples_per_pixel_);

        const int size = getSize();

        writeUInt32(file, static_cast<uint32_t>(size));

        if ((flags & FLAG_8_BIT) != 0) {
            for (int i = 0; i < size; ++i) {
                int8_t min_value = static_cast<int8_t>(getMinSample(i) / 256);
                writeInt8(file, min_value);

                int8_t max_value = static_cast<int8_t>(getMaxSample(i) / 256);
                writeInt8(file, max_value);
            }
        }
        else {
            writeVector(file, data_);
        }
    }
    catch (const std::exception&) {
        reportWriteError(filename, strerror(errno));
        success = false;
    }

    return success;
*/

