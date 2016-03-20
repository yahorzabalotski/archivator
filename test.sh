FILES=./tests/*
COMPRESS_FILE_NAME=ofile
DECOMPRESS_FILE_NAME=ofile_decompress

for F in $FILES
do
	echo "./huff ${F} -c ${COMPRESS_FILE_NAME}"
	./huff ${F} -c ${COMPRESS_FILE_NAME}
	./huff ${COMPRESS_FILE_NAME} -x ${DECOMPRESS_FILE_NAME}
	OUTPUT=$(diff ${F} ${DECOMPRESS_FILE_NAME})
	if [[ ! -z ${OUTPUT} ]];
	then 
		echo "FAIL"
	else
		echo "OK"
	fi
	rm ${COMPRESS_FILE_NAME} ${DECOMPRESS_FILE_NAME}
done
