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

WaveformBuffer::WaveformBuffer() :
    sample_rate_(0),
    samples_per_pixel_(0),
    bits_(16)
{
	// Must always have at least one channel.
	channels_.push_back(vector_type());
}

//------------------------------------------------------------------------------

std::vector<WaveformBuffer> WaveformBuffer::SplitChannels()
{
	std::vector<WaveformBuffer> ret;
	for (int i = 0; i < getNumChannels(); ++i) {
		WaveformBuffer b;
		b.sample_rate_       = sample_rate_;
		b.samples_per_pixel_ = samples_per_pixel_;
		b.bits_              = bits_;
		b.channels_[0]       = channels_[i];
		ret.push_back(b);
	}
	return ret;
}

//------------------------------------------------------------------------------

void WaveformBuffer::setSampleRate(int sample_rate)
{
	if (sample_rate < 1) {
		throw std::runtime_error(
		    "WaveformBuffer::setSampleRate: Invalid sample rate: " +
			std::to_string(sample_rate) + " Hz, minimum 1 Hz");
		return;
	}
	sample_rate_ = sample_rate;
}

int WaveformBuffer::getSampleRate() const
{
	return sample_rate_;
}

//------------------------------------------------------------------------------

void WaveformBuffer::setSamplesPerPixel(int samples_per_pixel)
{
	if (samples_per_pixel < 2) {
		throw std::runtime_error(
		    "WaveformBuffer::setSamplesPerPixel: Invalid samples per pixel: " +
			std::to_string(samples_per_pixel) + ", minimum 2");
		return;
	}
	samples_per_pixel_ = samples_per_pixel;
}

int WaveformBuffer::getSamplesPerPixel() const
{
	return samples_per_pixel_;
}

//------------------------------------------------------------------------------

void WaveformBuffer::setBits(int bits) {
	if ((bits != 8) && (bits != 16)) {
		throw std::runtime_error("setBits: Invalid bits: must be either 8 or 16");
	} else {
		bits_ = bits;
	}
}

int WaveformBuffer::getBits() const
{
	return bits_;
}

//------------------------------------------------------------------------------

void WaveformBuffer::setSize(int32_t size)
{
	for (auto &d : channels_) {
		d.resize(static_cast<size_type>(size * 2));
	}
}

int32_t WaveformBuffer::getSize(int chan) const { 
	if (channels_.size() > static_cast<size_type>(chan)) {
		return static_cast<int32_t>(channels_[chan].size() / 2);
	} else {
		throw std::runtime_error(
		    "WaveformBuffer::getSize: channel " + std::to_string(chan) +
			" is not allocated.");
		return -1;
	}
}

//------------------------------------------------------------------------------

short WaveformBuffer::getMinSample(size_type index, int chan) const
{
	if (channels_.size() > static_cast<size_type>(chan)) {
		return channels_[chan][(2 * index)];
	}
	throw std::runtime_error("WaveformBuffer::getMinSample: Channel " +
	    std::to_string(chan) + " is not allocated.");
}

short WaveformBuffer::getMaxSample(size_type index, int chan) const
{
	if (channels_.size() > static_cast<size_type>(chan)) {
		return channels_[chan][(2 * index) + 1];
	}
	throw std::runtime_error("WaveformBuffer::getMaxSample: Channel " +
	    std::to_string(chan) + " is not allocated.");
}

//------------------------------------------------------------------------------

void WaveformBuffer::appendSamples(short min, short max, int chan)
{
	appendChannels(chan);
	channels_[chan].push_back(min);
	channels_[chan].push_back(max);
}


void WaveformBuffer::setSamples(size_type index, short min, short max, int chan)
{
	appendChannels(chan);
	channels_[chan][(2 * index)] = min;
	channels_[chan][(2 * index) + 1] = max;
}

//------------------------------------------------------------------------------

int WaveformBuffer::getNumChannels() const
{
	return static_cast<int>(channels_.size());
}

//------------------------------------------------------------------------------

bool WaveformBuffer::channelSizesMatch() {
	int32_t size = getSize();
	for (int i = 1; i < getNumChannels(); ++i) {
		if (getSize(i) != size) {
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------

WaveformBuffer::WaveformBuffer(const WaveformBuffer& buffer)
{
	sample_rate_       = buffer.sample_rate_;
	samples_per_pixel_ = buffer.samples_per_pixel_;
	bits_              = buffer.bits_;
	channels_          = buffer.channels_;
}

//------------------------------------------------------------------------------

void WaveformBuffer::appendChannels(int chan) {
	size_type chan_index = static_cast<size_type>(chan);
	if (channels_.size() <= chan_index) {
		while (channels_.size() <= chan_index) {
			channels_.push_back(vector_type());
		}
	}
}
