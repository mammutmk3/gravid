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
#define CMD_LENGTH_MIN 3
#define CMD_LENGTH_MAX 4

#define CMD_USAGE "gravid <{image_filter} | video_filter | fade_effect <input_video>> input_video < --display | output_video>"
#define CMD_IMAGE "g : gray filter\ns : sepia filter\ne : edge detection\nb : gaussian blur"
#define CMD_VIDEO "o : ghost\nq : blur\nc : camera stabilisation"
#define CMD_FADE "a : additive fade\nl : blinds\np : circle fade\nt : teeth\n"
#define CMD_EXAMPLE "./gravid gb video1.mpg video2.mpeg"

#define VALID_FLTR_FLAGS "gseboqcalpt"

using namespace GRAVID;

CmdLineParser::CmdLineParser(const int argc, char** argv){
	// initialize members
	this->nb_args = argc-1;
	this->args = NULL;
	this->imgEffects = NULL;
	
	this->nb_img_effects = 0;
	this->nb_fade_effects = 0;
	this->nb_vid_effects = 0;
	
	// check if the correct number of arguments where passed
	if( !((CMD_LENGTH_MIN <= this->nb_args) && (CMD_LENGTH_MAX >= this->nb_args)) ){
		this->printUsage();
		this->errorHappened("false number of arguments");
	}

	// allocate a string array for the single cmdline arguments excluding the current workdir
	this->args = new std::string[this->nb_args];
	for(int i=0;i<this->nb_args;i++){
		// fill the strings with the c-chars
		this->args[i] = argv[i+1];
	}

	// allocate an array for the image effects;
	this->imgEffects = new Image_Effect[this->args[0].size()];

	// check if only valid filter options are beeing used
	if(!this->containsValidFilters()){
		this->printUsage();
		this->errorHappened("invalid filter option");
	}

	// extract the wanted image effects, if any
	this->parseImageEffects();

	// get the video or the fade effect only, if no image effect was specified
	// set the paramter-position, where the output-file is expected
	int param_outfile = 2;
	if(0 == this->nb_img_effects){
		if(1 == this->args[0].size()) {
			if ( !this->parseVideoEffect() ) {
				if ( this->parseFadeEffect() ) {
					this->inputFile2 = this->args[2];
					param_outfile = 3;
				}
			}
		}
		else{
			this->printUsage();
			this->errorHappened("multiple video/fade effects or combined [video/fade]-image effects");
		}
	}

	// set the input file
	this->inputFile = this->args[1];

	// set the outputfile, if desired
	if(0 == this->args[param_outfile].compare("--display")){
		this->shallEncode = false;
	}
	else{
		this->shallEncode = true;
		this->outputFile = this->args[param_outfile];
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
	std::cout << "--Fade Effects--" << std::endl << CMD_FADE << std::endl << std::endl;
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

bool CmdLineParser::parseVideoEffect(){
	this->nb_vid_effects = 1;
	switch(this->args[0][0]){
	case 'o' : this->vidEffect = GHOST; return true;
	case 'q' : this->vidEffect = ECHO_BLUR; return true;
	case 'c' : this->vidEffect = CAM_STAB; return true;
	}
	this->nb_vid_effects = 0;
	return false;
}

bool CmdLineParser::parseFadeEffect(){
	this->nb_fade_effects = 1;
	switch(this->args[0][0]){
	case 'a' : this->fadeEffect = ADDITIVE; return true;
	case 'l' : this->fadeEffect = BLINDS; return true;
	case 'p' : this->fadeEffect = CIRCLE; return true;
	case 't' : this->fadeEffect = TEETH; return true;
	}
	this->nb_fade_effects = 0;
	return false;
}
