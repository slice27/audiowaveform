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
#include "Streams.h"
#include "WaveformBuffer.h"

const uint32_t FLAG_8_BIT = 0x00000001U;

/*******************************************************************************
*
* If a version 1 export is requested, this should behave as expected.  If a 
* version 2 file is requested, the header is written to the file, and the audio
* channel data is written in a non-interleaved mode.  This means that all
* channel 0 data will come before the channel 1 data.
*
*******************************************************************************/

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
                                 const boost::filesystem::path& output_filename):
	FileExporter(options, output_filename)
{
}
	
void DatFileExporter::writeHeader(std::ofstream& stream,
                                  const std::uint32_t chan,
                                  const std::uint32_t num_chans,
                                  const std::uint32_t size,
                                  const std::uint32_t sample_rate_,
                                  const std::uint32_t samples_per_pixel_)
{
	std::string filename = getOutputFilename(output_filename_, chan);
	output_stream << "Writing output file: " << filename
                  << "\nResolution: " << bits_ << " bits" << std::endl;
	stream.open(filename);
	
	writeInt32(stream, static_cast<std::int32_t>(version_));
	writeUInt32(stream, static_cast<std::uint32_t>((bits_ == 8) ? FLAG_8_BIT : 0));
	writeUInt32(stream, sample_rate_);
    writeUInt32(stream, samples_per_pixel_);
    writeUInt32(stream, static_cast<uint32_t>(size));
	if (VERSION_2 == version_) {
		writeUInt32(stream, num_chans);
	}
}

void DatFileExporter::writeChannel(std::ostream &stream,
                                   WaveformBuffer *data,
                                   const std::uint32_t chan_num)
{
	size_t curr_size = data->getSize();
	if (SIZE_MAX != last_size_) {
		if (curr_size != last_size_) {
			throw std::runtime_error("Channels are different sizes!");
		}
		curr_size = last_size_ = data->getSize();
	}
	if (VERSION_2 == version_) {
		writeUInt32(stream, chan_num);
	}
	if (bits_ == 8) {
		for (size_t i = 0; i < curr_size; ++i) {
			writeInt8(stream, static_cast<int8_t>(data->getMinSample(i) / 256));
			writeInt8(stream, static_cast<int8_t>(data->getMaxSample(i) / 256));
		}
	} else {
		writeVector(stream, data->getData());
	}
}

void DatFileExporter::writeFooter(std::ofstream& stream)
{
	stream.flush();
	stream.close();
}


