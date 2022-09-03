all: programa00 programa01

programa00:
	gcc -pthread MatrixT.c -o MatrixT

programa01:
	gcc MatrixP.c -o MatrixP -lrt