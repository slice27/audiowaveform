//------------------------------------------------------------------------------
//
// Copyright 2013-2018 BBC Research and Development
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

#include "WaveformGenerator.h"
#include "WaveformBuffer.h"
#include "Streams.h"

#include <boost/format.hpp>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

//------------------------------------------------------------------------------

ScaleFactor::~ScaleFactor()
{
}

//------------------------------------------------------------------------------

SamplesPerPixelScaleFactor::SamplesPerPixelScaleFactor(int samples_per_pixel) :
    samples_per_pixel_(samples_per_pixel)
{
}

//------------------------------------------------------------------------------

int SamplesPerPixelScaleFactor::getSamplesPerPixel(int /* sample_rate */) const
{
    return samples_per_pixel_;
}

//------------------------------------------------------------------------------

// Calculates samples_per_pixel such that the time range start_time to end_time
// fits the specified image width.

DurationScaleFactor::DurationScaleFactor(
    double start_time,
    double end_time,
    int width_pixels) :
    start_time_(start_time),
    end_time_(end_time),
    width_pixels_(width_pixels)
{
    if (end_time < start_time) {
        const std::string message = boost::str(
            boost::format("Invalid end time, must be greater than %1%") % start_time
        );

        throw std::runtime_error(message);
    }

    if (width_pixels < 1) {
        throw std::runtime_error("Invalid image width: minimum 1");
    }
}

//------------------------------------------------------------------------------

int DurationScaleFactor::getSamplesPerPixel(const int sample_rate) const
{
    const double seconds = end_time_ - start_time_;

    const int width_samples = static_cast<int>(seconds * sample_rate);

    const int samples_per_pixel = width_samples / width_pixels_;

    return samples_per_pixel;
}

//------------------------------------------------------------------------------

PixelsPerSecondScaleFactor::PixelsPerSecondScaleFactor(int pixels_per_second) :
    pixels_per_second_(pixels_per_second)
{
    if (pixels_per_second_ <= 0) {
        throw std::runtime_error("Invalid pixels per second: must be greater than zero");
    }
}

//------------------------------------------------------------------------------

int PixelsPerSecondScaleFactor::getSamplesPerPixel(int sample_rate) const
{
    return sample_rate / pixels_per_second_;
}

//------------------------------------------------------------------------------

const int MAX_SAMPLE = std::numeric_limits<short>::max();
const int MIN_SAMPLE = std::numeric_limits<short>::min();
const int MONO_CHANNEL = 0;
const int RESET_COUNT = 0;

//------------------------------------------------------------------------------

WaveformGenerator::WaveformGenerator(
    WaveformBuffer &buffer,
	const ScaleFactor& scale_factor,
	bool isMono) :
    buffer_(buffer),
    scale_factor_(scale_factor),
    channels_(0),
    samples_per_pixel_(0),
	mono_(isMono)
{
}

//------------------------------------------------------------------------------

bool WaveformGenerator::init(
    const int sample_rate,
    const int channels,
    const long /* frame_count */,
    const int /* buffer_size */)
{
    if (channels < 1 || channels > 2) {
        error_stream << "Can only generate waveform data from mono or stereo input files\n";
        return false;
    }

    channels_ = channels;
    samples_per_pixel_ = scale_factor_.getSamplesPerPixel(sample_rate);

    if (samples_per_pixel_ < 2) {
        error_stream << "Invalid zoom: minimum 2\n";
        return false;
    }
	for (int i = 0; i < (mono_ ? (MONO_CHANNEL+1) : channels_); ++i) {
		counts_.push_back(RESET_COUNT);
		mins_.push_back(MAX_SAMPLE);
		maxs_.push_back(MIN_SAMPLE);
		buffer_.setSamplesPerPixel(samples_per_pixel_);
		buffer_.setSampleRate(sample_rate);
	}

    output_stream << "Generating waveform data..." << std::endl
                  << "Samples per pixel: " << samples_per_pixel_ << std::endl
                  << "Input channels: " << channels_ << std::endl;

    return true;
}

//------------------------------------------------------------------------------

int WaveformGenerator::getSamplesPerPixel() const
{
    return samples_per_pixel_;
}

//------------------------------------------------------------------------------

void WaveformGenerator::reset(int chan_num)
{
	mins_[chan_num] = MAX_SAMPLE;
	maxs_[chan_num] = MIN_SAMPLE;
	counts_[chan_num] = RESET_COUNT;
}

//------------------------------------------------------------------------------

void WaveformGenerator::done()
{
	for (int chan = 0; chan < buffer_.getNumChannels(); ++chan) {
		if (counts_[chan] > RESET_COUNT) {
			buffer_.appendSamples(static_cast<short>(mins_[chan]), 
			                      static_cast<short>(maxs_[chan]), chan);
			output_stream << "(channel " << chan+1 << ") Generated " 
			              << buffer_.getSize(chan) << " points" << std::endl;
			reset(static_cast<int>(chan));
		}
	}
}

//------------------------------------------------------------------------------

// See BlockFile::CalcSummary in Audacity

bool WaveformGenerator::process(
    const short* input_buffer,
    const int input_frame_count)
{
    for (int i = 0; i < input_frame_count; ++i) {
        const int index = i * channels_;

		int sample = 0;
		for (int chan = 0; chan < channels_; ++chan) {
			sample += input_buffer[index + chan];
			if (!mono_) {
				process_channel(sample, chan);
				sample = 0;
			}
		}
		
        // Average samples from each input channel to make a single (mono) waveform
		if (mono_) {
			sample /= channels_;
			process_channel(sample, MONO_CHANNEL);
		}
    }
    return true;
}

void WaveformGenerator::process_channel(int sample, int chan_num)
{	
	// Avoid numeric overflow when converting to short
	if (sample > MAX_SAMPLE) {
		sample = MAX_SAMPLE;
	}
	else if (sample < MIN_SAMPLE) {
		sample = MIN_SAMPLE;
	}

	if (sample < mins_[chan_num]) {
		mins_[chan_num] = sample;
	}

	if (sample > maxs_[chan_num]) {
		maxs_[chan_num] = sample;
	}

	if (++counts_[chan_num] == samples_per_pixel_) {
		buffer_.appendSamples(static_cast<short>(mins_[chan_num]), 
		                      static_cast<short>(maxs_[chan_num]),
		                      chan_num);
		reset(chan_num);
	}
}
//------------------------------------------------------------------------------
