GXX = g++

all:
	$(GXX)  -O3 -fopenmp -march=native main_code.cpp -o lbp

	$(GXX)   eval_results.cpp -o eval_results

run-lbp:
	./lbp

run-eval:
	./eval_results

clean:
	rm lbp eval_results
