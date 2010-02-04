/*
 * cmdLineParser.h
 *
 *  Created on: Jan 31, 2010
 *      Author: lars
 */

#ifndef CMDLINEPARSER_H_
#define CMDLINEPARSER_H_

#include "types.h"
#include "common_gravid.h"
#include <string>

namespace GRAVID{

	class CmdLineParser : public Common{
	private:
		int nb_args;
		std::string *args;

		Image_Effect *imgEffects;
		unsigned char nb_img_effects;
		Video_Effect vidEffect;

		bool shallEncode;
		std::string inputFile;
		std::string outputFile;

		void printUsage();
		void parseVideoEffect();
		void parseImageEffects();

		/**
		 * checks the filter string for the occurence of invalid filter options
		 */
		bool containsValidFilters();
	public:
		CmdLineParser(const int argc, char** argv);
		~CmdLineParser();

		/**
		 * determines whether the user wants to print the frames via OpenGL or encode the result to a new video file
		 */
		bool hasOutputFile(){return this->shallEncode;}

		const char* getOutputFile(){return this->outputFile.c_str();}

		/**
		 * determines wether the user specified a video effect
		 */
		bool hasVideoEffect(){return (0 == this->nb_img_effects)?true:false;}

		unsigned char getNBImgEffects(){return this->nb_img_effects;}

		/**
		 * returns an array of all the image effects specified, if any
		 * NULL if no image effects have been specified
		 */
		Image_Effect* getImgEffects(){return this->imgEffects;}

		/**
		 * returns the video effect, if any is specified
		 */
		Video_Effect getVideoEffect(){return this->vidEffect;}

		/**
		 * returns the input video file
		 */
		const char* getInputVideo(){return this->inputFile.c_str();}
	};
}

#endif /* CMDLINEPARSER_H_ */
