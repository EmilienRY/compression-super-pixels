#!/bin/bash

# Paths
INPUT_IMAGE="vader.ppm"  # Input image
PALETTE_FILE="palette.json"     # Palette file
PSNR_DAT_WATERSHED="output/psnrwater.dat" # File to store PSNR results
PSNR_DAT_SLIC="output/psnrslic.dat" # File to store PSNR results
COMPRESSION_DAT_WATERSHED="output/compressionwater.dat" # File to store compression ratio results
COMPRESSION_DAT_SLIC="output/compressionslic.dat" # File to store compression ratio results

PSNR_DAT_SLICNBPIX="output/psnrslicpix.dat" # File to store PSNR results
COMPRESSION_DAT_SLICNBPIX="output/compressionslicpix.dat" # File to store compression ratio results


# Clear previous results
echo "# Parameter PSNR" > "$PSNR_DAT_WATERSHED"
echo "# Parameter PSNR" > "$PSNR_DAT_SLIC"
echo "# Parameter CompressionRatio" > "$COMPRESSION_DAT_WATERSHED"
echo "# Parameter CompressionRatio" > "$COMPRESSION_DAT_SLIC"
echo "# Parameter CompressionRatio" > "$PSNR_DAT_SLICNBPIX"
echo "# Parameter CompressionRatio" > "$COMPRESSION_DAT_SLICNBPIX"

# Parameters to test
MIN_SIZE_VALUES=(1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 60 70 80 90 100 200 300 400 500) # Example parameter values for watershed
COMPACTNESS_VALUES=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25)       # Example parameter values for SLIC
NBPIX_VALUES=(5 20 40 60 80 120 150 200 250 400 600 700 800 1000 1400 1700 2500 5000 10000 15000)       # Example parameter values for SLIC

# Test watershed algorithm
echo "Testing watershed algorithm..."
for MIN_SIZE in "${MIN_SIZE_VALUES[@]}"; do
    OUTPUT_IMAGE="watershed${INPUT_IMAGE}" # Watershed output format
    ./watershed "$INPUT_IMAGE" "$MIN_SIZE" > /dev/null 2>&1


    # Compress the output
    COMPRESSED_IMAGE="watershed_minSize_${MIN_SIZE}_compressed.pgm"
    ./compressionPalette compress "$OUTPUT_IMAGE" "$COMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1


    # Decompress and compute PSNR
    DECOMPRESSED_IMAGE="watershed_minSize_${MIN_SIZE}_decompressed.ppm"
    ./compressionPalette decompress "$COMPRESSED_IMAGE" "$DECOMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1



    PSNR=$(./psnr $INPUT_IMAGE $DECOMPRESSED_IMAGE)
    ORIGINAL_SIZE=$(stat -c%s "images/$INPUT_IMAGE")
    COMPRESSED_SIZE=$(stat -c%s "output/$COMPRESSED_IMAGE")
    COMPRESSION_RATIO=$(echo "scale=2; $ORIGINAL_SIZE / $COMPRESSED_SIZE" | bc)

    echo "$MIN_SIZE $PSNR" >> "$PSNR_DAT_WATERSHED"
    echo "$MIN_SIZE $COMPRESSION_RATIO" >> "$COMPRESSION_DAT_WATERSHED"
done

# Test SLIC algorithm
echo "Testing SLIC algorithm..."
for COMPACTNESS in "${COMPACTNESS_VALUES[@]}"; do
    OUTPUT_IMAGE="$INPUT_IMAGE" # SLIC output format
    ./slic "$INPUT_IMAGE" 128 "$COMPACTNESS" > /dev/null 2>&1

    # Compress the output
    COMPRESSED_IMAGE="slic_compactness_${COMPACTNESS}_compressed.pgm"
    ./compressionPalette compress "$OUTPUT_IMAGE" "$COMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1

    # Decompress and compute PSNR
    DECOMPRESSED_IMAGE="slic_compactness_${COMPACTNESS}_decompressed.ppm"
    ./compressionPalette decompress "$COMPRESSED_IMAGE" "$DECOMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1

    PSNR=$(./psnr $INPUT_IMAGE $DECOMPRESSED_IMAGE)
    ORIGINAL_SIZE=$(stat -c%s "images/$INPUT_IMAGE")
    COMPRESSED_SIZE=$(stat -c%s "output/$COMPRESSED_IMAGE")
    COMPRESSION_RATIO=$(echo "scale=2; $ORIGINAL_SIZE / $COMPRESSED_SIZE" | bc)

    echo "$COMPACTNESS $PSNR" >> "$PSNR_DAT_SLIC"
    echo "$COMPACTNESS $COMPRESSION_RATIO" >> "$COMPRESSION_DAT_SLIC"
done



# Test SLIC algorithm
echo "Testing SLIC algorithm pix..."
for NBPIX in "${NBPIX_VALUES[@]}"; do
    OUTPUT_IMAGE="$INPUT_IMAGE" # SLIC output format
    ./slic "$INPUT_IMAGE" "$NBPIX" 20 > /dev/null 2>&1

    # Compress the output
    COMPRESSED_IMAGE="slic_nbpix_${NBPIX}_compressed.pgm"
    ./compressionPalette compress "$OUTPUT_IMAGE" "$COMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1

    # Decompress and compute PSNR
    DECOMPRESSED_IMAGE="slic_nbpix_${NBPIX}_decompressed.ppm"
    ./compressionPalette decompress "$COMPRESSED_IMAGE" "$DECOMPRESSED_IMAGE" "$PALETTE_FILE" > /dev/null 2>&1

    PSNR=$(./psnr $INPUT_IMAGE $DECOMPRESSED_IMAGE)
    ORIGINAL_SIZE=$(stat -c%s "images/$INPUT_IMAGE")
    COMPRESSED_SIZE=$(stat -c%s "output/$COMPRESSED_IMAGE")
    COMPRESSION_RATIO=$(echo "scale=2; $ORIGINAL_SIZE / $COMPRESSED_SIZE" | bc)

    echo "$NBPIX $PSNR" >> "$PSNR_DAT_SLICNBPIX"
    echo "$NBPIX $COMPRESSION_RATIO" >> "$COMPRESSION_DAT_SLICNBPIX"
done

echo "Tests completed. Results saved in:"