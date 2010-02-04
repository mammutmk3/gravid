/*
 * cmdLineParser.cpp
 *
 *  Created on: Jan 31, 2010
 *      Author: lars
 */

#include "cmdLineParser.h"

#include <iostream>
#include <string>

// defines the current number of valid arguments, excluding program path
#define CMD_LENGTH 3

#define CMD_USAGE "gravid <{image_filter} | video_filter> input_video < --display | output_video>"
#define CMD_IMAGE "g : gray filter\ns : sepia filter\ne : edge detection\nb : gaussian blur"
#define CMD_VIDEO "o : overlay\nc : camera stabilisation"
#define CMD_EXAMPLE "./gravid gb video1.mpg video2.mpeg"

#define VALID_FLTR_FLAGS "gseboc"

using namespace GRAVID;

CmdLineParser::CmdLineParser(const int argc, char** argv){
	// initialize members
	this->nb_args = argc-1;
	this->args = NULL;
	this->imgEffects = NULL;

	// check if the correct number of arguments where passed
	if(CMD_LENGTH != this->nb_args){
		this->printUsage();
		this->errorHappened("false number of arguments");
	}

	// allocate a string array for the single cmdline arguments excluding the current workdir
	this->args = new std::string[this->nb_args];
	for(int i=0;i<this->nb_args;i++){
		// fill the strings with the c-chars
		this->args[i] = argv[i+1];
	}

	// allocate and array for the image effects;
	this->imgEffects = new Image_Effect[this->args[0].size()];

	// check if only valid filter options are beeing used
	if(!this->containsValidFilters()){
		this->printUsage();
		this->errorHappened("invalid filter option");
	}

	// extract the wanted image effects, if any
	this->parseImageEffects();

	// get the video effect only, if no video effect was specified
	if(0 == this->nb_img_effects){
		if(1 == this->args[0].size())
			this->parseVideoEffect();
		else{
			this->printUsage();
			this->errorHappened("multiple video effects or combined video and image effects");
		}
	}

	// set the input file
	this->inputFile = this->args[1];

	// set the outputfile, if desired
	if(0 == this->args[2].compare("--display")){
		this->shallEncode = false;
	}
	else{
		this->shallEncode = true;
		this->outputFile = this->args[2];
	}

}

CmdLineParser::~CmdLineParser(){
	if(NULL != this->args)
		delete[] this->args;

	if(NULL != this->imgEffects)
		delete this->imgEffects;
}

void CmdLineParser::printUsage(){
	std::cout << "------Usage------" << std::endl << CMD_USAGE << std::endl << std::endl;
	std::cout << "--Image Filters--" << std::endl << CMD_IMAGE << std::endl << std::endl;
	std::cout << "--Video Filters--" << std::endl << CMD_VIDEO << std::endl << std::endl;
	std::cout << "----Examples-----" << std::endl << CMD_EXAMPLE << std::endl << std::endl;
}

bool CmdLineParser::containsValidFilters(){
	std::string validFlags = VALID_FLTR_FLAGS;
	// args[0] contains the filter flags
	for(unsigned int i=0;i<this->args[0].size();i++){
		// if a character was passed, that is not valid
		if(std::string::npos == validFlags.find(this->args[0][i]))
			return false;
	}
	return true;
}

void CmdLineParser::parseImageEffects(){
	// args[0] contains the filter flags
	for(unsigned int i=0;i<this->args[0].size();i++){
		switch(this->args[0][i]){
		case 'g': this->imgEffects[i] = GRAY_FILTER; break;
		case 's': this->imgEffects[i] = SEPIA_FILTER; break;
		case 'e': this->imgEffects[i] = EDGE_DETECTION; break;
		case 'b': this->imgEffects[i] = GAUSS_BLUR; break;
		default: {this->nb_img_effects = 0; return;}
		}
	}
	this->nb_img_effects = this->args[0].size();
}

void CmdLineParser::parseVideoEffect(){
	switch(this->args[0][0]){
	case 'o' : this->vidEffect = IMG_OVRLAY; break;
	case 'c' : this->vidEffect = CAM_STAB; break;
	}
}
