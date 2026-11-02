#include "PluginProcessor.h"

using APF = juce::AudioParameterFloat;
using NR = juce::NormalisableRange<float>;

juce::AudioProcessorValueTreeState::ParameterLayout CreativeEchoProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<APF> ("time",    "Echo Time (ms)", NR (20.f, 1000.f, 1.f), 180.f));
    l.add (std::make_unique<APF> ("fb",      "Feedback",       NR (0.f, 0.9f, 0.01f),  0.30f));
    l.add (std::make_unique<APF> ("echoMix", "Echo Mix",       NR (0.f, 1.f, 0.01f),   0.25f));
    l.add (std::make_unique<APF> ("tone",    "Echo Tone (Hz)", NR (500.f, 16000.f, 1.f, 0.4f), 6000.f));
    l.add (std::make_unique<APF> ("size",    "Reverb Size",    NR (0.f, 1.f, 0.01f),   0.55f));
    l.add (std::make_unique<APF> ("damp",    "Reverb Damping", NR (0.f, 1.f, 0.01f),   0.60f));
    l.add (std::make_unique<APF> ("revMix",  "Reverb Mix",     NR (0.f, 1.f, 0.01f),   0.20f));
    return l;
}

CreativeEchoProcessor::CreativeEchoProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout()) {}

bool CreativeEchoProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo()) && o == l.getMainInputChannelSet();
}

void CreativeEchoProcessor::prepareToPlay (double rate, int)
{
    sr = rate;
    for (auto& b : buf) b.assign ((size_t) (rate * 2.0) + 4, 0.f);
    writePos = 0;
    lpState[0] = lpState[1] = 0.f;
    reverb.setSampleRate (rate);
    reverb.reset();
}

void CreativeEchoProcessor::processBlock (juce::AudioBuffer<float>& audio, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int n = audio.getNumSamples();
    const int chans = juce::jmin (audio.getNumChannels(), 2);

    const float timeMs = *apvts.getRawParameterValue ("time");
    const float fb     = *apvts.getRawParameterValue ("fb");
    const float eMix   = *apvts.getRawParameterValue ("echoMix");
    const float tone   = *apvts.getRawParameterValue ("tone");

    const int size = (int) buf[0].size();
    const float delaySamples = juce::jlimit (1.f, (float) size - 2.f, timeMs * 0.001f * (float) sr);
    const float a = 1.f - std::exp (-juce::MathConstants<float>::twoPi * tone / (float) sr);

    int wp = writePos;
    for (int c = 0; c < chans; ++c)
    {
        auto* d = audio.getWritePointer (c);
        auto& b = buf[c];
        float z = lpState[c];
        wp = writePos;
        for (int i = 0; i < n; ++i)
        {
            float rp = (float) wp - delaySamples;
            if (rp < 0.f) rp += (float) size;
            int i0 = (int) rp, i1 = (i0 + 1) % size;
            float fr = rp - (float) i0;
            float delayed = b[(size_t) i0] * (1.f - fr) + b[(size_t) i1] * fr;
            z += a * (delayed - z);
            const float in = d[i];
            b[(size_t) wp] = in + z * fb;
            d[i] = in + z * eMix;
            if (++wp >= size) wp = 0;
        }
        lpState[c] = z;
    }
    writePos = wp;

    juce::Reverb::Parameters p;
    p.roomSize = *apvts.getRawParameterValue ("size");
    p.damping  = *apvts.getRawParameterValue ("damp");
    p.wetLevel = *apvts.getRawParameterValue ("revMix");
    p.dryLevel = 1.f;
    p.width    = 1.f;
    reverb.setParameters (p);

    if (chans == 2) reverb.processStereo (audio.getWritePointer (0), audio.getWritePointer (1), n);
    else if (chans == 1) reverb.processMono (audio.getWritePointer (0), n);
}

void CreativeEchoProcessor::getStateInformation (juce::MemoryBlock& d)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, d);
}

void CreativeEchoProcessor::setStateInformation (const void* d, int size)
{
    if (auto xml = getXmlFromBinary (d, size))
        if (xml->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CreativeEchoProcessor(); }
