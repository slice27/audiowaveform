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

#include "JsonFileExporter.h"
#include "Streams.h"
#include "WaveformBuffer.h"
#include "Options.h"

JsonFileExporter::JsonFileExporter(WaveformBuffer& buffer,
                                   const Options& options,
                                   const boost::filesystem::path& output_filename) :
	FileExporter(buffer, options, output_filename)
{
}

//------------------------------------------------------------------------------

void JsonFileExporter::writeHeader(std::ofstream& stream)
{
	FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
	// Create all the headers for all the required files.
	for (int i = 0; i < buffer_.getNumChannels(); ++i) {
		// Write a header for the first file, or if a new file needs to be written for stereo version 1 files.
		std::string filename;
		if (openFile(stream, i, filename)) {
			output_stream << "Writing header to output file: " << filename << std::endl;
			prepareHeader(stream, i, version);
			closeFile(stream, i);
		}
	}
}

void JsonFileExporter::prepareHeader(std::ofstream& stream, int chan, FILE_VERSION version)
{
	stream << "{" << std::endl << "\t\"sample_rate\":" << buffer_.getSampleRate() << ',' << std::endl
		   << "\t\"samples_per_pixel\":" << buffer_.getSamplesPerPixel() << ',' << std::endl
		   << "\t\"channels\":" << buffer_.getNumChannels() << ',' << std::endl
		   << "\t\"bits\":" << buffer_.getBits() << ',' << std::endl
		   << "\t\"length\":" << buffer_.getSize(chan) << ',' << std::endl
		   << "\t\"version\":" << version << ',' << std::endl;
}

void JsonFileExporter::writeData(std::ofstream &stream)
{
	FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
	std::string filename;
	for (int i = 0; i < buffer_.getNumChannels(); ++i) {
		openFile(stream, i, filename);
		output_stream << "Writing channel " << std::to_string(i) << " to output file: " << filename << std::endl;
		prepareData(stream, i, version);
		closeFile(stream, i);
	}
	closeFile(stream); // Must close the stream so that the header can be written in version 2 files.
}

void JsonFileExporter::prepareData(std::ofstream& stream, int chan, FILE_VERSION version)
{
	int divisor = ((buffer_.getBits() == 8) ? 256 : 1);
	if (buffer_.getNumChannels() > chan) {
		switch (version) {
			case FileExporter::VERSION_1: stream << "\t\"data\":[" << std::endl; break;
			case FileExporter::VERSION_2: stream << "\t\"chan" << chan << "\":[" << std::endl; break;
			default:
				throw std::runtime_error("JsonFileExporter::prepareData: unknown file version " + std::to_string(version));
				return;
		}
		int chanBufferSize = buffer_.getSize(chan);
		if (chanBufferSize > 0) {
			stream << "\t\t" << (buffer_.getMinSample(0, chan) / divisor) 
				   << ','  << (buffer_.getMaxSample(0, chan) / divisor);
			for (int i = 1; i < chanBufferSize; ++i) {
				stream << ',' << (buffer_.getMinSample(i, chan) / divisor) 
					   << ',' << (buffer_.getMaxSample(i, chan) / divisor);
			}
		}
		stream << std::endl << "\t]" << std::endl;
		return;
	}
	throw std::runtime_error("JsonFileExporter::prepareData: Channel " + std::to_string(chan) + "does not exist.");
}

void JsonFileExporter::writeFooter(std::ofstream &stream)
{
	FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
	for (int i = 0; i < buffer_.getNumChannels(); ++i) {
		std::string filename;
		if (openFile(stream, i, filename)) {
			output_stream << "Writing footer to output file: " << filename << std::endl;
			prepareFooter(stream); // prepareFooter calls closeFile; no need to call closeFile to match openFile.
		}
	}
}

void JsonFileExporter::prepareFooter(std::ofstream& stream)
{
	if (stream.is_open()) {
		stream << '}' << std::endl;
		closeFile(stream);
	}
}


