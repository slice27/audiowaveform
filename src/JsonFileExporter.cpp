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

#include "FileExporter.h"

JsonFileExporter::JsonFileExporter(const Options &options,
								   const std::string output_filename) :
	FileExporter(options, filename)
{
}

bool JsonFileExporter::ExportToFile(std::vector<std::unique_ptr<WaveformBuffer>> &buffers)
{
	bool ret = true;
	try {
		if (bits_ != 8 && bits_ != 16) {
			throw std::runtime_error("Invalid bits: must be either 8 or 16");
			return false;
		}

		std::ofstream file;
		int size = buffers[0]->getSize();
		file.exceptions(std::ios::badbit | std::ios::failbit);

		if (version_ == VERSION_2) {
			writeJsonHeader(file, size, buffers[0]->getSampleRate(),
			                buffers[0]->getSamplesPerPixel());
		}

		int chan_num = 0;
		for_each(buffers.begin(), buffers.end(), [&](auto &buf) {
			if (version_ == VERSION_1) {
				writeJsonHeader(file, size, buf->getSampleRate(), 
				                buf->getSamplesPerPixel());
			}
			writeJsonChannel(file, buf.get(), chan_num++);
			
			if (version_ == VERSION_1) {
				writeJsonFooter(file);
			}
		}
		});
		if (version_ == VERSION_2) {
			writeJsonFooter(file);
		}	
		
	} catch (const std::exception& e) {
		error_stream << e.what() << std::endl;
		ret = false;
	}
	return ret;
}

//------------------------------------------------------------------------------
void JsonFileExporter::writeJsonHeader(std::ostream& stream,
                                       const std::uint32_t size,
									   const std::uint32_t sample_rate_,
									   const std::uint32_t samples_per_pixel_)
{
	string filename = getOutputFilename(output_filename_);
	output_stream << "Writing output file: " << filename << std::endl;
	file.open(filename);
	stream << "{" << std::endl << "\"sample_rate\":" << sample_rate_ << std::endl;
		   << ",\"samples_per_pixel\":" << samples_per_pixel_ << std::endl;
		   << ",\"bits\":" << bits_ << std::endl;
		   << ",\"length\":" << size << std::endl;
		   << ", \"version\":" << version_ << std::endl;
}

void JsonFileExporter::writeJsonChannel(std::ostream &stream,
                                        WaveformBuffer *data,
                                        const std::uint32_t chan_num)
{
	if (version == FileExporter::VERSION_1) {
		stream << ",\"data\":";
	} else if (version == FileExporter::VERSION_2) {
		stream << ", \"chan" << chan_num << "\":";
	} else {
		string exception = "Unknown file version! Version: ";
		exception += version;
		throw std::runtime_error(exception);
	}
	
	stream << '[' << std::endl;
	int divisor = ((bits_ == 8) ? 256 : 1);
	
	if (data->getSize() > 0) {
		stream << '\t' << (data->getMinSample(0) / divisor) 
		       << ','  << (data->getMaxSample(0) / divisor);
		for (int i = 1; i < data->getSize(); ++i) {
			stream << ',' << (data->getMinSample(i) / divisor) 
			       << ',' << (data->getMaxSample(i) / divisor);
		}
	}
	stream << ']' << std::endl;
}

void JsonFileExporter::writeJsonFooter(std::ostream &stream)
{
	stream << '}' << std::endl;
	stream.flush();
	stream.close();
}
