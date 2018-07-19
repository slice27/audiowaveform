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

JsonFileExporter::JsonFileExporter(const Options &options,
                                   const boost::filesystem::path& output_filename) :
	FileExporter(options, output_filename)
{
}

//------------------------------------------------------------------------------
void JsonFileExporter::writeHeader(std::ofstream& stream,
                                   const std::uint32_t chan,
                                   const std::uint32_t num_chans,
                                   const std::uint32_t size,
                                   const std::uint32_t sample_rate_,
                                   const std::uint32_t samples_per_pixel_)
{
	std::string filename = getOutputFilename(output_filename_, chan);
	output_stream << "Writing output file: " << filename << std::endl;
	stream.open(filename);
	stream << "{" << std::endl << "\t\"sample_rate\":" << sample_rate_ << ',' << std::endl
	       << "\t\"samples_per_pixel\":" << samples_per_pixel_ << ',' << std::endl
	       << "\t\"channels\":" << num_chans << ',' << std::endl
	       << "\t\"bits\":" << bits_ << ',' << std::endl
	       << "\t\"length\":" << size << ',' << std::endl
	       << "\t\"version\":" << version_ << ',' << std::endl;
}

void JsonFileExporter::writeChannel(std::ostream &stream,
                                    WaveformBuffer *data,
                                    const std::uint32_t chan_num)
{
	if (version_ == FileExporter::VERSION_1) {
		stream << "\t\"data\":";
	} else if (version_ == FileExporter::VERSION_2) {
		stream << "\t\"chan" << chan_num << "\":";
	} else {
		std::string ex = "Unknown file version! Version: ";
		ex += version_;
		throw std::runtime_error(ex);
	}
	
	stream << '[' << std::endl;
	int divisor = ((bits_ == 8) ? 256 : 1);
	
	if (data->getSize() > 0) {
		stream << '\t\t' << (data->getMinSample(0) / divisor) 
		       << ','  << (data->getMaxSample(0) / divisor);
		for (int i = 1; i < data->getSize(); ++i) {
			stream << ',' << (data->getMinSample(i) / divisor) 
			       << ',' << (data->getMaxSample(i) / divisor);
		}
	}
	stream << std::endl << '\t]' << std::endl;
}

void JsonFileExporter::writeFooter(std::ofstream &stream)
{
	stream << '}' << std::endl;
	stream.flush();
	stream.close();
}
