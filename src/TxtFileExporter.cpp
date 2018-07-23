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

#include "TxtFileExporter.h"
#include "Streams.h"
#include "WaveformBuffer.h"
#include "Options.h"
#include "Utils.h"

//------------------------------------------------------------------------------

TxtFileExporter::TxtFileExporter(WaveformBuffer &buffer,
                                 const Options &options,
                                 const fs::path& output_filename):
	FileExporter(buffer, options, output_filename),
	bits_(options.getBits())
{
}

//------------------------------------------------------------------------------

void TxtFileExporter::writeFile(std::ofstream& stream) {
	if (!buffer_.channelSizesMatch()) {
		throwError("TxtFileExporter::writeHeader", "channel sizes do not match.");
	}
	std::string filename;
	FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
	WaveformBuffer::size_type size = buffer_.getSize();
	switch (version) {
		case FileExporter::VERSION_1: {
			for (int chan = 0; chan < buffer_.getNumChannels(); ++chan) {
				if (openFile(stream, chan, filename)) {
					output_stream << "Writing channel " << std::to_string(chan) 
				                  << " to output file: " << filename << std::endl;
					for (WaveformBuffer::size_type len = 0; len < size; ++len) {
						writeData(stream, chan, len, version);
					}
					closeFile(stream);
				}
			}
		} break;
		case FileExporter::VERSION_2: {
			if (openFile(stream, 0, filename)) {
				output_stream << "Writing channel data to output file: " << filename << std::endl;
				for (WaveformBuffer::size_type len = 0; len < size; ++len) {
					for (int chan = 0; chan < buffer_.getNumChannels(); ++chan) {
						writeData(stream, chan, len, version);
					}
				}
				closeFile(stream);
			}
		} break;
		default:
			throwError("TxtFileExporter::writeFile",
			           "unknown file version " + std::to_string(version),
			           filename);
	}
}

//------------------------------------------------------------------------------

void TxtFileExporter::writeData(std::ofstream& stream, int chan, size_t len, FILE_VERSION version)
{
	short min = buffer_.getMinSample(len, chan);
	short max = buffer_.getMaxSample(len, chan);
	if (bits_ == 8) {
		min /= 256;
		max /= 256;
	}
	stream << min << ',' << max;
	if (VERSION_2 == version) {
		stream << ',' << chan;
	}
	stream << std::endl;
}
