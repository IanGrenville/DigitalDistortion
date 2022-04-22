#ifndef DIG_DISTORT_H
#define DIG_DISTORT_H

#include "Generator.h"
#include <stdlib.h>

namespace stk {

class digital_distortion : public Generator
{
public:
	//Getters and setters
	digital_distortion(char* args[7]);
	void set_format(std::string format);
	void set_packet_drop(int drop_amount);
	void set_bitrate(int quality);
	void set_sample(std::string file_name, StkFloat start, StkFloat duration);
	StkFloat get_duration();
	bool source_finished();
	void close_source();
	//The generator interface we implement
	StkFloat lastOut(void) const { return lastFrame_[0];}
	StkFloat tick(void);
	StkFrames& tick(StkFrames& frames, unsigned int channel);
	//The pipe from our FFMPEG calls
	FILE *source;
protected:
	std::string build_command();
	std::string format;
	std::string file_name;
	int drop_amount = 0;
	int bitrate;
	bool sample_done;
	StkFloat start_time = 0;//How many seconds in to start
	StkFloat duration; //How long to play for
	int opus_bitrates[11] = {500,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
	int sbc_bitrates[11] = {50000,60000,70000,80000,90000,100000,110000,120000,130000,140000,150000};
};

inline StkFloat digital_distortion :: tick(void)
{
	char buffer[200];
	//To get our next sample, read a floating point number from the FFMPEG call pipe
	fgets(buffer,200,source);
	if(strlen(buffer) > 12){
		return lastFrame_[0] = (StkFloat) std::stod(buffer);
	}else{
		sample_done = true;
		return 0;
	}
}
inline StkFrames& digital_distortion :: tick(StkFrames& frames, unsigned int channel = 0)
{
	//We don't have multiple channel support but need to implement this function for the inheritance to work properly
	return frames;
}

}

#endif
