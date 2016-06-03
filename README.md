# ddsm — Digital Database for Digital Mammography Software

## Introduction

This file describes how to install and use software to:

* download DDSM mammograms and convert them into a usable format;
* obtain the annotations (e.g., tumour segmentations) made by the DDSM radiologists and the associated metadata (i.e., malignancy type etc.)

Please note that while this GitHub repository was published in June 2016, the included software, instructions below, and the PDF manual are now pretty old (as are the digitised mammograms!), and absolutely no support is offered. However, this software (at its original home) was still being downloaded around 100 times per month as of June 2016. Please also note that this GitHub repository is not affiliated with the DDSM project, and questions about the database should be directed to the [DDSM project’s maintainers at the University of South Florida](http://marathon.csee.usf.edu/Mammography/Database.html).

## Copyright, Licensing, Warranty, and Fitness for Use

This repository contains software that is in the public domain, is publicly available, or has been specially adapted or developed to be used within the Windows and Cygwin environment. You are granted a non-exclusive license to use or modify the software for research purposes only. You are encouraged to submit pull requests if you can improve this software for the research community.

The accompanying software must be used for research purposes only. It has not been validated for use in clinical or similar environments. There is no warranty for this software, which is supplied “as is”, and no claims are made about fitness for any purpose. Portions of the software have, however, been used by many computer-aided mammography researchers for many years.

## Dependencies

The software has been tested on:

* Windows XP Professional with Service Pack 2.
* Linux (x86, 64-bit only).

Later versions will probably work, but are untested.

If you are using Windows, you will also need the [Cygwin UNIX environment](http://cygwin.com) (version 1.5.25-15 or later; installation instructions are available in [DDSM User Manual.pdf](DDSM User Manual.pdf)).

You will also need:

* [ImageMagick](http://imagemagick.org/script/index.php) (version 6.4.0.6-1 or later). 
* [Ruby](https://www.ruby-lang.org/en/) (version 1.8.7-p72-2 or later).
* [MATLAB](http://www.mathworks.com/products/matlab/) (version 7.6.0 (R2008a) or later).
* An FTP client (one is built in to Windows Explorer, though other options are available).

The [DDSM User Manual](DDSM User Manual.pdf) contains installation instructions for ImageMagick and Ruby (for Windows), but these may be very out of date now.

## Installation Instructions

The following instructions assume you have installed the dependencies above.

1. Clone this repository (or download it as a zip file and unzip it). Linux users can place their local copy wherever is convenient. Windows users should place their local copy in the Cygwin home directory (i.e., `C:\cygwin\home\<Your User Name>`).
2. User the terminal (i.e., the Cygwin window for Windows users) to change directory to the `dddm-software` directory using the command `cd ddsm-software`.
3. Verify that the correct files are present. Run `ls` and ensure that the following files are listed: `ddsmraw2pnm.exe`, `get_ddsm_groundtruth.m`, `get-ddsm-mammo`, `info-file.txt`, `jpeg.exe`, and `jpeg`.
4. Verify you can run the program that converts the mammograms. Run `./jpeg` (Linux users) or `./jpeg.exe` (Windows users). You should see some usage instructions printed to the screen. (You will never need to run this command directly, however, so you can ignore them.) If the program does not run, you may need to set the file to be executable (e.g., using the `chmod` command).
5. Verify that Ruby was installed correctly. Run `ruby -e “p ‘Hello world!’”` (don’t miss the single quotes!). You should see “Hello world!” printed to the screen.

The DDSM radiologist annotations and metadata (which includes all information provided about the annotations in the `.OVERLAY` files, such as the type of abnormality and its subtlety) are made available via the `get_ddsm_groundtruth.m` MATLAB function. Install this software as follows:

6. Copy or move the file `get_ddsm_groundtruth.m` from the `dddm-software` directory to a directory in which you keep your MATLAB software (create such a directory if one does not exist).
7. Start MATLAB.
8. Add the directory into which you placed the `get_ddsm_groundtruth.m` file in step 1 to your MATLAB path (see [What Is the MATLAB Search Path?](http://uk.mathworks.com/help/MATLAB/MATLAB_env/what-is-the-MATLAB-search-path.html)).
9. From the MATLAB command prompt, check that MATLAB can find the `get_ddsm_groundtruth.m` file by running `help get_ddsm_groundtruth`. MATLAB should print the documentation for the `get_ddsm_groundtruth` function.

## How to Obtain and Convert a DDSM Mammogram

The software provided makes it simple to get a DDSM mammogram in a usable file format. The following example shows how we can get the mammogram `A_1141_1.LEFT_MLO` in PNG format. PNG format files can be read by a wide range of software, including MATLAB, Photoshop, and Windows itself. It is a lossless file format and offers good compression ratios. (Note, however, that each DDSM mammogram is about 40MB.)

To download and convert the mammogram `A_1141_1.LEFT_MLO` into PNG format:

1. If you closed the terminal or Cygwin program, restart it and change to the `dddm-software` directory.
2. Run `./get-ddsm-mammo A_1141_1.LEFT_MLO`. The mammogram will be downloaded and the converted file will be placed in the `ddsm-software` directory. The full (Unix) path to the file will be printed to the screen. You should probably move the resulting files to a more convenient directory.
3. View the downloaded and converted file. For example in Windows, double-click on the file.

The `get-ddsm-mammo` step obtains and converts the mammogram named `A_1141_1.LEFT_MLO`. The program connects to the DDSM’s FTP server, downloads the corresponding “lossless” JPEG file, converts that file to a raw binary format, converts that file to a simple human-readable file format called PNM, converts that file to the desired PNG format, and finally deletes the “lossless” JPEG and all intermediate files. Because the DDSM files are large and conversion is processor intensive, it may take a nontrivial amount of time for the file to be downloaded and converted. Exactly how long depends on the speed of your Internet connection and how powerful your computer’s CPU is. When this software was originally written (c. 2006), it took about 3 minutes to download and convert a mammogram using a domestic 10Mbps cable modem and a 2006-vintage 2GHz Intel Core 2 Duo CPU. This will hopefully be appreciably faster for 2016-vintage Internet connections and CPUs.

If the `get-ddsm-mammo` program is interrupted, intermediate files may be left in the `ddsm-software` directory. Such files will have the suffix `.1` or `.pnm`, and should be deleted as they tend to be relatively large (80MB+).

Note that, as of c. 2006, the DDSM’s FTP server had a policy of allowing no more than 10 users at a time. If the `get-ddsm-mammo` program fails, the limit on the number of users is a possible reason. In the first instance, simply wait a few minutes and try again. (While testing this software, the DDSM’s FTP server went offline for several hours, so be aware that this may be a “weak link” in your workflow.)

## How to Obtain DDSM Radiologist Annotations and Metadata

For this example, we will obtain the annotations and metadata for the file `A_1580_1.LEFT_MLO`. This mammogram was chosen because it is an example of a non-trivial mammogram: it has multiple abnormalities (boundaries) and one of those has a core annotation. You can obtain ground truth (annotations and metadata) for this file as follows:

1. Using your FTP client (such as Windows Explorer), connect to the DDSM FTP server at `figment.csee.usf.edu`. If prompted, use the username “anonymous” and an empty password.
2. Navigate to the directory `/pub/DDSM/cases/cancers/cancer_10/case1580/`.
3. Copy the remote file `A_1580_1.LEFT_MLO.OVERLAY` to a convenient directory on your local computer. Let us call this directory `<overlay-directory>`.
4. Disconnect from the DDSM’s FTP server so that you aren’t preventing other users from accessing it.
5. At MATLAB’s command prompt, run `cd <overlay-directory>`.
6. At MATLAB’s command prompt, run `ground_truth = get_ddsm_groundtruth(‘A_1580_1.LEFT_MLO.OVERLAY’);`.
The rest of this tutorial is provided by the `get_ddsm_groundtruth` function’s documentation (read from “Example usage”). To see the documentation, run `help get_ddsm_groundtruth`.

Note that to get a binary mask image showing the region annotated by the radiologist, you will need to know the image dimensions. You can obtain this information in one of two ways:

* Use the DDSM’s website and view the thumbnails pages. These list the contents of the corresponding case’s `ICS` file, which gives the number of lines (i.e., rows) and pixels per line (i.e., columns) for each mammogram in the case.
* Obtain the corresponding mammogram (using the `get-ddsm-mammo` program) and use MATLAB’s `size` function to get the number of rows and columns in the mammogram.

Parsing the `.OVERLAY` file is very quick, but creating a binary mask matrix using the anonymous function(s) returned by `get_ddsm_groundtruth.m` can take a couple of seconds.

### Saving the Annotations

The `get_ddsm_groundtruth.m` code returns a cell array of structs, which can be saved to disk using MATLAB’s `save` command. Such files will require very little disk space relative to the full binary mask matrices (which are the same size as their corresponding mammograms).

## What Can Go Wrong

Apart from the data and this software now being very old, the weakest link in the workflow is the DDSM’s FTP server, which may permit a limited number of users to access the server at any given time.

The `get-ddsm-mammo` program should be quite robust, with the exception of its reliance on the DDSM’s FTP server. If you experience problems using it, please try connecting to the DDSM’s FTP server using your FTP client, to see if this is the cause of the problem.
