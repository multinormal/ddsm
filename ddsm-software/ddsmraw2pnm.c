/*
  The following is very brief; full program documentation can be found
  by compiling this program as described and running it witout
  arguments, or by reading the displayProgramHelp() function.

  This program converts from DDSM raw (as output by calling their jpeg
  program with "jpeg -d -s <some-ddsm-file>") to "plain" PNM image
  format (see http://netpbm.sourceforge.net/doc/pgm.html as of 15 Dec
  2005); from this format you should be able to convert the image to
  an actual standard image file format (e.g. by using the ImageMagick
  'convert' program: convert -depth 16 infile.pnm outfile.png)!

  Compilation: Compile this file using gcc: "g++ -Wall -O2 ddsmraw2pnm.c -o ddsmraw2pnm"
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

// These are program error codes and messages.
const int success = 0; // The code to return on program success.
const int syntax_error = -1; // Exit code if the user invokes program incorrectly.
const char* syntax_error_msg = ""; // We'll output a message manually if the program is not called correctly.
const int rows_not_positive_error = -2; // The number of rows must be positive.
const char* rows_not_positive_error_msg = "The number of rows must be positive.";
const int cols_not_positive_error = -3; // The number of cols must be positive.
const char* cols_not_positive_error_msg = "The number of cols must be positive.";
const int file_error = -4;
const char* file_error_msg = "A file error was detected at runtime.";
const int pnm_error = -5;
const char* pnm_error_msg = "Could not create the PNM file.";
const int program_error = -6;
const char* program_error_msg = "Sorry, there is a problem with the program's source code!";
const int image_size_error = -7; // Used to indicate the specified number of rows and cols seems to be wrong given size of the input file.

// This is the suffix applied to the input filename to create the outfile filename.
const std::string outputSuffix = "-ddsmraw2pnm.pnm";

// Define the three digitizer names.
const std::string dba = "dba";
const std::string howtek_mgh = "howtek-mgh";
const std::string howtek_ismd = "howtek-ismd";
const std::string lumisys = "lumisys";

// Define the maximum Optical Density value that will map to an output
// grey level value of 65535.
const double maxOD = 4.0;

// Define the number of bits used to represent the raw data and the
// output data. The define the maximum unsigned integer that can be
// represented using that number of bits.
const unsigned int numBits = 16;
const unsigned int maxUnsignedIntWithNumBits = 65535;


// Display program help information.
void displayProgramHelp()
{
  // We'll define the help message as an array of strings that we'll
  // iterate over. The end of the array is marked with a special
  // string called endString.
  const std::string endString = "<<<END"; // Marks the end of the message.

  // Here's the message array.
  std::string
    helpMessage[] =
    {
      "ddsmraw2pnm",
      "===========\n",

      "Convert a DDSM mammogram image from raw (LJPEG.1) format to PNM format.\n",

      "This program converts from the DDSM \"raw\" format (i.e. raw byte pairs)",
      "to the simple PNM image file format. Standard tools (such as ImageMagick's",
      "\"convert\" program) can then be used to convert to more convenient image",
      "formats (e.g. the PNG format, which is both standardised and lossless).",
      "In particular, note that the grey-level values produced by ddsmraw2pnm are",
      "normalised such that grey-levels are directly comparable for the four",
      "digitizers that were used (i.e. a particular grey level maps to a unique",
      "optical density for all images produced by ddsmraw2pnm; see below for more",
      "details).\n",

      "Usage: ddsmraw2pnm <some-ddsm-raw-file> <num-rows> <num-cols> <digitizer>\n",

      "* <some-ddsm-raw-file> is an \"LJPEG.1\" file produced by the DDSM's \"jpeg\"",
      "  program. On x86 Linux for example, call \"jpeg -d -s A_0069_1.LEFT_CC.LJPEG\",",
      "  where A_0069_1.LEFT_CC.LJPEG is a DDSM mammogram file. (Be careful to check",
      "  the endianness of your computer---this code was tested on Linux running on",
      "  an x86 processor.)\n",

      "* <num-rows> and <num-cols> specify the dimensions of the image; these can",
      "  be obtained from the \".ics\" file for the case.\n",

      "* <digitizer> is one of \"dba\", \"howtek-mgh\", \"howtek-ismd\" and",
      "  \"lumisys\" and is used to select a normalisation function which maps",
      "  the raw grey level values in the \"LJPEG.1\" file to optical densities.\n",

      "On success, the ddsmraw2pnm program will produce a PNM file with the name",
      "\"<some-ddsm-raw-file>-ddsmraw2pnm.pnm\" (overwiting the file if it already",
      "exists), writing the name of the output file to standard output and returning",
      "zero to the caller to indicate success. On failure a (hopefully) useful message",
      "will be printed to standard error and the program will return a non-zero value",
      "to the caller to indicate failure (see the code for the meanings of the error",
      "codes). The output PNM file may be partially written even on failure, so",
      "programs that call ddsmraw2pnm do need to check the program's exit code.\n",

      "The data in the PNM file will be calibrated and normalised according",
      "to the digitizer that was used to digitize the mammogram in",
      "question. The calibration functions were obtained from the DDSM",
      "website. We first convert from raw grey level value to optical",
      "density (the calibration step) and then convert from optical density",
      "to \"normalised grey level\"; normalised grey level uses all 16 bits",
      "such that an optical density of 0 has normalised grey level of 0 and",
      "the maximum optical density we expect (see code for specifics)",
      "corresponds to 65535 (i.e. the maximum value that can be represented by",
      "16 bits). The maximum value we expect to read is specified to be slightly",
      "larger than that specified on the DDSM website, as such larger values do",
      "exist in the data. A \"companding\" function is then applied to reduce the",
      "resolution with which small grey levels are represented and increase",
      "the resolution with which medium and large grey levels are",
      "represented (because we are typically much more interested in the",
      "fatty, glandular and calcium grey level values than we are in the",
      "air region of the mammogram). We use a quadratic companding function.",
      "The result of this calibration and normalisation is that the grey levels",
      "output by this program should be directly comparable for all four digitizers.\n",

      "You should note that PNM files are quite human-readable, use no compression and",
      "are therefore very large (e.g. 85MB)! You should therefore convert from PNM format",
      "to a losslessly compressed format and delete the intermediate PNM file to",
      "avoid wasting disk space. 16-bit PNG files are ideal as they are lossless,",
      "a standard exists and most reasonable software can read them.",


      endString
    };

  // Now print the message line by line.
  unsigned int i = 0; // An iterator.
  std::string thisLine = ""; // We'll change this.
  while((thisLine = helpMessage[i]) != endString)
    {
      std::cout << thisLine << std::endl; // Print the line.
      i++; // Increment iterator.
    }
}


// Function that exits with an error code and a particular error
// message.
void exitWith(const int errorCode, const char* errorMsg)
{
  std::cerr << errorMsg << std::endl;
  exit(errorCode);
}

// Check that the input values does not lie outside of the range 0 to
// 65535. Return true if the input value is in the range, otherwise
// return false.
inline bool checkRange(unsigned int i)
{
  const bool retVal = (i <= maxUnsignedIntWithNumBits);
  if(!retVal)
    {
      std::cout << "Data outside range. Data is: " << i << std::endl;
    }

  return retVal;
}

// Convert an optical density value to our normalised grey level
// quantity. retVal must point to a memory location that we can write
// to (i.e, it will return the result) and od is the optical density
// value we operate on. This functio will return true if everything is
// OK, otherwise it will return false.
inline bool od2NormGreyLevel(unsigned int* retVal, const double od)
{
  *retVal = static_cast<unsigned int>((static_cast<double>(maxUnsignedIntWithNumBits) / maxOD) * od);
  if(*retVal > maxUnsignedIntWithNumBits)
    {
      // There's a problem.
      std::cout << "Optical density value was out of range; value was " << od << std::endl;
      return false;
    }

  // The dat from the digitizer is inverted, so uninvert.
  *retVal = maxUnsignedIntWithNumBits - *retVal;

  // Now perform quadratic companding, so we give more binary
  // precision to the high grey levels. The quadratic maps zero to
  // zero and 65535 to 65535 and is quadratic in between.
  *retVal = static_cast<unsigned int>((1.0/static_cast<double>(maxUnsignedIntWithNumBits)) * (static_cast<double>(*retVal) * static_cast<double>(*retVal)));

  // Force things to be in range.
  /* 
     if(*retVal > maxUnsignedIntWithNumBits)
     {
     *retVal = maxUnsignedIntWithNumBits;
     }
  */

  // Everything's OK.
  return true;
}

// The calibration function for the dba digitizer. retVal must point
// to a memory location we can write an unsigned int to; raw is the
// input argument. We return true if everything was OK, false
// otherwise.
inline bool dbaCalibration(unsigned int* retVal, unsigned int raw)
{
  double rawDouble = 0.0;

  if(0 != raw)
    {
      // Need to correct for input that is over 64064, as the equation
      // below will give -ve results for such data!
      if(raw > 64064)
	{
	  raw = 64064;
	}

      // Need to correct for input that is less than 4, as this will
      // generate optical density values that are greater than 4.0
      // (which is currently set as the value of maxOD).
      if(raw < 4)
	{
	  raw = 4;
	}

      rawDouble = static_cast<double>(raw);
      rawDouble = (log10(rawDouble) - 4.80662) / (-1.07553);
      // Above eqn from: http://marathon.csee.usf.edu/Mammography/DDSM/calibrate/DBA_Scanner_info.html
    }

  // Now convert to our normalised grey level.
  return od2NormGreyLevel(retVal, rawDouble); // Returns true if OK, false otherwise.
}

// The calibration function for the howtek-ismd digitizer. retVal must
// point to a memory location we can write an unsigned int to; raw is
// the input argument. We return true if everything was OK, false
// otherwise.
inline bool howtekMghCalibration(unsigned int* retVal, unsigned int raw)
{
  // Need to correct for input values that are over 4006, as the eqn
  // below gives -ve results for such data.
  if(raw > 4006)
    {
      raw = 4006; 
    }

  // Convert from raw to optical density.
  double od = 3.789 + ((-0.00094568) * static_cast<double>(raw));

  // Now convert to our normalised grey level.
  return od2NormGreyLevel(retVal, od); // Returns true if OK, false otherwise.
}

// The calibration function for the howtek-ismd digitizer. retVal must
// point to a memory location we can write an unsigned int to; raw is
// the input argument. We return true if everything was OK, false
// otherwise.
inline bool howtekIsmdCalibration(unsigned int* retVal, unsigned int raw)
{
  // Need to correct for input values that are over 4003, as the eqn
  // below gives -ve results for such data.
  if(raw > 4003)
    {
      raw = 4003; 
    }  

  // Convert from raw to optical density.
  double od = 3.96604096240593 + ((-0.00099055807612) * static_cast<double>(raw));

  // Now convert to our normalised grey level.
  return od2NormGreyLevel(retVal, od); // Returns true if OK, false otherwise.
}

// The calibration function for the lumisys digitizer.
inline bool lumisysCalibration(unsigned int* retVal, unsigned int raw)
{
  // Need to correct for input values that are less than 58, as the
  // eqn below gives results over 4.0 (our choice for maxOD, but check
  // this!) for such data.
  if(raw < 61)
    {
      raw = 61; 
    }  

  // Need to correct for input values that are over 4097, as the eqn
  // below gives -ve results for such data.
  if(raw > 4097)
    {
      raw = 4097; 
    }

  // Convert from raw to optical density.
  double od = (static_cast<double>(raw) - 4096.99) / (-1009.01);

  // Now convert to our normalised grey level.
  return od2NormGreyLevel(retVal, od); // Returns true if OK, false otherwise.
}

// Return a comment string that will be embedded in the PNM file
// (ImageMagick's convert utility maintains the comment). We pass in a
// function pointer which lets us work out which digitizer was used
// and therefore how many bits/pixel the scanner operated at.
std::string getPnmCommentString(bool (*calibrationFunc)(unsigned int* retVal, unsigned int raw))
{
  // Define the number of bits per pixel used by the digitizers.
  unsigned char bitsPerPixel = 12;
  if(calibrationFunc == dbaCalibration)
    {
      // Only the DBA scanner digitized at 16 bits per pixel.
      bitsPerPixel = 16;
    }

  std::string retVal = "# Generated by ddsmraw2pnm. Original data was digitized at ";
  retVal += bitsPerPixel;
  retVal += " bits/pixel.\n";
  return retVal;
}

// Make the PNM file. Return a non-zero return value if things didn't
// go well. The last argument is a pointer to one of the calibration
// functions; we apply calibration by calling that function after
// reading each pixel value from the file. We also use the calibration
// function pointer to write a comment to the PNM file which specifies
// how many bits/pixel the original digitizer operated at (though we
// normalise the data we output so it is comparable across all
// digitizers).
int makePnmFile(FILE* input,
		FILE* output,
		const int numRows,
		const int numCols,
                bool (*calibrationFunc)(unsigned int* retVal, unsigned int raw))
{
  fprintf(output, "P2\n");
  fprintf(output, (getPnmCommentString(calibrationFunc)).c_str());
  fprintf(output, "%u\n", numCols);
  fprintf(output, "%u\n", numRows);
  fprintf(output, "%u\n", maxUnsignedIntWithNumBits);  // Here we assume 16-bit data.
  
  // A byte-sized buffer, a record of whether we are reading an odd or
  // even-numbered byte and the pixel value of the current pixel.
  bool oddByte = true; // We start on byte 1, an odd byte.
  unsigned int thisPixel = 0; // This will be used to hold the value of each pixel.

  // Count how many values we read, so we can verify that the file
  // will at least have the correct header for the volume of data it
  // will carry.
  unsigned int numPixels = 0;

  // The PNM specification says that the file should have no more than
  // 70 characters per line. The following counter, along with the
  // assumption that each pixel value will have no more than 5
  // characters, allows us to insert a newline character at
  // appropriate points.
  int charColCounter = 0;
  const int maxCharsPerPixel = 5;
  const int breakAroundCol = 50; // Put newlines at about column number 50.

  // For debugging.
  unsigned int minPixelValue = maxUnsignedIntWithNumBits;
  unsigned int maxPixelValue = 0;

  // Keeps track if the calibration function thinks things are OK.
  bool okSoFar = true;

  // Read the data in and write the rest of the PNM file.
  unsigned char thisByte[1];

  unsigned int byteValue = 0; // We'll read the value of the byte into here.
  while(!feof(input))
    {
      fread(thisByte, 1, 1, input);

      // See if a read error occurred.
      if(ferror(input))
	{
	  std::cout << "A file read error occurred." << std::endl;
	  return -1;
	}

      // See if byte value makes sense.
      byteValue = static_cast<unsigned int>(*thisByte);
      if(byteValue < 0
	 ||
	 byteValue > 255)
	{
	  std::cout << "Error: Read a byte that seemed to have the value " << byteValue << std::endl;
	  return -1; // Quit---something is wrong.
	}

      // Write the next pixel value if we have read an 'even' byte,
      // otherwise accumulate.
      if(oddByte)
	{
	  thisPixel = (256 * byteValue); // Most significant byte.
	}
      else
	{
	  thisPixel += byteValue; // Least significant byte.

	  // Now apply calibration to the pixel value.
	  okSoFar = (*calibrationFunc)(&thisPixel, thisPixel);

	  // Check to make sure the range is OK.
	  if(!checkRange(thisPixel) || !okSoFar)
	    {
	      std::cout << "Error: A pixel value error was detected. Pixel value is: " << thisPixel << std::endl;
	      return -1;
	    }
	  
	  // Now write this pixel value to output.
	  fprintf(output, "%u ", thisPixel);

	  // Increment the character column counter and check to see
	  // if we need a newline.
	  charColCounter++;
	  if((charColCounter * maxCharsPerPixel) >= breakAroundCol)
	    {
	      fprintf(output, "\n");
	      charColCounter = 0;
	    }

	  // Increment the count of the pixels we've read.
	  numPixels++;

	  // Debugging.
	  if(thisPixel < minPixelValue){minPixelValue = thisPixel;}
	  if(thisPixel > maxPixelValue){maxPixelValue = thisPixel;}

	}

      // Invert oddByte.
      oddByte = !oddByte;
    }

  //std::cout << "Min raw pixel value was: " << minPixelValue << std::endl;
  //std::cout << "Max raw pixel value was: " << maxPixelValue << std::endl;

  // Setup a return value.
  int retVal = -1; // Assume error.

  if(numPixels == (unsigned int)(numRows * numCols))
    {
      retVal = 0; // It's OK.
    }
  else
    {
      std::cout << "Error: The specified number of pixels seems to be incorrect for the input file. We read " << std::endl
		<< numPixels << " pixels, which is not equal to " << numRows << " x " << numCols << "." << std::endl;
      retVal = image_size_error;
    }

  return retVal;
}


// This function checks the calibration functions to make sure they
// produce output with a suitable range of values. The function
// returns true if the functions are OK and false otherwise.
bool checkCalibrationFunctions(void)
{
  // Define an array of functions pointers to test.
  bool (*calibrationFuncs[4])(unsigned int*, unsigned int) = 
    {
      dbaCalibration, howtekMghCalibration, howtekIsmdCalibration, lumisysCalibration
    };

  // Define an array of digitizer names for output.
  std::string digitizerNames[4] = {dba, howtek_mgh, howtek_ismd, lumisys};

  // Iterate over the digitizer/calibration functions.
  bool (*thisCalibrationFunc)(unsigned int*, unsigned int) = NULL;
  for(unsigned int i = 0; i < 4; i++)
    {
      thisCalibrationFunc = calibrationFuncs[i];

      // Iterate over every possible input value and see if calling
      // the calibration function on it gives a result that is out of
      // bounds.
      unsigned int outVal = 0; // This is where we'll store the returned value from the calibration functions.
      for(unsigned int inVal = 0; inVal <= maxUnsignedIntWithNumBits; inVal++)
	{
	  // Call the function.
	  (*thisCalibrationFunc)(&outVal, inVal);

	  // Test the return value.
	  if(outVal > maxUnsignedIntWithNumBits)
	    {
	      std::cout << "The calibration function for the " << digitizerNames[i] << " digitizer has a range problem." << std::endl;
	      std::cout << "The input value that generated this error was " << inVal << std::endl;
	      return false; // It's broken.
	    }
	}
    }

  // If we get here, everything's OK.
  return true;
}


// Entry point.
int main(int argc, char* argv[])
{
  // Do some error checking to make sure that the user invoked us
  // properly.
  if(argc != 5)
    {
      // Output some help info and then exit.
      displayProgramHelp();
      exitWith(syntax_error, syntax_error_msg);
    }

  // Get the name of the file to read and the number of rows and cols
  // that the output image must have and the digitizer that was used.
  const std::string inputFile = argv[1];
  const std::string numRowsString = argv[2];
  const int numRows = atoi(numRowsString.c_str());
  const std::string numColsString = argv[3];
  const int numCols = atoi(numColsString.c_str());
  const std::string digitizer = argv[4];

  // Check the image sets (ranges) of the calibration functions to
  // ensure that produce output with suitable ranges.
  if(!checkCalibrationFunctions())
    {
      exitWith(program_error, program_error_msg);
    }

  // Choose the appropriate calibration function to apply to the grey
  // levels (yielding optical density values) based upon the name of
  // the digitizer. Exit if we've got an illegal digitizer name.
  bool (*calibrationFunc)(unsigned int*, unsigned int) = NULL; // Will be set to point to one of the three calibration functions.
  //
  if(digitizer.compare(dba) == 0)
    {
      calibrationFunc = dbaCalibration;
    }
  else if(digitizer.compare(howtek_mgh) == 0)
    {
      calibrationFunc = howtekMghCalibration;
    }
  else if(digitizer.compare(howtek_ismd) == 0)
    {
      calibrationFunc = howtekIsmdCalibration;
    }
  else if(digitizer.compare(lumisys) == 0)
    {
      calibrationFunc = lumisysCalibration;
    }
  else
    {
      exitWith(syntax_error, syntax_error_msg);
    }
    

  // Make a filename for the PNM file that will be created. If the
  // file already exists, it will be overwritten!
  const std::string outputFile = inputFile + outputSuffix;

  // Make sure that the number of rows and cols are sensible.
  if(numRows < 1) { exitWith(rows_not_positive_error, rows_not_positive_error_msg); }
  if(numCols < 1) { exitWith(cols_not_positive_error, cols_not_positive_error_msg); }

  // Open the input and output files for reading.
  FILE* input = fopen(inputFile.c_str(), "rb");
  FILE* output = fopen(outputFile.c_str(), "wb");

  if(NULL == input && NULL == output)
    {
      // No files to close.
      exitWith(file_error, file_error_msg);
    }
  else if(NULL != input && NULL == output)
    {
      fclose(input);
      exitWith(file_error, file_error_msg);
    }
  else if(NULL == input && NULL != output)
    {
      fclose(output);
      exitWith(file_error, file_error_msg);
    }

  // Let's now make the PNM file.
  const int status = makePnmFile(input, output, numRows, numCols, calibrationFunc);
  if(status != 0)
    {
      // There was an error.
      fclose(input);
      fclose(output);
      exitWith(pnm_error, pnm_error_msg);
    }


  // Cleanup.
  fclose(input);
  fclose(output);

  // Everything's OK, so send the name of the PNM file to stdout.
  std::cout << outputFile << std::endl;

  // Exit with a success exit code.
  exit(success);
}
