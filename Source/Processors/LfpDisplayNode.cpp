/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "LfpDisplayNode.h"
#include <stdio.h>

LfpDisplayNode::LfpDisplayNode()
	: GenericProcessor("LFP Viewer"),
	  bufferLength(5.0f), displayGain(1),
	  displayBufferIndex(0)

{

	numInputs = 2;
	numOutputs = 0;
	sampleRate = 44100.0;

	setPlayConfigDetails(2,0,44100.0,128);

	displayBuffer = new AudioSampleBuffer (8, 100);
	eventBuffer = new MidiBuffer();
}

LfpDisplayNode::~LfpDisplayNode()
{

	deleteAndZero(displayBuffer);
	deleteAndZero(eventBuffer);
}

AudioProcessorEditor* LfpDisplayNode::createEditor()
{

	LfpDisplayEditor* editor = new LfpDisplayEditor(this, viewport, getDataViewport());

	//editor->setBuffers (displayBuffer, eventBuffer);
	editor->setUIComponent (getUIComponent());
	editor->setConfiguration (config);
	//editor->updateNumInputs(getNumInputs());
	//editor->updateSampleRate(sampleRate);

	setEditor(editor);
	
	return editor;

}

void LfpDisplayNode::setNumInputs(int inputs)
{
	std::cout << "Setting num inputs on LfpDisplayNode to " << inputs << std::endl;
	numInputs = inputs;	
	setNumOutputs(0);

	setPlayConfigDetails(getNumInputs(),0,44100.0,128);

	LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
	editor->updateNumInputs(inputs);
}

void LfpDisplayNode::setSampleRate(float r)
{
	sampleRate = r;

	LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
	editor->updateSampleRate(r);
}

bool LfpDisplayNode::resizeBuffer()
{
	int nSamples = (int) sampleRate*bufferLength;
	int nInputs = getNumInputs();

	std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

	if (nSamples > 0 && nInputs > 0)
	{
		
		displayBuffer->setSize(nInputs, nSamples);
		return true;
	} else {
		return false;
	}

}

bool LfpDisplayNode::enable()
{

	if (resizeBuffer())
	{
		LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
		editor->enable();
		return true;
	} else {
		return false;
	}

}

bool LfpDisplayNode::disable()
{
	LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
	editor->disable();
	return true;
}

void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{

}

void LfpDisplayNode::process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples)
{
	// 1. place any new samples into the displayBuffer
	int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex;
	
	//lock->enterWrite();

	if (nSamples < samplesLeft)
	{

		for (int chan = 0; chan < buffer.getNumChannels(); chan++)
		{	
			displayBuffer->copyFrom(chan,  				// destChannel
							    displayBufferIndex, // destStartSample
							    buffer, 			// source
							    chan, 				// source channel
							    0,					// source start sample
							    nSamples); 			// numSamples

		}
		displayBufferIndex += (nSamples);

	} else {

		int extraSamples = nSamples - samplesLeft;

		for (int chan = 0; chan < buffer.getNumChannels(); chan++)
		{	
			displayBuffer->copyFrom(chan,  				// destChannel
							    displayBufferIndex, // destStartSample
							    buffer, 			// source
							    chan, 				// source channel
							    0,					// source start sample
							    samplesLeft); 		// numSamples

			displayBuffer->copyFrom(chan,
								0,
								buffer,
								chan,
								samplesLeft,
								extraSamples);
		}

		displayBufferIndex = extraSamples+1;
	}

	//lock->exitWrite();

	//std::cout << displayBufferIndex << std::endl;

	// 2. update the screenBuffer
	// if (true)
	// {
	// 	float nPixels = (float) getWidth();
	// 	float ratio = sampleRate * timebase / 1000.0f / nPixels;
	// 	float subSampleOffset = 0.0f;

	// 	int valuesNeeded = nSamples / (int) ratio;

	//     for (int index = screenBufferIndex; index < valuesNeeded; index++)
	//     {

	//         for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {

	//         	screenBuffer->copyFrom(channel, 		// destChannel
	//         						index,  			// destSampleOffset
	//         						buffer,				// source
	//         						channel,			// sourceChannel
	//         						(int) subSampleOffset,// sourceSampleOffset
	//         						1);					// number of samples
	        	
	//         	subSampleOffset += ratio;
	//         	subSampleOffset = jmin(subSampleOffset, (float) buffer.getNumSamples());
	//        	}

	// 	}
	// }

	// repaintCounter++;
	// // 3. if it's time, repaint the display
	// if (false && repaintCounter >= repaintInterval)
	// {
	// 	repaint();

	// }
}


// void LfpDisplayNode::newOpenGLContextCreated()
// {

// 	setUp2DCanvas();
// 	activateAntiAliasing();

// 	glClearColor (0.8, 0.4, 0.9, 1.0);
// 	resized();

// }

// void LfpDisplayNode::renderOpenGL()
// {

// 	repaintCounter = 0;
	
// 	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

// 	for (int i = 0; i < getNumInputs(); i++)
// 	{
// 		bool isSelected = false;

// 		if (selectedChan == i)
// 			isSelected = true;

// 		if (checkBounds(i)) {
// 			setViewport(i);
// 			drawBorder(isSelected);
// 			drawChannelInfo(i,isSelected);
// 			//drawWaveform(i,isSelected);
// 		}	
// 	}
// 	drawScrollBars();
// }

// void LfpDisplayNode::drawWaveform(int chan, bool isSelected)
// {
// 	// draw the screen buffer for a given channel

// 	glBegin(GL_LINE_STRIP);

// 	for (int i = 0; i < getWidth(); i++)
// 	{
// 		glVertex2f(i,0.5);//*screenBuffer->getSampleData(chan, i)+0.5);
// 	}

// 	glEnd();
// }


// void LfpDisplayNode::drawTicks()
// {
	
// 	glViewport(0,0,getWidth(),getHeight());

// 	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

// 	for (int i = 0; i < 10; i++)
// 	{
// 		if (i == 5)
// 			glLineWidth(3.0);
// 		else if (i == 1 || i == 3 || i == 7 || i == 9)
// 			glLineWidth(2.0);
// 		else
// 			glLineWidth(1.0);

// 		glBegin(GL_LINE_STRIP);
// 		glVertex2f(0.1*i,0);
// 		glVertex2f(0.1*i,1);
// 		glEnd();
// 	}
// }


// bool LfpDisplayNode::checkBounds(int chan)
// {
// 	bool isVisible;

// 	int lowerBound = (chan+1)*(plotHeight+yBuffer);
// 	int upperBound = chan*(plotHeight+yBuffer);

// 	if (getScrollAmount() < lowerBound && getScrollAmount() + getHeight() > upperBound)
// 		isVisible = true;
// 	else
// 		isVisible = false;
	
// 	return isVisible;

// }

// void LfpDisplayNode::setViewport(int chan)
// {
// 	glViewport(xBuffer,
// 			   getHeight()-(chan+1)*(plotHeight+yBuffer)+getScrollAmount(),
// 	           getWidth()-2*xBuffer,
// 	           plotHeight);
// }

// void LfpDisplayNode::drawBorder(bool isSelected)
// {
// 	float alpha = 0.5f;

// 	if (isSelected)
// 		alpha = 1.0f;

// 	glColor4f(0.0f, 0.0f, 0.0f, alpha);
// 	glBegin(GL_LINE_STRIP);
//  	glVertex2f(0.0f, 0.0f);
//  	glVertex2f(1.0f, 0.0f);
//  	glVertex2f(1.0f, 1.0f);
//  	glVertex2f(0.0f, 1.0f);
//  	glVertex2f(0.0f, 0.0f);
//  	glEnd();

// }

// void LfpDisplayNode::drawChannelInfo(int chan, bool isSelected)
// {
// 	float alpha = 0.5f;

// 	if (isSelected)
// 		alpha = 1.0f;

// 	glColor4f(0.0f,0.0f,0.0f,alpha);
// 	glRasterPos2f(5.0f/getWidth(),0.3);
// 	String s = String("Channel ");
// 	s += (chan+1);

// 	getFont(String("miso-regular"))->FaceSize(16);
// 	getFont(String("miso-regular"))->Render(s);
// }

// int LfpDisplayNode::getTotalHeight() 
// {
// 	return (plotHeight+yBuffer)*getNumInputs() + yBuffer;
// }


// void LfpDisplayNode::resized()

// {

// 	canvasWasResized();
// 	// glClear(GL_COLOR_BUFFER_BIT);

// 	// int h, w;

// 	// if (inWindow)
// 	// {
// 	// 	h = getParentComponent()->getHeight();
// 	// 	w = getParentComponent()->getWidth();
// 	// } else {
// 	// 	h = getHeight();
// 	// 	w = getWidth();
// 	// }

// 	// if (getScrollAmount() + h > getTotalHeight() && getTotalHeight() > h)
// 	// 	setScrollAmount(getTotalHeight() - h);
// 	// else
// 	// 	setScrollAmount(0);

// 	// showScrollBars();

// }
