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

#include "DatFileImporter.h"
#include "Streams.h"
#include "WaveformBuffer.h"
#include "Utils.h"
#include "Options.h"

//------------------------------------------------------------------------------

template<typename T>
static T read(std::istream& stream)
{
	T value;
	stream.read(reinterpret_cast<char*>(&value), sizeof(T));

	return value;
}

//------------------------------------------------------------------------------

static int32_t readInt32(std::istream& stream)
{
    return read<int32_t>(stream);
}

//------------------------------------------------------------------------------

static uint32_t readUInt32(std::istream& stream)
{
    return read<uint32_t>(stream);
}

//------------------------------------------------------------------------------

static int16_t readInt16(std::istream& stream)
{
	return read<int16_t>(stream);
}

//------------------------------------------------------------------------------

static int8_t readInt8(std::istream& stream)
{
	return read<int8_t>(stream);
}

//------------------------------------------------------------------------------

DatFileImporter::DatFileImporter(WaveformBuffer &buffer,
                                 const Options &options,
                                 const fs::path& input_filename) :
	FileImporter(buffer, options, input_filename),
	version_(FileExporter::VERSION_1),
	channels_(1),
	size_(0)
{
}

//------------------------------------------------------------------------------
				
void DatFileImporter::readFile(std::ifstream& stream)
{
	UNUSED(stream);
	std::ifstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);

	std::string filename = input_filename_.string();
    try {
        file.open(filename, std::ios::in | std::ios::binary);
		readHeader(file);
		readData(file);
	} catch (std::exception& e) {

		// Note: Catching std::exception instead of std::ios::failure is a
        // workaround for a g++ v5 / v6 libstdc++ ABI bug.
        //
        // See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
        // and http://stackoverflow.com/questions/38471518

        if (!file.eof()) {
			throwErrorEx("DatFileImporter::load", strerror(errno), filename);
        }
	}
	file.clear();
}


//------------------------------------------------------------------------------

void DatFileImporter::readHeader(std::ifstream& stream)
{
	version_ = static_cast<FileExporter::FILE_VERSION>(readInt32(stream));
	if ((FileExporter::VERSION_1 != version_) &&
	    (FileExporter::VERSION_2 != version_)) {
		throwErrorEx("DatFileImporter::readHeader", "Unknown file version " +
		           std::to_string(version_), input_filename_.string());
	}
	
	buffer_.setBits(((readUInt32(stream) == WaveformBuffer::FLAG_8_BIT) ? 8 : 16));
	buffer_.setSampleRate(readUInt32(stream));
	buffer_.setSamplesPerPixel(readUInt32(stream));
	size_ = readUInt32(stream);
	if (FileExporter::VERSION_2 == version_) {
		channels_ = readUInt32(stream);
	}
	
	output_stream << "File version: " << version_ << std::endl
	              << "Sample rate: " << buffer_.getSampleRate() << " Hz" << std::endl
	              << "Bits: " << buffer_.getBits() << std::endl
	              << "Samples per pixel: " << buffer_.getSamplesPerPixel() << std::endl
	              << "Length: " << size_ << " points" << std::endl;
	if (FileExporter::VERSION_2 == version_) {
		output_stream << "Channels: " << channels_ << std::endl;
	}
}

void DatFileImporter::readData(std::ifstream& stream)
{
	int bits = buffer_.getBits();
	short min = 0, max = 0;
	bool mono = (options_.getMono() && (channels_ > 1));
	
	for (int32_t size = 0; size < size_; ++size) {
		if (mono) {
			int min_value = 0, max_value = 0;
			for (uint32_t chan = 0; chan < channels_; ++chan) {
				getSamples(stream, bits, min, max);
				min_value += min;
				max_value += max;
			}
			min = static_cast<short>(min_value / channels_);
			max = static_cast<short>(max_value / channels_);
			buffer_.appendSamples(min, max, 0);
		} else {
			for (uint32_t chan = 0; chan < channels_; ++chan) {
				getSamples(stream, bits, min, max);
				buffer_.appendSamples(min, max, chan);
			}
		}
	}
	
	output_stream << "Completed import of " << input_filename_ 
	              << ".  Total points: " << buffer_.getSize() << std::endl;
	if (buffer_.getSize() != size_) {
		throwErrorEx("DatFileImporter::readData", 
		    "Corrupted input file. expected " + 
		    std::to_string(size_) + " points, but " +
		    std::to_string(buffer_.getSize()) + " points found.",
		    input_filename_.string());
	}
}

void DatFileImporter::getSamples(std::ifstream& stream, int bits, 
                                 short& min, short& max)
{
	switch (bits) {
		case 8: {
			min = static_cast<short>(readInt8(stream) * 256);
			max = static_cast<short>(readInt8(stream) * 256);
		} break;
		case 16: {
			min = readInt16(stream);
			max = readInt16(stream);
		} break;
	}
}
/*
bool DatFileImporter::load(const char* filename)
{
    bool success = true;

    std::ifstream file;
    file.exceptions(std::ios::badbit | std::ios::failbit);

    uint32_t size = 0;

    try {
        file.open(filename, std::ios::in | std::ios::binary);

        output_stream << "Reading waveform data file: " << filename << std::endl;

        const int32_t version = readInt32(file);

        if ((version != 1) && (version !=2)) {
			throwErrorEx("DatFileImporter::load", "Cannot load data file version: " + std::to_string(version));
            return false;
        }

        const uint32_t flags = readUInt32(file);

		buffer_.setBits((flags & 0x01) ? 8 : 16);
        buffer_.setSampleRate(readInt32(file));
		buffer_.setSamplesPerPixel(readInt32(file));

        size = readUInt32(file);
		int channels = 1;
		if (version == 2) {
			channels = readUInt32(file);
		}

        if ((flags & 0x01) != 0) {
            for (uint32_t i = 0; i < size; ++i) {
				for (int chan = 0; chan < channels; ++chan) {
					int8_t min_value = readInt8(file);
					int8_t max_value = readInt8(file);
					buffer_.appendSamples(static_cast<int16_t>(min_value) * 256,
										  static_cast<int16_t>(max_value) * 256,
										  chan);
				}
            }
        }
        else {
            for (uint32_t i = 0; i < size; ++i) {
				for (int chan = 0; chan < channels; ++chan) {
					int16_t min_value = readInt16(file);
					int16_t max_value = readInt16(file);
					buffer_.appendSamples(min_value, max_value, chan);
				}
            }
        }

        output_stream << "Sample rate: " << buffer_.getSampleRate() << " Hz"
                      << "\nBits: " << buffer_.getBits()
                      << "\nSamples per pixel: " << buffer_.getSamplesPerPixel()
                      << "\nLength: " << buffer_.getSize() << " points" << std::endl;

        if (buffer_.getSamplesPerPixel() < 2) {
			throwErrorEx("DatFileImporter::load",
			             "Invalid samples per pixel: " + 
						 std::to_string(buffer_.getSamplesPerPixel()) + ", minimum 2");
            success = false;
        }
        else if (buffer_.getSampleRate() < 1) {
			throwErrorEx("DatFileImporter::load",
			             "Invalid sample rate: " + std::to_string(buffer_.getSampleRate()) + " Hz, minimum 1 Hz");
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
			throwErrorEx("DatFileImporter::load", strerror(errno), filename);
            success = false;
        }
    }

    const int actual_size = buffer_.getSize();

    if (size != static_cast<uint32_t>(actual_size)) {
        error_stream << "Expected " << size << " points, read "
                     << actual_size << " min and max points\n";
    }

    return success;
}
*/