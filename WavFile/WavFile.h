#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "WavHeader.h"

namespace WavFile
{
	//Samples are represented as an array a[x][y], where is x is channel number, y - samples of channel x as doubles in range [-1...1].
	class WavFileShared
	{
	public:
		std::vector<std::vector<double>> getSamples()
		{
			return samples;
		}
		std::vector<double> getSamplesOfChannel(int channelNum)
		{
			return samples[channelNum];
		}

		WavHeader getHeader()
		{
			return header;
		}

		WavHeader setHeader(const WavHeader& wh)
		{
			header = wh;
		}

		size_t getSampleIndexAtTime(double t)
		{
			return t * header.getNumChannels() * header.getSampleRate();
		}

		double getSampleAtIndex(size_t i, size_t channelNumber = 0)
		{
			return samples[channelNumber][i];
		}

		void setSampleAtIndex(size_t i, double value, size_t channelNumber = 0)
		{
			samples[channelNumber][i] = value;
		}

		double getSampleAt(double t, size_t channelNumber = 0)
		{
			auto i = getSampleIndexAtTime(t);
			return getSampleAtIndex(i, channelNumber);
		}
		
		void setSampleAt(double t, double value, size_t channelNumber = 0)
		{
			auto i = getSampleIndexAtTime(t);
			samples[channelNumber][t] = value;
		}

		size_t getSampleCount()
		{
			return std::max_element(samples.begin(), samples.end(), [&](const auto& v)
				{
					return v.size();
				})->size();
		}

		/*//Samples are represented as doubles in range [-1...1]. Use at your own risk, as class internals do not expect external sample changes
		std::vector<std::vector<double>>& getSamplesReference();*/

		//Sets sample rate of a file. Setting convertSamples to false will skip sample conversion, which will change the speed and duration of the file.
		/*void setSampleRate(uint32_t newSampleRate, bool convertSamples = true)
		{
			uint32_t oldSampleRate = header.getSampleRate();
			double oldSampleTime = 1.0 / oldSampleRate;
			header.sampleRate = newSampleRate;

			if (convertSamples)
			{
				uint32_t newSampleCount = header.getDuration()*newSampleRate;
				double newSampleTime = 1.0 / newSampleRate;
				double currTime = 0;
				for (auto& oldChan : samples)
				{
					std::vector<double> newChan(newSampleCount);
					for (uint32_t i = 0; i < newSampleCount; ++i) //interpolate samples
					{
						uint32_t oldInd = currTime * oldSampleRate;
						newChan[i] = oldChan[oldInd];
						currTime += newSampleTime;
					}
					oldChan = newChan;
				}
			}
		}*/

		//Sets new number of channels. Newly created channels will be initialized with zeros if init is 1, special scheme below if 2, and left unitialized if 0. If init == 2, then:
		//If newNumChannels < oldNumChannels, then channels with indices[newNumChannels-1...oldNumChannels-1] will be removed;
		//If newNumChannels > oldNumChannels, new channels will be copied from old ones. Channel n will be a copy of channel (n % oldNumChannels).
		//If newNumChannels == oldNumChannels, no changes are made
		/*void setNumChannels(uint16_t newNumChannels, int init = 1)
		{
			uint16_t oldNumChannels = header.numChannels;
			header.numChannels = newNumChannels;

			this->samples.resize(newNumChannels);
			if (newNumChannels > oldNumChannels)
			{
				if (init == 1)
				{
					for (size_t i = oldNumChannels-1; i < newNumChannels; ++i)
					{
						samples[i] = std::vector<double>()
					}
				}
			}
		}*/
	protected:
		WavHeader header;
		std::vector<std::vector<double>> samples;
	};
}