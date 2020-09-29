Dataset for Judging the MEMOCODE 2013 Design Contest.

http://www.memocode-conference.com


This package contains the following:

- Left.png and Right.png: the stereo image pairs used for judging the design contest

- vdata_in.txt: input vertex data (data cost) from judging image pair above

- ref_output_labels.txt: output labels from running compiled lbp.cpp 
  using vdata_in.txt as input. Provided here for reference of what the output should be.

- ground_truth_labels.txt: judging image's ground truth depth labels


Inference result on reference software: 

Using the supplied BP algorithm (lbp.cpp), the inference output (ref_output_labels.txt)
contains 11870 mismatches out of 87696 total labels, relative to the ground truth (ground_truth_labels.txt).


License:

The stereo image pair included in this package is provided for academic non-commercial 
purpose of the MEMOCODE 2013 Design Contest only. It was prepared by Skand Hurkat from 
Cornell University, based on a .blend file by Andrew Price (www.blenderguru.com). 
The .blend file was released under Creative Commons Attribution Non commercial license 
available at https://creativecommons.org/licenses/by-nc/3.0/

