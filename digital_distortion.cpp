//digital_distortion.cpp by Ian Grenville, Spring 2022
//Implements common digital distortion efffects induced by the SBC and OPUS codecs
//Can be used as an STK generator or invoked on its to play distorted audio out
//Compiled using: g++ -std=c++11 -o distort -Istk-4.6.2/include/ -Lstk-4.6.2/src/ digital_distortion.cpp digital_distortion.h -lstk -lpthread -lasound -D__LINUX_ALSA__


#include "RtWvOut.h"
#include "Generator.h"
#include <stdio.h>
#include "RtWvOut.h"
#include "FileWvOut.h"
#include <string>
#include "digital_distortion.h"

namespace stk{
	//Initialize the generator
	digital_distortion :: digital_distortion(char* args[7]){
		set_sample(args[1],std::stod(args[2]),std::stod(args[3]));
		set_format(args[4]);
		set_packet_drop(std::stoi(args[5]));
		set_bitrate(std::stoi(args[6]));
		this->source = popen(build_command().c_str(), "r");
		this->sample_done = false;
	}
	//Setter for the format to use for encoding/decoding
	void digital_distortion :: set_format(std::string format_name){
		if(format_name == "sbc" || format_name == "opus"){
			this->format = format_name;
		}else{
			this->format = "sbc";
		}
	}
	//Setter for the amount of packets dropped
	void digital_distortion :: set_packet_drop(int amount){
		this->drop_amount = amount;
	}
	//Setter for the bitrate
	void digital_distortion :: set_bitrate(int quality){
		if(format == "sbc"){
			this->bitrate = sbc_bitrates[quality];
		}else if(format == "opus"){
			this->bitrate = opus_bitrates[quality];
		}
	}
	//Setter for the sample
	void digital_distortion :: set_sample(std::string file, StkFloat start, StkFloat play_duration){
		this->file_name = file;
		this->start_time = start;
		this->duration = play_duration;
	}
	//Concatenatively build up our call to ffmpeg
	std::string digital_distortion :: build_command(){
		std::string command = "ffmpeg";
		//Set start time
		if(start_time > 0){
			command += " -ss "+std::to_string(start_time);
		}
		//Set duration
		if(duration > 0){
			command += " -t "+std::to_string(duration);
		}
		//Set the input file
		command += " -i "+ file_name + " ";

		//If we are dropping packets, set up the bitstream filter to do that
		if(drop_amount > 0){
			command += "-bsf noise=dropamount=";
			command += std::to_string(drop_amount);
			command += " ";
		}
		//Set the format and bitrate
		command += "-f " + format + " ";
		command += "-ab " + std::to_string(bitrate);

		//Sets up the decoder
		command += " pipe: 2> /dev/null | ffmpeg ";
		if(format == "opus"){
			command += "-f ogg ";
		}else {
			command += "-f " + format + " ";
		}
		//Take floating point output from the decoder and supply it as lines of floating point numbers
		command += "-i - -f f64le -ac 1 pipe: 2>/dev/null | od -v -t fD -w8 -An";
		return command;
	}
	//Duration getter
	StkFloat digital_distortion :: get_duration(){
		return this->duration;
	}
	//Check whether the sample has finished playing
	bool digital_distortion :: source_finished(){
		return (this->sample_done);
	}
	//Close the sample file
	void digital_distortion :: close_source(){
		pclose(this->source);
		this->sample_done = true;
	}
}

using namespace stk;
//Basic program for demoing the generator. Takes params from command line and distorts the specified audio accordingly
int main(int argc, char* argv[]){
	if(argc != 7){
		std::cout << "Incorrect usage \n Usage: " << argv[0] << " file_name start duration format packet_drop bitrate \n";
		return 1;
	}
	digital_distortion distorter = *(new digital_distortion(argv));
	Stk::setSampleRate(48000.0);
	//FileWvOut output;
	//output.openFile("output.wav",1,FileWrite::FILE_WAV,Stk::STK_SINT16);
	RtWvOut output;
	output.start();

	while(!distorter.source_finished()){
		output.tick(distorter.tick());
	}
	distorter.close_source();
	//output.closeFile();
	return 0;
}
