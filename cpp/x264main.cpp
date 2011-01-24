#include "x264encoder.h"
#include "y4minput.h"
#include "inputframe.h"
#include "x264picture.h"

int main(int argc, char **argv)
{	
	Y4MInput input;
	if(!input.open("y:\\TestImages\\riverbed.y4m"))
		return -1;

	X264Encoder encoder;

	encoder.parameters().setResolution(input.width(), input.height());
	encoder.parameters().setFrameRate(input.frameRateNumerator(), input.frameRateDenominator());
	encoder.parameters().setVariableFrameRateInput(input.variableFrameRate());
	encoder.parameters().setTimeBase(input.timeBaseNumerator(), input.timeBaseDenominator());
	encoder.parameters().setInterlaced(input.interlaced());
	encoder.parameters().setTopFieldFirst(input.topFieldFirst());
	encoder.parameters().setFrameCount(input.frameCount());

	InputFrame *frame = input.allocateFrame();
	if(!frame)
		return -1;	

	encoder.startEncoding();

	for(uint64_t frameIndex=0; frameIndex<input.frameCount(); frameIndex++)
	{
		if(!input.readFrame(frame, frameIndex))
			return -1;

		frame->setPresentationTimeStamp(frameIndex);

		X264Picture picture;
		picture.setInputFrame(frame);

		X264Picture encodedPicture;
		int nal;
		int encodedCount = encoder.encode(picture, encodedPicture);
	}

	delete frame;

	encoder.finishEncoding();
	return 0;
}
