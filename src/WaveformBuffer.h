//------------------------------------------------------------------------------
//
// Copyright 2013-2014 BBC Research and Development
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

#if !defined(INC_WAVEFORM_BUFFER_H)
#define INC_WAVEFORM_BUFFER_H

//------------------------------------------------------------------------------

#include <vector>
#include <stdexcept>

//------------------------------------------------------------------------------

class WaveformBuffer
{
    public:
		typedef std::vector<short> vector_type;
        typedef vector_type::size_type size_type;

        WaveformBuffer();

        WaveformBuffer(const WaveformBuffer& buffer) = delete;
        WaveformBuffer& operator=(const WaveformBuffer& buffer) = delete;

    public:
        void setSampleRate(int sample_rate)
        {
            sample_rate_ = sample_rate;
        }

        void setSamplesPerPixel(int samples_per_pixel)
        {
            samples_per_pixel_ = samples_per_pixel;
        }

		int getNumChannels() const { return static_cast<int>(channels_.size()); }
		
        int getSampleRate() const { return sample_rate_; }
        int getSamplesPerPixel() const { return samples_per_pixel_; }

        int getBits() const { return bits_; }

        int getSize(int chan = 0) const { 
			int ret = -1;
			if (channels_.size() > static_cast<size_type>(chan)) {
				ret = static_cast<int>(channels_[chan].size() / 2);
			}
			return ret;
		}

        void setSize(int size)
        {
			for (auto &d : channels_) {
				d.resize(static_cast<size_type>(size * 2));
			}
        }
		
		bool channelSizesMatch() {
			int size = getSize();
			for (int i = 1; i < getNumChannels(); ++i) {
				if (getSize(i) != size) {
					return false;
				}
			}
			return true;
		}

		vector_type& getData(int chan = 0) { return channels_[chan]; }

        short getMinSample(size_type index, int chan = 0) const
        {
			if (channels_.size() > static_cast<size_type>(chan)) {
				return channels_[chan][(2 * index)];
			}
			throw std::runtime_error("getMinSample: Channel does not exist.");
        }

        short getMaxSample(size_type index, int chan = 0) const
        {
			if (channels_.size() > static_cast<size_type>(chan)) {
				return channels_[chan][(2 * index) + 1];
			}
			throw std::runtime_error("getMaxSample: Channel does not exist.");
        }

        void appendSamples(short min, short max, int chan = 0)
        {
			appendChannels(chan);
			channels_[chan].push_back(min);
			channels_[chan].push_back(max);
        }
		

        void setSamples(size_type index, short min, short max, int chan = 0)
        {
			appendChannels(chan);
            channels_[chan][(2 * index)] = min;
            channels_[chan][(2 * index) + 1] = max;
        }

        bool load(const char* filename);
        bool saveAsText(const char* filename, int bits = 16) const;

    private:
        int sample_rate_;
        int samples_per_pixel_;
        int bits_;


		
		std::vector<vector_type> channels_;
		
		void appendChannels(int chan) {
			size_type chan_index = static_cast<size_type>(chan);
			if (channels_.size() <= chan_index) {
				while (channels_.size() <= chan_index) {
					channels_.push_back(vector_type());
				}
			}
		}
};

//------------------------------------------------------------------------------

#endif // #if !defined(INC_WAVEFORM_BUFFER_H)

//------------------------------------------------------------------------------
