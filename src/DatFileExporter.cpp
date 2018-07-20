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
#include "Options.h"
#include "Utils.h"

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
template<typename T>
static void write(std::ostream& stream, T value)
{
	stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

//------------------------------------------------------------------------------

static void writeInt32(std::ostream& stream, int32_t value)
{
    //stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
	write(stream, value);
}

//------------------------------------------------------------------------------

static void writeUInt32(std::ostream& stream, uint32_t value)
{
    //stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
	write(stream, value);
}

//------------------------------------------------------------------------------

static void writeInt8(std::ostream& stream, int8_t value)
{
	write(stream, value);
}

//------------------------------------------------------------------------------

static void writeInt16(std::ostream& stream, int16_t value) {
	write(stream, value);
}

//------------------------------------------------------------------------------

DatFileExporter::DatFileExporter(WaveformBuffer &buffer,
                                 const Options &options,
                                 const boost::filesystem::path& output_filename):
	FileExporter(buffer, options, output_filename),
	bits_(options.getBits())
{
}

//------------------------------------------------------------------------------

void DatFileExporter::writeHeader(std::ofstream& stream)
{
	if (!buffer_.channelSizesMatch()) {
		throw std::runtime_error("DatFileExporter::writeHeader: channel sizes do not match.");
		return;
	}
	FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
	// Create all the headers for all the required files.
	for (int i = 0; i < buffer_.getNumChannels(); ++i) {
		// Write a header for the first file, or if a new file needs to be written for stereo version 1 files.
		std::string filename;
		if (openFile(stream, i, filename)) {
			output_stream << "Writing header to output file: " << filename << std::endl;
			prepareHeader(stream, version);
			closeFile(stream, i);
		}
	}
}

void DatFileExporter::prepareHeader(std::ofstream& stream, FILE_VERSION version)
{
	writeInt32(stream, static_cast<std::int32_t>(version));
	writeUInt32(stream, static_cast<std::uint32_t>((bits_ == 8) ? FLAG_8_BIT : 0));
	writeUInt32(stream, buffer_.getSampleRate());
    writeUInt32(stream, buffer_.getSamplesPerPixel());
    writeUInt32(stream, static_cast<uint32_t>(buffer_.getSize()));
	if (FileExporter::VERSION_2 == version) {
		writeUInt32(stream, buffer_.getNumChannels());
	}
}

//------------------------------------------------------------------------------

void DatFileExporter::writeData(std::ofstream &stream)
{
	std::string filename;
	WaveformBuffer::size_type size = buffer_.getSize();
	if (needNewFile()) {
		// Write a dat file for each channel separately.
		for (int chan = 0; chan < buffer_.getNumChannels(); ++chan) {
			openFile(stream, chan, filename);
			output_stream << "Writing channel " << std::to_string(chan) << " to output file: " << filename << std::endl;
			for (WaveformBuffer::size_type len = 0; len < size; ++len) {
				prepareData(stream, chan, len);
			}
			closeFile(stream, chan);
		}
	} else {
		openFile(stream, 0, filename);
		output_stream << "Writing channel data to output file: " << filename << std::endl;
		for (WaveformBuffer::size_type len = 0; len < size; ++len) {
			for (int chan = 0; chan < buffer_.getNumChannels(); ++chan) {
				prepareData(stream, chan, len);
			}
		}
		closeFile(stream);
	}
}

void DatFileExporter::prepareData(std::ofstream& stream, int chan, size_t len)
{
	short min = buffer_.getMinSample(len, chan);
	short max = buffer_.getMaxSample(len, chan);

	if (bits_ == 8) {
		writeInt8(stream, static_cast<int8_t>(min / 256));
		writeInt8(stream, static_cast<int8_t>(max / 256));
	} else {
		writeInt16(stream, min);
		writeInt16(stream, max);
	}
}

//------------------------------------------------------------------------------

void DatFileExporter::writeFooter(std::ofstream& stream)
{
	UNUSED(stream);
	// Files are closed in writeData.  No footer required.
}
