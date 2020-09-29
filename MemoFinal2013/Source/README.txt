Belief Propagation is one of the methods used in the solving stereo matching problem. This method needs noticeable memory bandwidth and is normally highly serialized. In this paper we propose a new implementation of already proposed methods in order to reduce the memory bandwidth needed by compressing the data.  Furthermore, our methods will utilize parallelism in full extent. The quality evaluation according to the given evaluation code in the Memocode 2013 Design Contest shows %15 of pixels have different values than the ground truth in Tsukuba image set. The implementation runs several orders of magnitude faster than the reference code.

input and output file types and pathes are the same as the refrence code.

.How to compile and run the code:

Two following commands will compile and run the code in order

    make
   ./lbp [inupt_file.txt] [output_file.txt]

To modify compile flags and options edit the Makefile

the binary will generate a .txt output file

Note: for different images the LABELS parameter in the code should be set to the appropriate number in the code.  default is 16


The following command will evaluate the output result

./eval_result [output_file.txt] [ground_truth_labels_file.txt]

